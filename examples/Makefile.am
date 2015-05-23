bin_PROGRAMS = gamemap
noinst_PROGRAMS = hello

gamemap_SOURCES = gamemap.cpp
hello_SOURCES = hello.cpp

WARNINGS = -Wall -Wextra -Wno-unused-parameter

AM_CPPFLAGS  = -I $(top_srcdir)/include
AM_CPPFLAGS += $(BOOST_CPPFLAGS)
AM_CPPFLAGS += $(libpng_CPPFLAGS)
AM_CPPFLAGS += $(libgamecommon_CPPFLAGS)
AM_CPPFLAGS += $(libgamegraphics_CPPFLAGS)
AM_CPPFLAGS += $(WARNINGS)

AM_CXXFLAGS  = $(DEBUG_CXXFLAGS)
AM_CXXFLAGS += $(libpng_CFLAGS)
AM_CXXFLAGS += $(libgamecommon_CFLAGS)
AM_CXXFLAGS += $(libgamegraphics_CFLAGS)

AM_LDFLAGS  = $(top_builddir)/src/libgamemaps.la
AM_LDFLAGS += $(BOOST_LDFLAGS)
AM_LDFLAGS += $(BOOST_SYSTEM_LIBS)
AM_LDFLAGS += $(BOOST_PROGRAM_OPTIONS_LIBS)
AM_LDFLAGS += $(libpng_LIBS)
AM_LDFLAGS += $(libgamecommon_LIBS)
AM_LDFLAGS += $(libgamegraphics_LIBS)
