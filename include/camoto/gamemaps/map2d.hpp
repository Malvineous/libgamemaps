/**
 * @file   map2d.hpp
 * @brief  2D grid-based Map interface.
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

#ifndef _CAMOTO_GAMEMAPS_MAP2D_HPP_
#define _CAMOTO_GAMEMAPS_MAP2D_HPP_

#include <vector>
#include <map>
#include <camoto/gamemaps/map.hpp>

namespace camoto {
namespace gamemaps {

/// 2D grid-based Map.
class Map2D: virtual public Map {

// caps: resize layer, resize map, set grid size
	public:

		/// Capabilities this layer supports.
		enum Caps {
			NoCaps            = 0x00, ///< No caps set
			HasGlobalSize     = 0x01, ///< Does the map have one size for all layers?
			CanResize         = 0x02, ///< Can the map be resized as a whole?
			HasGlobalTileSize = 0x04, ///< Does the map have one tile size for all layers?
			ChangeTileSize    = 0x08, ///< Can the map's grid size be changed?
		};

		class Layer;
		/// Shared pointer to a layer instance.
		typedef boost::shared_ptr<Layer> LayerPtr;
		/// Vector of layers.
		typedef std::vector<LayerPtr> LayerPtrVector;

		/// Create a new 2D map.
		/**
		 * @param caps
		 *   Map capabilities.  One or more Caps values OR'd together.
		 *
		 * @param width
		 *   Global map width, in tiles.  Only required if caps includes
		 *   HasGlobalSize.
		 *
		 * @param height
		 *   Global map height, in tiles.  Only required if caps includes
		 *   HasGlobalSize.
		 *
		 * @param tileWidth
		 *   Tile width in pixels for all layers.  Only required if caps includes
		 *   HasGlobalTileSize.
		 *
		 * @param tileHeight
		 *   Tile height in pixels for all layers.  Only required if caps includes
		 *   HasGlobalTileSize.
		 *
		 * @param layers
		 *   Vector of map layers.
		 */
		Map2D(int caps, int width, int height, int tileWidth, int tileHeight,
			LayerPtrVector& layers)
			throw ();

		/// Destructor.
		virtual ~Map2D()
			throw ();

		/// Get the capabilities of this tileset format.
		/**
		 * @return One or more of the Caps enum values (OR'd together.)
		 */
		virtual int getCaps()
			throw ();

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
		virtual void getMapSize(int *x, int *y)
			throw ();

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
		virtual void setMapSize(int x, int y)
			throw ();

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
		virtual void getTileSize(int *x, int *y)
			throw ();

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
		virtual void setTileSize(int x, int y)
			throw ();

		/// Get the number of layers in the map.
		/**
		 * @return Number of layers.  All maps have at least one layer.
		 */
		virtual int getLayerCount()
			throw ();

		/// Get access to the given layer.
		/**
		 * @param index
		 *   Layer index.  Must be < getLayerCount().
		 *
		 * @return A shared pointer to the layer.
		 */
		virtual LayerPtr getLayer(int index)
			throw ();

	protected:
		int caps;       ///< Value to return in getCaps().
		int width;      ///< Width of map as number of tiles.
		int height;     ///< Height of map as number of tiles.
		int tileWidth;  ///< Width of tiles in all layers, in pixels.
		int tileHeight; ///< Height of tiles in all layers, in pixels.
		LayerPtrVector layers; ///< Map layers
};

/// Shared pointer to an MapType.
typedef boost::shared_ptr<Map2D> Map2DPtr;

/// A map is made up of multiple layers.
class Map2D::Layer {

	public:
		class Item;
		/// Shared pointer to an item.  This is used to avoid copying items around
		/// which can be problematic when they instances of descendent classes.
		typedef boost::shared_ptr<Item> ItemPtr;
		/// Vector of items.
		typedef std::vector<ItemPtr> ItemPtrVector;
		/// Shared pointer to a vector of items.
		typedef boost::shared_ptr<ItemPtrVector> ItemPtrVectorPtr;

		class Text;
		/// Shared pointer to a text element.
		typedef boost::shared_ptr<Text> TextPtr;
		/// Vector of text elements.
		typedef std::vector<TextPtr> TextPtrVector;

		/// Capabilities this layer supports.
		enum Caps {
			NoCaps          = 0x00, ///< No caps set
			HasOwnSize      = 0x01, ///< Does the layer have an independent size?
			CanResize       = 0x02, ///< Can just this layer be resized?
			HasOwnTileSize  = 0x04, ///< Does the layer have an independent tile size?
			ChangeTileSize  = 0x08, ///< Can this layer's grid size be changed?
		};

		/// Create a new layer.
		/**
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
		 */
		Layer(int caps, int width, int height,
			int tileWidth, int tileHeight, ItemPtrVectorPtr& items)
			throw ();

		/// Destructor.
		virtual ~Layer()
			throw ();

		/// Get the capabilities of this tileset format.
		/**
		 * @return One or more of the Caps enum values (OR'd together.)
		 */
		virtual int getCaps()
			throw ();

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
		virtual void getLayerSize(int *x, int *y)
			throw ();

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
		virtual void setLayerSize(int x, int y)
			throw ();

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
		virtual void getTileSize(int *x, int *y)
			throw ();

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
		virtual void setTileSize(int x, int y)
			throw ();

		/// Get a list of all tiles in the layer.
		/**
		 * @return Vector of all tiles.  The tiles are in any order.
		 */
		virtual const ItemPtrVectorPtr getAllItems()
			throw ();

	protected:
		int caps;         ///< Map capabilities
		int width;        ///< Map width, in tiles
		int height;       ///< Map height, in tiles
		int tileWidth;    ///< Tile width, in pixels
		int tileHeight;   ///< Tile height, in pixels
		ItemPtrVectorPtr items; ///< Vector of all items in the layer
		TextPtrVector strings;  ///< Vector of all text elements in the layer
};

/// Item within the layer (a tile)
class Map2D::Layer::Item {
	public:
		unsigned int x;  ///< Item location in units of tiles
		unsigned int y;  ///< Item location in units of tiles

		// Since many maps use a code like this, we'll put it here to save each
		// map format from having to derive its own almost idential class.
		unsigned int code; ///< Format-specific tile code
};

/// A text element stored within the layer.
class Map2D::Layer::Text: virtual public Map2D::Layer::Item {
	public:
		std::string content;  ///< Actual content of the text element
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP2D_HPP_
