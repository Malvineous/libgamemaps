/**
 * @file   gamemaps.hpp
 * @brief  Main header for libgamemaps (includes everything.)
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CAMOTO_GAMEMAPS_HPP_
#define _CAMOTO_GAMEMAPS_HPP_

/// Main namespace
namespace camoto {
/// Namespace for this library
namespace gamemaps {

/**

\mainpage libgamemaps

libgamemaps provides a standard interface to access different game levels
regardless of their file format.

\section structure Structure

The main interface to the library is the getManager() function, which returns
an instance of the Manager class.  The Manager is used to query supported
map formats, and for each supported file format it returns an instance of
the MapType class.

The MapType class can be used to examine files and check what file format
they are in, and if they are in the correct format, to open them.  Successfully
opening a map produces an instance of the Map class.  The MapType class can
also be used to create new maps from scratch, which will again return a Map
instance.

The Map class is used to directly manipulate map files, changing the level as
it appears in-game.  The Map class itself is a base class, and is extended in
the form of the Map2D class for editing maps from platform-style games.  There
are currently no plans to implement a Map3D class (e.g. for Build games)
however should someone be dedicated enough the library can support this.

\section example Examples

The libgamemaps distribution comes with example code in the form of the
<a href="http://github.com/Malvineous/libgamemaps/blob/master/examples/gamemap.cpp">gamemap
utility</a>, which provides a simple command-line interface to the
full functionality of the library.

For a small "hello world" example, try this:

@include hello.cpp

When run, this program produces output similar to the following:

@verbatim
This map has 2 layers.
@endverbatim

\section info More information

Additional information including a mailing list is available from the Camoto
homepage <http://www.shikadi.net/camoto>.

**/
}
}

// These are all in the camoto::gamemaps namespace
#include <camoto/gamemaps/map.hpp>
#include <camoto/gamemaps/maptype.hpp>
#include <camoto/gamemaps/manager.hpp>
#include <camoto/gamemaps/map2d.hpp>
#include <camoto/gamemaps/util.hpp>

#endif // _CAMOTO_GAMEMAPS_HPP_
