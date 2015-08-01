/**
 * @file  fmt-map-zone66.cpp
 * @brief MapType and Map2D implementation for Zone 66 levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Zone_66_Level_Format
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
#include "fmt-map-zone66.hpp"

/// Width of the map, in tiles
#define Z66_MAP_WIDTH  256

/// Height of the map, in tiles
#define Z66_MAP_HEIGHT 256

/// Length of the background layer, in bytes
#define Z66_LAYER_LEN_BG (Z66_MAP_WIDTH * Z66_MAP_HEIGHT)

/// Width of each tile, in pixels
#define Z66_TILE_WIDTH  32

/// Height of each tile, in pixels
#define Z66_TILE_HEIGHT 32

/// Map code to write for locations with no tile set
#define Z66_DEFAULT_BGTILE 0x00

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_Zone66_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Zone66_Background(stream::input& content, stream::input& tilemap)
		{
			// Read the background layer
			std::vector<uint8_t> bg(Z66_LAYER_LEN_BG, Z66_DEFAULT_BGTILE);
			auto amtRead = content.try_read(bg.data(), Z66_LAYER_LEN_BG);
			if (amtRead != Z66_LAYER_LEN_BG) {
				std::cout << "Warning: Zone 66 level file was "
					<< (Z66_LAYER_LEN_BG - amtRead)
					<< " bytes short - the last tiles will be left blank" << std::endl;
			}

			// Read the tile mapping table
			tilemap.seekg(0, stream::start);
			uint16_t lenTilemap, unknown;
			tilemap
				>> u16le(lenTilemap)
				>> u16le(unknown)
			;
			if (lenTilemap > 256) lenTilemap = 256;
			std::vector<unsigned int> tm(256, Z66_DEFAULT_BGTILE);
			for (unsigned int i = 0; i < lenTilemap; i++) {
				tilemap >> u16le(tm[i]);
			}

			this->v_allItems.reserve(Z66_LAYER_LEN_BG);
			for (unsigned int i = 0; i < Z66_LAYER_LEN_BG; i++) {
				if (bg[i] == Z66_DEFAULT_BGTILE) continue;

				Item t;
				t.type = Item::Type::Default;
				t.pos = {i % Z66_MAP_WIDTH, i / Z66_MAP_WIDTH};
				t.code = tm[bg[i]];
				this->v_allItems.push_back(t);
			}
		}

		void flush(stream::output& content, stream::output& tilemap)
		{
			// Write the background layer
			unsigned int numTileMappings = 0;
			unsigned int mapBG[256];

			std::vector<uint8_t> bg(Z66_LAYER_LEN_BG, Z66_DEFAULT_BGTILE);
			for (auto& i : this->items()) {
				if ((i.pos.x > Z66_MAP_WIDTH) || (i.pos.y > Z66_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}

				// Look for an existing tile mapping first
				bool found = false;
				for (unsigned int m = 0; m < numTileMappings; m++) {
					if (mapBG[m] == i.code) {
						bg[i.pos.y * Z66_MAP_WIDTH + i.pos.x] = m;
						found = true;
						break;
					}
				}
				if (!found) {
					// Have to add this tile to the mapping table
					if (numTileMappings >= 256) {
						throw stream::error("There are too many unique tiles in this level - "
							"Zone 66 only supports up to 256 different tiles in each level.  "
							"Please remove some tiles and try again.");
					}
					bg[i.pos.y * Z66_MAP_WIDTH + i.pos.x] = numTileMappings;
					mapBG[numTileMappings++] = i.code;
					/// @todo Use the correct "destroyed" tile code
					mapBG[numTileMappings++] = i.code;
				}
			}
			content.write(bg.data(), Z66_LAYER_LEN_BG);

			// Write the tile mapping table
			tilemap.seekp(0, stream::start);
			tilemap
				<< u16le(numTileMappings)
				<< u16le(0) /// @todo Animated tiles
#warning TODO: Animated tiles
			;
			for (unsigned int i = 0; i < numTileMappings; i++) {
				tilemap
					<< u16le(mapBG[i * 2])      // normal tile
					<< u16le(mapBG[i * 2 + 1])  // destroyed tile
				;
			}

#warning TODO: Write correct values for tile points/score
			/// @todo Write correct values for tile points/score
			tilemap << nullPadded("", numTileMappings);

#warning TODO: Write correct values for canDestroy flags
			/// @todo Write correct values for canDestroy flags
			tilemap << nullPadded("", numTileMappings);

			/// @todo Write animated tile info
#warning TODO: Write animated tile info

			tilemap.flush();

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
/// @todo Add all tiles instead of just ones already in the map, and rewrite the map on save
			for (unsigned int i = 0; i < 300; i++) {
				if (i == Z66_DEFAULT_BGTILE) continue;

				Item t;
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				validItems.push_back(t);
			}
			return validItems;
		}

};

class Map_Zone66: public MapCore, public Map2DCore
{
	public:
		Map_Zone66(std::unique_ptr<stream::inout> content,
			std::unique_ptr<stream::inout> tilemap)
			:	content(std::move(content)),
				tilemap(std::move(tilemap))
		{
			this->content->seekg(0, stream::start);

			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_Zone66_Background>(*this->content, *this->tilemap)
			);
		}

		virtual ~Map_Zone66()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
#warning TODO: Proper graphics filename
			return {};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 1);

			this->content->truncate(Z66_LAYER_LEN_BG);
			this->content->seekp(0, stream::start);

			// Write the background layer
			auto layerBG = dynamic_cast<Layer_Zone66_Background*>(this->v_layers[0].get());
			layerBG->flush(*this->content, *this->tilemap);

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
			return {320, 200};
		}

		virtual Point mapSize() const
		{
			return {Z66_MAP_WIDTH, Z66_MAP_HEIGHT};
		}

		virtual Point tileSize() const
		{
			return {Z66_TILE_WIDTH, Z66_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, Z66_DEFAULT_BGTILE);
		}

	private:
		std::unique_ptr<stream::inout> content;
		std::unique_ptr<stream::inout> tilemap;
};


std::string MapType_Zone66::code() const
{
	return "map2d-zone66";
}

std::string MapType_Zone66::friendlyName() const
{
	return "Zone 66 level";
}

std::vector<std::string> MapType_Zone66::fileExtensions() const
{
	return {"z66"};
}

std::vector<std::string> MapType_Zone66::games() const
{
	return {"Zone 66"};
}

MapType::Certainty MapType_Zone66::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_zone66_isinstance_c01
	if (lenMap != Z66_LAYER_LEN_BG) return MapType::DefinitelyNo;

	// TESTED BY: fmt_map_zone66_isinstance_c00
	return MapType::PossiblyYes;
}

std::unique_ptr<Map> MapType_Zone66::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Zone66::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	auto tilemap = suppData.find(SuppItem::Extra1);
	if (tilemap == suppData.end()) {
		throw stream::error("Missing content for layer: Extra1");
	}
	return std::make_unique<Map_Zone66>(std::move(content),
		std::move(tilemap->second));
}

SuppFilenames MapType_Zone66::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	SuppFilenames supps;
	supps[SuppItem::Extra1] = filename.substr(0, filename.length() - 4) + "dat.z66";
	return supps;
}

} // namespace gamemaps
} // namespace camoto
