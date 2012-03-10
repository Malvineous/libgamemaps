lib_LTLIBRARIES = libgamemaps.la

libgamemaps_la_SOURCES = main.cpp
libgamemaps_la_SOURCES += map.cpp
libgamemaps_la_SOURCES += maptype.cpp
libgamemaps_la_SOURCES += map2d.cpp
libgamemaps_la_SOURCES += map2d_layer.cpp
libgamemaps_la_SOURCES += util.cpp
libgamemaps_la_SOURCES += fmt-map-bash.cpp
libgamemaps_la_SOURCES += fmt-map-ccaves.cpp
libgamemaps_la_SOURCES += fmt-map-ccomic.cpp
libgamemaps_la_SOURCES += fmt-map-cosmo.cpp
libgamemaps_la_SOURCES += fmt-map-ddave.cpp
libgamemaps_la_SOURCES += fmt-map-harry.cpp
libgamemaps_la_SOURCES += fmt-map-hocus.cpp
libgamemaps_la_SOURCES += fmt-map-wacky.cpp
libgamemaps_la_SOURCES += fmt-map-wordresc.cpp
libgamemaps_la_SOURCES += fmt-map-xargon.cpp

EXTRA_libgamemaps_la_SOURCES = fmt-map-bash.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-ccaves.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-ccomic.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-cosmo.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-ddave.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-harry.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-hocus.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-wacky.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-wordresc.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-xargon.hpp

WARNINGS = -Wall -Wextra -Wno-unused-parameter

AM_CPPFLAGS = $(DEBUG_CPPFLAGS) $(BOOST_CPPFLAGS) -I $(top_srcdir)/include $(libgamecommon_CFLAGS) $(WARNINGS)
AM_CXXFLAGS = $(DEBUG_CXXFLAGS)

libgamemaps_la_LDFLAGS = $(AM_LDFLAGS) -version-info 0:0:0
libgamemaps_la_LIBADD = $(BOOST_SYSTEM_LIBS) $(BOOST_FILESYSTEM_LIBS) $(libgamecommon_LIBS)
