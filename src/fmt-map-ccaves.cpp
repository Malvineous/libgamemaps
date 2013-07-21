/**
 * @file   fmt-map-ccaves.cpp
 * @brief  MapType and Map2D implementation for Crystal Caves levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Crystal_Caves
 *
 * Copyright (C) 2010-2013 Adam Nielsen <malvineous@shikadi.net>
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

#include <iomanip>
#include <boost/scoped_array.hpp>
#include <camoto/iostream_helpers.hpp>
#include "map2d-generic.hpp"
#include "fmt-map-ccaves.hpp"

#define CC_MAP_WIDTH            40
#define CC_TILE_WIDTH           16
#define CC_TILE_HEIGHT          16

/// This is the largest number of rows ever expected to be seen.
#define CC_MAX_MAP_HEIGHT      100

/// This is the largest valid tile code in the background layer.
#define CC_MAX_VALID_TILECODE   0xFE

/// Width of map view during gameplay, in pixels
#define CC_VIEWPORT_WIDTH       320

/// Height of map view during gameplay, in pixels
#define CC_VIEWPORT_HEIGHT      192

/// Create a tile number from a tileset number and an index into the tileset.
#define MAKE_TILE(tileset, tile) (((tileset) << 8) | (tile))

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;


class CCavesBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		CCavesBackgroundLayer(ItemPtrVectorPtr& items,
			ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Background",
					Map2D::Layer::NoCaps,
					0, 0,
					0, 0,
					items, validItems
				)
		{
		}

		virtual gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset)
		{
			unsigned int ti, i;
			ti = item->code >> 8;
			i = item->code & 0xFF;

			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& ts = t->second->getItems();
			TilesetPtr tsub = t->second->openTileset(ts[ti]);
			const Tileset::VC_ENTRYPTR& images = tsub->getItems();
			if (i >= images.size()) return ImagePtr(); // out of range
			return tsub->openImage(images[i]);
		}
};


std::string CCavesMapType::getMapCode() const
{
	return "map-ccaves";
}

std::string CCavesMapType::getFriendlyName() const
{
	return "Crystal Caves level";
}

std::vector<std::string> CCavesMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("ccl");
	return vcExtensions;
}

std::vector<std::string> CCavesMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Crystal Caves");
	return vcGames;
}

MapType::Certainty CCavesMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	psMap->seekg(0, stream::start);

	// TESTED BY: fmt_map_ccaves_isinstance_c01
	if (lenMap < CC_MAP_WIDTH + 1) return MapType::DefinitelyNo; // too small

	uint8_t row[CC_MAP_WIDTH];
	unsigned int y;
	for (y = 0; (y < CC_MAX_MAP_HEIGHT) && lenMap; y++) {
		uint8_t lenRow;
		psMap >> u8(lenRow);
		lenMap--;

		// Incorrect row length
		// TESTED BY: fmt_map_ccaves_isinstance_c02
		if (lenRow != CC_MAP_WIDTH) return MapType::DefinitelyNo;

		// Incomplete row
		// TESTED BY: fmt_map_ccaves_isinstance_c03
		if (lenMap < CC_MAP_WIDTH) return MapType::DefinitelyNo;

		// Ensure the row data is valid
		psMap->read((char *)row, CC_MAP_WIDTH);
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

MapPtr CCavesMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr CCavesMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	stream::pos lenMap = input->size();

	input->seekg(0, stream::start);

	// Read the background layer
	uint8_t *bg = new uint8_t[lenMap];
	boost::scoped_array<uint8_t> sbg(bg);
	input->read((char *)bg, lenMap);

	unsigned int height = lenMap / (CC_MAP_WIDTH + 1);

	//unsigned int fgtile = MAKE_TILE(21, 20); // solid cyan
	//unsigned int fgtile = MAKE_TILE(19, 20); // blue rock
	//unsigned int fgtile = MAKE_TILE(22, 12); // solid blue
	//unsigned int fgtile = MAKE_TILE(21, 12); // wavy green
	unsigned int fgtile = MAKE_TILE(21, 32); // solid brown
	//unsigned int ibeam_tile = MAKE_TILE(19,  3); // blue
	unsigned int ibeam_tile = MAKE_TILE(19,  6); // red
	unsigned int underscore_tile = MAKE_TILE(19,  0); // blue

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(CC_MAP_WIDTH * height);
	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
	unsigned long c = 0;
	for (unsigned int y = 0; y < height; y++) {
		bg++; // skip row length byte
		for (unsigned int x = 0; x < CC_MAP_WIDTH; x++) {
			// Skip all empty tiles
			if (*bg == 0x20) {
				bg++;
				continue;
			}
			switch (*bg++) {
				case 0x0C: c = fgtile + 5; break; // same tile as 0x67, what's the difference?
				case 0x21: c = MAKE_TILE(13,  0); break; // blue dripping pipe
				case 0x22: c = MAKE_TILE(12, 30); break; // green stuff hanging down from block 2
				case 0x23: c = MAKE_TILE( 2, 24); break; // spider
				case 0x24: { // air compressor
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE(17, 14); // air compressor bottom
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					c = MAKE_TILE(17, 10); // red-door top
					break;
				}

				case 0x25: c = MAKE_TILE(10, 39); break; // green pipe, vert
				case 0x26: c = MAKE_TILE(13, 12); break; // robot enemy
				//   0x27 invalid
				case 0x28: c = MAKE_TILE( 3, 34); break; // brown stalactites 1
				case 0x29: c = MAKE_TILE( 3, 35); break; // brown stalactites 2
				case 0x2A: c = MAKE_TILE( 2,  4); break; // brown walking ball enemy
				case 0x2B: c = MAKE_TILE(12,  1); break; // yellow gem
				case 0x2C: c = MAKE_TILE(10, 37); break; // green pipe, down exit, left join
				case 0x2D: c = MAKE_TILE(10, 36); break; // green pipe, horiz
				case 0x2E: c = MAKE_TILE(10, 38); break; // green pipe, down exit, right join
				case 0x2F: c = MAKE_TILE( 9, 31); break; // flying bone enemy
				case 0x30: c = MAKE_TILE( 0, 43); break; // large chain end
				//   0x31 invalid
				case 0x32: c = fgtile + 5; break;
				//   0x33 invalid
				case 0x34: c = fgtile + 8; break;
				case 0x35: c = fgtile + 9; break;
				case 0x36: c = fgtile + 10; break;
				//   0x37 invalid
				case 0x38: c = MAKE_TILE( 0, 34); break; // large chain
				case 0x39: c = MAKE_TILE( 1, 46); break; // mine cart
				case 0x3A: c = MAKE_TILE(12, 29); break; // green stuff hanging down from block 1
				//   0x3B invalid
				//   0x3C invalid
				case 0x3D: c = MAKE_TILE(13, 44); break; // purple wall enemy, attacking to left
				//   0x3E invalid
				case 0x3F: c = MAKE_TILE( 1, 12); break; // green stripy enemy
				case 0x40: c = MAKE_TILE( 3, 40); break; // tornado
				case 0x41: c = MAKE_TILE(17, 32); break; // green fish enemy
				case 0x42: c = MAKE_TILE( 0,  6); break; // ice block  @todo could be 0,19 also, tiles are identical
				case 0x43: c = MAKE_TILE(21,  0); break; // random concrete blocks  @todo indicate randomness somehow
				case 0x44: c = ibeam_tile; break; // I-beam left
				case 0x45: c = MAKE_TILE(13, 35); break; // purple wall enemy, attacking to right
				case 0x46: c = MAKE_TILE( 9, 22); break; // flame
				case 0x47: c = MAKE_TILE( 5, 48); break; // gun/ammo
				case 0x48: c = MAKE_TILE(11, 40); break; // horiz moving platform
				case 0x49: c = MAKE_TILE( 4, 37); break; // popup floor spike
				case 0x4A: c = MAKE_TILE( 9,  3); break; // flame tower
				case 0x4B: c = MAKE_TILE(21,  0); break; // concrete block 0
				case 0x4C: c = MAKE_TILE(21,  1); break; // concrete block 1
				case 0x4D: c = MAKE_TILE( 6,  0); break; // emu enemy
				case 0x4E: c = MAKE_TILE(12,  9); break; // moon
				//   0x4F invalid
				//   0x50 invalid
				//   0x51 invalid
				case 0x52: c = MAKE_TILE(12,  0); break; // red gem
				case 0x53: c = MAKE_TILE( 3,  4); break; // purple snake enemy
				case 0x54: c = MAKE_TILE( 9, 24); break; // hammer guide
				case 0x55: { // hammer top
					c = MAKE_TILE( 9, 10);

					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE( 9, 14); // hammer bottom-left
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					next = bg + CC_MAP_WIDTH + 1;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x + 1;
						t->y = y + 1;
						t->code = MAKE_TILE( 9, 15); // hammer bottom-right
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0x56: c = MAKE_TILE(11, 44); break; // vert moving platform
				case 0x57: { // exhaust vacuum
					unsigned int c2 = 0;
					switch (*bg) {
						case 0x4C:
							c = MAKE_TILE( 3, 29); // wind lines
							c2 = MAKE_TILE( 3,  2); // left-facing sucker
							break;
						case 0x52:
							c = MAKE_TILE( 3,  0); // right-facing sucker
							c2 = MAKE_TILE( 3, 29); // wind lines
							break;
						default:
							std::cout << "Unknown vacuum type " << std::hex << std::setfill('0')
								<< std::setw(2) << (int)*(bg-1) << " in Crystal Caves map."
								<< std::dec << std::endl;
							continue;
					}
					Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
					t->type = Map2D::Layer::Item::Default;
					t->x = x;
					t->y = y;
					t->code = c;
					tiles->push_back(t);
					*bg++ = 0x20; // already handled this one
					c = c2;
					x++;
					break;
				}
				case 0x58: { // level exit
					c = MAKE_TILE(11, 12); // top-left exit door tile

					uint8_t *next = bg;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x + 1;
						t->y = y;
						t->code = MAKE_TILE(11, 20); // top-right exit door tile
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE(11, 16); // bottom-left exit door tile
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					next = bg + CC_MAP_WIDTH + 1;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x + 1;
						t->y = y + 1;
						t->code = MAKE_TILE(11, 24); // bottom-right exit door tile
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0x59: c = MAKE_TILE( 5,  0); break;
				case 0x5A: c = MAKE_TILE( 4, 32); break; // random map horizon/hill/light
				case 0x5B: { // sign
					x++;
					unsigned int c2 = 0;
					switch (*bg++) {
						//   0x00 - 0x22 are invalid
						case 0x23: { // air vent
							c = MAKE_TILE(14,  0); // air vent grill
							c2 = c + 1;

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE(14,  4); // bottom-left air vent grill
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE(14,  5); // bottom-right air vent grill
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						//   0x24 invalid
						//   0x25 invalid
						//   0x26 invalid
						//   0x27 invalid
						//   0x28 invalid
						//   0x29 invalid
						case 0x2A: { // cog
							c = MAKE_TILE( 8, 32); // top-left
							c2 = c + 1; // top-right

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 36); // bottom-left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 37); // bottom-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						//   0x2B invalid
						//   0x2C invalid
						case 0x2D: // brown metal supports holding red I-beam
							c = MAKE_TILE( 1, 40); // left
							c2 = c + 1; // right
							break;
						//   0x2E invalid
						//   0x2F invalid
						//   0x30 invalid
						case 0x31: // reverse gravity
							c = MAKE_TILE( 9,  4);
							c2 = c + 1;
							break;
						case 0x32: // low gravity
							c = MAKE_TILE( 9,  6);
							c2 = c + 1;
							break;
						case 0x33: // kilroy was here
							c = MAKE_TILE( 9, 12);
							c2 = c + 1;
							break;
						case 0x34:
							c = MAKE_TILE( 9,  0); // win (ners, don't)
							c2 = c + 1;
							break;
						case 0x35:
							c = MAKE_TILE(10, 24); // trading post
							c2 = c + 1;
							break;
						//   0x36 invalid
						//   0x37 invalid
						//   0x38 invalid
						//   0x39 invalid
						case 0x3A:
							c = MAKE_TILE(21,  4); // mario-style funnel top
							c2 = c + 1;
							break;
						case 0x3B:
							c = MAKE_TILE(21,  8); // mario-style funnel shaft
							c2 = c + 1;
							break;
						//   0x3C invalid
						case 0x3D:
							c = MAKE_TILE(16, 24); // red dinosaur head
							c2 = MAKE_TILE(16, 31); // red dinosaur mid
							break;
						//   0x3E invalid
						//   0x3F invalid
						//   0x40 invalid
						case 0x41: { // crystal caves planetoid
							c = MAKE_TILE(10, 10); // top-left
							c2 = c + 1; // top-right

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE(10, 14); // bottom-left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE(10, 15); // bottom-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						case 0x42: { // trading post planetoid
							c = MAKE_TILE(10, 26); // top-left
							c2 = c + 1; // top-right

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE(10, 14); // bottom-left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE(10, 15); // bottom-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						//   0x43 invalid
						case 0x44: // danger sign (falling?)
							c = MAKE_TILE( 9,  8); // dan (ger)
							c2 = c + 1;
							break;
						case 0x45: { // wide-eyed green enemy
							c = MAKE_TILE(20,  8); // left eye
							c2 = MAKE_TILE(20,  0); // mouth
							if (*bg == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x + 1;
								t->y = y;
								t->code = MAKE_TILE(20, 16); // bottom-left corner
								tiles->push_back(t);
								*bg = 0x20; // already handled this one
							}
							break;
						}
						//   0x46 invalid
						//   0x47 invalid
						//   0x48 invalid
						//   0x49 invalid
						//   0x4A invalid
						//   0x4B invalid
						//   0x4C invalid
						//   0x4D invalid
						//   0x4E invalid
						case 0x4F: { // large sewer outlet, no slime
							c = MAKE_TILE( 8, 34); // top-left
							c2 = c + 1; // top-right

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 38); // bottom-left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 39); // bottom-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						case 0x50: // spiky green multi-segment caterpillar enemy
							c = MAKE_TILE(14, 28); // last tail segment
							c2 = c + 1; // second-last tail segment
							// typically followed by two 0x6E to +1 for next two segments
							break;
						case 0x51: { // large sewer outlet, with slime
							c = MAKE_TILE( 8, 34); // top-left
							c2 = c + 1; // top-right

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 40); // bottom-left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 41); // bottom-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + (CC_MAP_WIDTH + 1) * 2 - 2;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 2;
								t->code = MAKE_TILE( 8, 44); // slime waterfall left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + (CC_MAP_WIDTH + 1) * 2 - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 2;
								t->code = MAKE_TILE( 8, 45); // slime waterfall right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + (CC_MAP_WIDTH + 1) * 3 - 2;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 3;
								t->code = MAKE_TILE( 9, 49); // slime with two chunks
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + (CC_MAP_WIDTH + 1) * 3 - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 3;
								t->code = MAKE_TILE( 9, 48); // slime with three chunks
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						//   0x52 invalid
						//   0x53 invalid
						case 0x54: { // funnel tube
							c = MAKE_TILE( 7, 38); // top-left
							c2 = MAKE_TILE( 7, 37); // top-mid

							uint8_t *next = bg;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x + 1;
								t->y = y;
								t->code = MAKE_TILE( 7, 39); // top-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 7, 36); // bottom-mid
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						//   0x55 invalid
						//   0x56 invalid
						//   0x57 invalid
						//   0x58 invalid
						//   0x59 invalid
						//   0x5A invalid
						//   0x5B invalid
						//   0x5C invalid
						case 0x5D: { // window into space
							c = MAKE_TILE( 2, 44); // top-left
							c2 = c + 1; // top-right

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 2, 48); // bottom-left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 2, 49); // bottom-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						case 0x5E: // ^ shaped brown metal supports
							c = MAKE_TILE( 1, 42); // left
							c2 = c + 1; // right
							break;
						//   0x5F invalid
						//   0x60 invalid
						//   0x61 invalid
						case 0x62: { // green wood box with yellow frame, 4x2
							c = MAKE_TILE( 8,  0); // top-left corner
							c2 = c + 1;
							// third+fourth segment on top level will be handled by 0x6E

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 8,  4); // bottom-left corner
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 8,  5); // bottom-middle segment
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH + 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x + 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 8,  6); // bottom-right segment
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH + 2;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x + 2;
								t->y = y + 1;
								t->code = MAKE_TILE( 8,  7); // bottom-right segment
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						case 0x63: { // yellow/black hazard box
							c = MAKE_TILE( 6, 36); // top-left
							c2 = c + 1; // top-right

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 6, 40); // bottom-left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 6, 41); // bottom-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						case 0x64: // danger sign
							c = MAKE_TILE( 9,  8); // dan (ger)
							c2 = c + 1;
							break;
						//   0x65 invalid
						case 0x66: // falling rocks sign
							c = MAKE_TILE( 2, 42); // fall (ing)
							c2 = c + 1;
							break;
						case 0x67: { // green wood box with yellow frame, 3x2
							c = MAKE_TILE( 8,  0); // top-left corner
							c2 = c + 2; // second top-middle segment
							// top-right corner

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 4); // bottom-left corner
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 6); // bottom-middle segment
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH + 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x + 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 7); // bottom-right segment
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						//   0x68 invalid
						//   0x69 invalid
						//   0x6A invalid
						//   0x6B invalid
						//   0x6C invalid
						case 0x6D: // mine-> sign
							c = MAKE_TILE( 4, 43); // min (e->)
							c2 = c + 1;
							break;
						//   0x6E invalid
						//   0x6F invalid
						//   0x70 invalid
						//   0x71 invalid
						case 0x72:
							c = MAKE_TILE( 2, 40); // boarded up box (left)
							c2 = c + 1; // boarded up box (right)
							break;
						//   0x73 invalid
						//   0x74 invalid
						//   0x75 invalid
						//   0x76 invalid
						//   0x77 invalid
						case 0x78: { // grey X box
							c = MAKE_TILE( 3, 36); // top-left
							c2 = c + 1; // top-right

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 3, 38); // bottom-left
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 3, 39); // bottom-right
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						case 0x79: { // green wood box with yellow frame, 2x2
							c = MAKE_TILE( 8,  0); // top-left corner
							c2 = c + 3; // top-right corner

							uint8_t *next = bg + CC_MAP_WIDTH - 1;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x - 1;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 4); // bottom-left corner
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							next = bg + CC_MAP_WIDTH;
							if (*next == 0x6E) {
								Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
								t->type = Map2D::Layer::Item::Default;
								t->x = x;
								t->y = y + 1;
								t->code = MAKE_TILE( 8, 7); // bottom-right corner
								tiles->push_back(t);
								*next = 0x20; // already handled this one
							}
							break;
						}
						//   0x7A invalid
						//   0x7B invalid
						case 0x7C: // || shaped brown metal supports
							c = MAKE_TILE( 0, 17); // left
							c2 = c + 1; // right
							break;
						//   0x7D-0xFF invalid
						default:
							std::cout << "Unknown sign type " << std::hex << std::setfill('0')
								<< std::setw(2) << (int)*(bg-1) << " in Crystal Caves map."
								<< std::dec << std::endl;
							continue;
					}
					Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
					t->type = Map2D::Layer::Item::Default;
					t->x = x - 1;
					t->y = y;
					t->code = c;
					tiles->push_back(t);
					c = c2;
					break;
				}
				//   0x5C invalid
				case 0x5D: c = MAKE_TILE( 5, 49); break; // P powerup
				case 0x5E: c = MAKE_TILE( 4,  5); break; // bird enemy
				case 0x5F: c = underscore_tile; break; // underscore platform (colour dependent on level)
				//   0x60 invalid
				case 0x61: c = MAKE_TILE(11,  9); break; // right-facing laser, moving vertically @todo more obvious icon to highlight motion
				case 0x62: c = MAKE_TILE(12,  2); break; // green gem
				case 0x63: c = MAKE_TILE(12,  3); break; // blue gem
				case 0x64: c = ibeam_tile + 1; break; // I-beam mid
				//   0x65 invalid
				case 0x66: c = fgtile + 4; break;
				case 0x67: c = fgtile + 5; break;
				case 0x68: c = fgtile + 6; break;
				case 0x69: {
					c = MAKE_TILE(13, 24); // stop sign face

					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE(13, 25); // stop sign pole
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0x6A: c = MAKE_TILE(12,  5); break; // inverted rubble pile, mid
				case 0x6B: c = MAKE_TILE(21,  2); break;
				case 0x6C: c = MAKE_TILE(21,  3); break;
				case 0x6D: c = MAKE_TILE(12,  8); break; // earth
				case 0x6E:
					if (*(bg + CC_MAP_WIDTH) == 0x87) {
						// The next line is a purple vine, so this probably is too.  0x6E is
						// supposed to mean the previous tile + 1, but CC only looks at
						// background tiles when calculating this.  So the previous tile
						// could actually be a gem, but since that's a foreground tile, CC
						// will still treat it as a -1 background tile, so adding 1 to that
						// gives tileset 0, index 0, or the purple vine.  Instead of
						// treating the background and foreground layers differently, we'll
						// just check the tile below and hope that there is never a vine
						// only one tile high...
						c = MAKE_TILE( 0,  0);
					} else if (*(bg - 2) == 0x44) {
						// Prev tile was start of I-beam, so this is actually the end
						c = ibeam_tile + 2;
					} else if (*(bg - 2) == 0x70) {
						// Prev tile was start of inverted rubble pile, so this is actually the end
						c = MAKE_TILE(12,  6);
					} else if (*(bg + CC_MAP_WIDTH) == 0x86) {
						// first part of hanging double-chain start with this for some reason
						c = MAKE_TILE( 8, 22); // hanging double-chain mid
					} else if (*(bg + CC_MAP_WIDTH) == 0x88) {
						// first part of green vines start with this for some reason
						c = MAKE_TILE( 0,  1); // green vine mid
					} else {
						c++; // next tile following previous one
					}
					break;
				case 0x6F: c = MAKE_TILE( 2,  0); break; // dormant brown walking ball enemy
				case 0x70: c = MAKE_TILE(12,  4); break; // inverted rubble pile, left
				case 0x71: c = MAKE_TILE(11, 10); break; // left-facing laser, static @todo same as 0x82
				case 0x72: c = fgtile + 0; break;
				case 0x73: c = MAKE_TILE(11,  9); break; // right-facing laser, moving vertically @todo more obvious icon to highlight motion
				case 0x74: c = fgtile + 1; break;
				case 0x75: c = MAKE_TILE(14, 20); break; // volcano eruption
				case 0x76: c = MAKE_TILE(11, 30); break; // horizontal switch, off
				case 0x77: c = MAKE_TILE(11,  9); break; // right-facing laser, static
				case 0x78: c = MAKE_TILE( 0, 12); break; // level entrance
				case 0x79: c = fgtile + 2; break;
				case 0x7A: c = MAKE_TILE(20, 45); break; // invisible blocking tile (made up mapping)
				//   0x7B invalid
				case 0x7C: c = MAKE_TILE( 3, 34); break; // brown stalactites 1 (same as 0x28) - maybe these fall?
				//   0x7D invalid
				case 0x7E: c = MAKE_TILE( 4, 12); break; // bat enemy
				//   0x7F invalid
				case 0x80: { // sector alpha sign
					c = MAKE_TILE(17,  8); // top-left
					// 0x6E will handle next part
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE(17, 12); // bottom-left
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					next = bg + CC_MAP_WIDTH + 1;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x + 1;
						t->y = y + 1;
						t->code = MAKE_TILE(17, 13); // bottom-right
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0x81: c = MAKE_TILE(11, 10); break; // left-facing laser, moving vertically @todo same as other one, what's the difference?
				case 0x82: c = MAKE_TILE(11, 10); break; // left-facing laser, static @todo same as 0x71
				case 0x83: c = MAKE_TILE(11,  9); break; // right-facing laser, moving vertically @todo same as other one, what's the difference?
				case 0x84: c = MAKE_TILE(11,  9); break; // right-facing laser, switched @todo more obvious icon to highlight switch
				case 0x85:
					if (*(bg + CC_MAP_WIDTH) == 0x85) c = MAKE_TILE( 8, 27); // hanging single-chain mid
					else c = MAKE_TILE( 8, 31); // hanging single-chain end with hook
					break;
				case 0x86:
					if (*(bg + CC_MAP_WIDTH) == 0x86) c = MAKE_TILE( 8, 22); // hanging double-chain mid
					else c = MAKE_TILE( 8, 23); // hanging double-chain end
					break;
				case 0x87:
					if (*(bg + CC_MAP_WIDTH) == 0x87) c = MAKE_TILE( 0,  0); // purple vine mid
					else c = MAKE_TILE( 0,  4); // purple vine end
					break;
				case 0x88:
					if (*(bg + CC_MAP_WIDTH) == 0x88) c = MAKE_TILE( 0,  1); // green vine mid
					else c = MAKE_TILE( 0,  5); // green vine end
					break;
				case 0x89: c = MAKE_TILE(11,  8); break; // tear revealing horizontal bar
				case 0x8A: c = MAKE_TILE(11,  4); break; // tear revealing vertical bar
				case 0x8B: c = MAKE_TILE( 0,  3); break; // candle
				case 0x8C: c = MAKE_TILE(11, 37); break; // G powerup
				//   0x8D invalid
				case 0x8E: c = MAKE_TILE(14,  9); break; // volcano top
				case 0x8F: c = MAKE_TILE(14, 12); break; // volcano bottom
				case 0x90: c = MAKE_TILE( 7, 36); break; // funnel tube stem
				case 0x91: c = MAKE_TILE( 9, 49); break; // slime with two chunks
				case 0x92: c = MAKE_TILE( 9, 26); break; // slime with two bones
				case 0x93: c = MAKE_TILE( 9, 27); break; // slime with helmet
				case 0x94: c = MAKE_TILE( 7, 28); break; // golden handrail left
				case 0x95: c = MAKE_TILE( 7, 29); break; // golden handrail mid
					// golden handrail right is handled by 0x6E
				case 0x96: c = MAKE_TILE( 7, 28); break; // wooden handrail left
				case 0x97: c = MAKE_TILE( 7, 29); break; // wooden handrail mid
					// wooden handrail right is handled by 0x6E
				case 0x98: { // hidden gem in I-beam left-end
					Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
					t->type = Map2D::Layer::Item::Default;
					t->x = x;
					t->y = y;
					t->code = MAKE_TILE(12, 36); // transparent gem
					fgtiles->push_back(t);
					c = ibeam_tile;
					break;
				}
				case 0x99: { // hidden gem in I-beam midsection
					Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
					t->type = Map2D::Layer::Item::Default;
					t->x = x;
					t->y = y;
					t->code = MAKE_TILE(12, 36); // transparent gem
					fgtiles->push_back(t);
					c = ibeam_tile + 1;
					break;
				}
				case 0x9A: { // hidden gem in I-beam right-end
					Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
					t->type = Map2D::Layer::Item::Default;
					t->x = x;
					t->y = y;
					t->code = MAKE_TILE(12, 36); // transparent gem
					fgtiles->push_back(t);
					c = ibeam_tile + 2;
					break;
				}
				//   0x9B invalid
				//   0x9C invalid
				//   0x9D invalid
				//   0x9E invalid
				case 0x9F: { // large fan blades
					c = MAKE_TILE(17, 24); // top-left
					// 0x6E will handle next part
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE(17, 28); // bottom-left
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					next = bg + CC_MAP_WIDTH + 1;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x + 1;
						t->y = y + 1;
						t->code = MAKE_TILE(17, 29); // bottom-right
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0xA0: c = MAKE_TILE( 8, 16); break; // red switch
				case 0xA1: c = MAKE_TILE( 8, 20); break; // green switch
				case 0xA2: c = MAKE_TILE( 8, 18); break; // blue switch
				case 0xA3: { // red door
					c = MAKE_TILE( 8, 24); // red-door top
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE( 8, 28); // red-door bottom
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0xA4: { // green door
					c = MAKE_TILE( 8, 26); // green-door top
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE( 8, 30); // green-door bottom
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0xA5: { // blue door
					c = MAKE_TILE( 8, 25); // blue-door top
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE( 8, 29); // blue-door bottom
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0xA6: c = MAKE_TILE( 8, 14); break; // vertical switch, on
				case 0xA7: c = MAKE_TILE(12, 49); break; // treasure chest
				case 0xA8: c = MAKE_TILE(12, 43); break; // key for treasure chest
				case 0xA9: c = MAKE_TILE(12, 46); break; // purple-spotted white egg
				case 0xAA: c = MAKE_TILE(12, 48); break; // blue mushroom
				case 0xAB: c = MAKE_TILE(12, 44); break; // red mushroom
				case 0xAC: c = MAKE_TILE(12, 45); break; // green mushroom
				//   0xAD invalid
				//   0xAE invalid
				//   0xAF invalid
				case 0xB0: c = MAKE_TILE( 0,  2); break; // hidden block revealed by head-butting
				case 0xB1: c = MAKE_TILE(11, 48); break; // thick horizontal wooden post
				case 0xB2: c = MAKE_TILE(11, 36); break; // thick vertical wooden post
				case 0xB3: c = MAKE_TILE(17,  6); break; // vertical thin wooden post (left)
				//   0xB4 invalid
				//   0xB5 invalid
				//   0xB6 invalid
				//   0xB7 invalid
				//   0xB8 invalid
				//   0xB9 invalid
				case 0xBA: c = MAKE_TILE(11, 33); break; // vertical thick metal support, middle
				case 0xBB: c = MAKE_TILE(11, 38); break; // purple mushroom
				case 0xBC: c = MAKE_TILE(11, 39); break; // tuft of grass
				case 0xBD: c = MAKE_TILE(11, 29); break; // \ ledge
				case 0xBE: c = MAKE_TILE(11, 28); break; // / ledge
				case 0xBF: c = MAKE_TILE(10, 40); break; // green pipe, left/down join
				case 0xC0: c = MAKE_TILE(10, 44); break; // green pipe, top/right join
				case 0xC1: c = MAKE_TILE(10, 46); break; // green pipe, top/left/right join
				case 0xC2: c = MAKE_TILE(10, 47); break; // green pipe, bottom/left/right join
				case 0xC3: c = MAKE_TILE(21,  7); break; // red vine, top
				case 0xC4: { // red vine, mid
					c = MAKE_TILE(21, 11);
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE(21, 15); // red vine, bottom
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0xC5: c = MAKE_TILE(10, 39); break; // green pipe, top/bottom/left/right join @todo proper pic
				case 0xC6: c = MAKE_TILE( 8, 42); break; // down arrow
				case 0xC7: c = MAKE_TILE( 8, 43); break; // up arrow
				case 0xC8: c = MAKE_TILE(16, 48); break; // barrier to contain green fish thing
				//   0xC9 invalid
				case 0xCA: c = MAKE_TILE(11, 34); break; // vertical thick metal support, bottom
				case 0xCB: c = MAKE_TILE(11, 32); break; // vertical thick metal support, top
				case 0xCC: c = MAKE_TILE( 8, 15); break; // brown lump on ceiling
				case 0xCD: c = MAKE_TILE(11, 43); break; // horiz moving platform, stationary
				case 0xCE: c = MAKE_TILE(21,  6); break; // low grass
				case 0xCF: c = MAKE_TILE(10, 43); break; // green pipe, bottom exit
				case 0xD0: c = MAKE_TILE(11,  7); break; // control panel
				case 0xD1: c = MAKE_TILE(10, 42); break; // green pipe, top exit
				//   0xD2 invalid
				//   0xD3 invalid
				//   0xD4 invalid
				case 0xD5: c = MAKE_TILE(12, 39); break; // flame
				case 0xD6: c = MAKE_TILE(11, 47); break; // vertical moving platform, stationary
				case 0xD7: c = MAKE_TILE(11, 47); break; // switched vert moving platform
				case 0xD8: c = MAKE_TILE(11, 31); break; // horizontal switch, on
				case 0xD9: c = MAKE_TILE(10, 45); break; // green pipe, top/left join
				case 0xDA: c = MAKE_TILE(10, 41); break; // green pipe, right/down join
				case 0xDB: c = MAKE_TILE(12,  8); break; // earth (intro)
				case 0xDC: c = MAKE_TILE(12,  9); break; // moving moon (intro)
				//   0xDD invalid
				//   0xDE invalid
				//   0xDF invalid
				case 0xE0: { // on/off funnel machine
					c = MAKE_TILE(17,  0); // top-left
					// 0x6E will handle next part
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE(17,  4); // bottom-left
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					next = bg + CC_MAP_WIDTH + 1;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x + 1;
						t->y = y + 1;
						t->code = MAKE_TILE(17,  5); // bottom-right
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				//   0xE1 invalid
				//   0xE2 invalid
				//   0xE3 invalid
				//   0xE4 invalid
				//   0xE5 invalid
				//   0xE6 invalid
				case 0xE7: c = MAKE_TILE(10, 48); break; // thick purple post
				case 0xE8: c = MAKE_TILE( 7, 44); break; // corrugated pipe vert
				case 0xE9: c = MAKE_TILE( 7, 45); break; // corrugated pipe horiz
				case 0xEA: c = MAKE_TILE( 7, 46); break; // corrugated pipe L-bend
				case 0xEB: c = MAKE_TILE( 7, 47); break; // corrugated pipe backwards-L-bend
				case 0xEC: c = MAKE_TILE( 7, 48); break; // corrugated pipe backwards-r-bend
				case 0xED: c = MAKE_TILE( 7, 49); break; // corrugated pipe r-bend
				//   0xEE invalid
				//   0xEF invalid
				case 0xF0: c = MAKE_TILE(17,  2); break; // wooden Y beam (left)
				//   0xF1 invalid
				case 0xF2: c = MAKE_TILE(19, 32); break; // dinosaur enemy (feet)
				case 0xF3: c = MAKE_TILE(20, 44); break; // blue ball enemy
				case 0xF4: c = MAKE_TILE( 0,  8); break; // pick
				case 0xF5: c = MAKE_TILE( 0, 10); break; // shovel
				case 0xF6: c = MAKE_TILE( 3, 32); break; // stalagmite 1
				case 0xF7: c = MAKE_TILE( 3, 33); break; // stalagmite 2
				case 0xF8: { // round glass thing
					c = MAKE_TILE(17, 16); // top-left exit door tile
					// 0x6E will handle next part
					uint8_t *next = bg + CC_MAP_WIDTH;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y + 1;
						t->code = MAKE_TILE(17, 20); // bottom-left exit door tile
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					next = bg + CC_MAP_WIDTH + 1;
					if (*next == 0x6E) {
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x + 1;
						t->y = y + 1;
						t->code = MAKE_TILE(17, 21); // bottom-right exit door tile
						tiles->push_back(t);
						*next = 0x20; // already handled this one
					}
					break;
				}
				case 0xF9: c = MAKE_TILE(11,  3); break; // clean barrel
				case 0xFA: c = MAKE_TILE(11, 11); break; // barrel leaking with green
				case 0xFB: c = MAKE_TILE(11, 35); break; // exploded barrel with red
				case 0xFC: c = MAKE_TILE( 9, 48); break; // slime with three chunks in
				case 0xFD: c = MAKE_TILE( 9, 49); break; // slime with two chunks in (duplicate of 0x91)
				case 0xFE: c = MAKE_TILE( 9, 26); break; // slime with two bones in (duplicate of 0x92)
				//   0xFF invalid
				default:
					std::cout << "Unknown tile " << std::hex << std::setfill('0')
						<< std::setw(2) << (int)*(bg-1) << " in Crystal Caves map." << std::dec
						<< std::endl;
					continue;
			}
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = x;
			t->y = y;
			t->code = c;
			tiles->push_back(t);
		}
	}

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr bgLayer(new CCavesBackgroundLayer(tiles, validBGItems));

	Map2D::Layer::ItemPtrVectorPtr validFGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr fgLayer(new CCavesBackgroundLayer(fgtiles, validFGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);

	Map2DPtr map(new GenericMap2D(
		Map::AttributePtrVectorPtr(), NO_GFX_CALLBACK,
		Map2D::HasViewport,
		CC_VIEWPORT_WIDTH, CC_VIEWPORT_HEIGHT,
		CC_MAP_WIDTH, height,
		CC_TILE_WIDTH, CC_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void CCavesMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	// Write the background layer
	unsigned long lenBG = mapWidth * mapHeight;
	uint8_t *bg = new uint8_t[lenBG];
	boost::scoped_array<uint8_t> sbg(bg);
	// Set the default background tile
	memset(bg, 0x20, lenBG);

	Map2D::LayerPtr layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		bg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	for (unsigned int y = 0; y < mapHeight; y++) {
		output << u8(mapWidth);
		output->write((char *)bg, mapWidth);
		bg += mapWidth;
	}
	output->flush();
	return;
}

SuppFilenames CCavesMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
