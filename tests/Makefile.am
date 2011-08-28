check_PROGRAMS = tests

tests_SOURCES = tests.cpp
tests_SOURCES += test-map-bash.cpp
tests_SOURCES += test-map-ccaves.cpp
tests_SOURCES += test-map-ccomic.cpp
tests_SOURCES += test-map-cosmo.cpp
tests_SOURCES += test-map-ddave.cpp
tests_SOURCES += test-map-harry.cpp
tests_SOURCES += test-map-wacky.cpp
tests_SOURCES += test-map-xargon.cpp

EXTRA_tests_SOURCES = tests.hpp
EXTRA_tests_SOURCES += test-map2d.hpp

TESTS = tests

AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(libgamecommon_CFLAGS) -I $(top_srcdir)/include
AM_LDFLAGS = $(BOOST_SYSTEM_LIBS) $(BOOST_UNIT_TEST_FRAMEWORK_LIBS) $(libgamecommon_LIBS) $(top_builddir)/src/libgamemaps.la
