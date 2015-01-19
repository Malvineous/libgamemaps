/**
 * @file   map2d-generic.hpp
 * @brief  Generic implementation of a Map2D interface.
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

#ifndef _CAMOTO_GAMEMAPS_MAP2D_GENERIC_HPP_
#define _CAMOTO_GAMEMAPS_MAP2D_GENERIC_HPP_

#include <camoto/gamemaps/map2d.hpp>

namespace camoto {
namespace gamemaps {

/// 2D grid-based Map.
class GenericMap2D: public Map2D
{
	public:
		class Layer;

		/// Create a new 2D map.
		/**
		 * @param attributes
		 *   List of attributes that apply to this map.
		 *
		 * @param caps
		 *   Map capabilities.  One or more Caps values OR'd together.
		 *
		 * @param viewportWidth
		 *   Width of the viewport in pixels.  Only required if caps includes
		 *   HasViewport.  See getViewport().
		 *
		 * @param viewportHeight
		 *   Height of the viewport in pixels.  Only required if caps includes
		 *   HasViewport.  See getViewport().
		 *
		 * @param width
		 *   Global map width, in number of tiles.  Width of each tile is
		 *   specified by tileWidth.  Applies to all layers unless a layer's caps
		 *   includes HasOwnSize.
		 *
		 * @param height
		 *   Global map height, in number of tiles.  Height of each tile is
		 *   specified by tileHeight.  Applies to all layers unless a layer's caps
		 *   includes HasOwnSize.
		 *
		 * @param tileWidth
		 *   Default tile width in pixels.  Applies to all layers unless a layer's
		 *   caps includes HasOwnTileSize.  This is also the smallest amount a
		 *   level can be resized by.  Must be > 0, use 1 if there are no tiles.
		 *
		 * @param tileHeight
		 *   Default tile height in pixels.  Applies to all layers unless a layer's
		 *   caps includes HasOwnTileSize.  This is also the smallest amount a
		 *   level can be resized by.  Must be > 0, use 1 if there are no tiles.
		 *
		 * @param layers
		 *   Vector of map layers.
		 *
		 * @param paths
		 *   Possibly empty vector of map paths.
		 *
		 * @note tileWidth and tileHeight should specify the smallest multiple of
		 *   the underlying tile size, in the event a map uses different tile sizes
		 *   between layers.  This way the level will be resized by a multiple of
		 *   this value, preventing the level from ever being a size where there is
		 *   only room for half a tile.
		 *
		 * @note A layer can specify a different tile size but the same total
		 *   dimensions.  In this case the pixel width is the same as the map, but
		 *   more tiles will fit in the area.  To find the size in units of tiles,
		 *   the map size will have to be multipled by the map tile size to get the
		 *   map size in pixels, then divided by the layer's different tile size to
		 *   reveal the dimensions of the layer in a number of tiles.
		 */
		GenericMap2D(const Attributes& attributes,
			const GraphicsFilenames& graphicsFilenames, unsigned int caps,
			unsigned int viewportWidth, unsigned int viewportHeight,
			unsigned int width, unsigned int height,
			unsigned int tileWidth, unsigned int tileHeight,
			const LayerPtrVector& layers, PathPtrVectorPtr paths);

		/// Destructor.
		virtual ~GenericMap2D();

		virtual void getMapSize(unsigned int *x, unsigned int *y) const;
		virtual void setMapSize(unsigned int x, unsigned int y);
		virtual void getTileSize(unsigned int *x, unsigned int *y) const;
		virtual void setTileSize(unsigned int x, unsigned int y);
		virtual unsigned int getLayerCount() const;
		virtual LayerPtr getLayer(unsigned int index);
		virtual PathPtrVectorPtr getPaths();

	protected:
		unsigned int viewportWidth;   ///< Width of viewport in pixels.
		unsigned int viewportHeight;  ///< Height of viewport in pixels.
		unsigned int width;           ///< Width of map as number of tiles.
		unsigned int height;          ///< Height of map as number of tiles.
		unsigned int tileWidth;       ///< Width of tiles in all layers, in pixels.
		unsigned int tileHeight;      ///< Height of tiles in all layers, in pixels.
		LayerPtrVector layers;        ///< Map layers
		PathPtrVectorPtr paths;       ///< Map paths
};

class GenericMap2D::Layer: virtual public Map2D::Layer
{
	public:
		/// Create a new layer.
		/**
		 * @param title
		 *   User-visible friendly name for the layer.
		 *
		 * @param caps
		 *   Capabilities to return in getCaps().
		 *
		 * @param width
		 *   Layer width as number of tiles.  Only used if caps includes HasOwnSize.
		 *
		 * @param height
		 *   Layer height as number of tiles.  Only used if caps includes HasOwnSize.
		 *
		 * @param tileWidth
		 *   Tile/grid width, in pixels.  Only used if caps includes HasOwnTileSize.
		 *
		 * @param tileHeight
		 *   Tile/grid height, in pixels.  Only used if caps includes HasOwnTileSize.
		 *
		 * @param items
		 *   Vector containing all items in the layer.
		 *
		 * @param validItems
		 *   Vector containing all valid items that could be placed in the layer.
		 *
		 * @see GenericMap2D::GenericMap2D() for more details on how the layer and tile
		 *   dimensions are handled.
		 */
		Layer(const std::string& title, int caps, unsigned int width,
			unsigned int height, unsigned int tileWidth, unsigned int tileHeight,
			ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems);

		/// Destructor.
		virtual ~Layer();

		virtual const std::string& getTitle() const;
		virtual unsigned int getCaps() const;
		virtual void getLayerSize(unsigned int *x, unsigned int *y) const;
		virtual void setLayerSize(unsigned int x, unsigned int y);
		virtual void getTileSize(unsigned int *x, unsigned int *y) const;
		virtual void setTileSize(unsigned int x, unsigned int y);
		virtual ItemPtrVectorPtr getAllItems();
		virtual camoto::gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset) const;
		virtual bool tilePermittedAt(const Map2D::Layer::ItemPtr& item,
			unsigned int x, unsigned int y, unsigned int *maxCount) const;
		virtual gamegraphics::PaletteTablePtr getPalette(
			const TilesetCollectionPtr& tileset) const;
		virtual const ItemPtrVectorPtr getValidItemList() const;

	protected:
		std::string title;       ///< Layer's friendly name
		int caps;                ///< Map capabilities
		unsigned int width;      ///< Map width, in tiles
		unsigned int height;     ///< Map height, in tiles
		unsigned int tileWidth;  ///< Tile width, in pixels
		unsigned int tileHeight; ///< Tile height, in pixels
		ItemPtrVectorPtr items;  ///< Vector of all items in the layer
		gamegraphics::PaletteTablePtr pal; ///< Optional palette for layer
		ItemPtrVectorPtr validItems; ///< Vector of possible items in the layer
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP2D_GENERIC_HPP_
