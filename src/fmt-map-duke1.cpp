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
#include <camoto/gamegraphics/util.hpp>
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
				t.pos.x = i % DN1_MAP_WIDTH;
				t.pos.y = i / DN1_MAP_WIDTH;
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
			ImagePurpose purpose = (ImagePurpose)((int)ImagePurpose::BackgroundTileset1 + ts_num);

			ImageFromCodeInfo ret;
			int box = 0; // 1=grey,2=blue,3=red,4=door
			if (item.code >= 0x3000) {
				switch (item.code) {
					case 0x3000: purpose = ImagePurpose::ForegroundTileset1; ts_index = 0; break;
					case 0x3001: purpose = ImagePurpose::ForegroundTileset1; ts_index = 5; break;
//case 0x3002: Left-moving conveyor belt start, must be left of end tile
//case 0x3003: Left-moving conveyor belt end, must be on right of start tile
//case 0x3004: Right-moving conveyor belt start, must be left of end tile
//case 0x3005: Right-moving conveyor belt end, must be on right of start tile
					case 0x3006: purpose = ImagePurpose::ForegroundTileset1; ts_index = 10; box = 1; break;

					/// @todo: Draw whole rocket instead of just the nosecone
					case 0x3007: purpose = ImagePurpose::ForegroundTileset1; ts_index = 11; break;

					/// @todo: Draw whole flame
					case 0x3008: purpose = ImagePurpose::ForegroundTileset1; ts_index = 24; break;
					case 0x3009: purpose = ImagePurpose::ForegroundTileset1; ts_index = 18; box = 1; break;

					/// @todo: Draw whole flame
					case 0x300A: purpose = ImagePurpose::ForegroundTileset1; ts_index = 29; break;
					case 0x300B: purpose = ImagePurpose::SpriteTileset1; ts_index = 0; break;
					case 0x300C: purpose = ImagePurpose::SpriteTileset1; ts_index = 10; break;
					case 0x300D: purpose = ImagePurpose::SpriteTileset1; ts_index = 34; break;
					case 0x300E: purpose = ImagePurpose::SpriteTileset2; ts_index = 0; break;
					case 0x300F: purpose = ImagePurpose::ForegroundTileset1; ts_index = 43; box = 1; break;
					case 0x3010: purpose = ImagePurpose::SpriteTileset2; ts_index = 32; break;
//case 0x3011: Unused, shows as blank in a level {{TODO|Confirm it doesn't to something special if placed next to another tile}}
					case 0x3012: purpose = ImagePurpose::SpriteTileset3; ts_index = 17; box = 1; break;
					case 0x3013: purpose = ImagePurpose::SpriteTileset3; ts_index = 24; break;
//case 0x3014: Shimmering water effect, also affects tile below this one
					case 0x3015: purpose = ImagePurpose::SpriteTileset3; ts_index = 32; box = 3; break;
					case 0x3016: purpose = ImagePurpose::SpriteTileset3; ts_index = 40; break;
					case 0x3017: purpose = ImagePurpose::SpriteTileset3; ts_index = 44; break;
					case 0x3018: purpose = ImagePurpose::ForegroundTileset1; ts_index = 44; box = 3; break;
					case 0x3019: purpose = ImagePurpose::SpriteTileset4; ts_index = 0; break;
					case 0x301A: purpose = ImagePurpose::ForegroundTileset2; ts_index = 0; break;
					case 0x301B: purpose = ImagePurpose::SpriteTileset4; ts_index = 12; break;
					case 0x301C: purpose = ImagePurpose::SpriteTileset4; ts_index = 12; break;
					case 0x301D: purpose = ImagePurpose::ForegroundTileset2; ts_index = 8; box = 2; break;
					case 0x301E: purpose = ImagePurpose::ForegroundTileset2; ts_index = 9; box = 2; break;
					case 0x301F: purpose = ImagePurpose::ForegroundTileset2; ts_index = 10; box = 2; break;
					case 0x3020: purpose = ImagePurpose::ForegroundTileset2; ts_index = 13; box = 2; break;
					case 0x3021: purpose = ImagePurpose::ForegroundTileset2; ts_index = 15; break;
					case 0x3022: purpose = ImagePurpose::SpriteTileset4; ts_index = 20; break;
					case 0x3023: purpose = ImagePurpose::ForegroundTileset2; ts_index = 19; break;
					case 0x3024: purpose = ImagePurpose::SpriteTileset5; ts_index = 8; break;
					case 0x3025: purpose = ImagePurpose::SpriteTileset5; ts_index = 11; break;
					case 0x3026: purpose = ImagePurpose::SpriteTileset5; ts_index = 12; break;
					case 0x3027: purpose = ImagePurpose::SpriteTileset5; ts_index = 13; break;
					case 0x3028: purpose = ImagePurpose::SpriteTileset5; ts_index = 14; break;
					case 0x3029: purpose = ImagePurpose::ForegroundTileset2; ts_index = 24; box = 1; break;
					case 0x302A: purpose = ImagePurpose::ForegroundTileset2; ts_index = 33; break;
					case 0x302B: purpose = ImagePurpose::ForegroundTileset2; ts_index = 34; break;
					case 0x302C: purpose = ImagePurpose::ForegroundTileset2; ts_index = 45; break;
					case 0x302D: purpose = ImagePurpose::ForegroundTileset2; ts_index = 47; box = 2; break;
					case 0x302E: purpose = ImagePurpose::ForegroundTileset3; ts_index = 2; box = 2; break;
					case 0x302F: purpose = ImagePurpose::SpriteTileset5; ts_index = 20; break;
					case 0x3030: purpose = ImagePurpose::SpriteTileset5; ts_index = 20; break;
					case 0x3031: purpose = ImagePurpose::SpriteTileset5; ts_index = 31; break;
//case 0x3032: Player start point
					case 0x3033: purpose = ImagePurpose::ForegroundTileset2; ts_index = 14; box = 1; break;
					case 0x3034: purpose = ImagePurpose::ForegroundTileset3; ts_index = 5; break;
					case 0x3035: purpose = ImagePurpose::ForegroundTileset3; ts_index = 14; break;
//case 0x3036: Red girder - does this get removed when something is activated?
					case 0x3037: purpose = ImagePurpose::ForegroundTileset3; ts_index = 21; box = 1; break;
					case 0x3038: purpose = ImagePurpose::ForegroundTileset3; ts_index = 21; box = 1; break;
					case 0x3039: purpose = ImagePurpose::ForegroundTileset3; ts_index = 21; box = 1; break;
					case 0x303A: purpose = ImagePurpose::ForegroundTileset3; ts_index = 21; box = 1; break;
					case 0x303B: purpose = ImagePurpose::SpriteTileset3; ts_index = 18; break;
					case 0x303C: purpose = ImagePurpose::SpriteTileset6; ts_index = 1; break;
					case 0x303D: purpose = ImagePurpose::SpriteTileset6; ts_index = 12; break;
					case 0x303E: purpose = ImagePurpose::SpriteTileset6; ts_index = 13; break;
					case 0x303F: purpose = ImagePurpose::SpriteTileset6; ts_index = 14; break;
					case 0x3040: purpose = ImagePurpose::ForegroundTileset3; ts_index = 23; break;
//case 0x3041: Unknown - behaviour seems to change depending on tile before it
					case 0x3042: purpose = ImagePurpose::SpriteTileset6; ts_index = 30; break;
					case 0x3043: purpose = ImagePurpose::SpriteTileset6; ts_index = 30; break;
					case 0x3044: purpose = ImagePurpose::ForegroundTileset3; ts_index = 24; break;
					case 0x3045: purpose = ImagePurpose::ForegroundTileset3; ts_index = 25; break;
					case 0x3046: purpose = ImagePurpose::ForegroundTileset3; ts_index = 26; break;
					case 0x3047: purpose = ImagePurpose::ForegroundTileset3; ts_index = 27; break;
					case 0x3048: purpose = ImagePurpose::ForegroundTileset3; ts_index = 37; break;
					case 0x3049: purpose = ImagePurpose::ForegroundTileset3; ts_index = 38; break;
					case 0x304A: purpose = ImagePurpose::ForegroundTileset3; ts_index = 39; break;
					case 0x304B: purpose = ImagePurpose::ForegroundTileset3; ts_index = 40; break;
					case 0x304C: purpose = ImagePurpose::ForegroundTileset3; ts_index = 24; box = 4; break; // door
					case 0x304D: purpose = ImagePurpose::ForegroundTileset3; ts_index = 25; box = 4; break; // door
					case 0x304E: purpose = ImagePurpose::ForegroundTileset3; ts_index = 26; box = 4; break; // door
					case 0x304F: purpose = ImagePurpose::ForegroundTileset3; ts_index = 27; box = 4; break; // door
					case 0x3050: purpose = ImagePurpose::ForegroundTileset2; ts_index = 8; break;
					case 0x3051: purpose = ImagePurpose::ForegroundTileset1; ts_index = 44; break;
					case 0x3052: purpose = ImagePurpose::SpriteTileset3; ts_index = 32; break;
					case 0x3053: purpose = ImagePurpose::ForegroundTileset2; ts_index = 10; break;
					case 0x3054: purpose = ImagePurpose::ForegroundTileset2; ts_index = 9; break;
					case 0x3055: purpose = ImagePurpose::ForegroundTileset2; ts_index = 47; break;
					case 0x3056: purpose = ImagePurpose::ForegroundTileset3; ts_index = 2; break;
					case 0x3057: purpose = ImagePurpose::SpriteTileset5; ts_index = 31; break;
					case 0x3058: purpose = ImagePurpose::ForegroundTileset3; ts_index = 48; break;
					case 0x3059: purpose = ImagePurpose::ForegroundTileset3; ts_index = 49; break;
					default:
						std::cout << "Unknown interactive tilecode 0x" << std::hex
							<< item.code << "\n";
						ret.type = ImageFromCodeInfo::ImageType::Unknown;
						return ret;
				}
				ts_num = 999999; // make error message below print purpose instead of ts_num
			}

			auto t = tileset.find(purpose);
			if (t == tileset.end()) { // no tileset?!
				std::cout << "fmt-map-duke1: Tileset ";
				if (ts_num < 1000) std::cout << "#" << ts_num;
				else std::cout << toString(purpose);
				std::cout << " not supplied.\n";
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			if (ts_index >= images.size()) { // out of range
				std::cout << "fmt-map-duke1: Tileset #" << ts_num << " has "
					<< images.size() << " images, but the tilecode maps to image #"
					<< ts_index << ".\n";
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[ts_index]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;

			// If a box was specified, draw the given image over the top of a box
			if (box) {
				switch (box) {
					case 1: // grey
						purpose = ImagePurpose::ForegroundTileset1;
						ts_index = 0;
						break;
					case 2: // blue
						purpose = ImagePurpose::ForegroundTileset3;
						ts_index = 0;
						break;
					case 3: // red
						purpose = ImagePurpose::ForegroundTileset3;
						ts_index = 1;
						break;
					case 4: // door
						purpose = ImagePurpose::ForegroundTileset3;
						ts_index = 28;
						break;
					default:
						assert(false); // should never happen
				}
				auto t = tileset.find(purpose);
				if (t != tileset.end()) {
					auto& images = t->second->files();
					if (ts_index < images.size()) {
						auto imgBox = t->second->openImage(images[ts_index]);
						ret.img = overlayImage(imgBox.get(), ret.img.get());
					}
				}
			}
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
					ImagePurpose::ForegroundTileset1,
					GraphicsFilename{"object0.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::ForegroundTileset2,
					GraphicsFilename{"object1.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::ForegroundTileset3,
					GraphicsFilename{"object2.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::SpriteTileset1,
					GraphicsFilename{"anim0.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::SpriteTileset2,
					GraphicsFilename{"anim1.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::SpriteTileset3,
					GraphicsFilename{"anim2.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::SpriteTileset4,
					GraphicsFilename{"anim3.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::SpriteTileset5,
					GraphicsFilename{"anim4.dn1", "tls-ccaves-sub"}
				),
				std::make_pair(
					ImagePurpose::SpriteTileset6,
					GraphicsFilename{"anim5.dn1", "tls-ccaves-sub"}
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
	return "map2d-duke1";
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
