/**
 * @file  fmt-map-wordresc.cpp
 * @brief MapType and Map2D implementation for Word Rescue levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Word_Rescue
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

#include <cassert>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp> // make_unique
#include "map-core.hpp"
#include "map2d-core.hpp"
#include "fmt-map-wordresc.hpp"

/// Width of tiles in background layer
#define WR_BGTILE_WIDTH           16
/// Height of tiles in background layer
#define WR_BGTILE_HEIGHT          16

/// Width of tiles in attribute layer
#define WR_ATTILE_WIDTH            8
/// Height of tiles in attribute layer
#define WR_ATTILE_HEIGHT           8

/// Map code to write for locations with no tile set.
#define WR_DEFAULT_BGTILE       0xFF

/// Map code to write for locations with no tile set.
#define WR_DEFAULT_ATTILE       0x20

/// This is the largest valid tile code in the background layer.
#define WR_MAX_VALID_TILECODE    240

/// Height of the door image, in pixels (to align it with the floor)
#define DOOR_HEIGHT               40

// Internal codes for various items
#define WR_CODE_GRUZZLE  1
#define WR_CODE_SLIME    2
#define WR_CODE_BOOK     3
#define WR_CODE_DRIP     4
#define WR_CODE_ANIM     5
#define WR_CODE_FG       6
#define WR_CODE_LETTER   7
#define WR_CODE_LETTER1  7 // same as WR_CODE_LETTER
#define WR_CODE_LETTER2  8
#define WR_CODE_LETTER3  9
#define WR_CODE_LETTER4  10
#define WR_CODE_LETTER5  11
#define WR_CODE_LETTER6  12
#define WR_CODE_LETTER7  13

#define WR_CODE_ENTRANCE 0x1001
#define WR_CODE_EXIT     0x1002

/// Fixed number of letters in each map (to spell a word)
#define WR_NUM_LETTERS   7

// Values used when writing items (also in isinstance)
#define INDEX_GRUZZLE 0
#define INDEX_DRIP    1
#define INDEX_SLIME   2
#define INDEX_BOOK    3
#define INDEX_LETTER  4
#define INDEX_ANIM    5
#define INDEX_FG      6
#define INDEX_SIZE    7

// Indices into attributes array
#define ATTR_BGCOLOUR 0
#define ATTR_TILESET  1
#define ATTR_BACKDROP 2

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

/// Write the given data to the stream, RLE encoded
int rleWrite(stream::output& output, const std::vector<uint8_t>& data)
{
	int lenWritten = 0;

	// RLE encode the data
	uint8_t lastCount = 0;
	uint8_t lastCode = data[0];
	for (auto& d : data) {
		if (d == lastCode) {
			if (lastCount == 0xFF) {
				output
					<< u8(lastCount)
					<< u8(lastCode)
				;
				lenWritten += 2;
				lastCount = 1;
			} else {
				lastCount++;
			}
		} else {
			output
				<< u8(lastCount)
				<< u8(lastCode)
			;
			lenWritten += 2;
			lastCode = d;
			lastCount = 1;
		}
	}
	// Write out the last tile
	if (lastCount > 0) {
		output
			<< u8(lastCount)
			<< u8(lastCode)
		;
		lenWritten += 2;
	}

	return lenWritten;
}


class Layer_WR_Background: public Map2DCore::LayerCore
{
	public:
		Layer_WR_Background(stream::inout& content, const Point& mapSize)
		{
			this->v_allItems.reserve(mapSize.x * mapSize.y);
			for (int i = 0; i < mapSize.x * mapSize.y; ) {
				uint8_t num, code;
				content >> u8(num) >> u8(code);

				if (code == WR_DEFAULT_BGTILE) {
					i += num;
				} else {
					while (num-- > 0) {
						this->v_allItems.emplace_back();
						auto& t = this->v_allItems.back();

						t.type = Item::Type::Default;
						t.pos.x = i % mapSize.x;
						t.pos.y = i / mapSize.x;
						t.code = code;
						i++;
					}
				}
			}
		}

		virtual ~Layer_WR_Background()
		{
		}

		void flush(stream::inout& content, const Point& mapSize)
		{
			std::vector<uint8_t> tiles(mapSize.x * mapSize.y, WR_DEFAULT_BGTILE);
			for (auto& t : this->v_allItems) {
				if ((t.pos.x > mapSize.x) || (t.pos.y > mapSize.y)) {
					throw stream::error(createString("Layer has tiles outside map "
							"boundary at (" << t.pos.x << "," << t.pos.y << ")"));
				}
				tiles[t.pos.y * mapSize.x + t.pos.x] = t.code;
			}
			rleWrite(content, tiles);
		}

		virtual std::string title() const
		{
			return "Background";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			auto t = tileset.find(ImagePurpose::BackgroundTileset1);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			if (item.code >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[item.code]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i <= WR_MAX_VALID_TILECODE; i++) {
				if (i == WR_DEFAULT_BGTILE) continue;

				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}

	private:
		std::unique_ptr<stream::inout> content;
};

class Layer_WR_Object_Small: public Map2DCore::LayerCore
{
	public:
		Layer_WR_Object_Small(stream::inout& content, const Point& ptStart,
			const Point& ptEnd)
		{
			uint16_t gruzzleCount;
			content >> u16le(gruzzleCount);
			for (unsigned int i = 0; i < gruzzleCount; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				content
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
				;
				t.code = WR_CODE_GRUZZLE;
			}

			uint16_t dripCount;
			content >> u16le(dripCount);

			for (unsigned int i = 0; i < dripCount; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Movement;
				t.movementFlags = Item::MovementFlags::DistanceLimit
					| Item::MovementFlags::SpeedLimit;
				t.movementDistLeft = 0;
				t.movementDistRight = 0;
				t.movementDistUp = 0;
				t.movementDistDown = Item::DistIndeterminate;
				t.movementSpeedX = 0;
				content
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
					>> u16le(t.movementSpeedY)
				;
				/// @todo Convert t.movementSpeedY from WR units to milliseconds-per-pixel
				t.code = WR_CODE_DRIP;
			}

			// Add the map entrance and exit as special items
			{
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Player;
				t.pos = ptStart;
				t.playerNumber = 0;
				t.code = WR_CODE_ENTRANCE;
			}
			{
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				t.pos = ptEnd;
				t.code = WR_CODE_EXIT;
			}
		}

		virtual ~Layer_WR_Object_Small()
		{
		}

		void flush()
		{
		}

		virtual std::string title() const
		{
			return "Small objects";
		}

		virtual Caps caps() const
		{
			return Caps::HasOwnTileSize | Caps::UseImageDims;
		}

		virtual Point tileSize() const
		{
			return {WR_ATTILE_WIDTH, WR_ATTILE_HEIGHT};
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			ImagePurpose purpose;
			unsigned int index;
			switch (item.code) {
				case WR_CODE_GRUZZLE:  purpose = ImagePurpose::SpriteTileset1;     index = 15; break;
				case WR_CODE_SLIME:    purpose = ImagePurpose::BackgroundTileset1; index = 238; break;
				case WR_CODE_BOOK:     purpose = ImagePurpose::BackgroundTileset1; index = 239; break;
				case WR_CODE_DRIP:     purpose = ImagePurpose::BackgroundTileset1; index = 238; break;
				case WR_CODE_LETTER1:
				case WR_CODE_LETTER2:
				case WR_CODE_LETTER3:
				case WR_CODE_LETTER4:
				case WR_CODE_LETTER5:
				case WR_CODE_LETTER6:
				case WR_CODE_LETTER7:
					purpose = ImagePurpose::ForegroundTileset1;
					index = item.code - WR_CODE_LETTER;
					break;
				case WR_CODE_ENTRANCE: purpose = ImagePurpose::SpriteTileset1; index = 1; break;
				case WR_CODE_EXIT:     purpose = ImagePurpose::SpriteTileset1; index = 3; break;

				case WR_CODE_ANIM: // fall through (no image)
				case WR_CODE_FG:
				default:
					ret.type = ImageFromCodeInfo::ImageType::Unknown;
					return ret;
			}

			auto t = tileset.find(purpose);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			if (index >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[index]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			{
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = WR_CODE_GRUZZLE;
			}
			{
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = WR_CODE_ENTRANCE;
			}
			{
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = WR_CODE_EXIT;
			}
			{
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Movement;
				t.pos = {0, 0};
				t.code = WR_CODE_DRIP;
				t.movementFlags = Item::MovementFlags::DistanceLimit
					| Item::MovementFlags::SpeedLimit;
				t.movementDistLeft = 0;
				t.movementDistRight = 0;
				t.movementDistUp = 0;
				t.movementDistDown = Item::DistIndeterminate;
				t.movementSpeedX = 0;
				t.movementSpeedY = 0x44;
			}
			return validItems;
		}

		virtual bool tilePermittedAt(const Item& item,
			const Point& pos, unsigned int *maxCount) const
		{
			if ((item.code == WR_CODE_ENTRANCE) || (item.code == WR_CODE_EXIT)) {
				*maxCount = 1; // only one level entrance/exit permitted
			} else {
				*maxCount = 0; // unlimited
			}
			return true; // anything can be placed anywhere
		}
};

class Layer_WR_Object_Large: public Map2DCore::LayerCore
{
	public:
		Layer_WR_Object_Large(stream::inout& content)
		{
			uint16_t slimeCount;
			content >> u16le(slimeCount);
			for (unsigned int i = 0; i < slimeCount; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				content
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
				;
				t.code = WR_CODE_SLIME;
			}

			uint16_t bookCount;
			content >> u16le(bookCount);
			for (unsigned int i = 0; i < bookCount; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				content
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
				;
				t.code = WR_CODE_BOOK;
			}

			for (unsigned int i = 0; i < WR_NUM_LETTERS; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				content
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
				;
				t.code = WR_CODE_LETTER + i;
			}

			uint16_t animCount;
			content >> u16le(animCount);
			for (unsigned int i = 0; i < animCount; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				content
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
				;
				t.code = WR_CODE_ANIM;
			}

			uint16_t fgCount;
			content >> u16le(fgCount);
			for (unsigned int i = 0; i < fgCount; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				content
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
				;
				t.code = WR_CODE_FG;
			}
		}

		virtual ~Layer_WR_Object_Large()
		{
		}

		void flush()
		{
		}

		virtual std::string title() const
		{
			return "Large objects";
		}

		virtual Caps caps() const
		{
			return Caps::UseImageDims;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			ImagePurpose purpose;
			unsigned int index;
			switch (item.code) {
				case WR_CODE_GRUZZLE:  purpose = ImagePurpose::SpriteTileset1;     index = 15; break;
				case WR_CODE_SLIME:    purpose = ImagePurpose::BackgroundTileset1; index = 238; break;
				case WR_CODE_BOOK:     purpose = ImagePurpose::BackgroundTileset1; index = 239; break;
				case WR_CODE_DRIP:     purpose = ImagePurpose::BackgroundTileset1; index = 238; break;
				case WR_CODE_LETTER1:
				case WR_CODE_LETTER2:
				case WR_CODE_LETTER3:
				case WR_CODE_LETTER4:
				case WR_CODE_LETTER5:
				case WR_CODE_LETTER6:
				case WR_CODE_LETTER7:
					purpose = ImagePurpose::ForegroundTileset1;
					index = item.code - WR_CODE_LETTER;
					break;
				case WR_CODE_ENTRANCE: purpose = ImagePurpose::SpriteTileset1; index = 1; break;
				case WR_CODE_EXIT:     purpose = ImagePurpose::SpriteTileset1; index = 3; break;

				case WR_CODE_ANIM: // fall through (no image)
				case WR_CODE_FG:
				default:
					ret.type = ImageFromCodeInfo::ImageType::Unknown;
					return ret;
			}

			auto t = tileset.find(purpose);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			if (index >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[index]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = WR_CODE_SLIME; i <= WR_CODE_LETTER7; i++) {
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}

		virtual bool tilePermittedAt(const Item& item,
			const Point& pos, unsigned int *maxCount) const
		{
			if ((item.code == WR_CODE_ENTRANCE) || (item.code == WR_CODE_EXIT)) {
				*maxCount = 1; // only one level entrance/exit permitted
			} else {
				*maxCount = 0; // unlimited
			}
			return true; // anything can be placed anywhere
		}
};

class Layer_WR_Attribute: public Map2DCore::LayerCore
{
	public:
		Layer_WR_Attribute(stream::inout& content, const Point& mapSize)
		{
			uint16_t atWidth = mapSize.x * 2;
			uint16_t atHeight = mapSize.y * 2;
			this->v_allItems.reserve(atWidth * atHeight);
			for (int i = 0; i < atWidth * atHeight; ) {
				uint8_t num, code;
				try {
					content >> u8(num) >> u8(code);
				} catch (const stream::incomplete_read&) {
					// Some level files seem to be truncated (maybe for efficiency)
					break;
				}
				if (code == WR_DEFAULT_ATTILE) {
					i += num;
				} else {
					while (num-- > 0) {
						this->v_allItems.emplace_back();
						auto& t = this->v_allItems.back();

						t.pos.x = i % atWidth + 1;
						t.pos.y = i / atWidth;
						t.code = code;
						switch (code) {
							case 0x73:
								t.type = Item::Type::Blocking;
								t.blockingFlags =
									Item::BlockingFlags::BlockLeft
									| Item::BlockingFlags::BlockRight
									| Item::BlockingFlags::BlockTop
									| Item::BlockingFlags::BlockBottom
								;
								break;
							case 0x74:
								t.type = Item::Type::Blocking;
								t.blockingFlags =
									Item::BlockingFlags::BlockTop
									| Item::BlockingFlags::JumpDown
								;
								break;
							default:
								t.type = Item::Type::Default;
								break;
						}
						i++;
					}
				}
			}
		}

		virtual ~Layer_WR_Attribute()
		{
		}

		void flush(stream::inout& content, const Point& mapSize)
		{
			unsigned long lenAttr = mapSize.x * mapSize.y * 4;
			std::vector<uint8_t> attr(lenAttr, WR_DEFAULT_ATTILE);
			for (auto& t : this->v_allItems) {
				uint16_t code = t.code;

				if (
					(t.type & Item::Type::Blocking)
					&& (t.blockingFlags != Item::BlockingFlags::Default)
				) {
					if (t.blockingFlags & Item::BlockingFlags::JumpDown) {
						code = 0x74;
					} else {
						// Probably all Block* flags set
						code = 0x73;
					}
				}

				unsigned int xpos = t.pos.x;
				if (xpos < 1) continue; // skip first column, just in case
				xpos--;
				if ((xpos > mapSize.x * 2) || (t.pos.y > mapSize.y * 2)) {
					throw stream::error(createString("Layer has tiles outside map "
							"boundary at (" << xpos << "," << t.pos.y << ")"));
				}
				attr[t.pos.y * mapSize.x * 2 + xpos] = code;
			}
			rleWrite(content, attr);
		}

		virtual std::string title() const
		{
			return "Attributes";
		}

		virtual Caps caps() const
		{
			return Caps::HasOwnTileSize;
		}

		virtual Point tileSize() const
		{
			return {WR_ATTILE_WIDTH, WR_ATTILE_HEIGHT};
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			ImagePurpose purpose;
			unsigned int index;
			switch (item.code) {
				case 0x0000: purpose = ImagePurpose::SpriteTileset1; index = 0; break; // first question mark box
				case 0x0001: purpose = ImagePurpose::SpriteTileset1; index = 0; break;
				case 0x0002: purpose = ImagePurpose::SpriteTileset1; index = 0; break;
				case 0x0003: purpose = ImagePurpose::SpriteTileset1; index = 0; break;
				case 0x0004: purpose = ImagePurpose::SpriteTileset1; index = 0; break;
				case 0x0005: purpose = ImagePurpose::SpriteTileset1; index = 0; break;
				case 0x0006: purpose = ImagePurpose::SpriteTileset1; index = 0; break; // last question mark box

				// These ones have other flags on the tile itself, so arrows or similar are drawn
				case 0x0073: // solid
				case 0x0074: // jump up through/climb
					ret.type = ImageFromCodeInfo::ImageType::Blank;
					return ret;

				case 0x00FD: // what is this? end of layer flag?
					// fall through
				default:
					ret.type = ImageFromCodeInfo::ImageType::Unknown;
					return ret;
			}

			auto t = tileset.find(purpose);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			if (index >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[index]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			{
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Blocking;
				t.pos = {0, 0};
				t.code = 0x0073;
				t.blockingFlags =
					Item::BlockingFlags::BlockLeft
					| Item::BlockingFlags::BlockRight
					| Item::BlockingFlags::BlockTop
					| Item::BlockingFlags::BlockBottom
				;
			}
			{
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Blocking;
				t.pos = {0, 0};
				t.code = 0x0074;
				t.blockingFlags =
					Item::BlockingFlags::BlockTop
					| Item::BlockingFlags::JumpDown
				;
			}

			// Question-mark boxes
			for (unsigned int i = 0; i < 7; i++) {
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}

			// Unknown (see tile mapping code)
			{
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = 0x00FD;
			}
			return validItems;
		}

		virtual bool tilePermittedAt(const Item& item,
			const Point& pos, unsigned int *maxCount) const
		{
			if (pos.x == 0) return false; // can't place tiles in this column
			return true; // otherwise unrestricted
		}

	private:
		std::unique_ptr<stream::inout> content;
};

class Map_WordRescue: public MapCore, public Map2DCore
{
	public:
		Map_WordRescue(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			this->content->seekg(0, stream::start);

			uint16_t bgColour; // EGA 0-15
			uint16_t tileset; // 3 == suburban, 2 == medieval (backX.wr)
			uint16_t backdrop; // dropX.wr, 0 == none
			Point ptStart, ptEnd;
			*this->content
				>> u16le(this->ptMapSize.x)
				>> u16le(this->ptMapSize.y)
				>> u16le(bgColour)
				>> u16le(tileset)
				>> u16le(backdrop)
				>> u16le(ptStart.x)
				>> u16le(ptStart.y)
				>> u16le(ptEnd.x)
				>> u16le(ptEnd.y)
			;

			{
				assert(this->v_attributes.size() == ATTR_BGCOLOUR); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Enum;
				a.name = "Background colour";
				a.desc = "Colour to draw where there are no tiles.  Only used if "
					"backdrop is not set.";
				a.enumValue = bgColour;
				a.enumValueNames.push_back("EGA 0 - Black");
				a.enumValueNames.push_back("EGA 1 - Dark blue");
				a.enumValueNames.push_back("EGA 2 - Dark green");
				a.enumValueNames.push_back("EGA 3 - Dark cyan");
				a.enumValueNames.push_back("EGA 4 - Dark red");
				a.enumValueNames.push_back("EGA 5 - Dark magenta");
				a.enumValueNames.push_back("EGA 6 - Brown");
				a.enumValueNames.push_back("EGA 7 - Light grey");
				a.enumValueNames.push_back("EGA 8 - Dark grey");
				a.enumValueNames.push_back("EGA 9 - Light blue");
				a.enumValueNames.push_back("EGA 10 - Light green");
				a.enumValueNames.push_back("EGA 11 - Light cyan");
				a.enumValueNames.push_back("EGA 12 - Light red");
				a.enumValueNames.push_back("EGA 13 - Light magenta");
				a.enumValueNames.push_back("EGA 14 - Yellow");
				a.enumValueNames.push_back("EGA 15 - White");
			};
			{
				assert(this->v_attributes.size() == ATTR_TILESET); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Enum;
				a.name = "Tileset";
				a.desc = "Tileset to use for this map.";
				if (tileset > 0) tileset--; // just in case it *is* ever zero
				a.enumValue = tileset;
				a.enumValueNames.push_back("Desert");
				a.enumValueNames.push_back("Castle");
				a.enumValueNames.push_back("Suburban");
				a.enumValueNames.push_back("Spooky (episode 3 only)");
				a.enumValueNames.push_back("Industrial");
				a.enumValueNames.push_back("Custom (back6.wr)");
				a.enumValueNames.push_back("Custom (back7.wr)");
				a.enumValueNames.push_back("Custom (back8.wr)");
			};
			{
				assert(this->v_attributes.size() == ATTR_BACKDROP); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Enum;
				a.name = "Backdrop";
				a.desc = "Image to show behind map (overrides background colour.)";
				a.enumValue = backdrop;
				a.enumValueNames.push_back("None (use background colour)");
				a.enumValueNames.push_back("Custom (drop1.wr)");
				a.enumValueNames.push_back("Cave (episodes 2-3 only)");
				a.enumValueNames.push_back("Desert");
				a.enumValueNames.push_back("Mountain");
				a.enumValueNames.push_back("Custom (drop5.wr)");
				a.enumValueNames.push_back("Custom (drop6.wr)");
				a.enumValueNames.push_back("Custom (drop7.wr)");
			};

			// Read data for each layer
			auto layerOS = std::make_shared<Layer_WR_Object_Small>(*this->content,
				ptStart, ptEnd);
			auto layerOL = std::make_shared<Layer_WR_Object_Large>(*this->content);
			auto layerBG = std::make_shared<Layer_WR_Background>(*this->content,
				this->ptMapSize);
			auto layerAT = std::make_shared<Layer_WR_Attribute>(*this->content,
				this->ptMapSize);

			this->v_layers.push_back(layerBG);
			this->v_layers.push_back(layerOS);
			this->v_layers.push_back(layerOL);
			this->v_layers.push_back(layerAT);
		}

		virtual ~Map_WordRescue()
		{
		}

		virtual void flush()
		{
			assert(this->v_layers.size() == 4);
			assert(this->v_attributes.size() == 3);

			this->content->seekp(0, stream::start);

			auto& attrBG = this->v_attributes[ATTR_BGCOLOUR];
			assert(attrBG.type == Attribute::Type::Enum);
			uint16_t bgColour = attrBG.enumValue;

			auto& attrTileset = this->v_attributes[ATTR_TILESET];
			assert(attrTileset.type == Attribute::Type::Enum);
			uint16_t tileset = attrTileset.enumValue + 1;

			auto& attrBackdrop = this->v_attributes[ATTR_BACKDROP];
			assert(attrBackdrop.type == Attribute::Type::Enum);
			uint16_t backdrop = attrBackdrop.enumValue;

			std::vector<Point> itemLocations[INDEX_SIZE];
			struct DripData {
				Point pos;
				unsigned int dripFreq;
			};
			std::vector<DripData> drips;

			// Prefill the letter vector with the fixed number of letters
			for (unsigned int i = 0; i < WR_NUM_LETTERS; i++) {
				itemLocations[INDEX_LETTER].push_back(Point{0, 0});
			}

			Point ptStart{0, 0}, ptEnd{0, 0};

			auto layerOS = this->v_layers[1];
			for (auto& t : layerOS->items()) {
				switch (t.code & 0xFFFF) {
					case WR_CODE_GRUZZLE: itemLocations[INDEX_GRUZZLE].push_back(t.pos); break;
					case WR_CODE_DRIP: {
						DripData dd;
						dd.pos = t.pos;
						dd.dripFreq = t.movementSpeedY;
						/// @todo Convert t.movementSpeedY from milliseconds-per-pixel back to WR units
						drips.push_back(dd);
						break;
					}
					case WR_CODE_ENTRANCE:
						ptStart = t.pos;
						break;
					case WR_CODE_EXIT:
						ptEnd = t.pos;
						break;
				}
			}

			auto layerOL = this->v_layers[2];
			for (auto& t : layerOL->items()) {
				switch (t.code) {
					case WR_CODE_SLIME:   itemLocations[INDEX_SLIME].push_back(t.pos); break;
					case WR_CODE_BOOK:    itemLocations[INDEX_BOOK].push_back(t.pos); break;
					case WR_CODE_LETTER1: itemLocations[INDEX_LETTER][0] = t.pos; break;
					case WR_CODE_LETTER2: itemLocations[INDEX_LETTER][1] = t.pos; break;
					case WR_CODE_LETTER3: itemLocations[INDEX_LETTER][2] = t.pos; break;
					case WR_CODE_LETTER4: itemLocations[INDEX_LETTER][3] = t.pos; break;
					case WR_CODE_LETTER5: itemLocations[INDEX_LETTER][4] = t.pos; break;
					case WR_CODE_LETTER6: itemLocations[INDEX_LETTER][5] = t.pos; break;
					case WR_CODE_LETTER7: itemLocations[INDEX_LETTER][6] = t.pos; break;
					case WR_CODE_ANIM:    itemLocations[INDEX_ANIM].push_back(t.pos); break;
					case WR_CODE_FG:      itemLocations[INDEX_FG].push_back(t.pos); break;
				}
			}

			*this->content
				<< u16le(this->ptMapSize.x)
				<< u16le(this->ptMapSize.y)
				<< u16le(bgColour)
				<< u16le(tileset)
				<< u16le(backdrop)
				<< u16le(ptStart.x)
				<< u16le(ptStart.y)
				<< u16le(ptEnd.x)
				<< u16le(ptEnd.y)
			;

			// Write out all the gruzzles, slime buckets and book positions
			for (unsigned int i = 0; i < INDEX_SIZE; i++) {

				// Write the number of items first, except for letters which are fixed at 7
				if (i == INDEX_DRIP) {
					uint16_t len = (uint16_t)drips.size();
					*this->content << u16le(len);
				} else if (i != INDEX_LETTER) {
					uint16_t len = (uint16_t)itemLocations[i].size();
					*this->content << u16le(len);
				}

				// Write the X and Y coordinates for each item
				if (i == INDEX_DRIP) {
					for (auto& j : drips) {
						// Add an extra value for the drip frequency
						*this->content
							<< u16le(j.pos.x)
							<< u16le(j.pos.y)
							<< u16le(j.dripFreq) //0x44); // continuous dripping
						;
					}
				} else {
					for (auto& j : itemLocations[i]) {
						*this->content
							<< u16le(j.x)
							<< u16le(j.y)
						;
					}
				}
			}

			auto layerBG = std::dynamic_pointer_cast<Layer_WR_Background>(this->v_layers[0]);
			layerBG->flush(*this->content, this->ptMapSize);

			auto layerAT = std::dynamic_pointer_cast<Layer_WR_Attribute>(this->v_layers[3]);
			layerAT->flush(*this->content, this->ptMapSize);

			this->content->truncate_here();
			this->content->flush();
			return;
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			std::map<ImagePurpose, GraphicsFilename> gf;
			gf[ImagePurpose::BackgroundTileset1] = GraphicsFilename{
				createString("back"
					<< (int)(this->v_attributes[ATTR_TILESET].enumValue + 1)
					<< ".wr"),
				"tls-wordresc"
			};

			unsigned int dropNum = this->v_attributes[ATTR_BACKDROP].enumValue;
			if (dropNum > 0) {
				gf[ImagePurpose::BackgroundImage] = GraphicsFilename{
					createString("drop" << dropNum << ".wr"),
					"pcx-1b4p"
				};
			}
			return gf;
		}

		virtual Caps caps() const
		{
			return
				Map2D::Caps::HasViewport
				| Map2D::Caps::HasMapSize
				| Map2D::Caps::SetMapSize
				| Map2D::Caps::HasTileSize
			;
		}

		virtual Point viewport() const
		{
			return {288, 152};
		}

		virtual Point mapSize() const
		{
			return this->ptMapSize;
		}

		virtual Point tileSize() const
		{
			return {WR_BGTILE_WIDTH, WR_BGTILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			unsigned int dropNum = this->v_attributes[ATTR_BACKDROP].enumValue;
			if (dropNum > 0) return this->backgroundUseBGImage(tileset);

			unsigned int bgColour = this->v_attributes[ATTR_BGCOLOUR].enumValue;
			auto pal = createPalette_DefaultEGA();
			Background bg;
			bg.att = Background::Attachment::SingleColour;
			bg.clr = pal->at(bgColour);
			return bg;
		}

	private:
		std::unique_ptr<stream::inout> content;
		Point ptMapSize;
};


std::string MapType_WordRescue::code() const
{
	return "map2d-wordresc";
}

std::string MapType_WordRescue::friendlyName() const
{
	return "Word Rescue level";
}

std::vector<std::string> MapType_WordRescue::fileExtensions() const
{
	return {
		"s0",
		"s1",
		"s2",
		"s3",
		"s4",
		"s5",
		"s6",
		"s7",
		"s8",
		"s9",
		"s10",
		"s11",
		"s12",
		"s13",
		"s14",
		"s15",
		"s16",
		"s17",
		"s18",
		"s19",
	};
}

std::vector<std::string> MapType_WordRescue::games() const
{
	return {
		"Word Rescue",
	};
}

MapType::Certainty MapType_WordRescue::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

#define WR_MIN_HEADER_SIZE (2*15 + 4*7) // includes INDEX_LETTER

	// Make sure file is large enough for the header
	// TESTED BY: fmt_map_wordresc_isinstance_c01
	if (lenMap < WR_MIN_HEADER_SIZE) return MapType::DefinitelyNo;

	uint16_t mapWidth, mapHeight;
	content.seekg(0, stream::start);
	content
		>> u16le(mapWidth)
		>> u16le(mapHeight)
	;

	// Map size of zero is invalid
	// TESTED BY: fmt_map_wordresc_isinstance_c05
	if (mapWidth * mapHeight == 0) return MapType::DefinitelyNo;

	content.seekg(2*7, stream::cur);

	// Check the items are each within range
	unsigned int minSize = WR_MIN_HEADER_SIZE;
	for (unsigned int i = 0; i < INDEX_SIZE; i++) {
		stream::len lenBlock;
		if (i == INDEX_LETTER) {
			lenBlock = WR_NUM_LETTERS * 4;
		} else {
			uint16_t count;
			content >> u16le(count);
			lenBlock = count * 4;
			if (i == INDEX_DRIP) {
				// Extra byte for each of these
				lenBlock += count * 2;
			}
		}

		minSize += lenBlock;
		// Don't need to count the u16le in minSize, it's in WR_MIN_HEADER_SIZE

		// Make sure the item count is within range
		// TESTED BY: fmt_map_wordresc_isinstance_c02
		if (lenMap < minSize) return MapType::DefinitelyNo;
		content.seekg(lenBlock, stream::cur);
	}

	// Read in the layer and make sure all the tile codes are within range
	for (int i = 0; i < mapWidth * mapHeight; ) {
		uint8_t num, code;
		minSize += 2;
		// Make sure the background layer isn't cut off
		// TESTED BY: fmt_map_wordresc_isinstance_c03
		if (lenMap < minSize) return MapType::DefinitelyNo;

		content >> u8(num) >> u8(code);
		i += num;

		// Ignore the default tile (otherwise it would be out of range)
		if (code == WR_DEFAULT_BGTILE) continue;

		// Make sure the tile values are within range
		// TESTED BY: fmt_map_wordresc_isinstance_c04
		if (code > WR_MAX_VALID_TILECODE) return MapType::DefinitelyNo;
	}

	// TESTED BY: fmt_map_wordresc_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_WordRescue::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_WordRescue::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_WordRescue>(std::move(content));
}

SuppFilenames MapType_WordRescue::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}


} // namespace gamemaps
} // namespace camoto
