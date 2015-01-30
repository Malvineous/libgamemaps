/**
 * @file  fmt-map-sagent.cpp
 * @brief MapType and Map2D implementation for Secret Agent levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Secret_Agent
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

#include <boost/scoped_array.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp>
#include "map2d-generic.hpp"
#include "fmt-map-sagent.hpp"

#define SAM_MAP_WIDTH            40 // not including CRLF
#define SAM_MAP_WIDTH_BYTES      42 // including CRLF

#define SAM_TILE_WIDTH           16
#define SAM_TILE_HEIGHT          16

/// Size of each map file, in bytes.
#define SAM_MAP_FILESIZE       2016

/// Maximum number of rows in a level with no foreground data.
#define SAM_MAX_ROWS (SAM_MAP_FILESIZE / SAM_MAP_WIDTH_BYTES - 2)

/// This is the largest valid tile code in the background layer.
#define SAM_MAX_VALID_TILECODE   0xfb

/// This is the largest valid tile code in the world map.
#define SAM_MAX_VALID_TILECODE_WORLD   0x7a

/// Width of map view during gameplay, in pixels
#define SAM_VIEWPORT_WIDTH       320

/// Height of map view during gameplay, in pixels
#define SAM_VIEWPORT_HEIGHT      192

/// Create a tile number from a tileset number and an index into the tileset.
#define MAKE_TILE(tileset, tile) (((tileset) << 8) | (tile))
#define ST(tileset, tile) (((tileset) << 8) | (tile))
#define __________ (-1)

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

typedef struct {
	uint8_t code;
	int tiles[4 * 3];
} TILE_MAP;

#include "fmt-map-sagent-mapping.hpp"

class SAgentCommonLayer: public GenericMap2D::Layer
{
	public:
		SAgentCommonLayer(const std::string& name, ItemPtrVectorPtr& items,
			ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					name,
					Map2D::Layer::NoCaps,
					0, 0,
					0, 0,
					items, validItems
				)
		{
		}

		virtual Map2D::Layer::ImageType imageFromCode(
			const Map2D::Layer::ItemPtr& item, const TilesetCollectionPtr& tileset,
			ImagePtr *out) const
		{
			unsigned int ti, i;
			ti = item->code >> 8;
			i = item->code & 0xFF;

			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			const Tileset::VC_ENTRYPTR& ts = t->second->getItems();
			if (ti >= ts.size()) {
				std::cerr << "[fmt-map-sagent] Out of range tile mapping to "
					"subtileset #" << ti << std::endl;
				return Map2D::Layer::Unknown;
			}
			TilesetPtr tsub = t->second->openTileset(ts[ti]);
			if (!tsub) {
				std::cerr << "[fmt-map-sagent] Unable to open subtileset #"
					<< ti << std::endl;
				return Map2D::Layer::Unknown;
			}
			const Tileset::VC_ENTRYPTR& images = tsub->getItems();
			if (i >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = tsub->openImage(images[i]);
			return Map2D::Layer::Supplied;
		}
};

class SAgentBackgroundLayer: public SAgentCommonLayer
{
	public:
		SAgentBackgroundLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	SAgentCommonLayer(
					"Background",
					items, validItems
				)
		{
		}
};

class SAgentForegroundLayer: public SAgentCommonLayer
{
	public:
		SAgentForegroundLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	SAgentCommonLayer(
					"Foreground",
					items, validItems
				)
		{
		}

		virtual bool tilePermittedAt(const Map2D::Layer::ItemPtr& item,
			unsigned int x, unsigned int y, unsigned int *maxCount) const
		{
			if (x == 0) return false; // can't place tiles in this column
			return true; // otherwise unrestricted
		}
};


std::string SAgentMapType::getMapCode() const
{
	if (this->isWorldMap()) {
		return "map-sagent-world";
	}
	return "map-sagent";
}

std::string SAgentMapType::getFriendlyName() const
{
	if (this->isWorldMap()) {
		return "Secret Agent world map";
	}
	return "Secret Agent level";
}

std::vector<std::string> SAgentMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("sam");
	return vcExtensions;
}

std::vector<std::string> SAgentMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Secret Agent");
	return vcGames;
}

MapType::Certainty SAgentMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// TESTED BY: fmt_map_sagent_isinstance_c01
	if (lenMap != SAM_MAP_FILESIZE) return MapType::DefinitelyNo; // too small

	bool worldMap = this->isWorldMap();

	// Skip first row
	psMap->seekg(SAM_MAP_WIDTH_BYTES, stream::start);

	uint8_t row[SAM_MAP_WIDTH_BYTES];
	unsigned int y;
	for (y = 0; (y < SAM_MAP_FILESIZE / SAM_MAP_WIDTH_BYTES - 1) && lenMap; y++) {
		// Ensure the row data is valid
		psMap->read(row, SAM_MAP_WIDTH_BYTES);
		for (unsigned int x = 0; x < SAM_MAP_WIDTH; x++) {
			// Invalid tile
			// TESTED BY: fmt_map_sagent_isinstance_c02
			if (row[x] > SAM_MAX_VALID_TILECODE) return MapType::DefinitelyNo;
			if ((worldMap) && (row[x] > SAM_MAX_VALID_TILECODE_WORLD))
				return MapType::DefinitelyNo;
		}

		lenMap -= SAM_MAP_WIDTH_BYTES;
		// No CRLF at end of each row
		// TESTED BY: fmt_map_sagent_isinstance_c03
		if (
			(
				(row[SAM_MAP_WIDTH_BYTES - 2] != 0x0D)
				|| (row[SAM_MAP_WIDTH_BYTES - 1] != 0x0A)
			) && (
				(row[SAM_MAP_WIDTH_BYTES - 2] != 0x00)
				|| (row[SAM_MAP_WIDTH_BYTES - 1] != 0x00)
			)
		) {
			return MapType::DefinitelyNo;
		}
	}

	// This is an assert because the logic above should never allow this to happen
	// (the for loop doesn't iterate enough times.)
	assert(y <= SAM_MAX_ROWS + 1);

	// TESTED BY: fmt_map_sagent_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr SAgentMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr SAgentMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	stream::pos lenMap = input->size();

	input->seekg(0, stream::start);

	// Read the background layer
	uint8_t *bg = new uint8_t[lenMap];
	boost::scoped_array<uint8_t> sbg(bg);
	input->read((char *)bg, lenMap);

	unsigned int height = lenMap / (SAM_MAP_WIDTH + 2);

	// Read the background code
	unsigned int bgcode = strtod((char *)bg, NULL);
	bg += SAM_MAP_WIDTH + 2; // skip full first line (it only has the BG code)
	bg += SAM_MAP_WIDTH + 2; // skip full second line (not displayed in the game)

	unsigned int bgtile;
	Map::Attributes attributes;
	{
		Map::Attribute attr;
		attr.type = Map::Attribute::Enum;
		attr.name = "Background tile";
		attr.desc = "Default tile to use as level background";
		switch (bgcode) {
			case 667: bgtile = MAKE_TILE( 6, 16); attr.enumValue = 0; break;
			case 695: bgtile = MAKE_TILE( 6, 44); attr.enumValue = 1; break;
			case 767: bgtile = MAKE_TILE( 8, 16); attr.enumValue = 2; break;
			case 771: bgtile = MAKE_TILE( 8, 20); attr.enumValue = 3; break;
			case 325: bgtile = MAKE_TILE( 9, 24); attr.enumValue = 4; break;
			case 329: bgtile = MAKE_TILE( 9, 28); attr.enumValue = 5; break;
			case 333: bgtile = MAKE_TILE( 9, 32); attr.enumValue = 6; break;
			case 337: bgtile = MAKE_TILE( 9, 36); attr.enumValue = 7; break;
			case 341: bgtile = MAKE_TILE( 9, 40); attr.enumValue = 8; break;
			case 209: bgtile = MAKE_TILE(11,  8); attr.enumValue = 9; break;
			case 213: bgtile = MAKE_TILE(11, 12); attr.enumValue = 10; break;
			case 217: bgtile = MAKE_TILE(11, 16); attr.enumValue = 11; break;
			case 233: bgtile = MAKE_TILE(11, 32); attr.enumValue = 12; break;
			case 237: bgtile = MAKE_TILE(11, 36); attr.enumValue = 13; break;
			case 241: bgtile = MAKE_TILE(11, 40); attr.enumValue = 14; break;
			case 245: bgtile = MAKE_TILE(11, 44); attr.enumValue = 15; break;
			case 501: bgtile = MAKE_TILE( 1,  0); attr.enumValue = 16; break;
			default: bgtile = MAKE_TILE( 6, 16); attr.enumValue = 0; break;
		}
		attr.enumValueNames.push_back("Blue sky/grey tiles");
		attr.enumValueNames.push_back("Grey tiles");
		attr.enumValueNames.push_back("Grey stone");
		attr.enumValueNames.push_back("Blue brick");
		attr.enumValueNames.push_back("Blue diamonds");
		attr.enumValueNames.push_back("Red stone");
		attr.enumValueNames.push_back("Night sky/grey tiles");
		attr.enumValueNames.push_back("Grey/spare 1");
		attr.enumValueNames.push_back("Grey/spare 2");
		attr.enumValueNames.push_back("Red/grey tiles");
		attr.enumValueNames.push_back("Grey pattern");
		attr.enumValueNames.push_back("Blue");
		attr.enumValueNames.push_back("Diagonal red brick");
		attr.enumValueNames.push_back("Grey with white line");
		attr.enumValueNames.push_back("Blue dirt");
		attr.enumValueNames.push_back("Overlapping red squares");
		attr.enumValueNames.push_back("Grass");
		attributes.push_back(attr);
	}

	Map2D::Layer::ItemPtrVectorPtr bgtiles(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr tiles;
	bgtiles->reserve(SAM_MAP_WIDTH * height);

	const TILE_MAP *tm = this->isWorldMap() ? worldMap : tileMap;

	for (unsigned int y = 0; y < height; y++) {
		// If the row starts with a '*' then the rest of the row goes to the FG layer
		if (*bg == 0x2A) {
			y--; height--;
			tiles = fgtiles;
		} else if ((bg[SAM_MAP_WIDTH] != 0x0D) || (bg[SAM_MAP_WIDTH+1] != 0x0A)) {
			// First blank line signals the end of the map
			height = y;
			if (height == 0) throw stream::error("Map height is zero");
			break;
		} else {
			tiles = bgtiles;
		}
		for (unsigned int x = 0; x < SAM_MAP_WIDTH; x++, bg++) {
			int code = -1;
			switch (*bg) {
				case 0x20: continue; // empty space
				case 0x35: code = bgtile + 1; break; // Light shadow left
				case 0x36: code = bgtile + 2; break; // Light shadow mid
				case 0x37: code = bgtile + 3; break; // Light shadow right

				// '*' fg layer marker, valid when x>0
				case 0x2A: if (x == 0) continue; // else fall through
				default:
					// Look in the tile map
					for (unsigned int i = 0; i < sizeof(tileMap) / sizeof(TILE_MAP); i++) {
						const TILE_MAP& m = tm[i];
						if (*bg == m.code) {
							for (unsigned int dy = 0; dy < 3; dy++) {
								for (unsigned int dx = 0; dx < 4; dx++) {
									int code = m.tiles[dy * 4 + dx];
									if (code >= 0) {
										Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
										t->type = Map2D::Layer::Item::Default;
										t->x = x - (3 - dx);
										t->y = y - (2 - dy);
										t->code = code;
										tiles->push_back(t);
									}
								}
							}
							break;
						}
					}
					break;
			}
			if (code >= 0) {
				// There's a tile from the first list (not from the tileMap) so add that
				Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
				t->type = Map2D::Layer::Item::Default;
				t->x = x;
				t->y = y;
				t->code = code;
				tiles->push_back(t);
			}
		}
		bg += 2; // skip CRLF
	}

	// Set up the list of valid tiles
	Map2D::Layer::ItemPtrVectorPtr validItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 1; i < 4; i++) {
		// Add the background tiles
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = bgtile + i;
		validItems->push_back(t);
	}
	for (const TILE_MAP *next = tm; next->code > 0; next++) {
		for (unsigned int i = 0; i < 4 * 3; i++) {
			if (next->tiles[i] >= 0) {
				Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
				t->type = Map2D::Layer::Item::Default;
				t->x = 0;
				t->y = 0;
				t->code = next->tiles[i];
				validItems->push_back(t);
			}
		}
	}
	Map2D::LayerPtr bgLayer(new SAgentBackgroundLayer(bgtiles, validItems));
	Map2D::LayerPtr fgLayer(new SAgentForegroundLayer(fgtiles, validItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);

	Map2DPtr map(new GenericMap2D(
		attributes, Map::GraphicsFilenames(),
		Map2D::HasViewport,
		SAM_VIEWPORT_WIDTH, SAM_VIEWPORT_HEIGHT,
		SAM_MAP_WIDTH, height,
		SAM_TILE_WIDTH, SAM_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void SAgentMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	if (mapWidth != SAM_MAP_WIDTH)
		throw stream::error("This map format can only be "
			TOSTRING(SAM_MAP_WIDTH) " cells wide.");

	if (mapHeight > SAM_MAX_ROWS)
		throw stream::error("This map format must be less than "
			"46"/*TOSTRING(SAM_MAX_ROWS)*/ " cells tall.");

	unsigned int lenMap = mapWidth * mapHeight;

	// Extract the tile codes into a big array so it's easier to cross reference
	int *bgsrc = new int[lenMap];
	boost::scoped_array<int> sbgsrc(bgsrc);
	memset(bgsrc, 0xFF, lenMap * sizeof(int));

	int *fgsrc = new int[lenMap];
	boost::scoped_array<int> sfgsrc(fgsrc);
	memset(fgsrc, 0xFF, lenMap * sizeof(int));

	Map2D::LayerPtr layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = items->begin(); i != items->end(); i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		bgsrc[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr fgitems = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = fgitems->begin(); i != fgitems->end(); i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		fgsrc[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	// Convert our codes into SAM ones
	uint8_t *bgdst = new uint8_t[SAM_MAP_WIDTH * SAM_MAX_ROWS];
	boost::scoped_array<uint8_t> sbgdst(bgdst);
	uint8_t *fgdst = new uint8_t[SAM_MAP_WIDTH * SAM_MAX_ROWS];
	boost::scoped_array<uint8_t> sfgdst(fgdst);
	unsigned int lineCount = 0; // one per BG line, one per FG line

	// Set the default background tile
	memset(bgdst, 0x20, SAM_MAP_WIDTH * SAM_MAX_ROWS);
	memset(fgdst, 0x20, SAM_MAP_WIDTH * SAM_MAX_ROWS);

	int *inbg = bgsrc;
	int *infg = fgsrc;
	uint8_t *outbg = bgdst;
	uint8_t *outfg = fgdst;

	const TILE_MAP *tm = this->isWorldMap() ? worldMap : tileMap;
	bool fgRowValid[SAM_MAX_ROWS];
	for (unsigned int y = 0; y < SAM_MAX_ROWS; y++) {
		if (y >= mapHeight) break;

		fgRowValid[y] = false;
		for (unsigned int x = 0; x < SAM_MAP_WIDTH; x++,
			inbg++, outbg++, infg++, outfg++
		) {

			// Skip this cell if there is no tile in either layer
			if ((*inbg == -1) && (*infg == -1)) continue;

			bool foundBG = false;
			bool foundFG = false;

			// Check lights
			switch (*inbg) {
				// Normal background tile that made it in somehow
				case MAKE_TILE( 6, 16):
				case MAKE_TILE( 6, 44):
				case MAKE_TILE( 8, 16):
				case MAKE_TILE( 8, 20):
				case MAKE_TILE( 9, 24):
				case MAKE_TILE( 9, 28):
				case MAKE_TILE( 9, 32):
				case MAKE_TILE( 9, 36):
				case MAKE_TILE( 9, 40):
				case MAKE_TILE(11,  8):
				case MAKE_TILE(11, 12):
				case MAKE_TILE(11, 16):
				case MAKE_TILE(11, 32):
				case MAKE_TILE(11, 36):
				case MAKE_TILE(11, 40):
				case MAKE_TILE(11, 44):
				//case MAKE_TILE( 1,  0):
					*inbg = -1; // don't process it below
					foundBG = true;
					*outbg = 0x20;
					break;
				case MAKE_TILE( 6, 16 + 1):
				case MAKE_TILE( 6, 44 + 1):
				case MAKE_TILE( 8, 16 + 1):
				case MAKE_TILE( 8, 20 + 1):
				case MAKE_TILE( 9, 24 + 1):
				case MAKE_TILE( 9, 28 + 1):
				case MAKE_TILE( 9, 32 + 1):
				case MAKE_TILE( 9, 36 + 1):
				case MAKE_TILE( 9, 40 + 1):
				case MAKE_TILE(11,  8 + 1):
				case MAKE_TILE(11, 12 + 1):
				case MAKE_TILE(11, 16 + 1):
				case MAKE_TILE(11, 32 + 1):
				case MAKE_TILE(11, 36 + 1):
				case MAKE_TILE(11, 40 + 1):
				case MAKE_TILE(11, 44 + 1):
				//case MAKE_TILE( 1,  0 + 1):
					*inbg = -1; // don't process it below
					foundBG = true;
					*outbg = 0x35;
					break;
				case MAKE_TILE( 6, 16 + 2):
				case MAKE_TILE( 6, 44 + 2):
				case MAKE_TILE( 8, 16 + 2):
				case MAKE_TILE( 8, 20 + 2):
				case MAKE_TILE( 9, 24 + 2):
				case MAKE_TILE( 9, 28 + 2):
				case MAKE_TILE( 9, 32 + 2):
				case MAKE_TILE( 9, 36 + 2):
				case MAKE_TILE( 9, 40 + 2):
				case MAKE_TILE(11,  8 + 2):
				case MAKE_TILE(11, 12 + 2):
				case MAKE_TILE(11, 16 + 2):
				case MAKE_TILE(11, 32 + 2):
				case MAKE_TILE(11, 36 + 2):
				case MAKE_TILE(11, 40 + 2):
				case MAKE_TILE(11, 44 + 2):
				//case MAKE_TILE( 1,  0 + 2):
					*inbg = -1; // don't process it below
					foundBG = true;
					*outbg = 0x36;
					break;
				case MAKE_TILE( 6, 16 + 3):
				case MAKE_TILE( 6, 44 + 3):
				case MAKE_TILE( 8, 16 + 3):
				case MAKE_TILE( 8, 20 + 3):
				case MAKE_TILE( 9, 24 + 3):
				case MAKE_TILE( 9, 28 + 3):
				case MAKE_TILE( 9, 32 + 3):
				case MAKE_TILE( 9, 36 + 3):
				case MAKE_TILE( 9, 40 + 3):
				case MAKE_TILE(11,  8 + 3):
				case MAKE_TILE(11, 12 + 3):
				case MAKE_TILE(11, 16 + 3):
				case MAKE_TILE(11, 32 + 3):
				case MAKE_TILE(11, 36 + 3):
				case MAKE_TILE(11, 40 + 3):
				case MAKE_TILE(11, 44 + 3):
				//case MAKE_TILE( 1,  0 + 3):
					*inbg = -1; // don't process it below
					foundBG = true;
					*outbg = 0x37;
					break;
			}

			// Check other tiles
			for (const TILE_MAP *tnext = tm; tnext->code > 0; tnext++) {
				if ((!foundBG) && (*inbg == tnext->tiles[4 * 3 - 1])) {
					// TODO: Check surrounding area?
					*outbg = tnext->code;
					if (foundFG) break;
					foundBG = true;
				}
				if ((!foundFG) && (*infg == tnext->tiles[4 * 3 - 1])) {
					// TODO: Check surrounding area?
					*outfg = tnext->code;
					fgRowValid[y] = true; // remember to write out this row later
					if (foundBG) break;
					foundFG = true;
				}
			}
		}
		lineCount++; // background layer
		if (fgRowValid[y]) lineCount++; // foreground layer
	}

	if (lineCount > SAM_MAX_ROWS) {
		throw stream::error(createString("Too many rows with foreground tiles.  "
			"You need to remove all foreground tiles from "
			<< (lineCount - SAM_MAX_ROWS) << " row(s) in order to save the level."));
	}

	if (map->attributes.size() != 1) {
		throw stream::error("Cannot write map as there is an incorrect number "
			"of attributes set.");
	}

	Map::Attribute& attrBG = map->attributes[0];
	if (attrBG.type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (bg != enum)");
	}
	int bgcode;
	switch (attrBG.enumValue) {
		case 0: bgcode = 667; break;
		case 1: bgcode = 695; break;
		case 2: bgcode = 767; break;
		case 3: bgcode = 771; break;
		case 4: bgcode = 325; break;
		case 5: bgcode = 329; break;
		case 6: bgcode = 333; break;
		case 7: bgcode = 337; break;
		case 8: bgcode = 341; break;
		case 9: bgcode = 209; break;
		case 10: bgcode = 213; break;
		case 11: bgcode = 217; break;
		case 12: bgcode = 233; break;
		case 13: bgcode = 237; break;
		case 14: bgcode = 241; break;
		case 15: bgcode = 245; break;
		case 16: bgcode = 501; break;
		default: bgcode = 667; break;
	}
	std::string strBGcode = createString(bgcode);
	strBGcode += std::string(SAM_MAP_WIDTH - strBGcode.length(), ' ');
	strBGcode.append("\x0D\x0A");

	// Write out the map
	outbg = bgdst;
	outfg = fgdst;
	output->write(strBGcode);
	output << std::string(SAM_MAP_WIDTH, ' ') << u8(0x0D) << u8(0x0A);
	unsigned int numLinesWritten = 2;
	for (unsigned int y = 0; y < SAM_MAX_ROWS - 2; y++) {
		if (y >= mapHeight) {
			// Past the end of the map, pad out with nulls
			output << nullPadded("", SAM_MAP_FILESIZE - numLinesWritten * SAM_MAP_WIDTH_BYTES);
			break;
		}
		output->write(outbg, SAM_MAP_WIDTH);
		output << u8(0x0D) << u8(0x0A);
		numLinesWritten++;
		if (fgRowValid[y]) {
			*outfg = 0x2A; // '*'
			output->write(outfg, SAM_MAP_WIDTH);
			output << u8(0x0D) << u8(0x0A);
			numLinesWritten++;
		}
		outbg += SAM_MAP_WIDTH;
		outfg += SAM_MAP_WIDTH;
	}
	assert(output->tellp() == SAM_MAP_FILESIZE);
	output->truncate_here();
	return;
}

SuppFilenames SAgentMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

bool SAgentMapType::isWorldMap() const
{
	return false;
}


bool SAgentWorldMapType::isWorldMap() const
{
	return true;
}

} // namespace gamemaps
} // namespace camoto
