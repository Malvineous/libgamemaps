/**
 * @file  fmt-map-vinyl.cpp
 * @brief MapType and Map2D implementation for Vinyl Goddess From Mars levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/VGFM_Level_Format
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
#include "fmt-map-vinyl.hpp"

#define VGFM_TILE_WIDTH             16
#define VGFM_TILE_HEIGHT            16

/// This is the largest valid tile code in the background layer.
#define VGFM_MAX_VALID_BGTILECODE  481 // number of tiles in tileset

/// This is the largest valid tile code in the foreground layer.
#define VGFM_MAX_VALID_FGTILECODE  255 // limit of 8-bit byte

/// Value used where no tile should appear in the foreground layer.
#define VGFM_DEFAULT_TILE_FG      0x00

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_Vinyl_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Vinyl_Background(stream::input& content, unsigned long mapWidth,
			unsigned long mapHeight)
		{
			auto lenLayer = mapWidth * mapHeight;
			this->v_allItems.reserve(lenLayer);
			for (unsigned int i = 0; i < lenLayer; i++) {
				uint16_t code;
				content >> u16le(code);

				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				t.pos.x = i % mapWidth;
				t.pos.y = i / mapWidth;
				t.code = code;
			}
		}

		void flush(stream::output& content, unsigned long mapWidth,
			unsigned long mapHeight)
		{
			std::vector<uint16_t> grid(mapWidth * mapHeight, 0x00);
			for (auto& i : this->items()) {
				if ((i.pos.x >= (long)mapWidth) || (i.pos.y >= (long)mapHeight)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				grid[i.pos.y * mapWidth + i.pos.x] = i.code;
			}
			for (auto& i : grid) {
				content << u16le(i);
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
			std::vector<Item> items;
			for (unsigned int i = 0; i <= VGFM_MAX_VALID_FGTILECODE; i++) {
				items.emplace_back();
				auto& t = items.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return items;
		}
};

class Layer_Vinyl_Foreground: public Map2DCore::LayerCore
{
	public:
		Layer_Vinyl_Foreground(stream::input& content, unsigned long mapWidth,
			unsigned long mapHeight)
		{
			auto lenLayer = mapWidth * mapHeight;
			this->v_allItems.reserve(lenLayer);
			for (unsigned int i = 0; i < lenLayer; i++) {
				uint8_t code;
				content >> u8(code);
				if (code == VGFM_DEFAULT_TILE_FG) continue;

				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();

				t.type = Item::Type::Default;
				t.pos.x = i % mapWidth;
				t.pos.y = i / mapWidth;
				t.code = code;
			}
		}

		void flush(stream::output& content, unsigned long mapWidth,
			unsigned long mapHeight)
		{
			std::vector<uint8_t> grid(mapWidth * mapHeight, VGFM_DEFAULT_TILE_FG);
			for (auto& i : this->items()) {
				if ((i.pos.x >= (long)mapWidth) || (i.pos.y >= (long)mapHeight)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				grid[i.pos.y * mapWidth + i.pos.x] = i.code;
			}
			content.write(grid.data(), grid.size());
			return;
		}

		virtual std::string title() const
		{
			return "Foreground";
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
			std::vector<Item> items;
			for (unsigned int i = 0; i <= VGFM_MAX_VALID_FGTILECODE; i++) {
				// The default tile actually has an image, so don't exclude it
				if (i == VGFM_DEFAULT_TILE_FG) continue;

				items.emplace_back();
				auto& t = items.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return items;
		}
};


class Map_Vinyl: public MapCore, public Map2DCore
{
	public:
		Map_Vinyl(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			assert(this->content);

			this->content->seekg(0, stream::start);
			*this->content
				>> u16le(this->mapHeight)
				>> u16le(this->mapWidth)
			;

			// Read the background layer
			this->v_layers.push_back(std::make_shared<Layer_Vinyl_Background>(
				*this->content,
				this->mapWidth,
				this->mapHeight
			));

			// Read the foreground layer
			this->v_layers.push_back(std::make_shared<Layer_Vinyl_Foreground>(
				*this->content,
				this->mapWidth,
				this->mapHeight
			));
		}

		virtual ~Map_Vinyl()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {};
		}

		virtual void flush()
		{
			this->content->seekp(0, stream::start);
			*this->content
				<< u16le(this->mapHeight)
				<< u16le(this->mapWidth)
			;

			auto layerBG = dynamic_cast<Layer_Vinyl_Background*>(this->v_layers[0].get());
			layerBG->flush(*this->content, this->mapWidth, this->mapHeight);

			auto layerFG = dynamic_cast<Layer_Vinyl_Foreground*>(this->v_layers[1].get());
			layerFG->flush(*this->content, this->mapWidth, this->mapHeight);

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
			return {320, 159};
		}

		virtual Point mapSize() const
		{
			return {(signed long)this->mapWidth, (signed long)this->mapHeight};
		}

		virtual Point tileSize() const
		{
			return {VGFM_TILE_WIDTH, VGFM_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			Background bg;
			bg.att = Background::Attachment::SingleColour;
			bg.clr = gamegraphics::PaletteEntry{0, 0, 0, 255};
			return bg;
		}

	private:
		std::unique_ptr<stream::inout> content;
		unsigned long mapWidth;
		unsigned long mapHeight;
};


std::string MapType_Vinyl::code() const
{
	return "map2d-vinyl";
}

std::string MapType_Vinyl::friendlyName() const
{
	return "Vinyl Goddess From Mars level";
}

std::vector<std::string> MapType_Vinyl::fileExtensions() const
{
	return {
		"m",
	};
}

std::vector<std::string> MapType_Vinyl::games() const
{
	return {
		"Vinyl Goddess From Mars",
	};
}

MapType::Certainty MapType_Vinyl::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_vinyl_isinstance_c01
	if (lenMap < 4) return MapType::DefinitelyNo;

	content.seekg(0, stream::start);
	unsigned int width, height;
	content >> u16le(height) >> u16le(width);

	// Make sure the dimensions cover the entire file
	// TESTED BY: fmt_map_vinyl_isinstance_c02
	unsigned int expLen = 4 + width * height * 3; // 3 = uint16 bg + uint8 fg
	if (lenMap != expLen) return MapType::DefinitelyNo;

	// Read in the map and make sure all the tile codes are within range
	for (unsigned int i = 0; i < width * height; i++) {
		// Make sure each tile is within range
		// TESTED BY: fmt_map_vinyl_isinstance_c03
		uint16_t code;
		try {
			content >> u16le(code);
		} catch (...) {
			return MapType::DefinitelyNo;
		}
		if (code > VGFM_MAX_VALID_BGTILECODE) {
			return MapType::DefinitelyNo;
		}
	}

	// TESTED BY: fmt_map_vinyl_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_Vinyl::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Vinyl::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_Vinyl>(std::move(content));
}

SuppFilenames MapType_Vinyl::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
