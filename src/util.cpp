/**
 * @file  util.cpp
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

#include <camoto/gamemaps/util.hpp>

namespace camoto {
namespace gamemaps {

void getLayerDims(const Map2D& map, const Map2D::Layer& layer, Point *layerSize,
	Point *tileSize)
{
	auto layerCaps = layer.caps();
	if (layerCaps & Map2D::Layer::Caps::HasOwnTileSize) {
		*tileSize = layer.tileSize();
	} else {
		// The layer doesn't have its own tile size, so use the map's.
		*tileSize = map.tileSize();
	}
	assert((tileSize->x != 0) && (tileSize->y != 0));

	if (layerCaps & Map2D::Layer::Caps::HasOwnSize) {
		*layerSize = layer.layerSize();
	} else {
		// Layer doesn't have own size, use map
		*layerSize = map.mapSize();
		// The values are in global tiles, but we might have to convert them to
		// layer tiles.
		if (layerCaps & Map2D::Layer::Caps::HasOwnTileSize) {
			auto globalTileSize = map.tileSize();
			// Convert from global tiles to pixels
			layerSize->x *= globalTileSize.x;
			layerSize->y *= globalTileSize.y;
			// Convert from pixels to layer tiles
			layerSize->x /= tileSize->x;
			layerSize->y /= tileSize->y;
		}
	}
	return;
}

} // namespace gamemaps
} // namespace camoto
