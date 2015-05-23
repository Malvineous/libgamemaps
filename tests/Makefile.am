check_PROGRAMS = tests

tests_SOURCES = tests.cpp
tests_SOURCES += test-map2d.cpp
tests_SOURCES += test-map-bash.cpp
tests_SOURCES += test-map-ccaves.cpp
tests_SOURCES += test-map-ccomic.cpp
tests_SOURCES += test-map-cosmo.cpp
tests_SOURCES += test-map-ddave.cpp
tests_SOURCES += test-map-got.cpp
tests_SOURCES += test-map-harry.cpp
tests_SOURCES += test-map-jill.cpp
tests_SOURCES += test-map-nukem2.cpp
tests_SOURCES += test-map-sagent.cpp
tests_SOURCES += test-map-vinyl.cpp
tests_SOURCES += test-map-wacky.cpp
tests_SOURCES += test-map-wordresc.cpp
tests_SOURCES += test-map-xargon.cpp

EXTRA_tests_SOURCES = tests.hpp
EXTRA_tests_SOURCES += test-map2d.hpp

TESTS = tests

AM_CPPFLAGS  = -I $(top_srcdir)/include
AM_CPPFLAGS += $(BOOST_CPPFLAGS)
AM_CPPFLAGS += $(libgamecommon_CPPFLAGS)
AM_CPPFLAGS += $(libgamegraphics_CPPFLAGS)

AM_CXXFLAGS  = $(DEBUG_CXXFLAGS)
AM_CXXFLAGS += $(libgamecommon_CFLAGS)
AM_CXXFLAGS += $(libgamegraphics_CFLAGS)

AM_LDFLAGS  = $(top_builddir)/src/libgamemaps.la
AM_LDFLAGS += $(BOOST_LDFLAGS)
AM_LDFLAGS += $(BOOST_UNIT_TEST_FRAMEWORK_LIB)
AM_LDFLAGS += $(libgamecommon_LIBS)
AM_LDFLAGS += $(libgamegraphics_LIBS)
