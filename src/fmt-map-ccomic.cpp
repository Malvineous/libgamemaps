/**
 * @file  fmt-map-ccomic.cpp
 * @brief MapType and Map2D implementation for Captain Comic levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Captain_Comic_Map_Format
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
#include "fmt-map-ccomic.hpp"

#define CC_TILE_WIDTH           16
#define CC_TILE_HEIGHT          16

/// Map code to write for locations with no tile set.
#define CC_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define CC_MAX_VALID_TILECODE   87 // number of tiles in tileset

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_CComic_Background: public Map2DCore::LayerCore
{
	public:
		Layer_CComic_Background(stream::input& content, const Point& mapSize)
		{
			// Read the background layer
			auto mapLen = mapSize.x * mapSize.y;
			std::vector<uint8_t> bg;
			bg.resize(mapLen);
			content.read(bg.data(), mapLen);

			this->v_allItems.reserve(mapLen);
			for (unsigned int i = 0; i < mapLen; i++) {
				if (bg[i] == CC_DEFAULT_BGTILE) continue;

				Item t;
				t.type = Item::Type::Default;
				t.pos = {i % mapSize.x, i / mapSize.x};
				t.code = bg[i];
				this->v_allItems.push_back(t);
			}
		}

		virtual ~Layer_CComic_Background()
		{
		}

		void flush(stream::output& content, const Point& mapSize)
		{
			// Write the background layer
			unsigned int mapLen = mapSize.x * mapSize.y;

			std::vector<uint8_t> bg(mapLen, CC_DEFAULT_BGTILE);
			for (auto& i : this->v_allItems) {
				if ((i.pos.x >= mapSize.x) || (i.pos.y >= mapSize.y)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				bg[i.pos.y * mapSize.x + i.pos.x] = i.code;
			}
			assert(bg.size() == mapLen);
			content.write(bg.data(), mapLen);
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
			for (unsigned int i = 0; i <= CC_MAX_VALID_TILECODE; i++) {
				if (i == CC_DEFAULT_BGTILE) continue;

				Item t;
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				validItems.push_back(t);
			}
			return validItems;
		}
};

class Map_CComic: public MapCore, public Map2DCore
{
	public:
		Map_CComic(std::unique_ptr<stream::inout> content,
			std::unique_ptr<stream::inout> exe)
			:	content(std::move(content)),
				exe(std::move(exe))
		{
			this->content->seekg(0, stream::start);

			*this->content
				>> u16le(this->mapSizeTiles.x)
				>> u16le(this->mapSizeTiles.y)
			;

			this->exe->seekg(0, stream::start);
			*this->exe >> nullPadded(this->tilesetFilename, 14);

			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_CComic_Background>(*this->content, this->mapSizeTiles)
			);
		}

		virtual ~Map_CComic()
		{
		}

		virtual void flush()
		{
			this->content->seekp(0, stream::start);
			*this->content
				<< u16le(this->mapSizeTiles.x)
				<< u16le(this->mapSizeTiles.y)
			;
			auto layerBG = dynamic_cast<Layer_CComic_Background*>(this->v_layers[0].get());
			layerBG->flush(*this->content, this->mapSizeTiles);
			return;
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {
				std::make_pair(
					ImagePurpose::BackgroundTileset1,
					GraphicsFilename{this->tilesetFilename, "tls-ccomic"}
				),
			};
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
			return {193, 160};
		}

		virtual Point mapSize() const
		{
			return this->mapSizeTiles;
		}

		virtual Point tileSize() const
		{
			return {CC_TILE_WIDTH, CC_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
/// @todo Is the background really black, or does it use the first tile?
			Background bg;
			bg.att = Background::Attachment::SingleColour;
			bg.clr = gamegraphics::PaletteEntry{0, 0, 0, 255};
			return bg;
		}

	private:
		std::unique_ptr<stream::inout> content;
		std::unique_ptr<stream::inout> exe;
		Point mapSizeTiles;
		std::string tilesetFilename;
};

std::string MapType_CComic::code() const
{
	return "map2d-ccomic";
}

std::string MapType_CComic::friendlyName() const
{
	return "Captain Comic level";
}

std::vector<std::string> MapType_CComic::fileExtensions() const
{
	return {
		"pt",
	};
}

std::vector<std::string> MapType_CComic::games() const
{
	return {
		"Captain Comic",
	};
}

MapType::Certainty MapType_CComic::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_ccomic_isinstance_c01
	if (lenMap < 4) return MapType::DefinitelyNo;

	content.seekg(0, stream::start);
	unsigned int width, height;
	content
		>> u16le(width)
		>> u16le(height)
	;

	// Make sure the dimensions cover the entire file
	// TESTED BY: fmt_map_ccomic_isinstance_c02
	unsigned int mapLen = width * height;
	if (lenMap != mapLen + 4) return MapType::DefinitelyNo;

	// Read in the map and make sure all the tile codes are within range
	std::vector<uint8_t> bg;
	bg.resize(mapLen);
	stream::len r = content.try_read(bg.data(), mapLen);
	if (r != mapLen) return MapType::DefinitelyNo; // read error
	for (unsigned int i = 0; i < mapLen; i++) {
		// Make sure each tile is within range
		// TESTED BY: fmt_map_ccomic_isinstance_c03
		if (bg[i] > CC_MAX_VALID_TILECODE) {
			return MapType::DefinitelyNo;
		}
	}

	// TESTED BY: fmt_map_ccomic_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_CComic::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_CComic::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	auto exe = suppData.find(SuppItem::Extra1);
	if (exe == suppData.end()) {
		throw stream::error("Missing content for layer: Extra1");
	}
	return std::make_unique<Map_CComic>(std::move(content), std::move(exe->second));
}

SuppFilenames MapType_CComic::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {
		std::make_pair(
			SuppItem::Extra1,
			"comic.exe"
		),
	};
}

} // namespace gamemaps
} // namespace camoto
