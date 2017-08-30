// Copyright (C) 2015-2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include <asiolink/io_service.h>
#include <config/base_command_mgr.h>
#include <config/command_mgr.h>
#include <config/hooked_command_mgr.h>
#include <cc/command_interpreter.h>
#include <hooks/hooks_manager.h>
#include <hooks/callout_handle.h>
#include <hooks/library_handle.h>
#include <string>
#include <vector>

using namespace isc::asiolink;
using namespace isc::config;
using namespace isc::data;
using namespace isc::hooks;
using namespace std;

// Test class for Command Manager
class CommandMgrTest : public ::testing::Test {
public:

    /// Default constructor
    CommandMgrTest()
        : io_service_(new IOService()) {

        CommandMgr::instance().setIOService(io_service_);

        handler_name_ = "";
        handler_params_ = ElementPtr();
        handler_called_ = false;
        callout_name_ = "";
        callout_argument_names_.clear();
        std::string processed_log_ = "";

        CommandMgr::instance().deregisterAll();
        CommandMgr::instance().closeCommandSocket();

        resetCalloutIndicators();
    }

    /// Default destructor
    virtual ~CommandMgrTest() {
        CommandMgr::instance().deregisterAll();
        CommandMgr::instance().closeCommandSocket();
        resetCalloutIndicators();
    }

    /// @brief Returns socket path (using either hardcoded path or env variable)
    /// @return path to the unix socket
    std::string getSocketPath() {

        std::string socket_path;
        const char* env = getenv("KEA_SOCKET_TEST_DIR");
        if (env) {
            socket_path = std::string(env) + "/test-socket";
        } else {
            socket_path = std::string(TEST_DATA_BUILDDIR) + "/test-socket";
        }
        return (socket_path);
    }

    /// @brief Resets indicators related to callout invocation.
    ///
    /// It also removes any registered callouts.
    static void resetCalloutIndicators() {
        callout_name_ = "";
        callout_argument_names_.clear();

        // Iterate over existing hook points and for each of them remove
        // callouts registered.
        std::vector<std::string> hooks = ServerHooks::getServerHooksPtr()->getHookNames();
        for (auto h = hooks.cbegin(); h != hooks.cend(); ++h) {
                HooksManager::preCalloutsLibraryHandle().deregisterAllCallouts(*h);
        }
    }

    /// @brief A simple command handler that always returns an eror
    static ConstElementPtr my_handler(const std::string& name,
                                      const ConstElementPtr& params) {

        handler_name_ = name;
        handler_params_ = params;
        handler_called_ = true;

        return (createAnswer(123, "test error message"));
    }

    /// @brief Test callback which stores callout name and passed arguments and
    /// which handles the command.
    ///
    /// @param callout_handle Handle passed by the hooks framework.
    /// @return Always 0.
    static int
    hook_lib_callout(CalloutHandle& callout_handle) {
        callout_name_ = "hook_lib_callout";

        ConstElementPtr command;
        callout_handle.getArgument("command", command);

        ConstElementPtr arg;
        std::string command_name = parseCommand(arg, command);

        callout_handle.setArgument("response",
                                   createAnswer(234, "text generated by hook handler"));

        callout_argument_names_ = callout_handle.getArgumentNames();
        // Sort arguments alphabetically, so as we can access them on
        // expected positions and verify.
        std::sort(callout_argument_names_.begin(), callout_argument_names_.end());
        return (0);
    }

    /// @brief Test callback which stores callout name and passed arguments and
    /// which handles the command.
    ///
    /// @param callout_handle Handle passed by the hooks framework.
    /// @return Always 0.
    static int
    command_processed_callout(CalloutHandle& callout_handle) {
        callout_name_ = "command_processed_handler";

        std::string name;
        callout_handle.getArgument("name", name);

        ConstElementPtr arguments;
        callout_handle.getArgument("arguments", arguments);

        ConstElementPtr response;
        callout_handle.getArgument("response", response);
        std::ostringstream os;
        os << name << ":" << arguments->str() << ":" << response->str();
        processed_log_ = os.str();

        if (name == "change-response") {
            callout_handle.setArgument("response",
                createAnswer(777, "replaced response text"));
        }

        return (0);
    }

    /// @brief IO service used by these tests.
    IOServicePtr io_service_;

    /// @brief Name of the command (used in my_handler)
    static std::string handler_name_;

    /// @brief Parameters passed to the handler (used in my_handler)
    static ConstElementPtr handler_params_;

    /// @brief Indicates whether my_handler was called
    static bool handler_called_;

    /// @brief Holds invoked callout name.
    static std::string callout_name_;

    /// @brief Holds a list of arguments passed to the callout.
    static std::vector<std::string> callout_argument_names_;

    /// @brief Holds the generated command process log
    static std::string processed_log_;
};

/// Name passed to the handler (used in my_handler)
std::string CommandMgrTest::handler_name_("");

/// Parameters passed to the handler (used in my_handler)
ConstElementPtr CommandMgrTest::handler_params_;

/// Indicates whether my_handler was called
bool CommandMgrTest::handler_called_(false);

/// Holds invoked callout name.
std::string CommandMgrTest::callout_name_("");

/// @brief Holds the generated command process log
std::string CommandMgrTest::processed_log_;

/// Holds a list of arguments passed to the callout.
std::vector<std::string> CommandMgrTest::callout_argument_names_;

// Test checks whether the internal command 'list-commands'
// is working properly.
TEST_F(CommandMgrTest, listCommandsEmpty) {

    ConstElementPtr command = createCommand("list-commands");

    ConstElementPtr answer;

    EXPECT_NO_THROW(answer = CommandMgr::instance().processCommand(command));

    ASSERT_TRUE(answer);

    EXPECT_EQ("{ \"arguments\": [ \"list-commands\" ], \"result\": 0 }",
              answer->str());
}

// Test checks whether calling a bogus command is handled properly.
TEST_F(CommandMgrTest, bogusCommand) {

    ConstElementPtr command = createCommand("no-such-command");

    ConstElementPtr answer;

    EXPECT_NO_THROW(answer = CommandMgr::instance().processCommand(command));

    // Make sure the status code is non-zero
    ASSERT_TRUE(answer);
    int status_code;
    parseAnswer(status_code, answer);
    EXPECT_EQ(CONTROL_RESULT_COMMAND_UNSUPPORTED, status_code);
}

// Test checks whether handlers installation is sanitized. In particular,
// whether NULL handler and attempt to install handlers for the same
// command twice are rejected.
TEST_F(CommandMgrTest, handlerInstall) {

    // Check that it's not allowed to install NULL pointer instead of a real
    // command.
    EXPECT_THROW(CommandMgr::instance().registerCommand("my-command", 0),
                 InvalidCommandHandler);

    // This registration should succeed.
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("my-command",
                                                           my_handler));

    // Check that it's not possible to install handlers for the same
    // command twice.
    EXPECT_THROW(CommandMgr::instance().registerCommand("my-command",
                 my_handler), InvalidCommandName);
}

// Test checks whether the internal list-commands command is working
// correctly. Also, checks installation and deinstallation of other
// command handlers.
TEST_F(CommandMgrTest, listCommands) {

    // Let's install two custom commands.
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("make-a-coffee",
                                                           my_handler));
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("do-the-dishes",
                                                           my_handler));

    // And then run 'list-commands'
    ConstElementPtr list_all = createCommand("list-commands");
    ConstElementPtr answer;

    // Now check that the command is returned by list-commands
    EXPECT_NO_THROW(answer = CommandMgr::instance().processCommand(list_all));
    ASSERT_TRUE(answer);
    EXPECT_EQ("{ \"arguments\": [ \"do-the-dishes\", \"list-commands\", "
              "\"make-a-coffee\" ], \"result\": 0 }", answer->str());

    // Now unregister one command
    EXPECT_NO_THROW(CommandMgr::instance().deregisterCommand("do-the-dishes"));

    // Now check that the command is returned by list-commands
    EXPECT_NO_THROW(answer = CommandMgr::instance().processCommand(list_all));
    ASSERT_TRUE(answer);
    EXPECT_EQ("{ \"arguments\": [ \"list-commands\", "
              "\"make-a-coffee\" ], \"result\": 0 }", answer->str());

    // Now test deregistration. It should work the first time.
    EXPECT_NO_THROW(CommandMgr::instance().deregisterCommand("make-a-coffee"));

    // Second time should throw an exception as the handler is no longer there.
    EXPECT_THROW(CommandMgr::instance().deregisterCommand("make-a-coffee"),
                 InvalidCommandName);

    // You can't uninstall list-commands as it's the internal handler.
    // It always must be there.
    EXPECT_THROW(CommandMgr::instance().deregisterCommand("list-commands"),
                 InvalidCommandName);

    // Attempt to register a handler for existing command should fail.
    EXPECT_THROW(CommandMgr::instance().registerCommand("list-commands",
                 my_handler), InvalidCommandName);
}

// Test checks whether deregisterAll method uninstalls all handlers,
// except list-commands.
TEST_F(CommandMgrTest, deregisterAll) {

    // Install a couple handlers.
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("my-command1",
                                                           my_handler));
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("my-command2",
                                                           my_handler));

    EXPECT_NO_THROW(CommandMgr::instance().deregisterAll());

    ConstElementPtr answer;
    EXPECT_NO_THROW(answer = CommandMgr::instance()
                    .processCommand(createCommand("list-commands")));
    ASSERT_TRUE(answer);
    EXPECT_EQ("{ \"arguments\": [ \"list-commands\" ], \"result\": 0 }",
              answer->str());
}

// Test checks whether a command handler can be installed and then
// runs through processCommand to check that it's indeed called.
TEST_F(CommandMgrTest, processCommand) {
    // Install my handler
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("my-command",
                                                           my_handler));

    // Now tell CommandMgr to process a command 'my-command' with the
    // specified parameter.
    ElementPtr my_params = Element::fromJSON("[ \"just\", \"some\", \"data\" ]");
    ConstElementPtr command = createCommand("my-command", my_params);
    ConstElementPtr answer;
    EXPECT_NO_THROW(answer = CommandMgr::instance().processCommand(command));

    // There should be an answer.
    ASSERT_TRUE(answer);

    // my_handler remembers all passed parameters and returns status code of 123.
    ConstElementPtr answer_arg;
    int status_code;
    // Check that the returned status code is correct.
    EXPECT_NO_THROW(answer_arg = parseAnswer(status_code, answer));
    EXPECT_EQ(123, status_code);

    // Check that the parameters passed are correct.
    EXPECT_EQ(true, handler_called_);
    EXPECT_EQ("my-command", handler_name_);
    ASSERT_TRUE(handler_params_);
    EXPECT_EQ("[ \"just\", \"some\", \"data\" ]", handler_params_->str());

    // Command handlers not installed so expecting that callouts weren't
    // called.
    EXPECT_TRUE(callout_name_.empty());
}

// Verify that processing a command can be delegated to a hook library.
TEST_F(CommandMgrTest, delegateProcessCommand) {
    // Register callout so as we can check that it is called before
    // processing the command by the manager.
    HooksManager::preCalloutsLibraryHandle().registerCommandCallout(
        "my-command", hook_lib_callout);

    // Install local handler
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("my-command",
                                                           my_handler));

    // Now tell CommandMgr to process a command 'my-command' with the
    // specified parameter.
    ElementPtr my_params = Element::fromJSON("[ \"just\", \"some\", \"data\" ]");
    ConstElementPtr command = createCommand("my-command", my_params);
    ConstElementPtr answer;
    ASSERT_NO_THROW(answer = CommandMgr::instance().processCommand(command));

    // There should be an answer.
    ASSERT_TRUE(answer);

    // Local handler shouldn't be called because the command is handled by the
    // hook library.
    ASSERT_FALSE(handler_called_);

    // Returned status should be unique for the hook library.
    ConstElementPtr answer_arg;
    int status_code;
    ASSERT_NO_THROW(answer_arg = parseAnswer(status_code, answer));
    EXPECT_EQ(234, status_code);

    EXPECT_EQ("hook_lib_callout", callout_name_);

    // Check that the appropriate arguments have been set. Include the
    // 'response' which should have been set by the callout.
    ASSERT_EQ(2, callout_argument_names_.size());
    EXPECT_EQ("command", callout_argument_names_[0]);
    EXPECT_EQ("response", callout_argument_names_[1]);
}

// Verify that 'list-command' command returns combined list of supported
// commands from hook library and from the Kea Command Manager.
TEST_F(CommandMgrTest, delegateListCommands) {
    // Register callout so as we can check that it is called before
    // processing the command by the manager.
    HooksManager::preCalloutsLibraryHandle().registerCommandCallout(
        "my-command", hook_lib_callout);

    // Create my-command-bis which is unique for the local Command Manager,
    // i.e. not supported by the hook library. This command should also
    // be returned as a result of processing 'list-commands'.
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("my-command-bis",
                                                           my_handler));

    // Process command. The command should be routed to the hook library
    // and the hook library should return the commands it supports.
    ConstElementPtr command = createCommand("list-commands");
    ConstElementPtr answer;
    ASSERT_NO_THROW(answer = CommandMgr::instance().processCommand(command));

    // There should be an answer.
    ASSERT_TRUE(answer);

    ConstElementPtr answer_arg;
    int status_code;
    ASSERT_NO_THROW(answer_arg = parseAnswer(status_code, answer));
    EXPECT_EQ(0, status_code);

    // The hook library supports: my-command and list-commands commands. The
    // local Command Manager supports list-commands and my-command-bis. The
    // combined list should include 3 unique commands.
    const std::vector<ElementPtr>& commands_list = answer_arg->listValue();
    ASSERT_EQ(3, commands_list.size());
    std::vector<std::string> command_names_list;
    for (auto cmd = commands_list.cbegin(); cmd != commands_list.cend();
         ++cmd) {
        command_names_list.push_back((*cmd)->stringValue());
    }
    std::sort(command_names_list.begin(), command_names_list.end());
    EXPECT_EQ("list-commands", command_names_list[0]);
    EXPECT_EQ("my-command", command_names_list[1]);
    EXPECT_EQ("my-command-bis", command_names_list[2]);
}

// This test verifies that a Unix socket can be opened properly and that input
// parameters (socket-type and socket-name) are verified.
TEST_F(CommandMgrTest, unixCreate) {
    // Null pointer is obviously a bad idea.
    EXPECT_THROW(CommandMgr::instance().openCommandSocket(ConstElementPtr()),
                 isc::config::BadSocketInfo);

    // So is passing no parameters.
    ElementPtr socket_info = Element::createMap();
    EXPECT_THROW(CommandMgr::instance().openCommandSocket(socket_info),
                 isc::config::BadSocketInfo);

    // We don't support ipx sockets
    socket_info->set("socket-type", Element::create("ipx"));
    EXPECT_THROW(CommandMgr::instance().openCommandSocket(socket_info),
                 isc::config::BadSocketInfo);

    socket_info->set("socket-type", Element::create("unix"));
    EXPECT_THROW(CommandMgr::instance().openCommandSocket(socket_info),
                 isc::config::BadSocketInfo);

    socket_info->set("socket-name", Element::create(getSocketPath()));
    EXPECT_NO_THROW(CommandMgr::instance().openCommandSocket(socket_info));
    EXPECT_GE(CommandMgr::instance().getControlSocketFD(), 0);

    // It should be possible to close the socket.
    EXPECT_NO_THROW(CommandMgr::instance().closeCommandSocket());
}

// This test checks that when unix path is too long, the socket cannot be opened.
TEST_F(CommandMgrTest, unixCreateTooLong) {
    ElementPtr socket_info = Element::fromJSON("{ \"socket-type\": \"unix\","
        "\"socket-name\": \"/tmp/toolongtoolongtoolongtoolongtoolongtoolong"
        "toolongtoolongtoolongtoolongtoolongtoolongtoolongtoolongtoolong"
        "\" }");

    EXPECT_THROW(CommandMgr::instance().openCommandSocket(socket_info),
                 SocketError);
}

// This test verifies that a registered callout for the command-processed
// hookpoint is invoked and passed the correct information.
TEST_F(CommandMgrTest, commandProcessedHook) {
    // Register callout so as we can check that it is called before
    // processing the command by the manager.
    HooksManager::preCalloutsLibraryHandle().registerCallout(
        "command-processed", command_processed_callout);

    // Install local handler
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("my-command",
                                                           my_handler));

    // Now tell CommandMgr to process a command 'my-command' with the
    // specified parameter.
    ElementPtr my_params = Element::fromJSON("[ \"just\", \"some\", \"data\" ]");
    ConstElementPtr command = createCommand("my-command", my_params);
    ConstElementPtr answer;
    ASSERT_NO_THROW(answer = CommandMgr::instance().processCommand(command));

    // There should be an answer.
    ASSERT_TRUE(answer);

    // Local handler should be called
    ASSERT_TRUE(handler_called_);

    // Verify that the response came through intact
    EXPECT_EQ("{ \"result\": 123, \"text\": \"test error message\" }",
              answer->str());

    // Make sure invoked the command-processed callout
    EXPECT_EQ("command_processed_handler", callout_name_);

    // Verify the callout could extract all the context arguments
    EXPECT_EQ("my-command:[ \"just\", \"some\", \"data\" ]:"
              "{ \"result\": 123, \"text\": \"test error message\" }",
              processed_log_);
}

// This test verifies that a registered callout for the command-processed
// hookpoint is invoked and can replace the command response content.
TEST_F(CommandMgrTest, commandProcessedHookReplaceResponse) {
    // Register callout so as we can check that it is called before
    // processing the command by the manager.
    HooksManager::preCalloutsLibraryHandle().registerCallout(
        "command-processed", command_processed_callout);

    // Install local handler
    EXPECT_NO_THROW(CommandMgr::instance().registerCommand("my-command",
                                                           my_handler));

    // Now tell CommandMgr to process a command 'my-command' with the
    // specified parameter.
    ElementPtr my_params = Element::fromJSON("[ \"just\", \"some\", \"data\" ]");
    ConstElementPtr command = createCommand("change-response", my_params);
    ConstElementPtr answer;
    ASSERT_NO_THROW(answer = CommandMgr::instance().processCommand(command));

    // There should be an answer.
    ASSERT_TRUE(answer);

    // Local handler should not have been called, command isn't recognized
    ASSERT_FALSE(handler_called_);

    // Verify that we overrode response came
    EXPECT_EQ("{ \"result\": 777, \"text\": \"replaced response text\" }",
              answer->str());

    // Make sure invoked the command-processed callout
    EXPECT_EQ("command_processed_handler", callout_name_);

    // Verify the callout could extract all the context arguments
    EXPECT_EQ("change-response:[ \"just\", \"some\", \"data\" ]:"
             "{ \"result\": 2, \"text\": \"'change-response' command not supported.\" }",
              processed_log_);
}
