/**
 * @file  camoto/gamemaps/map2d.hpp
 * @brief 2D grid-based Map interface.
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
#include <camoto/gamegraphics/palette.hpp>
#include <camoto/gamemaps/map.hpp>

namespace camoto {
namespace gamemaps {

using gamegraphics::Point;

/// 2D grid-based Map.
class Map2D: virtual public Map
{
	public:
		class Layer;
		class Path;

		/// Capabilities this map supports.
		enum class Caps {
			Default           = 0,      ///< No caps set
			HasViewport       = 1 << 0, ///< Can use viewport() to get viewport size
			HasMapSize        = 1 << 1, ///< Can use mapSize() to get global map size
			SetMapSize        = 1 << 2, ///< Can use mapSize() to set global map size
			HasTileSize       = 1 << 3, ///< Can use tileSize() to get global tile size
			SetTileSize       = 1 << 4, ///< Can use tileSize() to set global tile size
			AddPaths          = 1 << 5, ///< Can add or remove paths, if unset then can only modify existing paths, if any
		};

		inline virtual ~Map2D() {};

		/// Get the capabilities of this map format.
		/**
		 * One or more of the Caps enum values (OR'd together.)
		 */
		virtual Caps caps() const = 0;

		/// Get the size of the in-game viewport.
		/**
		 * These dimensions indicate how much of the level can be seen by the player
		 * inside the game.  Given the age of most DOS games, it is typically how
		 * many tiles can be seen on a 320x200 display (minus the space used for the
		 * status bar).
		 *
		 * @return Viewport width and height, in pixels.
		 */
		virtual Point viewport() const = 0;

		/// Retrieve the size of the map.
		/**
		 * @pre caps() must include HasGlobalSize.
		 *
		 * @note If caps() does not include HasOwnTileSize then this value is in
		 * pixels instead (i.e. a tile size of 1x1 is assumed.)
		 *
		 * @return Map size as number of tiles.
		 */
		virtual Point mapSize() const = 0;

		/// Change the size of each cell in the layer.
		/**
		 * @pre caps() must include CanResize.
		 *
		 * @param newSize
		 *   New layer width and height, as number of tiles.
		 */
		virtual void mapSize(const Point& newSize) = 0;

		/// Retrieve the size of each cell in the layer's grid.
		/**
		 * @pre caps() includes HasGlobalTileSize.
		 *
		 * @return Tile size, in pixels.
		 */
		virtual Point tileSize() const = 0;

		/// Change the size of each cell in the layer.
		/**
		 * @pre caps() must include SetTileSize.
		 *
		 * @param newSize
		 *   New tile width and height, in pixels.
		 */
		virtual void tileSize(const Point& newSize) = 0;

		/// Get access to the map's layers.
		/**
		 * @return A vector containing a shared pointer to each layer.
		 */
		virtual std::vector<std::shared_ptr<Map2D::Layer>> layers() = 0;
		virtual std::vector<std::shared_ptr<const Map2D::Layer>> layers() const = 0;

		/// Get a list of paths in the level.
		/**
		 * A path is a series of points/vectors defining a travel route.  Unlike
		 * layers, paths are always expressed in pixels, irrespective of tile size.
		 *
		 * @note If the map format does not support paths, this function will return
		 *   an empty vector.  You can still check caps() for the presence of
		 *   AddPaths, and if present, it means the map supports paths but there are
		 *   just none at present.
		 *
		 * @return A vector of Path instances.  The paths in the vector can be
		 *   edited, but entries cannot be added or removed unless caps() includes
		 *   AddPaths.
		 */
		virtual std::vector<std::shared_ptr<Path>>& paths() = 0;

		struct Background {
			/// How the map background is drawn behind the level.
			enum class Attachment {
				/// No background image, display as transparent.
				NoBackground,
				/// Display img centered in the middle of the viewport.
				SingleImageCentred,
				/// Display img repeated to fill the largest map layer.
				SingleImageTiled,
				/// Background is the solid colour clr.
				SingleColour,
			};
			Attachment att;
			std::shared_ptr<gamegraphics::Image> img;
			gamegraphics::PaletteEntry clr;
		};

		/// Get the content to draw as the background behind all map layers.
		/**
		 * Since any empty/default map tiles will not be drawn, those areas will
		 * show through to this background image.
		 *
		 * @param tileset
		 *   List of tilesets, same as passed to Map2DLayer::imageFromCode().
		 *
		 * @param outImage
		 *   On return, contains the image to draw.  An empty pointer is returned
		 *   if the image is not required (e.g. no backdrop or single colour.)
		 *
		 * @param outColour
		 *   On return, if outAttach is SingleColour then this field contains the
		 *   colour to draw as the background.  If outAttach is another value then
		 *   this field is ignored.
		 *
		 * @return An ImageBackground value specifying how the map background should
		 *   be drawn.
		 */
		virtual Background background(const TilesetCollection& tileset)
			const = 0;
/*
		inline Map2D(const Attributes& attributes, const GraphicsFilenames& graphicsFilenames,
			unsigned int caps, unsigned int viewportWidth,
			unsigned int viewportHeight)
			:	Map(
					attributes,
					graphicsFilenames
				),
				caps(caps),
				viewportX(viewportWidth),
				viewportY(viewportHeight)
		{
		}
*/
};

inline Map2D::Caps operator| (Map2D::Caps a, Map2D::Caps b) {
	return static_cast<Map2D::Caps>(
		static_cast<unsigned int>(a) | static_cast<unsigned int>(b)
	);
}

inline bool operator& (Map2D::Caps a, Map2D::Caps b) {
	return
		static_cast<unsigned int>(a) & static_cast<unsigned int>(b)
	;
}

/// A map is made up of multiple layers.
class Map2D::Layer
{
	public:
		class Item;

		/// Capabilities this layer supports.
		enum class Caps {
			Default         = 0x00, ///< No caps set
			HasOwnSize      = 0x01, ///< Does the layer have an independent size?
			SetOwnSize      = 0x02, ///< Can just this layer be resized?
			HasOwnTileSize  = 0x04, ///< Does the layer have an independent tile size?
			SetOwnTileSize  = 0x08, ///< Can this layer's grid size be changed?
			HasPalette      = 0x10, ///< Palette is obtained from layer instead of tileset
			UseImageDims    = 0x20, ///< Draw each tile the size of the image itself, instead of the tile size
		};

		/// Get the layer's friendly name.
		/**
		 * This isn't from the map metadata, this is a name for a level editor to
		 * display, for example "Foreground" or "Background".
		 *
		 * @return A string containing a name suitable for display to the user.
		 */
		virtual std::string title() const = 0;

		/// Get the capabilities of this layer.
		/**
		 * @return One or more of the Caps enum values (OR'd together.)
		 */
		virtual Caps caps() const = 0;

		/// Retrieve the size of the layer.
		/**
		 * @pre caps() includes HasOwnSize.  Otherwise the map's size must be
		 *   used.
		 *
		 * @return Layer height and width, as number of tiles.
		 */
		virtual Point layerSize() const = 0;

		/// Change the size of each cell in the layer.
		/**
		 * @pre caps() must include CanResize.
		 *
		 * @param newSize
		 *   New layer width and height, as number of tiles.
		 */
		virtual void layerSize(const Point& newSize) = 0;

		/// Retrieve the size of each cell in the layer's grid.
		/**
		 * @pre caps() includes HasOwnTileSize.  If not, the map's tile size
		 *   must be used.
		 *
		 * @return Tile width and height, in pixels.
		 */
		virtual Point tileSize() const = 0;

		/// Change the size of each cell in the layer.
		/**
		 * @pre caps() must include SetTileSize.
		 *
		 * @param newSize
		 *   New tile width and height, in pixels.
		 */
		virtual void tileSize(const Point& newSize) = 0;

		/// Get a list of all tiles in the layer.
		/**
		 * @return Vector of all tiles.  The tiles are in any order.  The vector is
		 *   returned by reference, so elements can be added and removed to add or
		 *   remove tiles from the layer.  Make sure any potential additions are
		 *   allowed by tilePermittedAt() first.
		 */
		virtual std::vector<Item>& items() = 0;
		virtual std::vector<Item> items() const = 0;

		/// Return value from imageFromCode()
		struct ImageFromCodeInfo {
			/// Image types
			enum class ImageType {
				Blank,        ///< Don't display any image
				Supplied,     ///< Use the supplied image
				Unknown,      ///< Display the 'unknown tile' indicator
				HexDigit,     ///< Small character 0-9,A-F given in var 'digit'
				Interactive,  ///< Interactive item
				NumImageTypes ///< Must be last
			};
			/// Which image to display, and which other members are valid
			ImageType type;
			/// When type == HexDigit, values 0-15 for digits 0-9,A-F
			/// 0x10..0x1F = 0..F, 0x100-0x1FF = 00..FF, 0x10000-0x1FFFF = 0000..FFFF
			unsigned long digit;
			/// When type == Supplied, the image to draw
			std::shared_ptr<camoto::gamegraphics::Image> img;
		};

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
		 * @param outImgType
		 *   On return, an ImageType code indicating what image to display.
		 *
		 * @return If outImgType is ImageType::Supplied, this is a shared pointer
		 *   to a camoto::gamegraphics::Image instance.  If outImgType is anything
		 *   else, the return value is ignored.
		 */
		virtual ImageFromCodeInfo imageFromCode(
			const Map2D::Layer::Item& item, const TilesetCollection& tileset)
			const = 0;

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
		virtual bool tilePermittedAt(const Map2D::Layer::Item& item,
			unsigned int x, unsigned int y, unsigned int *maxCount) const = 0;

		/// Get the palette to use with this layer.
		/**
		 * Some tilesets don't have a palette, so in this case the palette to use
		 * can be supplied here.  Palettes applied to individual tiles will still
		 * override this.
		 *
		 * @pre caps() return value includes HasPalette.
		 *
		 * @return Shared pointer to a PaletteTable.
		 */
		virtual std::shared_ptr<const gamegraphics::Palette> palette(
			const TilesetCollection& tileset) const = 0;

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
		virtual std::vector<Item> availableItems() const = 0;
};

inline Map2D::Layer::Caps operator| (Map2D::Layer::Caps a, Map2D::Layer::Caps b) {
	return static_cast<Map2D::Layer::Caps>(
		static_cast<unsigned int>(a) | static_cast<unsigned int>(b)
	);
}

inline bool operator& (Map2D::Layer::Caps a, Map2D::Layer::Caps b) {
	return
		static_cast<unsigned int>(a) & static_cast<unsigned int>(b)
	;
}

/// Item within the layer (a tile)
class Map2D::Layer::Item
{
	public:
		enum class Type {
			Default   = 0x0000, ///< Default type, no extra fields
			Player    = 0x0001, ///< Set if player fields are valid
			Text      = 0x0002, ///< Set if text fields are valid
			Movement  = 0x0004, ///< Set if movement fields are valid
			Blocking  = 0x0008, ///< Set if blocking fields are valid
			Flags     = 0x0010, ///< Set if generalFlags field is valid
		};
		Type type; ///< Which fields are valid?

		// Default fields
		/// Item location, in units of tiles
		Point pos;

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
		enum class MovementFlags {
			DistanceLimit = 0x0001, ///< Set if dist* vars indicate movement limits
			SpeedLimit    = 0x0002, ///< Set if speedX and speedY are valid
		};
		MovementFlags movementFlags;       ///< One or more of MovementFlags

		/// Set movementDist* to this value to indicate movement in the
		/// specified direction, of an indeterminate nature.
		const static unsigned int DistIndeterminate = (unsigned int)-1;

		unsigned int movementDistLeft;    ///< How far left the item can go, in grid units
		unsigned int movementDistRight;   ///< How far right the item can go, in grid units
		unsigned int movementDistUp;      ///< How far up the item can go, in grid units
		unsigned int movementDistDown;    ///< How far down the item can go, in grid units
		unsigned int movementSpeedX;      ///< Horizontal speed, in milliseconds per pixel
		unsigned int movementSpeedY;      ///< Vertical speed, in milliseconds per pixel

		enum class BlockingFlags {
			BlockLeft     = 0x0001, ///< Prevent movement right, through the left edge
			BlockRight    = 0x0002, ///< Prevent movement left, through the right edge
			BlockTop      = 0x0004, ///< Prevent movement down through the top edge (can stand on)
			BlockBottom   = 0x0008, ///< Prevent movement up through the bottom edge
			JumpDown      = 0x0010, ///< Can down-jump to fall through
			Slant45       = 0x0020, ///< Slanted tile /, 45 degrees CCW from the horizontal
			Slant135      = 0x0040, ///< Slanted tile \, 135 degrees CCW from the horizontal
		};
		BlockingFlags blockingFlags;       ///< One or more of BlockingFlags

		enum class GeneralFlags {
			Interactive   = 0x0001, ///< This tile hosts an interactive item
		};
		GeneralFlags generalFlags;
};

inline Map2D::Layer::Item::Type operator| (Map2D::Layer::Item::Type a, Map2D::Layer::Item::Type b) {
	return static_cast<Map2D::Layer::Item::Type>(
		static_cast<unsigned int>(a) | static_cast<unsigned int>(b)
	);
}

inline Map2D::Layer::Item::Type operator|= (Map2D::Layer::Item::Type& a, const Map2D::Layer::Item::Type& b) {
	a = a | b;
	return a;
}

inline bool operator& (Map2D::Layer::Item::Type a, Map2D::Layer::Item::Type b) {
	return
		static_cast<unsigned int>(a) & static_cast<unsigned int>(b)
	;
}

inline Map2D::Layer::Item::MovementFlags operator| (Map2D::Layer::Item::MovementFlags a, Map2D::Layer::Item::MovementFlags b) {
	return static_cast<Map2D::Layer::Item::MovementFlags>(
		static_cast<unsigned int>(a) | static_cast<unsigned int>(b)
	);
}

inline bool operator& (Map2D::Layer::Item::MovementFlags a, Map2D::Layer::Item::MovementFlags b) {
	return
		static_cast<unsigned int>(a) & static_cast<unsigned int>(b)
	;
}

inline Map2D::Layer::Item::BlockingFlags operator| (Map2D::Layer::Item::BlockingFlags a, Map2D::Layer::Item::BlockingFlags b) {
	return static_cast<Map2D::Layer::Item::BlockingFlags>(
		static_cast<unsigned int>(a) | static_cast<unsigned int>(b)
	);
}

inline bool operator& (Map2D::Layer::Item::BlockingFlags a, Map2D::Layer::Item::BlockingFlags b) {
	return
		static_cast<unsigned int>(a) & static_cast<unsigned int>(b)
	;
}

/// Value to use for tilecodes that have not yet been set.
const unsigned int INVALID_TILECODE = (unsigned int)-1;

/// A path of points in a map.
class Map2D::Path
{
	public:
		/// Starting point(s) of this path.
		/**
		 * This vector contains one or more starting points for this path.  If
		 * multiple starting points are given, the same path is duplicated at
		 * each point (i.e. changing one path will also modify the others.)
		 */
		std::vector<Point> start;

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
		std::vector<Point> points;

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
