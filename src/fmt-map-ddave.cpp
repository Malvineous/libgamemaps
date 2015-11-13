/**
 * @file  fmt-map-ddave.cpp
 * @brief MapType and Map2D implementation for Dangerous Dave levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DDave_Map_Format
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
#include "fmt-map-ddave.hpp"

#define DD_MAP_WIDTH            100
#define DD_MAP_HEIGHT           10
#define DD_TILE_WIDTH           16
#define DD_TILE_HEIGHT          16

#define DD_LAYER_OFF_PATH       0
#define DD_LAYER_LEN_PATH       256
#define DD_LAYER_OFF_BG         (DD_LAYER_OFF_PATH + DD_LAYER_LEN_PATH)
#define DD_LAYER_LEN_BG         (DD_MAP_WIDTH * DD_MAP_HEIGHT)
#define DD_PAD_LEN              24 // to round DD_LAYER_LEN_BG to nearest power of two
#define DD_FILESIZE             (DD_LAYER_LEN_PATH + DD_LAYER_LEN_BG + DD_PAD_LEN)

/// Map code to write for locations with no tile set.
#define DD_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define DD_MAX_VALID_TILECODE 52

/// This is the code used in both X and Y coords to terminate a path.
#define DD_PATH_END  0xEA

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_DDave_Background: public Map2DCore::LayerCore
{
	public:
		Layer_DDave_Background(stream::input& content)
		{
			// Read the background layer
			std::vector<uint8_t> bg(DD_LAYER_LEN_BG, DD_DEFAULT_BGTILE);
			content.read(bg.data(), DD_LAYER_LEN_BG);

			this->v_allItems.reserve(DD_LAYER_LEN_BG);
			for (unsigned int i = 0; i < DD_LAYER_LEN_BG; i++) {
				Item t;
				t.type = Item::Type::Default;
				t.pos.x = i % DD_MAP_WIDTH;
				t.pos.y = i / DD_MAP_WIDTH;
				t.code = bg[i];
				if (t.code != DD_DEFAULT_BGTILE) this->v_allItems.push_back(t);
			}
		}

		virtual ~Layer_DDave_Background()
		{
		}

		void flush(stream::output& content)
		{
			// Write the background layer
			std::vector<uint8_t> bg(DD_LAYER_LEN_BG, DD_DEFAULT_BGTILE);
			for (auto& i : this->items()) {
				if ((i.pos.x >= DD_MAP_WIDTH) || (i.pos.y >= DD_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				bg[i.pos.y * DD_MAP_WIDTH + i.pos.x] = i.code;
			}
			content.write(bg.data(), DD_LAYER_LEN_BG);
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
			for (unsigned int i = 0; i <= DD_MAX_VALID_TILECODE; i++) {
				if (i == DD_DEFAULT_BGTILE) continue;

				Item t;
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				validItems.push_back(t);
			}
			return validItems;
		}
};

class Map_DDave: public MapCore, public Map2DCore
{
	public:
		Map_DDave(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			this->content->seekg(0, stream::start);

			// Read the path
			uint8_t pathdata[DD_LAYER_LEN_PATH];
			this->content->read((char *)pathdata, DD_LAYER_LEN_PATH);
			auto path = std::make_shared<Map2D::Path>();
			path->fixed = true;
			path->forceClosed = false;
			path->maxPoints = 128;
			Point next{0, 0};
			for (unsigned int i = 0; i < DD_LAYER_LEN_PATH; i += 2) {
				if ((pathdata[i] == DD_PATH_END) && (pathdata[i+1] == DD_PATH_END)) break; // end of path
				next.x += (int8_t)pathdata[i];
				next.y += (int8_t)pathdata[i + 1];
				path->points.push_back(next);
			}
			// Hard-code the starting point and copies of each path
			unsigned int level = 3;
			switch (level) {
				case 3:
					path->start.push_back(Point{44 * DD_TILE_WIDTH, 4 * DD_TILE_HEIGHT});
					path->start.push_back(Point{59 * DD_TILE_WIDTH, 4 * DD_TILE_HEIGHT});
					break;
			}
			this->v_paths.push_back(path);

			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_DDave_Background>(*this->content)
			);
		}

		virtual ~Map_DDave()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {
				std::make_pair(
					ImagePurpose::BackgroundTileset1,
					GraphicsFilename{"vgadave.dav", "tls-ddave-vga"}
				),
			};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 1);
			assert(this->paths().size() == 1);

			this->content->truncate(DD_LAYER_LEN_PATH + DD_LAYER_LEN_BG + DD_PAD_LEN);
			this->content->seekp(0, stream::start);

			// Write the path
			uint8_t path[DD_LAYER_LEN_PATH];
			memset(path, 0, DD_LAYER_LEN_PATH);
			auto& first_path = this->paths().at(0);
			unsigned int pathpos = 0;
			unsigned int lastX = 0, lastY = 0;
			int8_t x = 0, y = 0;
			for (const auto& i : first_path->points) {
				if (pathpos > 256) throw stream::error("Path too long (max 128 segments)");

				// Convert from relative to (0,0), to relative to previous point
				// Have to cast these to int8_t first so they're 8-bit but the sign is kept
				// intact (-1 still is -1) then to uint8_t so -1 becomes 255.
				x = i.x - lastX;
				y = i.y - lastY;
				lastX = i.x;
				lastY = i.y;

				if (((uint8_t)x == DD_PATH_END) && ((uint8_t)y == DD_PATH_END)) {
					// Can't write these magic values, so tweak the data slightly
					lastY++;
					y++;
					// This should work fine unless this is the last point in the path, but
					// the condition below will catch that.
				}
				path[pathpos++] = (uint8_t)x;
				path[pathpos++] = (uint8_t)y;
			}
			if (((uint8_t)x == DD_PATH_END) && ((uint8_t)y == DD_PATH_END)) {
				throw stream::error("The last point in the path happens to have a "
					"special magic offset that cannot be saved in a Dangerous Dave map.  "
					"Please move the last or second last point by at least one pixel.");
			}

			// Add terminator if there's enough room.
/// @todo Test to see if this is correct, or if a terminator is always required
			if (pathpos < 256) {
				path[pathpos++] = DD_PATH_END;
				path[pathpos++] = DD_PATH_END;
			}
			this->content->write((char *)path, DD_LAYER_LEN_PATH);

			// Write the background layer
			auto layerBG = dynamic_cast<Layer_DDave_Background*>(this->v_layers[0].get());
			layerBG->flush(*this->content);

			// Write out padding
			uint8_t pad[DD_PAD_LEN];
			memset(pad, 0, DD_PAD_LEN);
			this->content->write(pad, DD_PAD_LEN);

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
			return {20 * DD_TILE_WIDTH, 10 * DD_TILE_HEIGHT};
		}

		virtual Point mapSize() const
		{
			return {DD_MAP_WIDTH, DD_MAP_HEIGHT};
		}

		virtual Point tileSize() const
		{
			return {DD_TILE_WIDTH, DD_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, DD_DEFAULT_BGTILE);
		}

	private:
		std::unique_ptr<stream::inout> content;
};


std::string MapType_DDave::code() const
{
	return "map2d-ddave";
}

std::string MapType_DDave::friendlyName() const
{
	return "Dangerous Dave level";
}

std::vector<std::string> MapType_DDave::fileExtensions() const
{
	return {"dav"};
}

std::vector<std::string> MapType_DDave::games() const
{
	return {"Dangerous Dave"};
}

MapType::Certainty MapType_DDave::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// Wrong size
	// TESTED BY: fmt_map_ddave_isinstance_c01
	if (lenMap != DD_FILESIZE) return MapType::DefinitelyNo;

	// Read in the layer and make sure all the tile codes are within range
	std::vector<uint8_t> bg(DD_LAYER_LEN_BG, DD_DEFAULT_BGTILE);
	content.seekg(DD_LAYER_OFF_BG, stream::start);
	stream::len r = content.try_read(bg.data(), DD_LAYER_LEN_BG);
	if (r != DD_LAYER_LEN_BG) return MapType::DefinitelyNo; // read error
	for (unsigned int i = 0; i < DD_LAYER_LEN_BG; i++) {
		// Invalid tile
		// TESTED BY: fmt_map_ddave_isinstance_c02
		if (bg[i] > DD_MAX_VALID_TILECODE) return MapType::DefinitelyNo;
	}

	// TESTED BY: fmt_map_ddave_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_DDave::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_DDave::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_DDave>(std::move(content));
}

SuppFilenames MapType_DDave::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
