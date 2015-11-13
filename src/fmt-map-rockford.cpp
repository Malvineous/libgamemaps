/**
 * @file  fmt-map-rockford.cpp
 * @brief MapType and Map2D implementation for Rockford levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Rockford
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
#include "fmt-map-rockford.hpp"

/// Width of a tile, in pixels.
#define RF_TILE_WIDTH         16

/// Height of a tile, in pixels.
#define RF_TILE_HEIGHT        16

/// Width of a map, in tiles.
#define RF_MAP_WIDTH          40

/// Height of a map, in tiles.
#define RF_MAP_HEIGHT         22

/// Length of background layer, in bytes/tiles
#define RF_LAYER_LEN_BG       (RF_MAP_WIDTH * RF_MAP_HEIGHT)

/// Map code to write for locations with no tile set.
#define RF_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define RF_MAX_VALID_TILECODE (10*20) // number of tiles in tileset

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_Rockford_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Rockford_Background(stream::input& content)
		{
			// Read the background layer
			std::vector<uint8_t> bg(RF_LAYER_LEN_BG, RF_DEFAULT_BGTILE);
			content.read(bg.data(), RF_LAYER_LEN_BG);

			this->v_allItems.reserve(RF_LAYER_LEN_BG);
			for (unsigned int i = 0; i < RF_LAYER_LEN_BG; i++) {
				Item t;
				t.type = Item::Type::Default;
				t.pos.x = i % RF_MAP_WIDTH;
				t.pos.y = i / RF_MAP_WIDTH;
				t.code = bg[i];
				if (t.code != RF_DEFAULT_BGTILE) this->v_allItems.push_back(t);
			}
		}

		void flush(stream::output& content)
		{
			// Write the background layer
			std::vector<uint8_t> bg(RF_LAYER_LEN_BG, RF_DEFAULT_BGTILE);
			for (auto& i : this->items()) {
				if ((i.pos.x > RF_MAP_WIDTH) || (i.pos.y > RF_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				bg[i.pos.y * RF_MAP_WIDTH + i.pos.x] = i.code;
			}
			content.write(bg.data(), RF_LAYER_LEN_BG);
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

			unsigned int index = item.code;
			// Special case for one image!
			if (index == 3) index++;

			auto t = tileset.find(ImagePurpose::BackgroundTileset1);
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
			std::vector<uint8_t> validItemCodes = {
				0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
				0x08, 0x09, 0x0A, 0x0B, 0x0C,
				0x10,
				0x28, 0x2C, 0x2D, 0x2E,
				0x30, 0x34, 0x35, 0x36, 0x37,
				0x38,
				0x53,
				0x70, 0x74, 0x7C,
				0x80, 0x82, 0x84, 0x88,
				0xC4,
			};
			std::vector<Item> validItems;
			for (auto i : validItemCodes) {
				if (i == RF_DEFAULT_BGTILE) continue;

				Item t;
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				validItems.push_back(t);
			}
			return validItems;
		}
};

class Map_Rockford: public MapCore, public Map2DCore
{
	public:
		Map_Rockford(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			this->content->seekg(0, stream::start);

			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_Rockford_Background>(*this->content)
			);
		}

		virtual ~Map_Rockford()
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
					GraphicsFilename{"", ""}
				),
			};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 1);

			this->content->truncate(RF_LAYER_LEN_BG);
			this->content->seekp(0, stream::start);

			// Write the background layer
			auto layerBG = dynamic_cast<Layer_Rockford_Background*>(this->v_layers[0].get());
			layerBG->flush(*this->content);

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
			return {320, 176};
		}

		virtual Point mapSize() const
		{
			return {RF_MAP_WIDTH, RF_MAP_HEIGHT};
		}

		virtual Point tileSize() const
		{
			return {RF_TILE_WIDTH, RF_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, RF_DEFAULT_BGTILE);
		}

	private:
		std::unique_ptr<stream::inout> content;
};


std::string MapType_Rockford::code() const
{
	return "map2d-rockford";
}

std::string MapType_Rockford::friendlyName() const
{
	return "Rockford level";
}

std::vector<std::string> MapType_Rockford::fileExtensions() const
{
	return {"bin"};
}

std::vector<std::string> MapType_Rockford::games() const
{
	return {"Rockford"};
}

MapType::Certainty MapType_Rockford::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// Wrong size
	// TESTED BY: fmt_map_rockford_isinstance_c01
	if (lenMap != RF_LAYER_LEN_BG) return MapType::DefinitelyNo;

	// Read in the layer and make sure all the tile codes are within range
	std::vector<uint8_t> bg(RF_LAYER_LEN_BG, RF_DEFAULT_BGTILE);
	content.seekg(0, stream::start);
	stream::len r = content.try_read(bg.data(), RF_LAYER_LEN_BG);
	if (r != RF_LAYER_LEN_BG) return MapType::DefinitelyNo; // read error
	for (unsigned int i = 0; i < RF_LAYER_LEN_BG; i++) {
		// Invalid tile
		// TESTED BY: fmt_map_rockford_isinstance_c02
		if (bg[i] > RF_MAX_VALID_TILECODE) return MapType::DefinitelyNo;
	}

	// TESTED BY: fmt_map_rockford_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_Rockford::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Rockford::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_Rockford>(std::move(content));
}

SuppFilenames MapType_Rockford::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
