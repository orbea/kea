SUBDIRS = . tests testutils

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib
AM_CPPFLAGS += $(BOOST_INCLUDES)
AM_CXXFLAGS = $(KEA_CXXFLAGS)

lib_LTLIBRARIES = libkea-stats.la
libkea_stats_la_SOURCES = observation.h observation.cc
libkea_stats_la_SOURCES += context.h context.cc
libkea_stats_la_SOURCES += stats_mgr.h stats_mgr.cc

libkea_stats_la_CPPFLAGS = $(AM_CPPFLAGS)
libkea_stats_la_LDFLAGS = -no-undefined -version-info 18:0:0

libkea_stats_la_LIBADD  = $(top_builddir)/src/lib/cc/libkea-cc.la
libkea_stats_la_LIBADD += $(top_builddir)/src/lib/asiolink/libkea-asiolink.la
libkea_stats_la_LIBADD += $(top_builddir)/src/lib/util/libkea-util.la
libkea_stats_la_LIBADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
libkea_stats_la_LIBADD += $(BOOST_LIBS)

EXTRA_DIST = stats.dox

libkea_stats_includedir = $(pkgincludedir)/stats
libkea_stats_include_HEADERS = \
	context.h \
	observation.h \
	stats_mgr.h

