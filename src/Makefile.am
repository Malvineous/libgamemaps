lib_LTLIBRARIES = libgamemaps.la

libgamemaps_la_SOURCES  = main.cpp
libgamemaps_la_SOURCES += map-core.cpp
libgamemaps_la_SOURCES += map2d-core.cpp
libgamemaps_la_SOURCES += fmt-map-bash.cpp
libgamemaps_la_SOURCES += fmt-map-ccaves.cpp
libgamemaps_la_SOURCES += fmt-map-ccomic.cpp
libgamemaps_la_SOURCES += fmt-map-cosmo.cpp
libgamemaps_la_SOURCES += fmt-map-darkages.cpp
libgamemaps_la_SOURCES += fmt-map-ddave.cpp
libgamemaps_la_SOURCES += fmt-map-duke1.cpp
libgamemaps_la_SOURCES += fmt-map-got.cpp
libgamemaps_la_SOURCES += fmt-map-harry.cpp
libgamemaps_la_SOURCES += fmt-map-hocus.cpp
libgamemaps_la_SOURCES += fmt-map-nukem2.cpp
libgamemaps_la_SOURCES += fmt-map-rockford.cpp
libgamemaps_la_SOURCES += fmt-map-sagent.cpp
libgamemaps_la_SOURCES += fmt-map-vinyl.cpp
libgamemaps_la_SOURCES += fmt-map-wacky.cpp
libgamemaps_la_SOURCES += fmt-map-wordresc.cpp
libgamemaps_la_SOURCES += fmt-map-xargon.cpp
libgamemaps_la_SOURCES += fmt-map-zone66.cpp
libgamemaps_la_SOURCES += util.cpp

EXTRA_libgamemaps_la_SOURCES  = map-core.hpp
EXTRA_libgamemaps_la_SOURCES += map2d-core.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-bash.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-ccaves.hpp fmt-map-ccaves-mapping.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-ccomic.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-cosmo.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-darkages.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-ddave.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-duke1.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-got.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-harry.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-hocus.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-nukem2.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-rockford.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-sagent.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-sagent-mapping.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-vinyl.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-wacky.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-wordresc.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-xargon.hpp
EXTRA_libgamemaps_la_SOURCES += fmt-map-zone66.hpp

WARNINGS = -Wall -Wextra -Wno-unused-parameter

AM_CPPFLAGS  = -I $(top_srcdir)/include
AM_CPPFLAGS += $(libgamecommon_CPPFLAGS)
AM_CPPFLAGS += $(libgamegraphics_CPPFLAGS)
AM_CPPFLAGS += $(WARNINGS)

AM_CXXFLAGS  = $(DEBUG_CXXFLAGS)
AM_CXXFLAGS += $(libgamecommon_CFLAGS)
AM_CXXFLAGS += $(libgamegraphics_CFLAGS)

libgamemaps_la_LDFLAGS  = $(AM_LDFLAGS)
libgamemaps_la_LDFLAGS += -version-info 2:0:0

libgamemaps_la_LIBADD  = $(libgamecommon_LIBS)
libgamemaps_la_LIBADD += $(libgamegraphics_LIBS)
