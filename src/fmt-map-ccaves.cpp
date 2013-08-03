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
#include "fmt-map-ccaves-mapping.hpp"

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
#define CCT(tileset, tile) (((tileset) << 8) | (tile))

/// Tile code that means "next tile"
#define CCT_NEXT  0x6E

/// Tile code that means "no tile here"
#define CCT_EMPTY 0x20

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
			const TilesetCollectionPtr& tileset) const
		{
			unsigned int ti, i;
			ti = item->code >> 8;
			i = item->code & 0xFF;

			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& ts = t->second->getItems();
			if (ti >= ts.size()) {
				std::cerr << "[fmt-map-ccaves] Out of range tile mapping to "
					"subtileset #" << ti << std::endl;
				return ImagePtr();
			}
			TilesetPtr tsub = t->second->openTileset(ts[ti]);
			if (!tsub) {
				std::cerr << "[fmt-map-ccaves] Unable to open subtileset #"
					<< ti << std::endl;
				return ImagePtr();
			}
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
	unsigned int block_tile = MAKE_TILE(21, 32); // solid brown
	//unsigned int ibeam_tile = MAKE_TILE(19,  3); // blue
	unsigned int ibeam_tile = MAKE_TILE(19,  6); // red
	unsigned int underscore_tile = MAKE_TILE(19,  0); // blue

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(CC_MAP_WIDTH * height);
	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
//	unsigned long c = 0;

/// Add a tile to the map vector
#define INSERT_TILE(dx, dy, val, flags) { \
	Map2D::Layer::ItemPtr t(new Map2D::Layer::Item()); \
	t->type = Map2D::Layer::Item::Default; \
	t->x = x + (dx); \
	t->y = y + (dy); \
	if (IS_IBEAM(val)) { \
		t->code = CCT_IBEAM(ibeam_tile, (val)); \
	} else if (IS_BLOCK(val)) { \
		t->code = CCT_BLOCK(block_tile, (val)); \
	} else if ((val) == CCT_USCORE) { \
		t->code = underscore_tile; \
	} else { \
		t->code = (val); \
	} \
	switch (flags) { \
		case CCTF_MV_NONE: \
			break; \
		case CCTF_MV_VERT: \
			t->type |= Map2D::Layer::Item::Movement; \
			t->movementFlags = Map2D::Layer::Item::DistanceLimit; \
			t->movementDistLeft = 0; \
			t->movementDistRight = 0; \
			t->movementDistUp = Map2D::Layer::Item::DistIndeterminate; \
			t->movementDistDown = Map2D::Layer::Item::DistIndeterminate; \
			break; \
		case CCTF_MV_HORZ: \
			t->type |= Map2D::Layer::Item::Movement; \
			t->movementFlags = Map2D::Layer::Item::DistanceLimit; \
			t->movementDistLeft = Map2D::Layer::Item::DistIndeterminate; \
			t->movementDistRight = Map2D::Layer::Item::DistIndeterminate; \
			t->movementDistUp = 0; \
			t->movementDistDown = 0; \
			break; \
		case CCTF_MV_DROP: \
			t->type |= Map2D::Layer::Item::Movement; \
			t->movementFlags = Map2D::Layer::Item::DistanceLimit; \
			t->movementDistLeft = 0; \
			t->movementDistRight = 0; \
			t->movementDistUp = 0; \
			t->movementDistDown = Map2D::Layer::Item::DistIndeterminate; \
			break; \
	} \
	tiles->push_back(t); \
}

/// Return the tile code at the given delta coords
#define BGTILE(dx, dy) *(bg + (CC_MAP_WIDTH + 1) * dy + dx)

/// If the given tile is CCT_NEXT, set the code and blank out the tile
#define SET_NEXT_TILE(x, y, val)	  \
	if ( \
		(val != ___________) \
		&& ((x) < CC_MAP_WIDTH) \
		&& ((y) < height) \
		&& (BGTILE((x), (y)) == CCT_NEXT) \
	) { \
		INSERT_TILE(x, y, val, CCTF_MV_NONE); \
		BGTILE(x, y) = CCT_EMPTY; \
	}

	for (unsigned int y = 0; y < height; y++) {
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
						Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
						t->type = Map2D::Layer::Item::Default;
						t->x = x;
						t->y = y;
						t->code = m.tileIndexFG;
						fgtiles->push_back(t);
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

	// Create a list of all the tiles that can be placed in a level
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr validFGItems(new Map2D::Layer::ItemPtrVector());

	for (unsigned int i = 0; i < sizeof(tileMapVine) / sizeof(TILE_MAP_VINE); i++) {
		TILE_MAP_VINE& m = tileMapVine[i];

		Map2D::Layer::ItemPtr item(new Map2D::Layer::Item());
		item->type = Map2D::Layer::Item::Default;
		item->code = m.tileIndexMid;
		validBGItems->push_back(item);

		item.reset(new Map2D::Layer::Item());
		item->type = Map2D::Layer::Item::Default;
		item->code = m.tileIndexEnd;
		validBGItems->push_back(item);
	}

	for (unsigned int i = 0; i < sizeof(tileMapSign) / sizeof(TILE_MAP_SIGN); i++) {
		TILE_MAP_SIGN& m = tileMapSign[i];

		for (unsigned int j = 0; j < sizeof(m.tileIndexBG) / sizeof(int); j++) {
			if (m.tileIndexBG[j] == ___________) continue;
			Map2D::Layer::ItemPtr item(new Map2D::Layer::Item());
			item->type = Map2D::Layer::Item::Default;
			item->code = m.tileIndexBG[j];
			validBGItems->push_back(item);
		}
	}

	for (unsigned int i = 0; i < sizeof(tileMap) / sizeof(TILE_MAP); i++) {
		TILE_MAP& m = tileMap[i];

		for (unsigned int j = 0; j < sizeof(m.tileIndexBG) / sizeof(int); j++) {
			if (m.tileIndexBG[j] == ___________) continue;
			Map2D::Layer::ItemPtr item(new Map2D::Layer::Item());
			item->type = Map2D::Layer::Item::Default;
			if (IS_IBEAM(m.tileIndexBG[j])) {
				item->code = CCT_IBEAM(ibeam_tile, m.tileIndexBG[j]);
			} else if (IS_BLOCK(m.tileIndexBG[j])) {
				item->code = CCT_BLOCK(block_tile, m.tileIndexBG[j]);
			} else if (m.tileIndexBG[j] == CCT_USCORE) {
				item->code = underscore_tile;
			} else {
				item->code = m.tileIndexBG[j];
			}
			switch (m.flags) {
				case CCTF_MV_NONE:
					break;
				case CCTF_MV_VERT:
					item->type |= Map2D::Layer::Item::Movement;
					item->movementFlags = Map2D::Layer::Item::DistanceLimit;
					item->movementDistLeft = 0;
					item->movementDistRight = 0;
					item->movementDistUp = Map2D::Layer::Item::DistIndeterminate;
					item->movementDistDown = Map2D::Layer::Item::DistIndeterminate;
					break;
				case CCTF_MV_HORZ:
					item->type |= Map2D::Layer::Item::Movement;
					item->movementFlags = Map2D::Layer::Item::DistanceLimit;
					item->movementDistLeft = Map2D::Layer::Item::DistIndeterminate;
					item->movementDistRight = Map2D::Layer::Item::DistIndeterminate;
					item->movementDistUp = 0;
					item->movementDistDown = 0;
					break;
				case CCTF_MV_DROP:
					item->type |= Map2D::Layer::Item::Movement;
					item->movementFlags = Map2D::Layer::Item::DistanceLimit;
					item->movementDistLeft = 0;
					item->movementDistRight = 0;
					item->movementDistUp = 0;
					item->movementDistDown = Map2D::Layer::Item::DistIndeterminate;
					break;
			}
			validBGItems->push_back(item);
		}
		if (m.tileIndexFG != ___________) {
			Map2D::Layer::ItemPtr item(new Map2D::Layer::Item());
			item->type = Map2D::Layer::Item::Default;
			item->code = m.tileIndexFG;
			validFGItems->push_back(item);
		}
	}

	for (unsigned int i = 0; i < sizeof(tileMap4x1) / sizeof(TILE_MAP); i++) {
		TILE_MAP& m = tileMap4x1[i];

		for (unsigned int j = 0; j < sizeof(m.tileIndexBG) / sizeof(int); j++) {
			if (m.tileIndexBG[j] == ___________) continue;
			Map2D::Layer::ItemPtr item(new Map2D::Layer::Item());
			item->type = Map2D::Layer::Item::Default;
			item->code = m.tileIndexBG[j];
			validBGItems->push_back(item);
		}
	}

	Map2D::LayerPtr bgLayer(new CCavesBackgroundLayer(tiles, validBGItems));
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
	if (map2d->getLayerCount() != 2)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	if (mapWidth != 40)
		throw stream::error("This map format can only be 40 cells wide.");

	// Safety to avoid excessive memory use
	if (mapHeight > 200)
		throw stream::error("This map format must be less than 200 cells tall.");

	unsigned long lenBG = mapWidth * mapHeight;

	// Extract the tile codes into a big array so it's easier to cross reference
	unsigned int *bgsrc = new unsigned int[lenBG];
	boost::scoped_array<unsigned int> sbgsrc(bgsrc);
	memset(bgsrc, 0xFF, lenBG * sizeof(unsigned int));

	unsigned int *bgattr = new unsigned int[lenBG];
	boost::scoped_array<unsigned int> sbgattr(bgattr);
	memset(bgattr, 0x00, lenBG * sizeof(unsigned int));

	unsigned int *fgsrc = new unsigned int[lenBG];
	boost::scoped_array<unsigned int> sfgsrc(fgsrc);
	memset(fgsrc, 0xFF, lenBG * sizeof(unsigned int));

	Map2D::LayerPtr layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = items->begin(); i != items->end(); i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		bgsrc[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
		if (
			((*i)->type & Map2D::Layer::Item::Movement)
			&& ((*i)->movementFlags & Map2D::Layer::Item::DistanceLimit)
		) {
			if (
				((*i)->movementDistUp == Map2D::Layer::Item::DistIndeterminate)
				&& ((*i)->movementDistDown == Map2D::Layer::Item::DistIndeterminate)
				&& ((*i)->movementDistLeft == 0)
				&& ((*i)->movementDistRight == 0)
			) {
				bgattr[(*i)->y * mapWidth + (*i)->x] = CCTF_MV_VERT;
			} else if (
				((*i)->movementDistUp == 0)
				&& ((*i)->movementDistDown == 0)
				&& ((*i)->movementDistLeft == Map2D::Layer::Item::DistIndeterminate)
				&& ((*i)->movementDistRight == Map2D::Layer::Item::DistIndeterminate)
			) {
				bgattr[(*i)->y * mapWidth + (*i)->x] = CCTF_MV_HORZ;
			} else if (
				((*i)->movementDistUp == 0)
				&& ((*i)->movementDistDown == Map2D::Layer::Item::DistIndeterminate)
				&& ((*i)->movementDistLeft == 0)
				&& ((*i)->movementDistRight == 0)
			) {
				bgattr[(*i)->y * mapWidth + (*i)->x] = CCTF_MV_DROP;
			} else {
				bgattr[(*i)->y * mapWidth + (*i)->x] = CCTF_MV_NONE;
			}
		} else {
			bgattr[(*i)->y * mapWidth + (*i)->x] = CCTF_MV_NONE;
		}
	}

	layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr fgitems = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = fgitems->begin(); i != fgitems->end(); i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		fgsrc[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	// Convert our codes into CC ones
	uint8_t *bgdst = new uint8_t[lenBG];
	boost::scoped_array<uint8_t> sbgdst(bgdst);

	// Set the default background tile
	memset(bgdst, 0x20, lenBG);

	unsigned int *inbg = bgsrc;
	unsigned int *inattr = bgattr;
	unsigned int *infg = fgsrc;
	uint8_t *out = bgdst;
#define REL(x, y) (*(inbg + ((y) * mapWidth) + (x)))
#define REL_FG(x, y) (*(infg + ((y) * mapWidth) + (x)))
#define PUT(x, y, c) (*(out + ((y) * mapWidth) + (x))) = c
#define IF_REL(x, y, t, c) \
	if ( \
		(t != ___________) \
		&& ((x) < mapWidth) \
		&& ((y) < mapHeight) \
		&& (REL((x), (y)) == (unsigned)t) \
	) { \
		PUT((x), (y), (c)); \
		(*(inbg + ((y) * mapWidth) + (x))) = (unsigned int)-1; \
	}

	for (unsigned int j = 0; j < lenBG; j++, inbg++, inattr++, infg++, out++) {
		if (*inbg == (unsigned int)-1) continue; // no tile here
		bool matched = false;

		// Check vines first
		for (unsigned int i = 0; i < sizeof(tileMapVine) / sizeof(TILE_MAP_VINE); i++) {
			TILE_MAP_VINE& m = tileMapVine[i];
			if (
				(
					(REL(0, 0) == (unsigned)m.tileIndexMid)
					|| (REL(0, 0) == (unsigned)m.tileIndexEnd)
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
				(REL(0, 0) == (unsigned)m.tileIndexBG[0])
				&& (REL(1, 0) == (unsigned)m.tileIndexBG[1])
			) {
				unsigned int confidence = 2;
				if (*inattr == m.flags) confidence++;
				if ((m.tileIndexBG[2] != -1) && (REL(2, 0) != CCT_EMPTY) && (REL(2, 0) == (unsigned)m.tileIndexBG[2])) confidence++;
				if ((m.tileIndexBG[3] != -1) && (REL(3, 0) != CCT_EMPTY) && (REL(3, 0) == (unsigned)m.tileIndexBG[3])) confidence++;
				if ((m.tileIndexBG[4] != -1) && (REL(0, 1) != CCT_EMPTY) && (REL(0, 1) == (unsigned)m.tileIndexBG[4])) confidence++;
				if ((m.tileIndexBG[5] != -1) && (REL(1, 1) != CCT_EMPTY) && (REL(1, 1) == (unsigned)m.tileIndexBG[5])) confidence++;
				if ((m.tileIndexBG[6] != -1) && (REL(2, 1) != CCT_EMPTY) && (REL(2, 1) == (unsigned)m.tileIndexBG[6])) confidence++;
				if ((m.tileIndexBG[7] != -1) && (REL(3, 1) != CCT_EMPTY) && (REL(3, 1) == (unsigned)m.tileIndexBG[7])) confidence++;
				if ((m.tileIndexBG[8] != -1) && (REL(0, 2) != CCT_EMPTY) && (REL(0, 2) == (unsigned)m.tileIndexBG[8])) confidence++;
				if ((m.tileIndexBG[9] != -1) && (REL(1, 2) != CCT_EMPTY) && (REL(1, 2) == (unsigned)m.tileIndexBG[9])) confidence++;
				if ((m.tileIndexBG[10] != -1) && (REL(2, 2) != CCT_EMPTY) && (REL(2, 2) == (unsigned)m.tileIndexBG[10])) confidence++;
				if ((m.tileIndexBG[11] != -1) && (REL(3, 2) != CCT_EMPTY) && (REL(3, 2) == (unsigned)m.tileIndexBG[11])) confidence++;
				if ((m.tileIndexBG[12] != -1) && (REL(0, 3) != CCT_EMPTY) && (REL(0, 3) == (unsigned)m.tileIndexBG[12])) confidence++;
				if ((m.tileIndexBG[13] != -1) && (REL(1, 3) != CCT_EMPTY) && (REL(1, 3) == (unsigned)m.tileIndexBG[13])) confidence++;
				if ((m.tileIndexBG[14] != -1) && (REL(2, 3) != CCT_EMPTY) && (REL(2, 3) == (unsigned)m.tileIndexBG[14])) confidence++;
				if ((m.tileIndexBG[15] != -1) && (REL(3, 3) != CCT_EMPTY) && (REL(3, 3) == (unsigned)m.tileIndexBG[15])) confidence++;

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
				(REL(0, 0) == (unsigned)m.tileIndexBG[0])
				&& (REL_FG(0, 0) == (unsigned)m.tileIndexFG)
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
				(REL(0, 0) == (unsigned)m.tileIndexBG[0])
				&& (REL_FG(0, 0) == (unsigned)m.tileIndexFG)
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
				(REL(0, 0) == (unsigned)m.tileIndexBG)
				&& (REL_FG(0, 0) == (unsigned)m.tileIndexFG)
			) {
				matched = true;
				PUT(0, 0, m.code);
				break;
			}
		}
		if (matched) continue;

	}

	// Write the background layer
	for (unsigned int y = 0; y < mapHeight; y++) {
		output << u8(mapWidth);
		output->write((char *)bgdst, mapWidth);
		bgdst += mapWidth;
	}
	output->flush();
	output->truncate_here();
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
