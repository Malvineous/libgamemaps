/**
 * @file  map2d-core.hpp
 * @brief Implementation of Map2D functions inherited by most format handlers.
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

#ifndef _CAMOTO_GAMEMAPS_MAP2D_CORE_HPP_
#define _CAMOTO_GAMEMAPS_MAP2D_CORE_HPP_

#include <camoto/gamemaps/map2d.hpp>

namespace camoto {
namespace gamemaps {

/// Common implementation of 2D grid-based Map.
class Map2DCore: virtual public Map2D
{
	public:
		class LayerCore;

		virtual ~Map2DCore();

		// These are all default functions so descendent classes don't have to
		// implement functions that end up being no-ops.
		virtual Point viewport() const;
		virtual Point mapSize() const;
		virtual void mapSize(const Point& newSize);
		virtual Point tileSize() const;
		virtual void tileSize(const Point& newSize);
		virtual std::vector<std::shared_ptr<Map2D::Layer>> layers();
		virtual std::vector<std::shared_ptr<const Map2D::Layer>> layers() const;
		virtual std::vector<std::shared_ptr<Map2D::Path>>& paths();
		virtual Background background(const TilesetCollection& tileset)
			const;

	protected:
		/// Use a tilecode for the map background.
		/**
		 * This is a helper function for descendent classes to use when implementing
		 * background(), for those maps that display a specific tile for the map
		 * background.
		 *
		 * Often the map background is the first tile and it's completely black,
		 * however just using a solid black background would be incorrect because
		 * then changing that first tile to a different image would cause the map
		 * background to change in the game, so that behaviour should be mirrored in
		 * the level editor also.  This means care must be taken to ensure the
		 * correct background is specified! (first tile vs solid colour)
		 */
		Background backgroundFromTilecode(const TilesetCollection& tileset,
			unsigned int code) const;

		std::vector<std::shared_ptr<Layer>> v_layers; ///< Layers for layers()
		std::vector<std::shared_ptr<Path>> v_paths; ///< Paths for paths()
};

class Map2DCore::LayerCore: virtual public Map2D::Layer
{
	public:
		/// Create a new layer.
		/**
		 * @param layerSize
		 *   Layer width and height, as number of tiles.  Only used if caps()
		 *   includes HasOwnSize.
		 *
		 * @param tileSize
		 *   Tile/grid width and height, in pixels.  Only used if caps includes
		 *   HasOwnTileSize.
		 *
		 * @param items
		 *   Vector containing all items in the layer.
		 *
		 * @param validItems
		 *   Vector containing all valid items that could be placed in the layer.
		 *
		 * @see GenericMap2D::GenericMap2D() for more details on how the layer and
		 *   tile dimensions are handled.
		 */
		/*
		LayerCore(Point layerSize, Point tileSize,
			std::vector<std::shared_ptr<Item>> items,
			std::vector<std::shared_ptr<Item>> validItems);
		*/

		/// Destructor.
		virtual ~LayerCore();

		virtual Point layerSize() const;
		virtual void layerSize(const Point& newSize);
		virtual Point tileSize() const;
		virtual void tileSize(const Point& newSize);

		virtual std::vector<Item>& items();
		virtual std::vector<Item> items() const;
		virtual ImageFromCodeInfo imageFromCode(const Map2D::Layer::Item& item,
			const TilesetCollection& tileset) const;
		virtual bool tilePermittedAt(const Map2D::Layer::Item& item,
			const Point& pos, unsigned int *maxCount) const;
		virtual std::shared_ptr<const gamegraphics::Palette> palette(
			const TilesetCollection& tileset) const;

	protected:
		Point v_layerSize;       ///< Map width and height, in tiles
		Point v_tileSize;        ///< Tile width and height, in pixels
		std::vector<Item> v_allItems;

		std::shared_ptr<const gamegraphics::Palette> pal; ///< Optional palette for layer
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP2D_CORE_HPP_
