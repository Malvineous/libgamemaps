/**
 * @file   map-generic.hpp
 * @brief  Generic implementation of Map interface.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_GAMEMAPS_MAP_GENERIC_HPP_
#define _CAMOTO_GAMEMAPS_MAP_GENERIC_HPP_

#include <boost/function.hpp>
#include <camoto/gamemaps/map.hpp>

namespace camoto {
namespace gamemaps {

/// Callback to use when the list of graphics filenames is required
/**
 * This is used as a convenience function, to avoid having to derive from
 * GenericMap and implement getGraphicsFilenames().
 */
typedef boost::function<Map::GraphicsFilenamesPtr(const Map *)> GraphicsFilenamesCallback;

/// Value to use when there is no callback to get the list of graphics filenames
#define NO_GFX_CALLBACK ((GraphicsFilenamesCallback)NULL)

/// Generic implementation of a Map.
class GenericMap: virtual public Map
{
	public:
		GenericMap(AttributePtrVectorPtr attributes,
			GraphicsFilenamesCallback fnGfxFiles);
		virtual ~GenericMap();

		virtual AttributePtrVectorPtr getAttributes();
		virtual const AttributePtrVectorPtr getAttributes() const;
		virtual GraphicsFilenamesPtr getGraphicsFilenames() const;

	protected:
		/// Vector holding the current attributes
		AttributePtrVectorPtr attributes;

		/// Callback function to get the names of the graphics files
		GraphicsFilenamesCallback fnGfxFiles;
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_GENERIC_HPP_
