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

void getLayerDims(Map2DPtr map, Map2D::LayerPtr layer, unsigned int *layerWidth,
	unsigned int *layerHeight, unsigned int *tileWidth, unsigned int *tileHeight)
{
	int layerCaps = layer->getCaps();
	if (layerCaps & Map2D::Layer::HasOwnTileSize) {
		layer->getTileSize(tileWidth, tileHeight);
	} else {
		// The layer doesn't have its own tile size, so use the map's.
		map->getTileSize(tileWidth, tileHeight);
	}
	assert((*tileWidth != 0) && (*tileHeight != 0));

	if (layerCaps & Map2D::Layer::HasOwnSize) {
		layer->getLayerSize(layerWidth, layerHeight);
	} else {
		// Layer doesn't have own size, use map
		map->getMapSize(layerWidth, layerHeight);
		// The values are in global tiles, but we might have to convert them to
		// layer tiles.
		if (layerCaps & Map2D::Layer::HasOwnTileSize) {
			unsigned int globalTileWidth, globalTileHeight;
			map->getTileSize(&globalTileWidth, &globalTileHeight);
			// Convert from global tiles to pixels
			*layerWidth *= globalTileWidth;
			*layerHeight *= globalTileHeight;
			// Convert from pixels to layer tiles
			*layerWidth /= *tileWidth;
			*layerHeight /= *tileHeight;
		}
	}
	return;
}

} // namespace gamemaps
} // namespace camoto
