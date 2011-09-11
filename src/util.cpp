/**
 * @file   util.cpp
 * @brief  Game map utility functions.
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

#include <camoto/gamemaps/util.hpp>

namespace camoto {
namespace gamemaps {

bool getLayerDims(Map2DPtr map, Map2D::LayerPtr layer, int *layerWidth,
	int *layerHeight, int *tileWidth, int *tileHeight)
{
	int mapCaps = map->getCaps();
	int layerCaps = layer->getCaps();
	if (layerCaps & Map2D::Layer::HasOwnTileSize) {
		layer->getTileSize(tileWidth, tileHeight);
	} else if (mapCaps & Map2D::HasGlobalTileSize) {
		// The layer doesn't have its own tile size, so use the map's.
		map->getTileSize(tileWidth, tileHeight);
	} else {
		// Neither the map nor the layer have a tile size, use the default.
		*tileWidth = *tileHeight = 1;
	}
	assert((*tileWidth != 0) && (*tileHeight != 0));

	if (layerCaps & Map2D::Layer::HasOwnSize) {
		layer->getLayerSize(layerWidth, layerHeight);
	} else if (mapCaps & Map2D::HasGlobalSize) {
		// Layer doesn't have own size, use map
		map->getMapSize(layerWidth, layerHeight);
		// The values are in pixels, so we need to convert them to tiles
		*layerWidth /= *tileWidth;
		*layerHeight /= *tileHeight;
	} else {
		return false;
	}
	return true;
}

} // namespace gamemaps
} // namespace camoto
