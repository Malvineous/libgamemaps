Camoto: Classic-game Modding Tools
==================================
Copyright 2010-2016 Adam Nielsen <<malvineous@shikadi.net>>  
http://www.shikadi.net/camoto/  
Linux/OSX: [![Build Status](https://travis-ci.org/Malvineous/libgamemaps.svg?branch=master)](https://travis-ci.org/Malvineous/libgamemaps)

Camoto is a collection of utilities for editing (modding) "classic" PC
games - those running under MS-DOS from the 1980s and 1990s.

This is **libgamemaps**, one component of the Camoto suite.  libgamemaps is a
library that provides a uniform method of accessing the different file formats
used by games to store their levels.

Currently it only supports 2D levels, constructed via a grid of "tiles" (small
images, usually 16x16 pixels each.)  Each game-specific format can be read as
a list of tiles, the list can be modified, and then written back to disk in
the game-specific format.  Multiple layers are supported (to handle games
which have foreground and background tiles) and each layer can have a different
grid size (however the grid size must be consistent within the layer, although
a grid size of 1x1 pixel is possible.)

The library also provides an interface to examine map rules, such as whether a
particular tile is allowed to be placed at a given location.  This can be used
by graphical map editors to provide real-time feedback to a user.

The information provided about each grid element also contains an image
reference, so that a visual representation of the map can be drawn in a user
interface.  The image references are tied in with the libgamegraphics library,
so any application wishing to display a game map as it would look in the game
itself will also need to interact with libgamegraphics to obtain the
appropriate images.  Because of this, the libgamegraphics library is a
dependency of libgamemaps.

Also note that any user of this library will most likely need additional
game-specific information (not supplied by the library) such as which tileset
to use for particular levels and where (in which files) that data is stored.
Some formats (Duke Nukem II) supply filenames of tilesets, while others
(Xargon) do not.  libgamemaps makes this information available if it exists.

Editing maps for the following games are supported:

  * Captain Comic
  * Cosmo's Cosmic Adventures (partial)
  * Crystal Caves
  * Dangerous Dave
  * Dark Ages
  * Duke Nukem I (partial)
  * Duke Nukem II (partial)
  * God of Thunder (partial)
  * Halloween Harry (partial)
  * Hocus Pocus (partial)
  * Jill of the Jungle (partial)
  * Monster Bash (partial)
  * Rockford
  * Scubaventure
  * Secret Agent
  * Vinyl Goddess From Mars (partial)
  * Wacky Wheels
  * Word Rescue
  * Xargon (partial)
  * Zone 66 (partial)

Many more formats are planned.

This distribution includes an example program `gamemap` which serves as both
a command-line interface to the library as well as an example of how to use
the library.  This program is installed as part of the `make install` process.
See `man gamemap` for full details.

The library is compiled and installed in the usual way:

    ./autogen.sh          # Only if compiling from git
    ./configure && make
    make check            # Optional, compile and run tests
    sudo make install
    sudo ldconfig

You will need the following prerequisites already installed:

  * [libgamecommon](https://github.com/Malvineous/libgamecommon) >= 2.0
  * [libgamegraphics](https://github.com/Malvineous/libgamegraphics) >= 2.0
  * Boost >= 1.59 (Boost >= 1.46 will work if not using `make check`)
  * libpng
  * [png++](http://www.nongnu.org/pngpp/) >= 0.2.7
  * xmlto (optional for tarball releases, required for git version and if
    manpages are to be changed)

All supported file formats are fully documented on the
[ModdingWiki](http://www.shikadi.net/moddingwiki/Category:Map_formats).

This library is released under the GPLv3 license.
