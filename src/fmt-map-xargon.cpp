/**
 * @file  fmt-map-xargon.cpp
 * @brief MapType and Map2D implementation for Jill of the Jungle and Xargon.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Jill_of_the_Jungle_Map_Format
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
#include <iostream>
#include <list>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp> // make_unique
#include "map-core.hpp"
#include "map2d-core.hpp"
#include "fmt-map-xargon.hpp"

/// Length of an entry in the object layer
#define SW_OBJ_ENTRY_LEN        31

/// Map width, in tiles
#define SW_MAP_WIDTH            128

/// Map height, in tiles
#define SW_MAP_HEIGHT           64

/// Offset of the object layer, in bytes from the start of the file
#define SW_OFFSET_OBJLAYER      (SW_MAP_WIDTH * SW_MAP_HEIGHT * 2)

/// Tile dimensions in background layer
#define SW_TILE_WIDTH   16
#define SW_TILE_HEIGHT  16

/// Default tile in background layer
#define SW_DEFAULT_BGTILE  0

/// Maximum number of strings in the stringdata section
#define SW_SAFETY_MAX_STRINGS   512

/// Object number for the player ("hero") object
#define SW_OBJCODE_PLAYER 0

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

struct GameData {
	stream::len lenSavedata;     ///< Length of savegame data, in bytes
	unsigned int numObjectTypes; ///< Number of different object types
	Point viewport;              ///< Viewport size, in pixels
};

GameData getJillData()
{
	GameData gdJill;
	gdJill.lenSavedata = 70;
	gdJill.numObjectTypes = 10;
	gdJill.viewport = {232, 160};
	return gdJill;
}

GameData getXargonData()
{
	GameData gdXargon;
	gdXargon.lenSavedata = 97;
	gdXargon.numObjectTypes = 89; // numobjkinds from Xargon x_obj.h
	gdXargon.viewport = {20 * SW_TILE_WIDTH, 10 * SW_TILE_HEIGHT};
	return gdXargon;
}

class Layer_Sweeney_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Sweeney_Background(stream::input& content, stream::input& contentDMA,
			const Point& mapSize)
			:	mapSize(mapSize)
		{
			// Read the tile properties from the suppdata
			stream::delta len = contentDMA.size();
			contentDMA.seekg(0, stream::start);
			do {
				uint16_t mapCode, flags;
				uint8_t namelen;
				TileCode tc;
				contentDMA
					>> u16le(mapCode)
					>> u8(tc.imageIndex)
					>> u8(tc.tilesetIndex)
					>> u16le(flags)
					>> u8(namelen)
				;

				tc.tilesetIndex &= 0x3F;
				this->dmaMap[mapCode] = tc;

				// Skip name
				contentDMA.seekg(namelen, stream::cur);
				len -= 7 + namelen;
			} while (len > 7);


			// Read the background layer
			this->v_allItems.reserve(SW_MAP_WIDTH * SW_MAP_HEIGHT);

			for (unsigned int x = 0; x < SW_MAP_WIDTH; x++) {
				for (unsigned int y = 0; y < SW_MAP_HEIGHT; y++) {
					uint16_t code;
					content >> u16le(code);

					// Don't push zero codes (these will "show through" to the map
					// background, which is this image tiled, prevening tiles from being
					// completely deleted (deleting a tile just appears to set it back
					// to the default tile.)
					if ((code & 0x3FFF) == SW_DEFAULT_BGTILE) continue;

					this->v_allItems.emplace_back();
					auto& t = this->v_allItems.back();

					t.type = Item::Type::Default;
					t.pos.x = x;
					t.pos.y = y;
					t.code = code;
				}
			}
		}

		virtual ~Layer_Sweeney_Background()
		{
		}

		void flush(stream::output& content)
		{
			std::vector<uint16_t> tiles(this->mapSize.x * this->mapSize.y,
				SW_DEFAULT_BGTILE);
			for (auto& t : this->v_allItems) {
				if ((t.pos.x > this->mapSize.x) || (t.pos.y > this->mapSize.y)) {
					throw stream::error(createString("Layer has tiles outside map "
							"boundary at (" << t.pos.x << "," << t.pos.y << ")"));
				}
				tiles[t.pos.x * this->mapSize.y + t.pos.y] = t.code;
			}

			for (auto& code : tiles) {
				content << u16le(code);
			}
			return;
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

			auto itTC = this->dmaMap.find(item.code & 0x3FFF);
			if (itTC == this->dmaMap.end()) {
				std::cout << "Xargon tilecode 0x" << std::hex
					<< (unsigned int)(item.code & 0x3FFF) << std::dec
					<< " not found in DMA file.\n";
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}
			auto& tc = itTC->second;

			auto& tilesets = t->second->files();

			if (tc.tilesetIndex >= tilesets.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			if (tilesets[tc.tilesetIndex]->fAttr & gamearchive::Archive::File::Attribute::Vacant) {
				std::cerr << "[Layer_Sweeney_Background] Tried to open tileset "
					<< (int)tc.tilesetIndex << " but it's an empty slot!" << std::endl;
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto subTileset = t->second->openTileset(tilesets[tc.tilesetIndex]);
			auto& images = subTileset->files();
			if (images.size() <= 0) { // no images
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}
			if (tc.imageIndex >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}
			if (images[tc.imageIndex]->fAttr & gamearchive::Archive::File::Attribute::Vacant) {
				std::cerr << "[SweeneyBackgroundLayer] Tried to open image "
					<< (int)tc.tilesetIndex << "." << (int)tc.imageIndex
					<< " but it's an empty slot!" << std::endl;
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = subTileset->openImage(images[tc.imageIndex]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (auto& i : this->dmaMap) {
				validItems.emplace_back();
				auto& t = validItems.back();

				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i.first;
			}
			return validItems;
		}

	protected:
		struct TileCode {
			uint8_t tilesetIndex;
			uint8_t imageIndex;
		};
		std::map<uint16_t, TileCode> dmaMap;

		Point mapSize; ///< Size of the layer, in tiles
};

class Layer_Sweeney_Object: public Map2DCore::LayerCore
{
	public:
		Layer_Sweeney_Object(stream::input& content, const GameData& gameData)
			:	gameData(gameData)
		{
			uint16_t numObjects;
			content >> u16le(numObjects);

			this->v_allItems.reserve(numObjects);
			for (unsigned int i = 0; i < numObjects; i++) {
				uint8_t code;
				uint16_t x, y;
				uint16_t spdHoriz, spdVert;
				uint16_t width, height;
				uint16_t subType;
				uint16_t subState;
				uint16_t stateCount;
				uint16_t link;
				uint16_t flags;
				uint32_t pointer;
				uint16_t info;
				uint16_t zapHold;
				content
					>> u8(code)
					>> u16le(x)
					>> u16le(y)
					>> u16le(spdHoriz)
					>> u16le(spdVert)
					>> u16le(width)
					>> u16le(height)
					>> u16le(subType)
					>> u16le(subState)
					>> u16le(stateCount)
					>> u16le(link)
					>> u16le(flags)
					>> u32le(pointer)
					>> u16le(info)
					>> u16le(zapHold)
				;

				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				t.pos = {x, y};
				t.code = code | (subType << 8);

				if (pointer) {
					// This one refers to a text entry
					t.type |= Item::Type::Text;
					t.textFont = 0; ///< @todo Correct font
				}
				if (spdHoriz || spdVert) {
					t.type |= Item::Type::Movement;
					t.movementSpeedX = spdHoriz; ///< @todo Correct calculation
					t.movementSpeedY = spdVert;  ///< @todo Correct calculation
					t.movementDistLeft = 0;
					t.movementDistRight = 0;
					t.movementDistUp = 0;
					t.movementDistDown = 0;
				}
			}
		}

		virtual ~Layer_Sweeney_Object()
		{
		}

		void readText(stream::input& content)
		{
			for (auto& t : this->v_allItems) {
				if (t.type & Item::Type::Text) {
					try {
						// Read a text element, if any are present
						uint16_t lenStr;
						content >> u16le(lenStr);
						lenStr++; // include terminating null
						content >> nullPadded(t.textContent, lenStr);
					} catch (const stream::incomplete_read&) {
						throw camoto::error("Map file has been truncated! (text section "
							"cut unexpectedly)");
					}
				}
			}
			return;
		}

		void flush(stream::output& content)
		{
			uint16_t numObjects = (uint16_t)this->v_allItems.size();
			content << u16le(numObjects);

			for (auto& t : this->v_allItems) {
				uint8_t code = t.code & 0xFF;
				uint16_t x = t.pos.x;
				uint16_t y = t.pos.y;
				uint16_t spdHoriz;
				uint16_t spdVert;
				if (t.type & Item::Type::Movement) {
					spdHoriz = t.movementSpeedX;
					spdVert = t.movementSpeedY;
				} else {
					spdHoriz = 0;
					spdVert = 0;
				}
				uint16_t width = 16; ///< @todo How to get size when we don't have tileset?
				uint16_t height = 16; ///< @todo Throw exception when there is no tileset maybe?  Or hardcode object sizes?
				uint16_t subType = t.code >> 8;
				uint16_t subState = 0;
				uint16_t stateCount = 0;
				uint16_t link = 0;
				uint16_t flags = 0;
				uint32_t pointer;
				if (t.type & Item::Type::Text) {
					pointer = 1; // non-zero means text is present
				} else {
					pointer = 0;
				}
				uint16_t info = 0;
				uint16_t zapHold = 0;

				content
					<< u8(code)
					<< u16le(x)
					<< u16le(y)
					<< u16le(spdHoriz)
					<< u16le(spdVert)
					<< u16le(width)
					<< u16le(height)
					<< u16le(subType)
					<< u16le(subState)
					<< u16le(stateCount)
					<< u16le(link)
					<< u16le(flags)
					<< u32le(pointer)
					<< u16le(info)
					<< u16le(zapHold)
				;
			}
			return;
		}

		void flushText(stream::output& content)
		{
			for (auto& t : this->v_allItems) {
				if (t.type & Item::Type::Text) {
					unsigned int len = t.textContent.length();
					if (len > 65535) throw camoto::error("Cannot write a text element "
						"longer than 65535 characters to a Sweeney map.");
					content << u16le(len);
					content.write(t.textContent);
					content << u8(0); // terminating null, not included in length count
				}
			}
			return;
		}

		virtual std::string title() const
		{
			return "Objects";
		}

		virtual Caps caps() const
		{
			return Caps::HasOwnTileSize;
		}

		virtual Point tileSize() const
		{
			return {1, 1};
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;
			ret.type = ImageFromCodeInfo::ImageType::Unknown;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i < this->gameData.numObjectTypes; i++) {
				validItems.emplace_back();
				auto& t = validItems.back();

				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}

	protected:
		GameData gameData;
};


class Map_Sweeney: public MapCore, public Map2DCore
{
	public:
		Map_Sweeney(std::unique_ptr<stream::inout> content,
			stream::input& contentDMA, const GameData& gameData)
			:	content(std::move(content)),
				gameData(gameData)
		{
			// Read the background layer
			this->content->seekg(0, stream::start);
			auto layerBG = std::make_shared<Layer_Sweeney_Background>(
				*this->content, contentDMA, this->mapSize()
			);

			// Read the object layer
			auto layerOB = std::make_shared<Layer_Sweeney_Object>(
				*this->content, this->gameData
			);

			// Savedata
			int16_t level;
			*this->content >> s16le(level);
			{
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Integer;
				a.name = "Level number";
				a.desc = "Level number to show in game status bar";
				a.integerValue = level;
				a.integerMaxValue = 0;
				a.integerMinValue = 0;
			};
			// Ignore rest of data
			this->content->seekg(this->gameData.lenSavedata - 2, stream::cur);

			// Read all the text strings at the end of the map first
			layerOB->readText(*this->content);

			this->v_layers.push_back(layerBG);
			this->v_layers.push_back(layerOB);
		}

		virtual ~Map_Sweeney()
		{
		}

		virtual void flush()
		{
			assert(this->v_layers.size() == 2);

			this->content->seekp(0, stream::start);

			// Write the background layer
			auto layerBG = std::dynamic_pointer_cast<Layer_Sweeney_Background>(this->v_layers[0]);
			layerBG->flush(*this->content);

			// Write the object layer
			auto layerOB = std::dynamic_pointer_cast<Layer_Sweeney_Object>(this->v_layers[1]);
			layerOB->flush(*this->content);

			// Write out savedata
			auto& attrLevel = this->v_attributes[0];
			assert(attrLevel.type == Attribute::Type::Integer);
			int16_t level = attrLevel.integerValue;
			*this->content << s16le(level);
			std::string blank(this->gameData.lenSavedata - 2, '\0');
			this->content->write(blank);

			// Write out text strings
			layerOB->flushText(*this->content);

			this->content->truncate_here();
			this->content->flush();
			return;
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {};
		}

		virtual Caps caps() const
		{
			return
				Map2D::Caps::HasViewport
				| Map2D::Caps::HasMapSize
				| Map2D::Caps::HasTileSize
			;
		}

		virtual Point viewport() const
		{
			return this->gameData.viewport;
		}

		virtual Point mapSize() const
		{
			return {SW_MAP_WIDTH, SW_MAP_HEIGHT};
		}

		virtual Point tileSize() const
		{
			return {SW_TILE_WIDTH, SW_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, SW_DEFAULT_BGTILE);
		}

		/// Shared function for Jill + Xargon's MapType::isInstance()
		static MapType::Certainty isInstance(stream::input& content,
			stream::len lenSavedata)
		{
			stream::pos lenMap = content.size();

			// Too short
			// TESTED BY: fmt_map_xargon_isinstance_c01
			if (lenMap < SW_OFFSET_OBJLAYER + 2) return MapType::DefinitelyNo;

			content.seekg(SW_OFFSET_OBJLAYER, stream::start);
			uint16_t numObjects;
			content >> u16le(numObjects);

			stream::pos offStrings = SW_OFFSET_OBJLAYER + 2 +
				numObjects * SW_OBJ_ENTRY_LEN + lenSavedata;

			// Make sure the object and savedata layers aren't cut off
			// TESTED BY: fmt_map_xargon_isinstance_c02
			if (lenMap < offStrings) return MapType::DefinitelyNo;

			uint8_t code;
			content
				>> u8(code)
				;
			// Player object must be first
			// TESTED BY: fmt_map_xargon_isinstance_c03
			if (code != SW_OBJCODE_PLAYER) return MapType::DefinitelyNo;

			content.seekg(SW_OBJ_ENTRY_LEN - 1, stream::cur);

			// Read the objects, make sure there aren't any more player objects
			for (unsigned int i = 1; i < numObjects; i++) {
				uint8_t code;
				content
					>> u8(code)
					;
				content.seekg(SW_OBJ_ENTRY_LEN - 1, stream::cur);
				// Wrong number of player objects
				// TESTED BY: fmt_map_xargon_isinstance_c04
				if (code == 0x00) return MapType::DefinitelyNo;
			}

			// Map is exact size with no string table
			// TESTED BY: fmt_map_xargon_isinstance_c05
			if (lenMap == offStrings) return MapType::DefinitelyYes;

			// Savedata is untested

			unsigned int i;
			for (i = 0; i < SW_SAFETY_MAX_STRINGS; i++) {
				// Another string, but the length bytes are cut off
				// TESTED BY: fmt_map_xargon_isinstance_c06
				if (offStrings + 3 > lenMap) return MapType::DefinitelyNo;
				content.seekg(offStrings, stream::start);

				uint16_t lenStr;
				content >> u16le(lenStr);
				// Let's assume empty strings aren't allowed
				// TESTED BY: fmt_map_xargon_isinstance_c07
				if (lenStr == 0) return MapType::DefinitelyNo;

				offStrings += lenStr + 2 + 1; // +2 for uint16le, +1 for terminating null
				if (offStrings == lenMap) break; // reached EOF

				// Make sure the string itself isn't cut
				// TESTED BY: fmt_map_xargon_isinstance_c08
				if (offStrings > lenMap) return MapType::DefinitelyNo;
			}
			// Too many strings
			// TESTED BY: fmt_map_xargon_isinstance_c09
			if (i == SW_SAFETY_MAX_STRINGS) return MapType::DefinitelyNo;

			// TESTED BY: fmt_map_xargon_isinstance_c00
			return MapType::DefinitelyYes;
		}

	private:
		std::unique_ptr<stream::inout> content;
		GameData gameData;
};


//
// MapType_Jill
//

std::string MapType_Jill::code() const
{
	return "map2d-jill";
}

std::string MapType_Jill::friendlyName() const
{
	return "Jill of the Jungle map";
}

std::vector<std::string> MapType_Jill::fileExtensions() const
{
	return {
		"jn1",
		"jn2",
		"jn3",
	};
}

std::vector<std::string> MapType_Jill::games() const
{
	return {
		"Jill of the Jungle",
	};
}

MapType::Certainty MapType_Jill::isInstance(stream::input& content) const
{
	auto gd = getJillData();
	return Map_Sweeney::isInstance(content, gd.lenSavedata);
}

std::unique_ptr<Map> MapType_Jill::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Jill::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	auto suppTileInfo = suppData.find(SuppItem::Extra1);
	if (suppTileInfo == suppData.end()) {
		throw camoto::error("Missing content for Extra1 (tile info) "
			"supplementary item.");
	}

	return std::make_unique<Map_Sweeney>(std::move(content),
		*(suppTileInfo->second), getJillData());
}

SuppFilenames MapType_Jill::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	SuppFilenames supps;
	supps[SuppItem::Extra1] = "jill.dma";
	return supps;
}


//
// MapType_Xargon
//

std::string MapType_Xargon::code() const
{
	return "map2d-xargon";
}

std::string MapType_Xargon::friendlyName() const
{
	return "Xargon map";
}

std::vector<std::string> MapType_Xargon::fileExtensions() const
{
	return {
		"xr0",
		"xr1",
		"xr2",
		"xr3",
	};
}

std::vector<std::string> MapType_Xargon::games() const
{
	return {
		"Xargon",
	};
}

MapType::Certainty MapType_Xargon::isInstance(stream::input& content) const
{
	auto gd = getXargonData();
	return Map_Sweeney::isInstance(content, gd.lenSavedata);
}

std::unique_ptr<Map> MapType_Xargon::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Xargon::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	auto suppTileInfo = suppData.find(SuppItem::Extra1);
	if (suppTileInfo == suppData.end()) {
		throw camoto::error("Missing content for Extra1 (tile info) "
			"supplementary item.");
	}

	return std::make_unique<Map_Sweeney>(std::move(content),
		*(suppTileInfo->second), getXargonData());
}

SuppFilenames MapType_Xargon::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	// Take the extension from the file being opened and use the corresponding
	// tiles file, i.e. "blah.xr1" -> "tiles.xr1".  There are no ".xr0" levels.
	SuppFilenames supps;
	std::string ext = filename.substr(filename.find_last_of('.'));
	supps[SuppItem::Extra1] = "tiles" + ext;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
