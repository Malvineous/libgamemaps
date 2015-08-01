/**
 * @file  fmt-map-got.cpp
 * @brief MapType and Map2D implementation for God of Thunder levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/God_of_Thunder_Level_Format
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

#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp> // make_unique
#include "map-core.hpp"
#include "map2d-core.hpp"
#include "fmt-map-got.hpp"

/// Maximum number of actors in a level
#define GOT_NUM_ACTORS           16

/// Maximum number of objects in a level
#define GOT_NUM_OBJECTS          30

/// Length of background layer, in bytes
#define GOT_LAYER_LEN_BG        240

/// Length of actor layer, in bytes
#define GOT_LAYER_LEN_ACTOR     (GOT_NUM_ACTORS * 5)

/// Length of object layer, in bytes
#define GOT_LAYER_LEN_OBJECT    (GOT_NUM_OBJECTS * 5)

/// Length of a whole map (one screen), in bytes (should be 512)
#define GOT_MAP_LEN  (GOT_LAYER_LEN_BG \
	+ 2 \
	+ GOT_LAYER_LEN_ACTOR \
	+ GOT_LAYER_LEN_OBJECT \
	+ 10 + 10 \
	+ 20)

/// Width of each cell
#define GOT_TILE_WIDTH           16

/// Height of each cell
#define GOT_TILE_HEIGHT          16

/// Width of map, in cells
#define GOT_MAP_WIDTH            20

/// Height of map, in cells
#define GOT_MAP_HEIGHT           12

/// Map code to write for locations with no tile set
#define GOT_DEFAULT_BGTILE     0xB0 // grass

/// Map code to write for locations with no tile set
#define GOT_DEFAULT_ACTORTILE  0x00

/// Map code to write for locations with no tile set
#define GOT_DEFAULT_OBJTILE    0x00

/// This is the largest valid tile code in the background layer
#define GOT_MAX_VALID_BG_TILECODE  229 // number of tiles in tileset

/// This is the largest valid tile code in the object layer
#define GOT_MAX_VALID_ACTOR_TILECODE  76 // number of tiles in tileset

/// This is the largest valid tile code in the object layer
#define GOT_MAX_VALID_OBJ_TILECODE  32 // number of tiles in tileset

/// Total number of screens in the map file
#define GOT_MAP_NUMSCREENS 120

/// Number of screens to draw in the horizontal direction
#define GOT_MAP_SCREENCOUNT_HORIZ 10

/// Number of screens to draw in the vertical direction
#define GOT_MAP_SCREENCOUNT_VERT (GOT_MAP_NUMSCREENS / GOT_MAP_SCREENCOUNT_HORIZ)

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_GOT_Background: public Map2DCore::LayerCore
{
	public:
		Layer_GOT_Background(stream::input& content)
		{
			std::vector<uint8_t> buf(GOT_LAYER_LEN_BG, GOT_DEFAULT_BGTILE);
			content.read(buf.data(), GOT_LAYER_LEN_BG);

			this->v_allItems.reserve(GOT_LAYER_LEN_BG);
			for (unsigned int i = 0; i < GOT_LAYER_LEN_BG; i++) {
				if (buf[i] == GOT_DEFAULT_BGTILE) continue;

				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();
				t.type = Item::Type::Default;
				t.pos = {i % GOT_MAP_WIDTH, i / GOT_MAP_WIDTH};
				t.code = buf[i];
			}
		}

		virtual ~Layer_GOT_Background()
		{
		}

		void flush(stream::output& content)
		{
			std::vector<uint8_t> buf(GOT_LAYER_LEN_BG, GOT_DEFAULT_BGTILE);
			for (auto& t : this->items()) {
				if ((t.pos.x >= GOT_MAP_WIDTH) || (t.pos.y >= GOT_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				buf[t.pos.y * GOT_MAP_WIDTH + t.pos.x] = t.code;
			}
			content.write(buf.data(), GOT_LAYER_LEN_BG);
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

			auto& images = t->second->files();
			if (item.code >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[item.code]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
#warning TODO: For tiles between 0xDC and 0xE5, show the hole/ladder number
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i <= GOT_MAX_VALID_BG_TILECODE; i++) {
				if (i == GOT_DEFAULT_BGTILE) continue;

				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}
};

class Layer_GOT_Actor: virtual public Map2DCore::LayerCore
{
	public:
		Layer_GOT_Actor(stream::input& content)
		{
			std::vector<uint8_t> buf(GOT_LAYER_LEN_ACTOR, GOT_DEFAULT_ACTORTILE);
			content.read(buf.data(), GOT_LAYER_LEN_ACTOR);

			this->v_allItems.reserve(GOT_LAYER_LEN_ACTOR);
			for (unsigned int i = 0; i < GOT_NUM_ACTORS; i++) {
				if (buf[i] == GOT_DEFAULT_ACTORTILE) continue;

				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();
				t.type = Item::Type::Default;
				t.pos = {
					buf[16 + i] % GOT_MAP_WIDTH,
					buf[16 + i] / GOT_MAP_WIDTH
				};
				t.code = buf[i] - 1;
			}
		}

		virtual ~Layer_GOT_Actor()
		{
		}

		void flush(stream::output& content)
		{
			for (auto& t : this->items()) {
				if ((t.pos.x >= GOT_MAP_WIDTH) || (t.pos.y >= GOT_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
			}
			auto numItems = this->items().size();
			unsigned int padItems = numItems >= GOT_NUM_ACTORS ? 0 : GOT_NUM_ACTORS - numItems;
			for (auto& t : this->items()) content << u8(t.code + 1);
			content << nullPadded("", padItems); // pad out to 16 bytes
			for (auto& t : this->items()) content << u8(t.pos.y * GOT_MAP_WIDTH + t.pos.x);
			content << nullPadded("", padItems); // pad out to 16 bytes

			// Padding, this data is unknown
#warning TODO: Work out what this data is used for
			content << nullPadded("", 16*3);
			return;
		}

		virtual std::string title() const
		{
			return "Actors";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			auto t = tileset.find(ImagePurpose::SpriteTileset1);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			unsigned int num = images.size();
			if (item.code >= num) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto tsActor = t->second->openTileset(images[item.code]);
			auto& actorFrames = tsActor->files();
			if (actorFrames.size() <= 0) { // no images
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = tsActor->openImage(actorFrames[0]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i <= GOT_MAX_VALID_ACTOR_TILECODE; i++) {
				if (i == GOT_DEFAULT_ACTORTILE) continue;

				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}
};

class Layer_GOT_Object: virtual public Map2DCore::LayerCore
{
	public:
		Layer_GOT_Object(stream::input& content)
		{
			std::vector<uint8_t> buf(GOT_LAYER_LEN_OBJECT, GOT_DEFAULT_OBJTILE);
			content.read(buf.data(), GOT_LAYER_LEN_OBJECT);

			this->v_allItems.reserve(GOT_LAYER_LEN_OBJECT);
			for (unsigned int i = 0; i < GOT_NUM_OBJECTS; i++) {
				if (buf[i] == GOT_DEFAULT_OBJTILE) continue;

				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();
				t.type = Item::Type::Default;
				t.pos = {
					buf[30 + i * 2] | (buf[30 + i * 2 + 1] << 8),
					buf[90 + i * 2] | (buf[90 + i * 2 + 1] << 8)
				};
				t.code = buf[i] - 1;
			}
		}

		virtual ~Layer_GOT_Object()
		{
		}

		void flush(stream::output& content)
		{
			for (auto& t : this->items()) {
				if ((t.pos.x >= GOT_MAP_WIDTH) || (t.pos.y >= GOT_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
			}
			auto numItems = this->items().size();
			unsigned int padItems = numItems >= GOT_NUM_OBJECTS ? 0 : GOT_NUM_OBJECTS - numItems;
			for (auto& t : this->items()) content << u8(t.code + 1);
			content << nullPadded("", padItems);
			for (auto& t : this->items()) content << u16le(t.pos.x);
			content << nullPadded("", padItems * 2);
			for (auto& t : this->items()) content << u16le(t.pos.y);
			content << nullPadded("", padItems * 2);
			return;
		}

		virtual std::string title() const
		{
			return "Objects";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			auto t = tileset.find(ImagePurpose::ForegroundTileset2);
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
			for (unsigned int i = 0; i <= GOT_MAX_VALID_OBJ_TILECODE; i++) {
				if (i == GOT_DEFAULT_OBJTILE) continue;

				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}
};

class Map_GOT: public MapCore, public Map2DCore
{
	public:
		Map_GOT(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			this->content->seekg(0, stream::start);

			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_GOT_Background>(*this->content)
			);

			uint8_t defaultTileBG, defaultSong;
			*this->content
				>> u8(defaultTileBG)
				>> u8(defaultSong)
			;

			this->attr.emplace_back();
			auto& attrBGTile = this->attr.back();
			attrBGTile.type = Attribute::Type::Enum;
			attrBGTile.name = "Background";
			attrBGTile.desc = "Default background tile to display behind level.";
			attrBGTile.enumValue = defaultTileBG;
			attrBGTile.enumValueNames = {
#warning TODO: Tile list
				"0 - todo: tile list",
			};

			this->attr.emplace_back();
			auto& attrMusic = this->attr.back();
			attrMusic.type = Attribute::Type::Enum;
			attrMusic.name = "Music";
			attrMusic.desc = "Index of the song to play as background music in the level.";
			attrMusic.enumValue = defaultSong;
			attrMusic.enumValueNames = {
#warning TODO: Song list
				"0 - song1?",
				"1 - ?",
				"2 - todo",
				"3 - etc",
			};

			// Read the actor layer
			this->v_layers.push_back(
				std::make_shared<Layer_GOT_Actor>(*this->content)
			);

			// Read the object layer
			this->v_layers.push_back(
				std::make_shared<Layer_GOT_Object>(*this->content)
			);

			// Read the hole/ladder details
			uint8_t holeScr[10], holePos[10];
			this->content->read(holeScr, 10);
			this->content->read(holePos, 10);

			for (int i = 0; i < 10; i++) {
				this->attr.emplace_back();
				auto& attrHoleScreen = this->attr.back();
				attrHoleScreen.type = Attribute::Type::Integer;
				attrHoleScreen.name = createString("Hole/ladder " << i << " target");
				attrHoleScreen.desc = "Screen number of hole/ladder destination.";
				attrHoleScreen.integerValue = holeScr[i];
				attrHoleScreen.integerMinValue = 0;
				attrHoleScreen.integerMaxValue = GOT_MAP_NUMSCREENS - 1;

				this->attr.emplace_back();
				auto& attrHolePosX = this->attr.back();
				attrHolePosX.type = Attribute::Type::Integer;
				attrHolePosX.name = createString("Hole/ladder " << i << " target X");
				attrHolePosX.desc = "Player X coordinate on destination screen, after exiting hole/ladder.";
				attrHolePosX.integerValue = holePos[i] % GOT_MAP_WIDTH;
				attrHolePosX.integerMinValue = 0;
				attrHolePosX.integerMaxValue = GOT_MAP_WIDTH - 1;

				this->attr.emplace_back();
				auto& attrHolePosY = this->attr.back();
				attrHolePosY.type = Attribute::Type::Integer;
				attrHolePosY.name = createString("Hole/ladder " << i << " target Y");
				attrHolePosY.desc = "Player Y coordinate on destination screen, after exiting hole/ladder.";
				attrHolePosY.integerValue = holePos[i] / GOT_MAP_WIDTH;
				attrHolePosY.integerMinValue = 0;
				attrHolePosY.integerMaxValue = GOT_MAP_HEIGHT - 1;
			}
		}

		virtual ~Map_GOT()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			// Graphics filenames aren't stored in the map file, so we can't return
			// anything here, they'll have to be supplied manually.
			return {};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 3);
			assert(this->attributes().size() == 2 + 10*3);

			this->content->truncate(GOT_MAP_LEN);
			this->content->seekp(0, stream::start);
			auto attributes = this->attributes();

			// Write the background layer
			auto layerBG = dynamic_cast<Layer_GOT_Background*>(this->v_layers[0].get());
			layerBG->flush(*this->content);

			*this->content
				<< u8(attributes[0].enumValue)
				<< u8(attributes[1].enumValue)
			;

			// Write the actor layer
			auto layerAC = dynamic_cast<Layer_GOT_Actor*>(this->v_layers[1].get());
			layerAC->flush(*this->content);

			// Write the object layer
			auto layerOB = dynamic_cast<Layer_GOT_Object*>(this->v_layers[2].get());
			layerOB->flush(*this->content);

			// Read the hole/ladder details
			uint8_t holeScr[10], holePos[10];
			for (int i = 0; i < 10; i++) {
				int attBase = 2 + i * 3;
				holeScr[i] = attributes[attBase + 0].integerValue;
				holePos[i] = attributes[attBase + 2].integerValue * GOT_MAP_WIDTH
					+ attributes[attBase + 1].integerValue;
			}
			this->content->write(holeScr, 10);
			this->content->write(holePos, 10);

			// TEMP: Pad file to 512 bytes until the format of this data is known
			*this->content << nullPadded("", 20);

			this->content->flush();
			return;
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
			return {320, 192};
		}

		virtual Point mapSize() const
		{
			return {
				GOT_MAP_WIDTH,
				GOT_MAP_HEIGHT
			};
		}

		virtual Point tileSize() const
		{
			return {GOT_TILE_WIDTH, GOT_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, this->defaultTileBG);
		}

	private:
		std::unique_ptr<stream::inout> content;
		uint8_t defaultTileBG;
		uint8_t defaultSong;
};


std::string MapType_GOT::code() const
{
	return "map2d-got";
}

std::string MapType_GOT::friendlyName() const
{
	return "God of Thunder level";
}

std::vector<std::string> MapType_GOT::fileExtensions() const
{
	return {};
}

std::vector<std::string> MapType_GOT::games() const
{
	return {"God of Thunder"};
}

MapType::Certainty MapType_GOT::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// Make sure there's enough data
	// TESTED BY: fmt_map_got_isinstance_c01
	if (lenMap != GOT_MAP_LEN) return MapType::DefinitelyNo;

	content.seekg(0, stream::start);
	uint8_t bg[GOT_LAYER_LEN_BG];
	content.read(bg, GOT_LAYER_LEN_BG);
	for (int i = 0; i < GOT_LAYER_LEN_BG; i++) {
		// Background layer code out of range
		// TESTED BY: fmt_map_got_isinstance_c02
		if (bg[i] > GOT_MAX_VALID_BG_TILECODE) return MapType::DefinitelyNo;
	}
	content.seekg(2, stream::cur);

	uint8_t ac[GOT_LAYER_LEN_ACTOR];
	content.read(ac, GOT_LAYER_LEN_ACTOR);
	for (int i = 0; i < GOT_NUM_ACTORS; i++) {
		// Actor layer code out of range
		// TESTED BY: fmt_map_got_isinstance_c03
		if (ac[i] > GOT_MAX_VALID_ACTOR_TILECODE) return MapType::DefinitelyNo;
	}

	uint8_t ob[GOT_LAYER_LEN_OBJECT];
	content.read(ob, GOT_LAYER_LEN_OBJECT);
	for (int i = 0; i < GOT_NUM_OBJECTS; i++) {
		// Object layer code out of range
		// TESTED BY: fmt_map_got_isinstance_c04
		if (ob[i] > GOT_MAX_VALID_OBJ_TILECODE) return MapType::DefinitelyNo;
	}

	// TESTED BY: fmt_map_got_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_GOT::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_GOT::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_GOT>(std::move(content));
}

SuppFilenames MapType_GOT::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
