/**
 * @file   map2d.hpp
 * @brief  2D grid-based Map interface.
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

#ifndef _CAMOTO_GAMEMAPS_MAP2D_HPP_
#define _CAMOTO_GAMEMAPS_MAP2D_HPP_

#include <vector>
#include <map>
#include <camoto/gamegraphics/tileset.hpp>
#include <camoto/gamegraphics/palettetable.hpp>
#include <camoto/gamemaps/map.hpp>

namespace camoto {
namespace gamemaps {

/// 2D grid-based Map.
class Map2D: virtual public Map {

// caps: resize layer, resize map, set grid size
	public:

		/// Capabilities this layer supports.
		enum Caps {
			NoCaps            = 0x0000, ///< No caps set
			CanResize         = 0x0001, ///< Can the map be resized as a whole?
			ChangeTileSize    = 0x0002, ///< Can the map's grid size be changed?
			HasViewport       = 0x0004, ///< Does this map have a viewport?
			HasPaths          = 0x0008, ///< Does the map support paths?
			FixedPathCount    = 0x0010, ///< If set, paths cannot be added/removed, only edited
		};

		class Layer;
		/// Shared pointer to a layer instance.
		typedef boost::shared_ptr<Layer> LayerPtr;
		/// Vector of layers.
		typedef std::vector<LayerPtr> LayerPtrVector;

		class Path;
		/// Shared pointer to a path instance.
		typedef boost::shared_ptr<Path> PathPtr;
		/// Vector of paths.
		typedef std::vector<PathPtr> PathPtrVector;
		/// Shared pointer to a vector of paths.
		typedef boost::shared_ptr<PathPtrVector> PathPtrVectorPtr;

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
		Map2D(AttributePtrVectorPtr attributes, int caps,
			unsigned int viewportWidth, unsigned int viewportHeight,
			unsigned int width, unsigned int height, unsigned int tileWidth,
			unsigned int tileHeight, LayerPtrVector& layers, PathPtrVectorPtr paths)
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
		virtual void getViewport(unsigned int *x, unsigned int *y)
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
		virtual void getMapSize(unsigned int *x, unsigned int *y)
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
		virtual void setMapSize(unsigned int x, unsigned int y)
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
		virtual void getTileSize(unsigned int *x, unsigned int *y)
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
		virtual void setTileSize(unsigned int x, unsigned int y)
			throw ();

		/// Get the number of layers in the map.
		/**
		 * @return Number of layers.  All maps have at least one layer.
		 */
		virtual unsigned int getLayerCount()
			throw ();

		/// Get access to the given layer.
		/**
		 * @param index
		 *   Layer index.  Must be < getLayerCount().
		 *
		 * @return A shared pointer to the layer.
		 */
		virtual LayerPtr getLayer(unsigned int index)
			throw ();

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
		virtual PathPtrVectorPtr getPaths()
			throw ();

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
			HasPalette      = 0x10, ///< Palette is obtained from layer instead of tileset
		};

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
		 * @see Map2D::Map2D() for more details on how the layer and tile
		 *   dimensions are handled.
		 */
		Layer(const std::string& title, int caps, unsigned int width,
			unsigned int height, unsigned int tileWidth, unsigned int tileHeight,
			ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			throw ();

		/// Destructor.
		virtual ~Layer()
			throw ();

		/// Get the layer's friendly name.
		/**
		 * @return A string containing a name suitable for display to the user.
		 */
		virtual const std::string& getTitle()
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
		virtual void getLayerSize(unsigned int *x, unsigned int *y)
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
		virtual void setLayerSize(unsigned int x, unsigned int y)
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
		virtual void getTileSize(unsigned int *x, unsigned int *y)
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
		virtual void setTileSize(unsigned int x, unsigned int y)
			throw ();

		/// Get a list of all tiles in the layer.
		/**
		 * @return Vector of all tiles.  The tiles are in any order.
		 */
		virtual const ItemPtrVectorPtr getAllItems()
			throw ();

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
			gamegraphics::VC_TILESET& tileset)
			throw ();

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
			unsigned int y, unsigned int *maxCount)
			throw ();

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
			gamegraphics::VC_TILESET& tileset)
			throw ();

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
		virtual const ItemPtrVectorPtr getValidItemList()
			throw ();

	protected:
		std::string title;       ///< Layer's friendly name
		int caps;                ///< Map capabilities
		unsigned int width;      ///< Map width, in tiles
		unsigned int height;     ///< Map height, in tiles
		unsigned int tileWidth;  ///< Tile width, in pixels
		unsigned int tileHeight; ///< Tile height, in pixels
		ItemPtrVectorPtr items;  ///< Vector of all items in the layer
		TextPtrVector strings;   ///< Vector of all text elements in the layer
		gamegraphics::PaletteTablePtr pal; ///< Optional palette for layer
		ItemPtrVectorPtr validItems; ///< Vector of possible items in the layer
};

/// Item within the layer (a tile)
class Map2D::Layer::Item {
	public:
		virtual ~Item() throw ();

		unsigned int x;  ///< Item location in units of tiles
		unsigned int y;  ///< Item location in units of tiles

		// Since many maps use a code like this, we'll put it here to save each
		// map format from having to derive its own almost idential class.
		unsigned int code; ///< Format-specific tile code
};

/// Value to use for tilecodes that have not yet been set.
const unsigned int INVALID_TILECODE = (unsigned int)-1;

/// A text element stored within the layer.
class Map2D::Layer::Text: virtual public Map2D::Layer::Item {
	public:
		virtual ~Text() throw ();

		std::string content;  ///< Actual content of the text element
};

/// A path of points in a map.
class Map2D::Path {
	public:
		typedef std::pair<int, int> point;
		typedef std::vector<point> point_vector;

		/// Starting point(s) of this path.
		/**
		 * This vector contains one or more starting points for this path.  If
		 * multiple starting points are given, the same path is duplicated at
		 * each point (i.e. changing one path will also modify the others.)
		 */
		point_vector start;

		/// Are the start points fixed (true) or can they be changed (false)?
		bool fixed;

		/// The points in this path.
		/**
		 * This vector contains a number of points, which when joined by lines
		 * represent the path.  The coordinates are relative to (0,0), which is
		 * transposed to one of the starting points.  An implicit point is placed
		 * at (0,0) which will appear at the exact coordinates of the starting
		 * point.  If the path is a closed loop, the last point should NOT be (0,0)
		 * but rather forceClosed should be set to true.
		 */
		point_vector points;

		/// Maximum size of points vector.
		/**
		 * Some paths only have a fixed amount of space, so this value limits the
		 * the number of points that can exist in a path.  If it is set to zero
		 * then there is no specific limit.
		 */
		unsigned int maxPoints;

		/// Is this path required to be a closed loop?
		/**
		 * If this is set to true, the last point in the points vector will be
		 * immediately followed by the point from the start vector, i.e. the
		 * path is a closed loop (rather than a line with different points at
		 * the start and the end.)  This is intended to be a hint to a GUI to
		 * ensure the path is drawn as a closed loop and cannot be opened.
		 */
		bool forceClosed;
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP2D_HPP_
