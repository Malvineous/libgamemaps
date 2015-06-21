/**
 * @file  fmt-map-darkages.cpp
 * @brief MapType and Map2D implementation for Dark Ages levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Dark_Ages_Map_Format
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
#include "fmt-map-darkages.hpp"

#define DA_TILE_WIDTH           16
#define DA_TILE_HEIGHT          16

#define DA_MAP_WIDTH           128 ///< Width of map, in tiles
#define DA_MAP_HEIGHT            9 ///< Height of map, in tiles
#define DA_LAYER_LEN_BG         (DA_MAP_WIDTH * DA_MAP_HEIGHT)

/// Map code to write for locations with no tile set.
#define DA_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define DA_MAX_VALID_TILECODE  255 // number of tiles in tileset

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_DarkAges_Background: public Map2DCore::LayerCore
{
	public:
		Layer_DarkAges_Background(stream::input& content)
		{
			// Read the background layer
			std::vector<uint8_t> bg(DA_LAYER_LEN_BG, DA_DEFAULT_BGTILE);
			content.read(bg.data(), DA_LAYER_LEN_BG);

			this->v_allItems.reserve(DA_LAYER_LEN_BG);
			for (unsigned int i = 0; i < DA_LAYER_LEN_BG; i++) {
				Item t;
				t.type = Item::Type::Default;
				t.pos = {i % DA_MAP_WIDTH, i / DA_MAP_WIDTH};
				t.code = bg[i];
				if (t.code != DA_DEFAULT_BGTILE) this->v_allItems.push_back(t);
			}
		}

		virtual ~Layer_DarkAges_Background()
		{
		}

		void flush(stream::output& content)
		{
			// Write the background layer
			std::vector<uint8_t> bg(DA_LAYER_LEN_BG, DA_DEFAULT_BGTILE);
			for (auto& i : this->items()) {
				if ((i.pos.x >= DA_MAP_WIDTH) || (i.pos.y >= DA_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				bg[i.pos.y * DA_MAP_WIDTH + i.pos.x] = i.code;
			}

			content.write(bg.data(), bg.size());
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

			unsigned int tilesetIndex, imageIndex;
			switch (item.code) {
				// This mapping was borrowed from Frenkel's DAVIEW.BAS
				case 100: tilesetIndex = 3; imageIndex = 1; break;
				case 101: tilesetIndex = 3; imageIndex = 8; break;
				case 102: tilesetIndex = 3; imageIndex = 18; break;
				case 103: tilesetIndex = 3; imageIndex = 24; break;
				case 104: tilesetIndex = 3; imageIndex = 34; break;
				case 105: tilesetIndex = 3; imageIndex = 38; break;
				case 106: tilesetIndex = 3; imageIndex = 42; break;
				case 107: tilesetIndex = 3; imageIndex = 48; break;
				case 108: tilesetIndex = 4; imageIndex = 0; break;
				case 109: tilesetIndex = 4; imageIndex = 30; break;
				case 110: tilesetIndex = 4; imageIndex = 35; break;
				case 111: tilesetIndex = 4; imageIndex = 36; break;
				case 112: tilesetIndex = 4; imageIndex = 39; break;
				case 113: tilesetIndex = 4; imageIndex = 42; break;
				case 114: tilesetIndex = 4; imageIndex = 46; break;
				case 115: tilesetIndex = 5; imageIndex = 0; break;
				case 116: tilesetIndex = 5; imageIndex = 1; break;
				case 117: tilesetIndex = 5; imageIndex = 2; break;
				case 118: tilesetIndex = 5; imageIndex = 3; break;
				case 119: tilesetIndex = 5; imageIndex = 4; break;
				case 120: tilesetIndex = 3; imageIndex = 4; break;
				case 121: tilesetIndex = 3; imageIndex = 5; break;
				case 122: tilesetIndex = 3; imageIndex = 6; break;
				case 123: tilesetIndex = 3; imageIndex = 7; break;
				case 124: tilesetIndex = 5; imageIndex = 12; break;
				case 125: tilesetIndex = 5; imageIndex = 14; break;
				case 126: tilesetIndex = 5; imageIndex = 8; break;
				case 127: tilesetIndex = 5; imageIndex = 9; break;
				case 128: tilesetIndex = 5; imageIndex = 10; break;
				case 129: tilesetIndex = 5; imageIndex = 23; break;
				case 130: tilesetIndex = 5; imageIndex = 24; break;
				case 131: tilesetIndex = 5; imageIndex = 25; break;
				case 132: tilesetIndex = 2; imageIndex = 10; break;
				case 133: tilesetIndex = 5; imageIndex = 30; break;
				case 134: tilesetIndex = 5; imageIndex = 31; break;
				case 135: tilesetIndex = 5; imageIndex = 32; break;
				case 136: tilesetIndex = 5; imageIndex = 33; break;
				case 137: tilesetIndex = 5; imageIndex = 35; break;
				case 138: tilesetIndex = 5; imageIndex = 36; break;
				case 139: tilesetIndex = 5; imageIndex = 40; break;
				case 140: tilesetIndex = 3; imageIndex = 32; break;
				case 141: tilesetIndex = 5; imageIndex = 47; break;
				case 142: tilesetIndex = 3; imageIndex = 30; break;
				case 143: tilesetIndex = 6; imageIndex = 0; break;
				default:
					tilesetIndex = item.code / 50;
					imageIndex = item.code % 50;
					break;
			}

			auto& subtilesets = t->second->files();
			if (tilesetIndex >= subtilesets.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto ts = t->second->openTileset(subtilesets[tilesetIndex]);
			auto& images = ts->files();
			if (imageIndex >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = ts->openImage(images[imageIndex]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i <= DA_MAX_VALID_TILECODE; i++) {
				if (i == DA_DEFAULT_BGTILE) continue;

				Item t;
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				validItems.push_back(t);
			}
			return validItems;
		}
};

class Map_DarkAges: public MapCore, public Map2DCore
{
	public:
		Map_DarkAges(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			this->content->seekg(0, stream::start);

			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_DarkAges_Background>(*this->content)
			);
		}

		virtual ~Map_DarkAges()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
#warning Implement graphicsFilename()
			return {};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 1);

			this->content->truncate(DA_LAYER_LEN_BG);
			this->content->seekp(0, stream::start);
			auto layerBG = dynamic_cast<Layer_DarkAges_Background*>(this->v_layers[0].get());
			assert(layerBG);
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
			return {240, 144};
		}

		virtual Point mapSize() const
		{
			return {DA_MAP_WIDTH, DA_MAP_HEIGHT};
		}

		virtual Point tileSize() const
		{
			return {DA_TILE_WIDTH, DA_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, DA_DEFAULT_BGTILE);
		}

	private:
		std::unique_ptr<stream::inout> content;
};


std::string MapType_DarkAges::code() const
{
	return "map-darkages";
}

std::string MapType_DarkAges::friendlyName() const
{
	return "Dark Ages level";
}

std::vector<std::string> MapType_DarkAges::fileExtensions() const
{
	return {"dal"}; // made up, inside file05.da[123]
}

std::vector<std::string> MapType_DarkAges::games() const
{
	return {"Dark Ages"};
}

MapType::Certainty MapType_DarkAges::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// Wrong length
	// TESTED BY: fmt_map_darkages_isinstance_c01
	if (lenMap != DA_LAYER_LEN_BG) return MapType::DefinitelyNo;

	// TESTED BY: fmt_map_darkages_isinstance_c00
	return MapType::PossiblyYes;
}

std::unique_ptr<Map> MapType_DarkAges::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_DarkAges::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_DarkAges>(std::move(content));
}

SuppFilenames MapType_DarkAges::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
