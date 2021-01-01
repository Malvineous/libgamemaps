/**
 * @file  fmt-map-ccaves.cpp
 * @brief MapType and Map2D implementation for Crystal Caves levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Crystal_Caves
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
#include "fmt-map-ccaves.hpp"
#include "fmt-map-ccaves-mapping.hpp"

#define CC_MAP_WIDTH            40
#define CC_TILE_WIDTH           16
#define CC_TILE_HEIGHT          16

/// This is the largest number of rows ever expected to be seen.
#define CC_MAX_MAP_HEIGHT      100

/// This is the largest valid tile code in the background layer.
#define CC_MAX_VALID_TILECODE   0xFE

/// Create a tile number from a tileset number and an index into the tileset.
#define MAKE_TILE(tileset, tile) (((tileset) << 8) | (tile))
#define CCT(tileset, tile) (((tileset) << 8) | (tile))

/// Tile code that means "next tile"
#define CCT_NEXT  0x6E

/// Tile code that means "no tile here"
#define CCT_EMPTY 0x20

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

			//unsigned int fgtile = MAKE_TILE(21, 20); // solid cyan
			//unsigned int fgtile = MAKE_TILE(19, 20); // blue rock
			//unsigned int fgtile = MAKE_TILE(22, 12); // solid blue
			//unsigned int fgtile = MAKE_TILE(21, 12); // wavy green
			unsigned int block_tile = MAKE_TILE(21, 32); // solid brown
			//unsigned int ibeam_tile = MAKE_TILE(19,  3); // blue
			unsigned int ibeam_tile = MAKE_TILE(19,  6); // red
			unsigned int underscore_tile = MAKE_TILE(19,  0); // blue

/// Convert the CCTF_* flags into Map2D flags
void setFlags(Map2D::Layer::Item& item, unsigned int flags)
{
	switch (flags) {
		case CCTF_MV_NONE:
			break;
		case CCTF_MV_VERT:
			item.type |= Map2D::Layer::Item::Type::Movement;
			item.movementFlags = Map2D::Layer::Item::MovementFlags::DistanceLimit;
			item.movementDistLeft = 0;
			item.movementDistRight = 0;
			item.movementDistUp = Map2D::Layer::Item::DistIndeterminate;
			item.movementDistDown = Map2D::Layer::Item::DistIndeterminate;
			break;
		case CCTF_MV_HORZ:
			item.type |= Map2D::Layer::Item::Type::Movement;
			item.movementFlags = Map2D::Layer::Item::MovementFlags::DistanceLimit;
			item.movementDistLeft = Map2D::Layer::Item::DistIndeterminate;
			item.movementDistRight = Map2D::Layer::Item::DistIndeterminate;
			item.movementDistUp = 0;
			item.movementDistDown = 0;
			break;
		case CCTF_MV_DROP:
			item.type |= Map2D::Layer::Item::Type::Movement;
			item.movementFlags = Map2D::Layer::Item::MovementFlags::DistanceLimit;
			item.movementDistLeft = 0;
			item.movementDistRight = 0;
			item.movementDistUp = 0;
			item.movementDistDown = Map2D::Layer::Item::DistIndeterminate;
			break;
	}
	return;
}

class Layer_CCaves_Common: public Map2DCore::LayerCore
{
	public:
		Layer_CCaves_Common()
		{
		}

		virtual ~Layer_CCaves_Common()
		{
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			unsigned int ti, i;
			ti = item.code >> 8;
			i = item.code & 0xFF;

			auto t = tileset.find(ImagePurpose::BackgroundTileset1);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& subtilesets = t->second->files();
			if (ti >= subtilesets.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto subtileset = t->second->openTileset(subtilesets[ti]);
			auto& images = subtileset->files();
			if (i >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = subtileset->openImage(images[i]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}
};

class Layer_CCaves_Background: public Layer_CCaves_Common
{
	public:
		Layer_CCaves_Background()
		{
		}

		virtual ~Layer_CCaves_Background()
		{
		}

		virtual std::string title() const
		{
			return "Background";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			Item item;
			for (unsigned int i = 0; i < sizeof(tileMapVine) / sizeof(TILE_MAP_VINE); i++) {
				TILE_MAP_VINE& m = tileMapVine[i];

				item.type = Item::Type::Default;
				item.pos = {0, 0}; // required for selections to work
				item.code = m.tileIndexMid;
				validItems.push_back(item);

				item.type = Item::Type::Default;
				item.pos = {0, 0}; // required for selections to work
				item.code = m.tileIndexEnd;
				validItems.push_back(item);
			}

			for (unsigned int i = 0; i < sizeof(tileMapSign) / sizeof(TILE_MAP_SIGN); i++) {
				TILE_MAP_SIGN& m = tileMapSign[i];

				for (unsigned int j = 0; j < sizeof(m.tileIndexBG) / sizeof(int); j++) {
					if (m.tileIndexBG[j] == ___________) continue;
					item.type = Item::Type::Default;
					item.pos = {0, 0}; // required for selections to work
					item.code = m.tileIndexBG[j];
					if (j == 0) setFlags(item, m.flags);
					validItems.push_back(item);
				}
			}

			for (unsigned int i = 0; i < sizeof(tileMap) / sizeof(TILE_MAP); i++) {
				TILE_MAP& m = tileMap[i];

				for (unsigned int j = 0; j < sizeof(m.tileIndexBG) / sizeof(int); j++) {
					if (m.tileIndexBG[j] == ___________) continue;
					item.type = Item::Type::Default;
					item.pos = {0, 0}; // required for selections to work
					if (IS_IBEAM(m.tileIndexBG[j])) {
						item.code = CCT_IBEAM(ibeam_tile, m.tileIndexBG[j]);
					} else if (IS_BLOCK(m.tileIndexBG[j])) {
						item.code = CCT_BLOCK(block_tile, m.tileIndexBG[j]);
					} else if (m.tileIndexBG[j] == CCT_USCORE) {
						item.code = underscore_tile;
					} else {
						item.code = m.tileIndexBG[j];
					}
					if (j == 0) setFlags(item, m.flags);
					validItems.push_back(item);
				}
			}

			for (unsigned int i = 0; i < sizeof(tileMap4x1) / sizeof(TILE_MAP); i++) {
				TILE_MAP& m = tileMap4x1[i];

				for (unsigned int j = 0; j < sizeof(m.tileIndexBG) / sizeof(int); j++) {
					if (m.tileIndexBG[j] == ___________) continue;
					Item item;
					item.type = Item::Type::Default;
					item.pos = {0, 0}; // required for selections to work
					item.code = m.tileIndexBG[j];
					if (j == 0) setFlags(item, m.flags);
					validItems.push_back(item);
				}
			}
			return validItems;
		}
};

class Layer_CCaves_Foreground: public Layer_CCaves_Common
{
	public:
		Layer_CCaves_Foreground()
		{
		}

		virtual ~Layer_CCaves_Foreground()
		{
		}

		virtual std::string title() const
		{
			return "Overlay";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			Item item;

			for (unsigned int i = 0; i < sizeof(tileMap) / sizeof(TILE_MAP); i++) {
				TILE_MAP& m = tileMap[i];

				if (m.tileIndexFG != ___________) {
					item.type = Item::Type::Default;
					item.pos = {0, 0}; // required for selections to work
					item.code = m.tileIndexFG;
					validItems.push_back(item);
				}
			}
			return validItems;
		}
};

class Map_CCaves: public MapCore, public Map2DCore
{
	public:
		Map_CCaves(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			auto layerBG = std::make_shared<Layer_CCaves_Background>();
			auto layerFG = std::make_shared<Layer_CCaves_Foreground>();

			this->v_layers.push_back(layerBG);
			this->v_layers.push_back(layerFG);

			auto& tilesBG = layerBG->items();
			auto& tilesFG = layerFG->items();

			stream::pos lenMap = this->content->size();
			this->mapHeight = lenMap / (CC_MAP_WIDTH + 1);

			// Read the background layer
			this->content->seekg(0, stream::start);
			std::vector<uint8_t> bgdata(lenMap, CCT_EMPTY);
			this->content->read(bgdata.data(), lenMap);

			tilesBG.reserve(CC_MAP_WIDTH * this->mapHeight);
//	unsigned long c = 0;

/// Add a tile to the map vector
#define INSERT_TILE(dx, dy, val, flags) {	  \
				tilesBG.emplace_back(); \
				Layer::Item& t = tilesBG.back(); \
				t.type = Layer::Item::Type::Default; \
				t.pos = {(long)(x + (dx)), (long)(y + (dy))}; \
				if (IS_IBEAM(val)) { \
					t.code = CCT_IBEAM(ibeam_tile, (val)); \
				} else if (IS_BLOCK(val)) { \
					t.code = CCT_BLOCK(block_tile, (val)); \
				} else if ((val) == CCT_USCORE) { \
					t.code = underscore_tile; \
				} else { \
					t.code = (val); \
				} \
				switch (flags) { \
					case CCTF_MV_NONE: \
						break; \
					case CCTF_MV_VERT: \
						t.type |= Map2D::Layer::Item::Type::Movement; \
						t.movementFlags = Map2D::Layer::Item::MovementFlags::DistanceLimit; \
						t.movementDistLeft = 0; \
						t.movementDistRight = 0; \
						t.movementDistUp = Map2D::Layer::Item::DistIndeterminate; \
						t.movementDistDown = Map2D::Layer::Item::DistIndeterminate; \
						break; \
					case CCTF_MV_HORZ: \
						t.type |= Map2D::Layer::Item::Type::Movement; \
						t.movementFlags = Map2D::Layer::Item::MovementFlags::DistanceLimit; \
						t.movementDistLeft = Map2D::Layer::Item::DistIndeterminate; \
						t.movementDistRight = Map2D::Layer::Item::DistIndeterminate; \
						t.movementDistUp = 0; \
						t.movementDistDown = 0; \
						break; \
					case CCTF_MV_DROP: \
						t.type |= Map2D::Layer::Item::Type::Movement; \
						t.movementFlags = Map2D::Layer::Item::MovementFlags::DistanceLimit; \
						t.movementDistLeft = 0; \
						t.movementDistRight = 0; \
						t.movementDistUp = 0; \
						t.movementDistDown = Map2D::Layer::Item::DistIndeterminate; \
						break; \
				} \
			}

/// Return the tile code at the given delta coords
#define BGTILE(dx, dy) bg[(CC_MAP_WIDTH + 1) * (dy) + (dx)]

/// If the given tile is CCT_NEXT, set the code and blank out the tile
#define SET_NEXT_TILE(dx, dy, val)	  \
			if ( \
				(val != ___________) \
				&& (x + (dx) < CC_MAP_WIDTH) \
				&& (y + (dy) < this->mapHeight) \
				&& (BGTILE((dx), (dy)) == CCT_NEXT) \
			) { \
				INSERT_TILE((dx), (dy), val, CCTF_MV_NONE); \
				BGTILE((dx), (dy)) = CCT_EMPTY; \
			}

			auto bg = bgdata.begin();
			for (unsigned int y = 0; y < this->mapHeight; y++) {
				bg++; // skip row length byte
				for (unsigned int x = 0; x < CC_MAP_WIDTH; x++, bg++) {
					// Skip all empty tiles
					if (*bg == CCT_EMPTY) {
						continue;
					}
					bool matched = false;

					// Check vines first
					for (unsigned int i = 0; i < sizeof(tileMapVine) / sizeof(TILE_MAP_VINE); i++) {
						TILE_MAP_VINE& m = tileMapVine[i];
						if (BGTILE(0, 0) == m.code) {
							matched = true;
							if (BGTILE(0, 1) == m.code) {
								// The vine continue on the row below, use a mid-tile
								INSERT_TILE(0, 0, m.tileIndexMid, m.flags);
							} else {
								// The vine stops here, use an end-tile
								INSERT_TILE(0, 0, m.tileIndexEnd, m.flags);
							}
							// Follow the vine up if need be
							for (int y2 = 1; y2 <= (signed)y; y2++) {
								if (BGTILE(0, -y2) == CCT_NEXT) {
									INSERT_TILE(0, -y2, m.tileIndexMid, CCTF_MV_NONE);
								} else {
									break; // vine stopped
								}
							}
							break;
						}
					}
					if (matched) continue;

					// Then check signs
					for (unsigned int i = 0; i < sizeof(tileMapSign) / sizeof(TILE_MAP_SIGN); i++) {
						TILE_MAP_SIGN& m = tileMapSign[i];
						if ((BGTILE(0, 0) == m.code1) && (BGTILE(1, 0) == m.code2)) {
							matched = true;
							INSERT_TILE(0, 0, m.tileIndexBG[0], m.flags);
							INSERT_TILE(1, 0, m.tileIndexBG[1], CCTF_MV_NONE);
							SET_NEXT_TILE(2, 0, m.tileIndexBG[2]);
							SET_NEXT_TILE(3, 0, m.tileIndexBG[3]);
							SET_NEXT_TILE(0, 1, m.tileIndexBG[4]);
							SET_NEXT_TILE(1, 1, m.tileIndexBG[5]);
							SET_NEXT_TILE(2, 1, m.tileIndexBG[6]);
							SET_NEXT_TILE(3, 1, m.tileIndexBG[7]);
							SET_NEXT_TILE(0, 2, m.tileIndexBG[8]);
							SET_NEXT_TILE(1, 2, m.tileIndexBG[9]);
							SET_NEXT_TILE(2, 2, m.tileIndexBG[10]);
							SET_NEXT_TILE(3, 2, m.tileIndexBG[11]);
							SET_NEXT_TILE(0, 3, m.tileIndexBG[12]);
							SET_NEXT_TILE(1, 3, m.tileIndexBG[13]);
							SET_NEXT_TILE(2, 3, m.tileIndexBG[14]);
							SET_NEXT_TILE(3, 3, m.tileIndexBG[15]);
							// All signs are at least two cells wide
							bg++;
							x++;
							break;
						}
					}
					if (matched) continue;

					// Lastly check the normal tiles
					for (unsigned int i = 0; i < sizeof(tileMap) / sizeof(TILE_MAP); i++) {
						TILE_MAP& m = tileMap[i];
						if (*bg == m.code) {
							matched = true;
							INSERT_TILE(0, 0, m.tileIndexBG[0], m.flags);
							SET_NEXT_TILE(1, 0, m.tileIndexBG[1]);
							SET_NEXT_TILE(0, 1, m.tileIndexBG[2]);
							SET_NEXT_TILE(1, 1, m.tileIndexBG[3]);
							if (m.tileIndexFG != ___________) {
								tilesFG.emplace_back();
								Layer::Item& t = tilesFG.back();
								t.type = Layer::Item::Type::Default;
								t.pos.x = x;
								t.pos.y = y;
								t.code = m.tileIndexFG;
							}
							break;
						}
					}
					if (matched) continue;

					for (unsigned int i = 0; i < sizeof(tileMap4x1) / sizeof(TILE_MAP); i++) {
						TILE_MAP& m = tileMap4x1[i];
						if (*bg == m.code) {
							matched = true;
							INSERT_TILE(0, 0, m.tileIndexBG[0], m.flags);
							SET_NEXT_TILE(1, 0, m.tileIndexBG[1]);
							SET_NEXT_TILE(2, 0, m.tileIndexBG[2]);
							SET_NEXT_TILE(3, 0, m.tileIndexBG[3]);
							break;
						}
					}
					if (matched) continue;
				}
			}
#undef INSERT_TILE
#undef BGTILE
#undef SET_NEXT_TILE
		}

		virtual ~Map_CCaves()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {
				std::make_pair(
					ImagePurpose::BackgroundTileset1,
					GraphicsFilename{"cc1.gfx", "tls-ccaves-main"}
				),
			};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 2);

			auto mapSize = this->mapSize();

			// Maximum width?
/// @todo Is this width changeable or is it fixed?
			if (mapSize.x > 255)
				throw stream::error("This map format must be less than 255 tiles wide.");

			// Safety to avoid excessive memory use
			if (mapSize.y > 200)
				throw stream::error("This map format must be less than 200 tiles tall.");

			unsigned long lenBG = mapSize.x * mapSize.y;

			// Extract the tile codes into a big array so it's easier to cross reference
			std::vector<int> bgsrc(lenBG, -1);
			std::vector<unsigned int> bgattr(lenBG, CCTF_MV_NONE);
			std::vector<int> fgsrc(lenBG, -1);

			for (const auto& i : this->v_layers[0]->items()) {
				if ((i.pos.x >= mapSize.x) || (i.pos.y >= mapSize.y)) {
					throw stream::error("Background layer has tiles outside map boundary!");
				}
				auto pos = i.pos.y * mapSize.x + i.pos.x;
				bgsrc[pos] = i.code;
				if (
					(i.type & Layer::Item::Type::Movement)
					&& (i.movementFlags & Layer::Item::MovementFlags::DistanceLimit)
				) {
					if (
						(i.movementDistUp == Layer::Item::DistIndeterminate)
						&& (i.movementDistDown == Layer::Item::DistIndeterminate)
						&& (i.movementDistLeft == 0)
						&& (i.movementDistRight == 0)
					) {
						bgattr[pos] = CCTF_MV_VERT;
					} else if (
						(i.movementDistUp == 0)
						&& (i.movementDistDown == 0)
						&& (i.movementDistLeft == Layer::Item::DistIndeterminate)
						&& (i.movementDistRight == Layer::Item::DistIndeterminate)
					) {
						bgattr[pos] = CCTF_MV_HORZ;
					} else if (
						(i.movementDistUp == 0)
						&& (i.movementDistDown == Layer::Item::DistIndeterminate)
						&& (i.movementDistLeft == 0)
						&& (i.movementDistRight == 0)
					) {
						bgattr[pos] = CCTF_MV_DROP;
					}
				}
			}

			for (const auto& i : this->v_layers[1]->items()) {
				if ((i.pos.x >= mapSize.x) || (i.pos.y >= mapSize.y)) {
					throw stream::error("Foreground layer has tiles outside map boundary!");
				}
				auto pos = i.pos.y * mapSize.x + i.pos.x;
				fgsrc[pos] = i.code;
			}

			// Convert our codes into CC ones
			std::vector<uint8_t> bgdst(lenBG, CCT_EMPTY);

			auto inbg = bgsrc.data();
			auto inattr = bgattr.data();
			auto infg = fgsrc.data();
			uint8_t *out = bgdst.data();
#define REL(px, py) (*(inbg + ((py) * mapSize.x) + (px)))
#define REL_FG(px, py) (*(infg + ((py) * mapSize.x) + (px)))
#define PUT(px, py, pc) (*(out + ((py) * mapSize.x) + (px))) = pc
#define IF_REL(px, py, pt, pc)	  \
			if ( \
				(pt != ___________) \
				&& ((inbg - bgsrc.data() + (py) * mapSize.x + (px)) < (int)lenBG) \
				&& (REL((px), (py)) == pt) \
			) { \
				PUT((px), (py), (pc)); \
				REL((px), (py)) = CCT_EMPTY; \
			}

			for (unsigned int j = 0; j < lenBG; j++, inbg++, inattr++, infg++, out++) {
				if (*inbg == -1) continue; // no tile here
				bool matched = false;

				// Check vines first
				for (unsigned int i = 0; i < sizeof(tileMapVine) / sizeof(TILE_MAP_VINE); i++) {
					TILE_MAP_VINE& m = tileMapVine[i];
					if (
						(
							(REL(0, 0) == m.tileIndexMid)
							|| (REL(0, 0) == m.tileIndexEnd)
						)
						&& (*inattr == m.flags)
					) {
						matched = true;
						PUT(0, 0, m.code);
						break;
					}
				}
				if (matched) continue;

				// Then check signs
				TILE_MAP_SIGN *m_final = NULL;
				unsigned int best_confidence = 0;
				for (unsigned int i = 0; i < sizeof(tileMapSign) / sizeof(TILE_MAP_SIGN); i++) {
					TILE_MAP_SIGN& m = tileMapSign[i];
					if (
						(REL(0, 0) == m.tileIndexBG[0])
						&& (REL(1, 0) == m.tileIndexBG[1])
					) {
						unsigned int confidence = 2;
						if (*inattr == m.flags) confidence++;
						if ((m.tileIndexBG[ 2] != -1) && (REL(2, 0) != CCT_EMPTY) && (REL(2, 0) == m.tileIndexBG[ 2])) confidence++;
						if ((m.tileIndexBG[ 3] != -1) && (REL(3, 0) != CCT_EMPTY) && (REL(3, 0) == m.tileIndexBG[ 3])) confidence++;
						if ((m.tileIndexBG[ 4] != -1) && (REL(0, 1) != CCT_EMPTY) && (REL(0, 1) == m.tileIndexBG[ 4])) confidence++;
						if ((m.tileIndexBG[ 5] != -1) && (REL(1, 1) != CCT_EMPTY) && (REL(1, 1) == m.tileIndexBG[ 5])) confidence++;
						if ((m.tileIndexBG[ 6] != -1) && (REL(2, 1) != CCT_EMPTY) && (REL(2, 1) == m.tileIndexBG[ 6])) confidence++;
						if ((m.tileIndexBG[ 7] != -1) && (REL(3, 1) != CCT_EMPTY) && (REL(3, 1) == m.tileIndexBG[ 7])) confidence++;
						if ((m.tileIndexBG[ 8] != -1) && (REL(0, 2) != CCT_EMPTY) && (REL(0, 2) == m.tileIndexBG[ 8])) confidence++;
						if ((m.tileIndexBG[ 9] != -1) && (REL(1, 2) != CCT_EMPTY) && (REL(1, 2) == m.tileIndexBG[ 9])) confidence++;
						if ((m.tileIndexBG[10] != -1) && (REL(2, 2) != CCT_EMPTY) && (REL(2, 2) == m.tileIndexBG[10])) confidence++;
						if ((m.tileIndexBG[11] != -1) && (REL(3, 2) != CCT_EMPTY) && (REL(3, 2) == m.tileIndexBG[11])) confidence++;
						if ((m.tileIndexBG[12] != -1) && (REL(0, 3) != CCT_EMPTY) && (REL(0, 3) == m.tileIndexBG[12])) confidence++;
						if ((m.tileIndexBG[13] != -1) && (REL(1, 3) != CCT_EMPTY) && (REL(1, 3) == m.tileIndexBG[13])) confidence++;
						if ((m.tileIndexBG[14] != -1) && (REL(2, 3) != CCT_EMPTY) && (REL(2, 3) == m.tileIndexBG[14])) confidence++;
						if ((m.tileIndexBG[15] != -1) && (REL(3, 3) != CCT_EMPTY) && (REL(3, 3) == m.tileIndexBG[15])) confidence++;

						if (confidence > best_confidence) {
							best_confidence = confidence;
							m_final = &m;
						}
					}
				}

				if (m_final) {
					// Must have at least two cells for a sign
					TILE_MAP_SIGN& m = *m_final;
					matched = true;
					PUT(0, 0, m.code1);
					PUT(1, 0, m.code2);
					REL(1, 0) = (unsigned int)0x20; // handled this code
					IF_REL(2, 0, m.tileIndexBG[2], CCT_NEXT);
					IF_REL(3, 0, m.tileIndexBG[3], CCT_NEXT);
					IF_REL(0, 1, m.tileIndexBG[4], CCT_NEXT);
					IF_REL(1, 1, m.tileIndexBG[5], CCT_NEXT);
					IF_REL(2, 1, m.tileIndexBG[6], CCT_NEXT);
					IF_REL(3, 1, m.tileIndexBG[7], CCT_NEXT);
					IF_REL(0, 2, m.tileIndexBG[8], CCT_NEXT);
					IF_REL(1, 2, m.tileIndexBG[9], CCT_NEXT);
					IF_REL(2, 2, m.tileIndexBG[10], CCT_NEXT);
					IF_REL(3, 2, m.tileIndexBG[11], CCT_NEXT);
					IF_REL(0, 3, m.tileIndexBG[12], CCT_NEXT);
					IF_REL(1, 3, m.tileIndexBG[13], CCT_NEXT);
					IF_REL(2, 3, m.tileIndexBG[14], CCT_NEXT);
					IF_REL(3, 3, m.tileIndexBG[15], CCT_NEXT);
					continue;
				}
				//if (matched) continue;

				// Lastly check the normal tiles
				for (unsigned int i = 0; i < sizeof(tileMap) / sizeof(TILE_MAP); i++) {
					TILE_MAP& m = tileMap[i];
					if (
						(REL(0, 0) == m.tileIndexBG[0])
						&& (REL_FG(0, 0) == m.tileIndexFG)
						&& (*inattr == m.flags)
					) {
						matched = true;
						PUT(0, 0, m.code);
						IF_REL(1, 0, m.tileIndexBG[1], CCT_NEXT);
						IF_REL(0, 1, m.tileIndexBG[2], CCT_NEXT);
						IF_REL(1, 1, m.tileIndexBG[3], CCT_NEXT);
						break;
					}
				}
				if (matched) continue;

				for (unsigned int i = 0; i < sizeof(tileMap4x1) / sizeof(TILE_MAP); i++) {
					TILE_MAP& m = tileMap4x1[i];
					if (
						(REL(0, 0) == m.tileIndexBG[0])
						&& (REL_FG(0, 0) == m.tileIndexFG)
						&& (*inattr == m.flags)
					) {
						matched = true;
						PUT(0, 0, m.code);
						IF_REL(1, 0, m.tileIndexBG[1], CCT_NEXT);
						IF_REL(2, 0, m.tileIndexBG[2], CCT_NEXT);
						IF_REL(3, 0, m.tileIndexBG[3], CCT_NEXT);
						break;
					}
				}
				if (matched) continue;

				// Also check reverse-map only tiles
				for (unsigned int i = 0; i < sizeof(tileRevMapBlocks) / sizeof(TILE_REVMAP_BLOCKS); i++) {
					TILE_REVMAP_BLOCKS& m = tileRevMapBlocks[i];
					if (
						(REL(0, 0) == m.tileIndexBG)
						&& (REL_FG(0, 0) == m.tileIndexFG)
					) {
						matched = true;
						PUT(0, 0, m.code);
						break;
					}
				}
				if (matched) continue;

			}

			// Write the background and foreground combined layer
			this->content->truncate((mapSize.x + 1) * mapSize.y);
			this->content->seekp(0, stream::start);
			uint8_t *dstPos = bgdst.data();
			for (unsigned int y = 0; y < mapSize.y; y++) {
				*this->content << u8(mapSize.x);
				this->content->write(dstPos, mapSize.x);
				dstPos += mapSize.x;
			}
			this->content->flush();
			return;
		}

		virtual Caps caps() const
		{
			return
				Caps::HasViewport
				| Caps::HasMapSize
				| Caps::HasTileSize
			;
		}

		virtual Point viewport() const
		{
			return {320, 192};
		}

		virtual Point mapSize() const
		{
			return {CC_MAP_WIDTH, (long)this->mapHeight};
		}

		virtual Point tileSize() const
		{
			return {CC_TILE_WIDTH, CC_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, CCT_EMPTY);
		}

	private:
		std::unique_ptr<stream::inout> content;
		unsigned int mapHeight;
};

std::string MapType_CCaves::code() const
{
	return "map2d-ccaves";
}

std::string MapType_CCaves::friendlyName() const
{
	return "Crystal Caves level";
}

std::vector<std::string> MapType_CCaves::fileExtensions() const
{
	return {"ccl"};
}

std::vector<std::string> MapType_CCaves::games() const
{
	return {"Crystal Caves"};
}

MapType::Certainty MapType_CCaves::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	content.seekg(0, stream::start);

	// TESTED BY: fmt_map_ccaves_isinstance_c01
	if (lenMap < CC_MAP_WIDTH + 1) return MapType::DefinitelyNo; // too small

	uint8_t row[CC_MAP_WIDTH];
	unsigned int y;
	for (y = 0; (y < CC_MAX_MAP_HEIGHT) && lenMap; y++) {
		uint8_t lenRow;
		content >> u8(lenRow);
		lenMap--;

		// Incorrect row length
		// TESTED BY: fmt_map_ccaves_isinstance_c02
		if (lenRow != CC_MAP_WIDTH) return MapType::DefinitelyNo;

		// Incomplete row
		// TESTED BY: fmt_map_ccaves_isinstance_c03
		if (lenMap < CC_MAP_WIDTH) return MapType::DefinitelyNo;

		// Ensure the row data is valid
		content.read(row, CC_MAP_WIDTH);
		for (unsigned int x = 0; x < CC_MAP_WIDTH; x++) {
			// TESTED BY: fmt_map_ccaves_isinstance_c04
			if (row[x] > CC_MAX_VALID_TILECODE) return MapType::DefinitelyNo; // invalid tile
		}

		lenMap -= CC_MAP_WIDTH;
	}

	// TESTED BY: fmt_map_ccaves_isinstance_c05
	if (y == CC_MAX_MAP_HEIGHT) return MapType::DefinitelyNo; // map too tall

	// TESTED BY: fmt_map_ccaves_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_CCaves::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_CCaves::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_CCaves>(std::move(content));
}

SuppFilenames MapType_CCaves::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
