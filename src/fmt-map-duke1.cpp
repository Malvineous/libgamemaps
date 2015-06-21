/**
 * @file  fmt-map-duke1.cpp
 * @brief MapType and Map2D implementation for Duke Nukem 1 levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Duke_1_Level_Format
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
#include "fmt-map-duke1.hpp"

#define DN1_MAP_WIDTH            128
#define DN1_MAP_HEIGHT           90
#define DN1_TILE_WIDTH           16
#define DN1_TILE_HEIGHT          16

#define DN1_LAYER_LEN_BG         (DN1_MAP_WIDTH * DN1_MAP_HEIGHT)
#define DN1_FILESIZE             (DN1_LAYER_LEN_BG * 2)

/// Map code to write for locations with no tile set.
#define DN1_DEFAULT_BGTILE     0x0000

/// This is the largest valid tile code in the background layer.
#define DN1_MAX_VALID_TILECODE 0xF000

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_Duke1_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Duke1_Background(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			// Read the background layer
			this->content->seekg(0, stream::start);
			this->v_allItems.reserve(DN1_LAYER_LEN_BG);
			Item t;
			for (unsigned int i = 0; i < DN1_LAYER_LEN_BG; i++) {
				t.type = Item::Type::Default;
				t.pos = {i % DN1_MAP_WIDTH, i / DN1_MAP_WIDTH};
				*this->content >> u16le(t.code);
				if (t.code != DN1_DEFAULT_BGTILE) this->v_allItems.push_back(t);
			}
		}

		virtual ~Layer_Duke1_Background()
		{
		}

		void flush()
		{
			// Write the background layer
			std::vector<uint16_t> bg(DN1_LAYER_LEN_BG, DN1_DEFAULT_BGTILE);
			for (auto& i : this->items()) {
				if ((i.pos.x >= DN1_MAP_WIDTH) || (i.pos.y >= DN1_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				bg[i.pos.y * DN1_MAP_WIDTH + i.pos.x] = i.code;
			}
			this->content->truncate(DN1_FILESIZE);
			this->content->seekp(0, stream::start);
			for (auto i : bg) {
				*this->content << u16le(i);
			}
			this->content->flush();
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
			unsigned int index = item.code / 32;
			unsigned int ts_num = index / 48;
			unsigned int ts_index = index % 48;

			ImageFromCodeInfo ret;

			ImagePurpose purpose = (ImagePurpose)((int)ImagePurpose::BackgroundTileset1 + ts_num);

			auto t = tileset.find(purpose);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			if (ts_index >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[ts_index]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i <= DN1_MAX_VALID_TILECODE; i++) {
				if (i == DN1_DEFAULT_BGTILE) continue;

				Item t;
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				validItems.push_back(t);
			}
			return validItems;
		}

	private:
		std::unique_ptr<stream::inout> content;
};

class Map_Duke1: public MapCore, public Map2DCore
{
	public:
		Map_Duke1(std::unique_ptr<stream::inout> content)
		{
			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_Duke1_Background>(std::move(content))
			);
		}

		virtual ~Map_Duke1()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {
				std::make_pair(
					ImagePurpose::BackgroundTileset1,
					GraphicsFilename{"back0.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::BackgroundTileset2,
					GraphicsFilename{"back1.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::BackgroundTileset3,
					GraphicsFilename{"back2.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::BackgroundTileset4,
					GraphicsFilename{"back3.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::BackgroundTileset5,
					GraphicsFilename{"solid0.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::BackgroundTileset6,
					GraphicsFilename{"solid1.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::BackgroundTileset7,
					GraphicsFilename{"solid2.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::BackgroundTileset8,
					GraphicsFilename{"solid3.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::BackgroundTileset9,
					GraphicsFilename{"border.dn1", "tls-ccaves-sub"}
				),
			};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 1);

			// Write the background layer
			auto layerBG = dynamic_cast<Layer_Duke1_Background*>(this->v_layers[0].get());
			layerBG->flush();

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
			return {13 * DN1_TILE_WIDTH, 10 * DN1_TILE_HEIGHT};
		}

		virtual Point mapSize() const
		{
			return {DN1_MAP_WIDTH, DN1_MAP_HEIGHT};
		}

		virtual Point tileSize() const
		{
			return {DN1_TILE_WIDTH, DN1_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, DN1_DEFAULT_BGTILE);
		}
};


std::string MapType_Duke1::code() const
{
	return "map-duke1";
}

std::string MapType_Duke1::friendlyName() const
{
	return "Duke Nukem 1 level";
}

std::vector<std::string> MapType_Duke1::fileExtensions() const
{
	return {"dn1", "dn2", "dn3"};
}

std::vector<std::string> MapType_Duke1::games() const
{
	return {"Duke Nukem 1"};
}

MapType::Certainty MapType_Duke1::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// TESTED BY: fmt_map_duke1_isinstance_c01
	if (lenMap != DN1_FILESIZE) return MapType::DefinitelyNo; // wrong size

	// Read in the layer and make sure all the tile codes are within range
	content.seekg(0, stream::start);
	for (unsigned int i = 0; i < DN1_LAYER_LEN_BG; i++) {
		uint16_t tile;
		content >> u16le(tile);
		// TESTED BY: fmt_map_duke1_isinstance_c02
		if (tile > DN1_MAX_VALID_TILECODE) return MapType::DefinitelyNo; // invalid tile
	}

	// TESTED BY: fmt_map_duke1_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_Duke1::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Duke1::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_Duke1>(std::move(content));
}

SuppFilenames MapType_Duke1::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
