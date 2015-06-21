/**
 * @file  fmt-map-hocus.cpp
 * @brief MapType and Map2D implementation for Hocus Pocus.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Hocus_Pocus
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
#include "fmt-map-hocus.hpp"

/// Width of each tile in pixels
#define HP_TILE_WIDTH 16

/// Height of each tile in pixels
#define HP_TILE_HEIGHT 16

/// Width of each map in tiles
#define HP_MAP_WIDTH 240

/// Height of each map in tiles
#define HP_MAP_HEIGHT 60

/// Number of grid cells in the map
#define HP_MAP_SIZE  (HP_MAP_WIDTH * HP_MAP_HEIGHT)

/// Map code used for 'no tile' in background and foreground layers
#define HP_DEFAULT_TILE 0xFF

/// Largest valid tilecode in bg/fg layers
#warning TODO: Confirm this value is correct
#define HP_MAX_VALID_TILECODE 0xFF

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_Hocus_8bit: public Map2DCore::LayerCore
{
	public:
		Layer_Hocus_8bit(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			// Read the background layer
			std::vector<uint8_t> bg(HP_MAP_SIZE, HP_DEFAULT_TILE);
			this->content->seekg(0, stream::start);
			this->content->read(bg.data(), HP_MAP_SIZE);

			this->v_allItems.reserve(HP_MAP_SIZE);
			for (unsigned int i = 0; i < HP_MAP_SIZE; i++) {
				Item t;
				t.type = Item::Type::Default;
				t.pos = {i % HP_MAP_WIDTH, i / HP_MAP_WIDTH};
				t.code = bg[i];
				if (t.code != HP_DEFAULT_TILE) this->v_allItems.push_back(t);
			}
		}

		virtual ~Layer_Hocus_8bit()
		{
		}

		void flush()
		{
			// Write the background layer
			std::vector<uint8_t> bg(HP_MAP_SIZE, HP_DEFAULT_TILE);
			for (auto& i : this->items()) {
				if ((i.pos.x >= HP_MAP_WIDTH) || (i.pos.y >= HP_MAP_HEIGHT)) {
					throw stream::error("Layer has tiles outside map boundary!");
				}
				bg[i.pos.y * HP_MAP_WIDTH + i.pos.x] = i.code;
			}
			this->content->truncate(HP_MAP_SIZE);
			this->content->seekp(0, stream::start);
			this->content->write(bg.data(), HP_MAP_SIZE);
			this->content->flush();
			return;
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
			for (unsigned int i = 0; i <= HP_MAX_VALID_TILECODE; i++) {
				if (i == HP_DEFAULT_TILE) continue;

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

class Layer_Hocus_Background: public Layer_Hocus_8bit
{
	public:
		Layer_Hocus_Background(std::unique_ptr<stream::inout> content)
			:	Layer_Hocus_8bit(std::move(content))
		{
		}

		virtual ~Layer_Hocus_Background()
		{
		}

		virtual std::string title() const
		{
			return "Background";
		}
};

class Layer_Hocus_Foreground: public Layer_Hocus_8bit
{
	public:
		Layer_Hocus_Foreground(std::unique_ptr<stream::inout> content)
			:	Layer_Hocus_8bit(std::move(content))
		{
		}

		virtual ~Layer_Hocus_Foreground()
		{
		}

		virtual std::string title() const
		{
			return "Foreground";
		}
};

class Map_Hocus: public MapCore, public Map2DCore
{
	public:
		Map_Hocus(std::unique_ptr<stream::inout> bg, std::unique_ptr<stream::inout> fg)
		{
			// Read the background layer
			this->v_layers.push_back(
				std::make_shared<Layer_Hocus_Background>(std::move(bg))
			);
			this->v_layers.push_back(
				std::make_shared<Layer_Hocus_Foreground>(std::move(fg))
			);
		}

		virtual ~Map_Hocus()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 2);

			// Write the background layer
			auto layerBG = dynamic_cast<Layer_Hocus_Background*>(this->v_layers[0].get());
			layerBG->flush();

			// Write the foreground layer
			auto layerFG = dynamic_cast<Layer_Hocus_Foreground*>(this->v_layers[1].get());
			layerFG->flush();

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
			return {320, 160};
		}

		virtual Point mapSize() const
		{
			return {HP_MAP_WIDTH, HP_MAP_HEIGHT};
		}

		virtual Point tileSize() const
		{
			return {HP_TILE_WIDTH, HP_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			Background bg;
			// Use no background until we work out how to find the background image
			bg.att = Background::Attachment::NoBackground;
			return bg;
		}
};


std::string MapType_Hocus::code() const
{
	return "map-hocus";
}

std::string MapType_Hocus::friendlyName() const
{
	return "Hocus Pocus level";
}

std::vector<std::string> MapType_Hocus::fileExtensions() const
{
	return {};
}

std::vector<std::string> MapType_Hocus::games() const
{
	return {"Hocus Pocus"};
}

MapType::Certainty MapType_Hocus::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();
	// TESTED BY: fmt_map_hocus_isinstance_c01
	if (lenMap != 14400) return MapType::DefinitelyNo; // wrong size

	// TESTED BY: fmt_map_hocus_isinstance_c00
	return MapType::PossiblyYes;
}

std::unique_ptr<Map> MapType_Hocus::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Hocus::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	auto layer1 = suppData.find(SuppItem::Layer1);
	if (layer1 == suppData.end()) {
		throw stream::error("Missing content for layer: Layer1");
	}
	return std::make_unique<Map_Hocus>(std::move(content), std::move(layer1->second));
}

SuppFilenames MapType_Hocus::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
