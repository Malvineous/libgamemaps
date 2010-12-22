/**
 * @file   map.hpp
 * @brief  Declaration of top-level Map class, for accessing files
 *         storing game map data.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_GAMEMAPS_MAP_HPP_
#define _CAMOTO_GAMEMAPS_MAP_HPP_

#include <boost/shared_ptr.hpp>
#include <exception>
#include <iostream>
#include <sstream>
#include <vector>

#include <camoto/types.hpp>
#include <camoto/metadata.hpp>

namespace camoto {
namespace gamemaps {

/// Generic "invalid map format" exception.
class EInvalidFormat: public std::exception {
};

/// Primary interface to a map file.
/**
 * This class represents a map file.  Its functions are used to edit the map.
 *
 * @note Multithreading: Only call one function in this class at a time.  Many
 *       of the functions seek around the underlying stream and thus will break
 *       if two or more functions are executing at the same time.
 */
class Map: virtual public Metadata {

	public:

		/// Truncate callback
		/**
		 * This function is called with a single unsigned long parameter when
		 * the underlying map file needs to be shrunk or enlarged to the given
		 * size.  It must be set to a valid function before flush() is called.
		 *
		 * The function signature is:
		 * @code
		 * void fnTruncate(unsigned long newLength);
		 * @endcode
		 *
		 * This example uses boost::bind to package up a call to the Linux
		 * truncate() function (which requires both a filename and size) such that
		 * the filename is supplied in advance and not required when flush() makes
		 * the call.
		 *
		 * @code
		 * Map *pMap = ...
		 * pMap->fnTruncate = boost::bind<void>(truncate, "map.dat", _1);
		 * pMap->flush();  // calls truncate("map.dat", 123)
		 * @endcode
		 *
		 * Unfortunately since there is no cross-platform method for changing a
		 * file's size from an open file handle, this is a necessary evil to avoid
		 * passing the map filename around all over the place.
		 */
		//FN_TRUNCATE fnTruncate;

		Map()
			throw ();

		virtual ~Map()
			throw ();

};

/// Shared pointer to a Map.
typedef boost::shared_ptr<Map> MapPtr;

/// Vector of Map shared pointers.
typedef std::vector<MapPtr> VC_MAP;

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_HPP_
