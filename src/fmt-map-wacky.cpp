/**
 * @file  fmt-map-wacky.cpp
 * @brief MapType and Map2D implementation for Wacky Wheels levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Wacky_Wheels
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

#define _USE_MATH_DEFINES
#include <math.h>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp> // make_unique
#include "map-core.hpp"
#include "map2d-core.hpp"
#include "fmt-map-wacky.hpp"

#define WW_MAP_WIDTH            64
#define WW_MAP_HEIGHT           64
#define WW_TILE_WIDTH           32
#define WW_TILE_HEIGHT          32

#define WW_LAYER_OFF_BG         0
#define WW_LAYER_LEN_BG         (WW_MAP_WIDTH * WW_MAP_HEIGHT)
#define WW_FILESIZE             (WW_LAYER_OFF_BG + WW_LAYER_LEN_BG)

/// Map code to write for locations with no tile set.
#define WW_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define WW_MAX_VALID_TILECODE 0x6C

/// After this many tiles, go to the next tileset.
#define WW_TILES_PER_TILESET    54

/// Angle value equivalent to 0
#define WW_ANGLE_MAX          1920

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_Wacky_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Wacky_Background(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			// Read the background layer
			this->content->seekg(0, stream::start);
			std::vector<uint8_t> bg(WW_LAYER_LEN_BG, WW_DEFAULT_BGTILE);
			this->content->read(bg.data(), WW_LAYER_LEN_BG);

			this->v_allItems.reserve(WW_LAYER_LEN_BG);
			for (unsigned int i = 0; i < WW_LAYER_LEN_BG; i++) {
				if (bg[i] == WW_DEFAULT_BGTILE) continue;

				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();
				t.type = Item::Type::Default;
				t.pos.x = i % WW_MAP_WIDTH;
				t.pos.y = i / WW_MAP_WIDTH;
				t.code = bg[i];
			}
		}

		virtual ~Layer_Wacky_Background()
		{
		}

		void flush()
		{
			this->content->truncate(WW_LAYER_LEN_BG);
			this->content->seekp(0, stream::start);

			// Write the background layer
			std::vector<uint8_t> bg(WW_LAYER_LEN_BG, WW_DEFAULT_BGTILE);
			for (auto& i : this->items()) {
				if ((i.pos.x >= WW_MAP_WIDTH) || (i.pos.y >= WW_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				bg[i.pos.y * WW_MAP_WIDTH + i.pos.x] = i.code;
			}
			this->content->write(bg.data(), WW_LAYER_LEN_BG);
			return;
		}

		virtual std::string title() const
		{
			return "Surface";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			unsigned int ti = item.code / WW_TILES_PER_TILESET;
			unsigned int index = item.code % WW_TILES_PER_TILESET;

			ImagePurpose purpose = (ImagePurpose)((unsigned int)ImagePurpose::BackgroundTileset1 + ti);
			auto t = tileset.find(purpose);
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
			std::vector<Item> validItems;
			for (unsigned int i = 0; i <= WW_MAX_VALID_TILECODE; i++) {
				if (i == WW_DEFAULT_BGTILE) continue;

				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
			}
			return validItems;
		}

	private:
		std::unique_ptr<stream::inout> content;
};

class Map_Wacky: public MapCore, public Map2DCore
{
	public:
		Map_Wacky(std::unique_ptr<stream::inout> content,
			std::unique_ptr<stream::inout> compPath)
			:	compPath(std::move(compPath))
		{
			// Read the computer player paths
			this->compPath->seekg(0, stream::start);
			uint16_t numPoints;
			*this->compPath >> u16le(numPoints);

			auto path = std::make_shared<Map2D::Path>();
			path->fixed = false;
			path->forceClosed = false;
			path->maxPoints = 0; // no limit
			Point start;
			*this->compPath
				>> u16le(start.x)
				>> u16le(start.y)
			;
			path->start.push_back(start);
			Point next;
			for (unsigned int i = 0; i < numPoints; i++) {
				if (i > 0) this->compPath->seekg(4, stream::cur);
				*this->compPath
					>> u16le(next.x)
					>> u16le(next.y)
				;
				this->compPath->seekg(6, stream::cur);
				next.x -= start.x;
				next.y -= start.y;
				path->points.push_back(next);
			}
			this->v_paths.push_back(path);

			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_Wacky_Background>(std::move(content))
			);
		}

		virtual ~Map_Wacky()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			// Can't supply filenames as they change unpredictably for each level
			return {};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 1);
			assert(this->paths().size() == 1);
			if (this->v_paths[0]->start.size() < 1) throw stream::error("Path has no starting point!");
			if (this->v_paths[0]->start.size() > 1) throw stream::error("Path has too many starting points!");

			auto& path = this->v_paths[0];

			uint16_t count = (uint16_t)path->points.size();
			this->compPath->truncate(2 + count * 14);
			this->compPath->seekp(0, stream::start);

			*this->compPath << u16le(count);

			Point ptFirst = path->start[0];
			Point ptNext = ptFirst;
			Point ptDelta;
			for (auto& pt : path->points) {
				Point ptLast = ptNext;
				*this->compPath
					<< u16le(ptLast.x)
					<< u16le(ptLast.y)
				;
				ptNext.x = ptFirst.x + pt.x;
				ptNext.y = ptFirst.y + pt.y;
				ptDelta.x = ptNext.x - ptLast.x;
				ptDelta.y = ptNext.y - ptLast.y;
				unsigned int angle = WW_ANGLE_MAX
					+ atan2((double)ptDelta.y, (double)ptDelta.x)
					* (WW_ANGLE_MAX/2) / M_PI;
				angle %= WW_ANGLE_MAX;
				unsigned int image = (int)(angle / 240.0 + 6.5) % 8;
				unsigned int dist = sqrt((double)(ptDelta.x * ptDelta.x + ptDelta.y * ptDelta.y));
				*this->compPath
					<< u16le(ptNext.x)
					<< u16le(ptNext.y)
					<< u16le(angle)
					<< u16le(image)
					<< u16le(dist)
				;
			}

			this->compPath->flush();

			// Write the background layer
			auto layerBG = dynamic_cast<Layer_Wacky_Background*>(this->v_layers[0].get());
			layerBG->flush();

			return;
		}

		virtual Caps caps() const
		{
			return
				Map2D::Caps::Default
			;
		}

		virtual Point viewport() const
		{
			return {20 * WW_TILE_WIDTH, 10 * WW_TILE_HEIGHT};
		}

		virtual Point mapSize() const
		{
			return {WW_MAP_WIDTH, WW_MAP_HEIGHT};
		}

		virtual Point tileSize() const
		{
			return {WW_TILE_WIDTH, WW_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, WW_DEFAULT_BGTILE);
		}

	private:
		std::unique_ptr<stream::inout> compPath;
};


std::string MapType_Wacky::code() const
{
	return "map2d-wacky";
}

std::string MapType_Wacky::friendlyName() const
{
	return "Wacky Wheels level";
}

std::vector<std::string> MapType_Wacky::fileExtensions() const
{
	return {"m"};
}

std::vector<std::string> MapType_Wacky::games() const
{
	return {"Wacky Wheels"};
}

MapType::Certainty MapType_Wacky::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// TESTED BY: fmt_map_wacky_isinstance_c01
	if (lenMap != WW_FILESIZE) return MapType::DefinitelyNo; // wrong size

	// Read in the layer and make sure all the tile codes are within range
	std::vector<uint8_t> bg(WW_LAYER_LEN_BG);
	content.seekg(WW_LAYER_OFF_BG, stream::start);
	// This will throw an exception on a short read which is ok, because we
	// wouldn't be here if the file was too small anyway (isinstance_c01)
	content.read(bg.data(), WW_LAYER_LEN_BG);

	for (auto code : bg) {
		// TESTED BY: fmt_map_wacky_isinstance_c02
		if (code > WW_MAX_VALID_TILECODE) return MapType::DefinitelyNo; // invalid tile
	}

	// TESTED BY: fmt_map_wacky_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_Wacky::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	/// @todo Implement MapType_Wacky::create()
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Wacky::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	auto compPath = suppData.find(SuppItem::Layer1);
	if (compPath == suppData.end()) {
		throw stream::error("Missing content for layer: Layer1 (need *.rd file)");
	}
	return std::make_unique<Map_Wacky>(
		std::move(content), std::move(compPath->second)
	);
}

SuppFilenames MapType_Wacky::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	SuppFilenames supps;
	std::string baseName = filename.substr(0, filename.length() - 1);
	supps[SuppItem::Layer1] = baseName + "rd";
	return supps;
}


} // namespace gamemaps
} // namespace camoto
