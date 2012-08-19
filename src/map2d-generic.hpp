/**
 * @file   map2d-generic.hpp
 * @brief  Generic implementation of a Map2D interface.
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

#ifndef _CAMOTO_GAMEMAPS_MAP2D_GENERIC_HPP_
#define _CAMOTO_GAMEMAPS_MAP2D_GENERIC_HPP_

#include <camoto/gamemaps/map2d.hpp>
#include "map-generic.hpp"

namespace camoto {
namespace gamemaps {

/// 2D grid-based Map.
class GenericMap2D: virtual public Map2D, virtual public GenericMap
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
		GenericMap2D(AttributePtrVectorPtr attributes, int caps,
			unsigned int viewportWidth, unsigned int viewportHeight,
			unsigned int width, unsigned int height, unsigned int tileWidth,
			unsigned int tileHeight, LayerPtrVector& layers, PathPtrVectorPtr paths);

		/// Destructor.
		virtual ~GenericMap2D();

		/// Get the capabilities of this map format.
		/**
		 * @return One or more of the Caps enum values (OR'd together.)
		 */
		virtual int getCaps();

		/// Retrieve the size of the in-game viewport.
		/**
		 * These dimensions indicate how much of the level can be seen by the player
		 * inside the game.  Given the age of most DOS games, it is typically how
		 * many tiles can be seen on a 320x200 display (minus the space used for the
		 * status bar).
		 *
		 * @param x
		 *   Pointer to store viewport width, in pixels.
		 *
		 * @param y
		 *   Pointer to store layer height, in pixels.
		 */
		virtual void getViewport(unsigned int *x, unsigned int *y);

		/// Retrieve the size of the map.
		/**
		 * @pre getCaps() must include HasGlobalSize.
		 *
		 * @param x
		 *   Pointer to store layer width, as number of tiles.  If getCaps() does
		 *   not include HasOwnTileSize then this value is in pixels instead (i.e.
		 *   a tile size of 1x1 is assumed.)
		 *
		 * @param y
		 *   Pointer to store layer height, as number of tiles.
		 */
		virtual void getMapSize(unsigned int *x, unsigned int *y);

		/// Change the size of each cell in the layer.
		/**
		 * @pre getCaps() must include CanResize.
		 *
		 * @param x
		 *   New layer width, as number of tiles.
		 *
		 * @param y
		 *   New layer height, as number of tiles.
		 */
		virtual void setMapSize(unsigned int x, unsigned int y);

		/// Retrieve the size of each cell in the layer's grid.
		/**
		 * @pre getCaps() includes HasGlobalTileSize.
		 *
		 * @param x
		 *   Pointer to store tile width in pixels.
		 *
		 * @param y
		 *   Pointer to store tile height in pixels.
		 */
		virtual void getTileSize(unsigned int *x, unsigned int *y);

		/// Change the size of each cell in the layer.
		/**
		 * @pre getCaps() must include ChangeTileSize.
		 *
		 * @param x
		 *   New tile width in pixels.
		 *
		 * @param y
		 *   New tile height in pixels.
		 */
		virtual void setTileSize(unsigned int x, unsigned int y);

		/// Get the number of layers in the map.
		/**
		 * @return Number of layers.  All maps have at least one layer.
		 */
		virtual unsigned int getLayerCount();

		/// Get access to the given layer.
		/**
		 * @param index
		 *   Layer index.  Must be < getLayerCount().
		 *
		 * @return A shared pointer to the layer.
		 */
		virtual LayerPtr getLayer(unsigned int index);

		/// Get a list of paths in the level.
		/**
		 * A path is a series of points/vectors defining a travel route.  Unlike
		 * layers, paths are always expressed in pixels, irrespective of tile size.
		 *
		 * @pre getCaps() includes HasPaths.
		 *
		 * @return A shared pointer to a vector of Path instances.  The paths in
		 *   the vector can be edited, but if getCaps() includes FixedPaths then
		 *   paths cannot be created or removed.
		 */
		virtual PathPtrVectorPtr getPaths();

	protected:
		int caps;               ///< Value to return in getCaps().
		unsigned int viewportWidth;      ///< Width of viewport in pixels.
		unsigned int viewportHeight;     ///< Height of viewport in pixels.
		unsigned int width;              ///< Width of map as number of tiles.
		unsigned int height;             ///< Height of map as number of tiles.
		unsigned int tileWidth;          ///< Width of tiles in all layers, in pixels.
		unsigned int tileHeight;         ///< Height of tiles in all layers, in pixels.
		LayerPtrVector layers;  ///< Map layers
		PathPtrVectorPtr paths; ///< Map paths
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

		/// Get the layer's friendly name.
		/**
		 * @return A string containing a name suitable for display to the user.
		 */
		virtual const std::string& getTitle();

		/// Get the capabilities of this layer.
		/**
		 * @return One or more of the Caps enum values (OR'd together.)
		 */
		virtual int getCaps();

		/// Retrieve the size of the layer.
		/**
		 * @pre getCaps() includes HasOwnSize.  Otherwise the map's size must be
		 *   used.
		 *
		 * @param x
		 *   Pointer to store layer width, as number of tiles.
		 *
		 * @param y
		 *   Pointer to store layer height, as number of tiles.
		 */
		virtual void getLayerSize(unsigned int *x, unsigned int *y);

		/// Change the size of each cell in the layer.
		/**
		 * @pre getCaps() must include CanResize.
		 *
		 * @param x
		 *   New layer width, as number of tiles.
		 *
		 * @param y
		 *   New layer height, as number of tiles.
		 */
		virtual void setLayerSize(unsigned int x, unsigned int y);

		/// Retrieve the size of each cell in the layer's grid.
		/**
		 * @pre getCaps() includes HasOwnTileSize.  If not, the map's tile size
		 *   must be used.
		 *
		 * @param x
		 *   Pointer to store tile width in pixels.
		 *
		 * @param y
		 *   Pointer to store tile height in pixels.
		 */
		virtual void getTileSize(unsigned int *x, unsigned int *y);

		/// Change the size of each cell in the layer.
		/**
		 * @pre getCaps() must include ChangeTileSize.
		 *
		 * @param x
		 *   New tile width in pixels.
		 *
		 * @param y
		 *   New tile height in pixels.
		 */
		virtual void setTileSize(unsigned int x, unsigned int y);

		/// Get a list of all tiles in the layer.
		/**
		 * @return Vector of all tiles.  The tiles are in any order.
		 */
		virtual const ItemPtrVectorPtr getAllItems();

		/// Convert a map code into an image.
		/**
		 * @param code
		 *   Map2D::Layer::Item::code obtained from getAllItems().
		 *
		 * @param tileset
		 *   Vector of Tileset instances used to obtain the Image to return.
		 *   Which tilesets to actually pass in is beyond the scope of this
		 *   library, and must be obtained by some caller defined method.
		 *   Camoto Studio reads this information from XML files distributed
		 *   with the application, for example.
		 *
		 * @return Shared pointer to a camoto::gamegraphics::Image instance.  A
		 *   return value of a null pointer will result in some sort of
		 *   unknown/question mark tile being used.
		 */
		virtual camoto::gamegraphics::ImagePtr imageFromCode(unsigned int code,
			gamegraphics::VC_TILESET& tileset);

		/// Is the given tile permitted at the specified location?
		/**
		 * @param code
		 *   Map2D::Layer::Item::code obtained from getAllItems().
		 *
		 * @param x
		 *   Proposed X coordinate, in tiles.
		 *
		 * @param y
		 *   Proposed Y coordinate, in tiles.
		 *
		 * @param maxCount
		 *   On return, set to the maximum number of instances of this tile code
		 *   permitted in this level.  For example if this value is 1, the tile
		 *   code must be unique in the level (e.g. it might be the level starting
		 *   point.)  A value of zero means unlimited.
		 *
		 * @return true if the tile is permitted at the current position (instance
		 *  limits notwithstanding) or false if the tile cannot be placed here.
		 */
		virtual bool tilePermittedAt(unsigned int code, unsigned int x,
			unsigned int y, unsigned int *maxCount);

		/// Get the palette to use with this layer.
		/**
		 * Some tilesets don't have a palette, so in this case the palette to use
		 * can be supplied here.  Palettes applied to individual tiles will still
		 * override this.
		 *
		 * @pre getCaps() return value includes HasPalette.
		 *
		 * @return Shared pointer to a PaletteTable.
		 */
		virtual gamegraphics::PaletteTablePtr getPalette(
			gamegraphics::VC_TILESET& tileset);

		/// Get a list of all possible items that can be placed in the layer.
		/**
		 * This is suitable for display to the user, to allow selection of items
		 * to insert into the layer.
		 *
		 * Items are copied (i.e. with the copy constructor) if they are to be
		 * inserted into a layer.
		 *
		 * @return Vector of all items.
		 */
		virtual const ItemPtrVectorPtr getValidItemList();

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
