SUBDIRS = .

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib
AM_CPPFLAGS += -I$(top_srcdir)/src/bin -I$(top_builddir)/src/bin
AM_CPPFLAGS += $(BOOST_INCLUDES)
AM_CPPFLAGS += -DTEST_DATA_BUILDDIR=\"$(abs_top_builddir)/src/bin/lfc/tests\"
AM_CPPFLAGS += -DINSTALL_PROG=\"$(abs_top_srcdir)/install-sh\"

CLEANFILES = $(builddir)/interfaces.txt $(builddir)/logger_lockfile

AM_CXXFLAGS = $(KEA_CXXFLAGS)

if USE_STATIC_LINK
AM_LDFLAGS = -static
endif

TESTS_ENVIRONMENT = \
        $(LIBTOOL) --mode=execute $(VALGRIND_COMMAND)

TESTS =
if HAVE_GTEST

TESTS += lfc_unittests

lfc_unittests_SOURCES = lfc_unittests.cc
lfc_unittests_SOURCES += lfc_controller_unittests.cc

lfc_unittests_CPPFLAGS = $(AM_CPPFLAGS) $(GTEST_INCLUDES)
lfc_unittests_LDFLAGS = $(AM_LDFLAGS) $(CRYPTO_LDFLAGS)
if HAVE_MYSQL
lfc_unittests_LDFLAGS += $(MYSQL_LIBS)
endif
if HAVE_PGSQL
lfc_unittests_LDFLAGS += $(PGSQL_LIBS)
endif
lfc_unittests_LDFLAGS += $(GTEST_LDFLAGS)

lfc_unittests_LDADD = $(top_builddir)/src/bin/lfc/liblfc.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/cfgrpt/libcfgrpt.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/dhcpsrv/libkea-dhcpsrv.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/eval/libkea-eval.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/dhcp_ddns/libkea-dhcp_ddns.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/process/libkea-process.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/stats/libkea-stats.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/config/libkea-cfgclient.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/http/libkea-http.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/dhcp/libkea-dhcp++.la

if HAVE_MYSQL
lfc_unittests_LDADD += $(top_builddir)/src/lib/mysql/libkea-mysql.la
endif
if HAVE_PGSQL
lfc_unittests_LDADD += $(top_builddir)/src/lib/pgsql/libkea-pgsql.la
endif

lfc_unittests_LDADD += $(top_builddir)/src/lib/database/libkea-database.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/asiolink/libkea-asiolink.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/cc/libkea-cc.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/dns/libkea-dns++.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/cryptolink/libkea-cryptolink.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/hooks/libkea-hooks.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/log/libkea-log.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/util/libkea-util.la
lfc_unittests_LDADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
lfc_unittests_LDADD += $(LOG4CPLUS_LIBS) $(CRYPTO_LIBS)
lfc_unittests_LDADD += $(BOOST_LIBS) $(GTEST_LDADD)

endif

noinst_PROGRAMS = $(TESTS)
