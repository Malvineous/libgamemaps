/**
 * @file  camoto/gamemaps/util.hpp
 * @brief Game map utility functions.
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_GAMEMAPS_UTIL_HPP_
#define _CAMOTO_GAMEMAPS_UTIL_HPP_

#include <camoto/gamemaps/map2d.hpp>

#ifndef CAMOTO_GAMEMAPS_API
#define CAMOTO_GAMEMAPS_API
#endif

namespace camoto {
namespace gamemaps {

/// Get the dimensions and tile size of a layer.
/**
 * @param map
 *   Map containing the layer (for those layers which don't have a size, and
 *   fall back to the map size.)
 *
 * @param layer
 *   Layer to query.
 *
 * @param layerSize
 *   Stores layer width and height on return, in tiles.
 *
 * @param tileSize
 *   Stores tile width and height on return, in pixels.
 */
void CAMOTO_GAMEMAPS_API getLayerDims(const Map2D& map, const Map2D::Layer& layer,
	Point *layerSize, Point *tileSize);

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_UTIL_HPP_
