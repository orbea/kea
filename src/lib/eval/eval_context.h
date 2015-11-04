// Copyright (C) 2015 Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifndef EVAL_CONTEXT_H
#define EVAL_CONTEXT_H
#include <string>
#include <map>
#include <eval/parser.h>
#include <exceptions/exceptions.h>

// Tell Flex the lexer's prototype ...
#define YY_DECL isc::eval::EvalParser::symbol_type yylex (EvalContext& driver)

// ... and declare it for the parser's sake.
YY_DECL;

/// @brief Evaluation error exception raised when trying to parse an axceptions.
class EvalError : public isc::exceptions::Exception {
public:
    EvalError(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) { };
};


/// @brief Evaluation context, an interface to the expression evaluation.
class EvalContext
{
public:
    /// @brief Default constructor.
    EvalContext();

    /// @brief destructor
    virtual ~EvalContext();

    /// @brief Parsed expression (output tokens are stored here)
    isc::dhcp::Expression expression;

    /// @brief Method called before scanning starts.
    void scanBegin();

    /// @brief Method called after the last tokens are scanned.
    void scanEnd();
    
    /// @brief Runs the parser on specified file.
    ///
    /// @param filename
    /// Return 0 on success.
    int parseFile(const std::string& filename);

    /// @brief Run the parser on the string specified.
    ///
    /// @param str string to be written
    int parseString(const std::string& str);

    /// @brief The name of the file being parsed.
    /// Used later to pass the file name to the location tracker.
    std::string file;

    /// @brief Error handler
    ///
    /// @param l location within the parsed file when experienced a problem.
    /// @param what string explaining the nature of the error.
    void error(const isc::eval::location& l, const std::string& what);

    /// @brief Error handler
    ///
    /// This is a simplified error reporting tool for possible future
    /// cases when the EvalParser is not able to handle the packet.
    void error(const std::string& what);

 private:
    /// @brief Flag determining scanner debugging.
    bool trace_scanning_;

    /// @brief Flag determing parser debugging.
    bool trace_parsing_;
  
};
#endif // ! EVALCONTEXT_HH
