/**
 * @file  fmt-map-harry.cpp
 * @brief MapType and Map2D implementation for Halloween Harry/Alien Carnage.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GMF_Format_(Halloween_Harry)
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
#include "fmt-map-harry.hpp"

/// Width of each tile in pixels
#define HH_TILE_WIDTH 16

/// Height of each tile in pixels
#define HH_TILE_HEIGHT 16

/// Number of bytes for each actor struct
#define HH_ACTOR_LEN  128

/// Map code used for 'no tile'
#define HH_DEFAULT_TILE 0xFE

/// Last valid map code
#define HH_MAX_VALID_TILECODE_BG 0xFF

/// Number of sprites
#define HH_MAX_VALID_TILECODE_ACTOR 466

/// Length of an actor data structure, in bytes
#define HH_ACTOR_LEN 128

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_Harry_Actor: public Map2DCore::LayerCore
{
	public:
		Layer_Harry_Actor(stream::input& content, unsigned int startX,
			unsigned int startY)
		{
			// Read in the actor layer
			uint16_t numActors;
			content >> u16le(numActors);
			this->v_allItems.reserve(numActors + 1);

			// Create a fake object with the player's starting location
			{
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();
				t.type = Item::Type::Player;
				t.pos.x = startX;
				t.pos.y = startY;
				t.code = 0; // unused
				t.playerNumber = 0; // player 1
				t.playerFacingLeft = false; // fixed?
			}

			// Read the real objects
			for (unsigned int i = 0; i < numActors; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();
				uint8_t code;
				content
					>> u8(code)
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
				;
				t.type = Item::Type::Default;
				t.code = code;
				content.seekg(128-1-2-2, stream::cur);
			}
		}

		virtual ~Layer_Harry_Actor()
		{
		}

		void flush(stream::output& content, const Point& dims)
		{
			auto actors = this->items();
			// There will be an actor for the player start point, but we don't want to
			// write that as that goes in the map format's player-start-point fields.
			unsigned int numActors = actors.size() - 1;

			content << u16le(numActors);
			for (auto& t : actors) {
				if ((t.pos.x >= dims.x) || (t.pos.y >= dims.y)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}

				// Don't write player start points here
				if (t.type & Item::Type::Player) continue;

				content
					<< u8(t.code)
					<< u16le(t.pos.x)
					<< u16le(t.pos.y)
				;
				/// @todo Work out what the remaining 123 bytes are for
				char pad[128-5];
				memset(pad, 0x00, 128-5);
				content.write(pad, 128-5);
			}
			return;
		}

		virtual std::string title() const
		{
			return "Actors";
		}

		virtual Caps caps() const
		{
			return Caps::HasOwnTileSize | Caps::UseImageDims;
		}

		virtual Point tileSize() const
		{
			return {1, 1};
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
			for (unsigned int i = 0; i <= HH_MAX_VALID_TILECODE_ACTOR; i++) {
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}
};

class Layer_Harry_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Harry_Background(stream::input& content, const Point& dims)
		{
			this->v_allItems.reserve(dims.x * dims.y);

			for (unsigned int y = 0; y < dims.y; y++) {
				for (unsigned int x = 0; x < dims.x; x++) {
					uint8_t code;
					content >> u8(code);
					if (code == HH_DEFAULT_TILE) continue;

					this->v_allItems.emplace_back();
					auto& t = this->v_allItems.back();
					t.type = Item::Type::Default;
					t.pos.x = x;
					t.pos.y = y;
					t.code = code;
				}
			}
		}

		virtual ~Layer_Harry_Background()
		{
		}

		void flush(stream::output& content, const Point& dims)
		{
			std::vector<uint8_t> buf(dims.x * dims.y, HH_DEFAULT_TILE);
			for (auto& t : this->items()) {
				if ((t.pos.x >= dims.x) || (t.pos.y >= dims.y)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				buf[t.pos.y * dims.x + t.pos.x] = t.code;
			}
			content.write(buf.data(), buf.size());
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
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i <= HH_MAX_VALID_TILECODE_BG; i++) {
				if (i == HH_DEFAULT_TILE) continue;

				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}
};

class Layer_Harry_Foreground: public Layer_Harry_Background
{
	public:
		Layer_Harry_Foreground(stream::input& content, const Point& dims)
			:	Layer_Harry_Background(content, dims)
		{
		}

		virtual std::string title() const
		{
			return "Foreground";
		}
};

class Map_Harry: public MapCore, public Map2DCore
{
	public:
		Map_Harry(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			this->content->seekg(0, stream::start);

			// Skip signature and flags
			this->content->seekg(0x12 + 4, stream::cur);

			uint16_t startX, startY;
			*this->content
				>> u16le(startX)
				>> u16le(startY)
			;

			this->content->seekg(2, stream::cur);

			uint8_t mapFlags;
			*this->content >> u8(mapFlags);

			this->v_attributes.emplace_back();
			auto& attrParallax = this->v_attributes.back();
			attrParallax.type = Attribute::Type::Enum;
			attrParallax.name = "Background";
			attrParallax.desc = "How to position the background layer as the player "
				"moves (parallax is only visible in-game).";
			attrParallax.enumValue = mapFlags == 0 ? 0 : 1;
			attrParallax.enumValueNames.push_back("Fixed");
			attrParallax.enumValueNames.push_back("Parallax");

			this->content->read(this->pal, 768);
			this->content->read(this->tileFlags, 256);

			// Skip unknown block
			this->content->seekg(10, stream::cur);

			auto layerAC = std::make_shared<Layer_Harry_Actor>(*this->content, startX,
				startY);

			*this->content
				>> u16le(this->v_mapSize.x)
				>> u16le(this->v_mapSize.y)
			;

			auto layerBG = std::make_shared<Layer_Harry_Background>(*this->content,
				this->v_mapSize);
			auto layerFG = std::make_shared<Layer_Harry_Foreground>(*this->content,
				this->v_mapSize);

			this->v_layers.push_back(layerBG);
			this->v_layers.push_back(layerFG);
			this->v_layers.push_back(layerAC);
		}

		virtual ~Map_Harry()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			// Graphics filenames aren't stored in the map file, so we can't return
			// anything here, they'll have to be supplied manually.  We just return
			// which tileset types are needed.
			return {
				std::make_pair(
					ImagePurpose::BackgroundTileset1,
					GraphicsFilename{"", "tls-harry-chr"}
				),
				std::make_pair(
					ImagePurpose::SpriteTileset1,
					GraphicsFilename{"harry.hsb", "tls-harry-hsb"}
				),
			};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 3);
			assert(this->v_attributes.size() == 1);

			auto dims = this->mapSize();
			this->content->truncate(
				0x12 // subzero header
				+  11 // other header
				+ 768 // pal
				+ 256 // tile flags
				+  10 // unknown
				+   2 // num actors
				+ (this->layers().at(2)->items().size() - 1) * HH_ACTOR_LEN
				+ 4 // map size
				+ dims.x * dims.y * 2 // bg + fg layer
			);
			this->content->seekp(0, stream::start);

			// Find the player-start-point objects
			uint16_t startX = 0, startY = 0;
			bool setPlayer = false;
			auto layer = this->layers().at(2);
			auto actors = layer->items();
			uint16_t numActors = actors.size();
			for (auto& t : actors) {
				if (t.type & Layer::Item::Type::Player) {
					// This is the player starting location
					if (setPlayer) {
						// We've already set the player position
						throw stream::error("This map format can only have one player.");
					}
					if (t.playerNumber == 0) {
						startX = t.pos.x;
						startY = t.pos.y;
						setPlayer = true;
					}
					numActors--;
				}
			}

			uint8_t mapFlags = this->v_attributes[0].enumValue;

			*this->content
				<< nullPadded("\x11SubZero Game File", 0x12)
				<< u32le(0)
				<< u16le(startX)
				<< u16le(startY)
				<< u16le(0)
				<< u8(mapFlags)
			;

			this->content->write(this->pal, 768);
			this->content->write(this->tileFlags, 256);

/// @todo Write the unknown data
			char unk[10];
			memset(unk, 0x00, 10);
			this->content->write(unk, 10);

			// Write the actor layer
			auto layerAC = dynamic_cast<Layer_Harry_Actor*>(this->v_layers[2].get());
			layerAC->flush(*this->content, dims);

			*this->content
				<< u16le(dims.x)
				<< u16le(dims.y)
			;

			// Write the background layer
			auto layerBG = dynamic_cast<Layer_Harry_Background*>(this->v_layers[0].get());
			layerBG->flush(*this->content, dims);

			// Write the foreground layer
			auto layerFG = dynamic_cast<Layer_Harry_Foreground*>(this->v_layers[1].get());
			layerFG->flush(*this->content, dims);

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
			return {288, 144};
		}

		virtual Point mapSize() const
		{
			return this->v_mapSize;
		}

		virtual Point tileSize() const
		{
			return {HH_TILE_WIDTH, HH_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, HH_DEFAULT_TILE);
		}

	private:
		std::unique_ptr<stream::inout> content;
		Point v_mapSize;
		uint8_t pal[768];
		uint8_t tileFlags[256];
};


std::string MapType_Harry::code() const
{
	return "map2d-harry";
}

std::string MapType_Harry::friendlyName() const
{
	return "Halloween Harry level";
}

std::vector<std::string> MapType_Harry::fileExtensions() const
{
	return {"gmf"};
}

std::vector<std::string> MapType_Harry::games() const
{
	return {
		"Alien Carnage",
		"Halloween Harry",
	};
}

MapType::Certainty MapType_Harry::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();
	// TESTED BY: fmt_map_harry_isinstance_c01
	if (lenMap < 29 + 768 + 256 + 10 + 2 + 4) return MapType::DefinitelyNo; // too short

	content.seekg(0, stream::start);

	// Check the signature
	char sig[0x12];
	content.read(sig, 0x12);
	// TESTED BY: fmt_map_harry_isinstance_c02
	if (strncmp(sig, "\x11SubZero Game File", 0x12) != 0) return MapType::DefinitelyNo;
	lenMap -= 0x12;

	// Skip flags
	content.seekg(11, stream::cur);
	lenMap -= 11;

	// Check palette is within range
	char pal[768];
	content.read(pal, 768);
	for (int i = 0; i < 768; i++) {
		// TESTED BY: fmt_map_harry_isinstance_c03
		if (pal[i] > 0x40) return MapType::DefinitelyNo;
	}
	lenMap -= 768;

	// Check tile flags are within range
	char tileFlags[256];
	content.read(tileFlags, 256);
	for (int i = 0; i < 256; i++) {
		// TESTED BY: fmt_map_harry_isinstance_c04
		if (tileFlags[i] > 0x01) return MapType::DefinitelyNo;
	}
	lenMap -= 256;

	// Skip unknown block
	content.seekg(10, stream::cur);
	lenMap -= 10;

	// isinstance_c01 should have prevented this
	assert(lenMap >= 6);

	uint16_t numActors;
	content >> u16le(numActors);
	lenMap -= 2;

	// TESTED BY: fmt_map_harry_isinstance_c05
	if (lenMap < (unsigned)(numActors * HH_ACTOR_LEN + 4)) return MapType::DefinitelyNo;

	content.seekg(numActors * HH_ACTOR_LEN, stream::cur);
	lenMap -= numActors * HH_ACTOR_LEN;

	assert(lenMap >= 4);
	uint16_t mapWidth, mapHeight;
	content >> u16le(mapWidth) >> u16le(mapHeight);
	lenMap -= 4;

	// TESTED BY: fmt_map_harry_isinstance_c06
	if (lenMap != (unsigned)(mapWidth * mapHeight * 2)) return MapType::DefinitelyNo;

	// TESTED BY: fmt_map_harry_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_Harry::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Harry::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_Harry>(std::move(content));
}

SuppFilenames MapType_Harry::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
