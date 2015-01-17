/**
 * @file   map2d.hpp
 * @brief  2D grid-based Map interface.
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
class Map2D: virtual public Map
{
	public:
		/// Capabilities this map supports.
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

		/// Get the capabilities of this map format.
		/**
		 * @return One or more of the Caps enum values (OR'd together.)
		 */
		virtual int getCaps() const = 0;

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
		virtual void getViewport(unsigned int *x, unsigned int *y) const = 0;

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
		virtual void getMapSize(unsigned int *x, unsigned int *y) const = 0;

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
		virtual void setMapSize(unsigned int x, unsigned int y) = 0;

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
		virtual void getTileSize(unsigned int *x, unsigned int *y) const = 0;

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
		virtual void setTileSize(unsigned int x, unsigned int y) = 0;

		/// Get the number of layers in the map.
		/**
		 * @return Number of layers.  All maps have at least one layer.
		 */
		virtual unsigned int getLayerCount() const = 0;

		/// Get access to the given layer.
		/**
		 * @param index
		 *   Layer index.  Must be < getLayerCount().
		 *
		 * @return A shared pointer to the layer.
		 */
		virtual LayerPtr getLayer(unsigned int index) = 0;

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
		virtual PathPtrVectorPtr getPaths() = 0;
};

/// Shared pointer to an MapType.
typedef boost::shared_ptr<Map2D> Map2DPtr;

/// A map is made up of multiple layers.
class Map2D::Layer
{
	public:
		class Item;
		/// Shared pointer to an item.  This is used to avoid copying items around
		/// which can be problematic when they instances of descendent classes.
		typedef boost::shared_ptr<Item> ItemPtr;
		/// Vector of items.
		typedef std::vector<ItemPtr> ItemPtrVector;
		/// Shared pointer to a vector of items.
		typedef boost::shared_ptr<ItemPtrVector> ItemPtrVectorPtr;

		/// Capabilities this layer supports.
		enum Caps {
			NoCaps          = 0x00, ///< No caps set
			HasOwnSize      = 0x01, ///< Does the layer have an independent size?
			CanResize       = 0x02, ///< Can just this layer be resized?
			HasOwnTileSize  = 0x04, ///< Does the layer have an independent tile size?
			ChangeTileSize  = 0x08, ///< Can this layer's grid size be changed?
			HasPalette      = 0x10, ///< Palette is obtained from layer instead of tileset
			UseImageDims    = 0x20, ///< Draw each tile the size of the image itself, instead of the tile size
		};

		/// Return values from imageFromCode()
		enum ImageType {
			Supplied = 0, ///< Use the supplied image
			Blank = 1,    ///< Don't display any image
			Unknown = 2,  ///< Display the 'unknown tile' indicator
			Digit0,       ///< Small character '0'
			Digit1,       ///< Small character '1'
			Digit2,       ///< Small character '2'
			Digit3,       ///< Small character '3'
			Digit4,       ///< Small character '4'
			Digit5,       ///< Small character '5'
			Digit6,       ///< Small character '6'
			Digit7,       ///< Small character '7'
			Digit8,       ///< Small character '8'
			Digit9,       ///< Small character '9'
			DigitA,       ///< Small character 'A'
			DigitB,       ///< Small character 'B'
			DigitC,       ///< Small character 'C'
			DigitD,       ///< Small character 'D'
			DigitE,       ///< Small character 'E'
			DigitF,       ///< Small character 'F'
			Interactive,  ///< Interactive item
			NumImageTypes ///< Must be last
		};

		/// Get the layer's friendly name.
		/**
		 * @return A string containing a name suitable for display to the user.
		 */
		virtual const std::string& getTitle() const = 0;

		/// Get the capabilities of this layer.
		/**
		 * @return One or more of the Caps enum values (OR'd together.)
		 */
		virtual int getCaps() const = 0;

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
		virtual void getLayerSize(unsigned int *x, unsigned int *y) const = 0;

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
		virtual void setLayerSize(unsigned int x, unsigned int y) = 0;

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
		virtual void getTileSize(unsigned int *x, unsigned int *y) const = 0;

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
		virtual void setTileSize(unsigned int x, unsigned int y) = 0;

		/// Get a list of all tiles in the layer.
		/**
		 * @return Vector of all tiles.  The tiles are in any order.
		 */
		virtual ItemPtrVectorPtr getAllItems() = 0;

		/// Convert a map code into an image.
		/**
		 * @param item
		 *   Pointer to a Map2D::Layer::Item obtained from getAllItems().
		 *
		 * @param tileset
		 *   Tileset instances used to obtain the Image to return.
		 *   Which tilesets to actually pass in is beyond the scope of this
		 *   library, and must be obtained by some caller defined method.
		 *   Camoto Studio reads this information from XML files distributed
		 *   with the application, for example.
		 *
		 * @param out
		 *   If the return value is ImageType::Supplied, this is a shared pointer
		 *   to a camoto::gamegraphics::Image instance.  If the return value is
		 *   anything else, this parameter is ignored.
		 *
		 * @return An ImageType code indicating what image to display.
		 */
		virtual ImageType imageFromCode(
			const Map2D::Layer::ItemPtr& item, const TilesetCollectionPtr& tileset,
			camoto::gamegraphics::ImagePtr *out) const = 0;

		/// Is the given tile permitted at the specified location?
		/**
		 * @param item
		 *   Map2D::Layer::Item obtained from getAllItems().
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
		virtual bool tilePermittedAt(const Map2D::Layer::ItemPtr& item,
			unsigned int x, unsigned int y, unsigned int *maxCount) const = 0;

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
			const TilesetCollectionPtr& tileset) const = 0;

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
		virtual const ItemPtrVectorPtr getValidItemList() const = 0;
};

/// Item within the layer (a tile)
class Map2D::Layer::Item
{
	public:
		enum Type {
			Default   = 0x0000, ///< Default type, no extra fields
			Player    = 0x0001, ///< Set if player fields are valid
			Text      = 0x0002, ///< Set if text fields are valid
			Movement  = 0x0004, ///< Set if movement fields are valid
			Blocking  = 0x0008, ///< Set if blocking fields are valid
			Flags     = 0x0010, ///< Set if generalFlags field is valid
		};

		unsigned int type; ///< Which fields are valid?

		// Default fields
		unsigned int x;  ///< Item location in units of tiles
		unsigned int y;  ///< Item location in units of tiles

		// Since many maps use a code like this, we'll put it here to save each
		// map format from having to derive its own almost identical class.
		unsigned int code; ///< Format-specific tile code

		/// Player type: 0 for main player, 1 for second player, etc.
		unsigned int playerNumber;
		/// Player type: true to face left, false to face right
		bool playerFacingLeft;

		/// Text type: index of font to use (0 reserved for VGA 8x8)
		unsigned int textFont;
		/// Text type: actual content of the text element
		std::string textContent;

		/// These flags control which movement fields are valid and able to be modified
		enum MovementFlags {
			DistanceLimit = 0x0001, ///< Set if dist* vars indicate movement limits
			SpeedLimit    = 0x0002, ///< Set if speedX and speedY are valid
		};
		unsigned int movementFlags;       ///< One or more of MovementFlags

		/// Set movementDist* to this value to indicate movement in the
		/// specified direction, of an indeterminate nature.
		const static unsigned int DistIndeterminate = (unsigned int)-1;

		unsigned int movementDistLeft;    ///< How far left the item can go, in grid units
		unsigned int movementDistRight;   ///< How far right the item can go, in grid units
		unsigned int movementDistUp;      ///< How far up the item can go, in grid units
		unsigned int movementDistDown;    ///< How far down the item can go, in grid units
		unsigned int movementSpeedX;      ///< Horizontal speed, in milliseconds per pixel
		unsigned int movementSpeedY;      ///< Vertical speed, in milliseconds per pixel

		enum BlockingFlags {
			BlockLeft     = 0x0001, ///< Prevent movement right, through the left edge
			BlockRight    = 0x0002, ///< Prevent movement left, through the right edge
			BlockTop      = 0x0004, ///< Prevent movement down through the top edge (can stand on)
			BlockBottom   = 0x0008, ///< Prevent movement up through the bottom edge
			JumpDown      = 0x0010, ///< Can down-jump to fall through
			Slant45       = 0x0020, ///< Slanted tile /, 45 degrees CCW from the horizontal
			Slant135      = 0x0040, ///< Slanted tile \, 135 degrees CCW from the horizontal
		};
		unsigned int blockingFlags;       ///< One or more of MovementFlags

		enum GeneralFlags {
			Interactive   = 0x0001, ///< This tile hosts an interactive item
		};
		GeneralFlags generalFlags;
};

/// Value to use for tilecodes that have not yet been set.
const unsigned int INVALID_TILECODE = (unsigned int)-1;

/// A path of points in a map.
class Map2D::Path
{
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
