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
#include <camoto/gamegraphics/tileset.hpp>
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
			HasPaths          = 0x10, ///< Does the map support paths?
			FixedPaths        = 0x20, ///< If set, paths cannot be added/removed, only edited
			HasViewport       = 0x40, ///< Does this map have a viewport?
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
		 *
		 * @param paths
		 *   Possibly empty vector of map paths.
		 */
		Map2D(AttributePtrVectorPtr attributes, int caps, int viewportWidth,
			int viewportHeight, int width, int height, int tileWidth, int tileHeight,
			LayerPtrVector& layers, PathPtrVectorPtr paths)
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
		virtual void getViewport(int *x, int *y)
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
		int viewportWidth;      ///< Width of viewport in pixels.
		int viewportHeight;     ///< Height of viewport in pixels.
		int width;              ///< Width of map as number of tiles.
		int height;             ///< Height of map as number of tiles.
		int tileWidth;          ///< Width of tiles in all layers, in pixels.
		int tileHeight;         ///< Height of tiles in all layers, in pixels.
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

		/// Function pointer to a callback for mapping tile codes to images.
		/**
		 * This function pointer is supplied in the constructor and is called to
		 * convert map codes into images.  The function is defined as:
		 *
		 * camoto::gamegraphics::ImagePtr convertMyCodes(unsigned int code) { ... }
		 *
		 * Where 'code' is the map code to convert, and the return value is the
		 * image to use for this map code.  A return value of a NULL pointer will
		 * result in some sort of unknown/question mark tile being used.
		 */
		typedef camoto::gamegraphics::ImagePtr (*FN_IMAGEFROMCODE)
			(unsigned int code, camoto::gamegraphics::VC_TILESET tileset);

		/// Function pointer to a callback for checking where tiles can be placed.
		/**
		 * This function pointer is supplied in the constructor and is called to
		 * check whether a tile can be placed at the given location.  The function
		 * is defined as:
		 *
		 * bool isTilePermittedAt(unsigned int code, unsigned int x, unsigned int y,
		 *   unsigned int *maxCodes) { ... }
		 *
		 * Where 'code' is the tile code, x and y are the proposed coordinates (in
		 * tile units as per this layer's current tile size) and maxCodes can be
		 * set to limit the number of times this code is used (zero means no limit.)
		 * Return false to prevent the tile from being placed at the given location,
		 * or true to allow it.  See also tilePermittedAt().
		 */
		typedef bool (*FN_TILEPERMITTEDAT) (unsigned int code, unsigned int x,
			unsigned int y, unsigned int *maxCodes);

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
		 * @param fnImageFromCode
		 *   Callback function to convert map codes into images.
		 *
		 * @param fnTilePermittedAt
		 *   Callback function to allow or prevent tiles from being placed at
		 *   certain locations or more than a limited number of times.  Can be
		 *   NULL if no restrictions are required.
		 */
		Layer(const std::string& title, int caps, int width, int height,
			int tileWidth, int tileHeight, ItemPtrVectorPtr& items,
			FN_IMAGEFROMCODE fnImageFromCode, FN_TILEPERMITTEDAT fnTilePermittedAt)
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
		 * @return Shared pointer to a camoto::gamegraphics::Image instance.
		 */
		virtual camoto::gamegraphics::ImagePtr imageFromCode(unsigned int code,
			camoto::gamegraphics::VC_TILESET tileset)
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

	protected:
		std::string title;      ///< Layer's friendly name
		int caps;               ///< Map capabilities
		int width;              ///< Map width, in tiles
		int height;             ///< Map height, in tiles
		int tileWidth;          ///< Tile width, in pixels
		int tileHeight;         ///< Tile height, in pixels
		ItemPtrVectorPtr items; ///< Vector of all items in the layer
		TextPtrVector strings;  ///< Vector of all text elements in the layer
		FN_IMAGEFROMCODE fnImageFromCode;      ///< Callback for imageFromCode()
		FN_TILEPERMITTEDAT fnTilePermittedAt;  ///< Callback for tilePermittedAt()
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

		point_vector points;
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP2D_HPP_
