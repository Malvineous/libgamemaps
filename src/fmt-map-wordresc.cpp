/**
 * @file   fmt-map-wordresc.cpp
 * @brief  MapType and Map2D implementation for Word Rescue levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Word_Rescue
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/gamemaps/map2d.hpp>
#include "fmt-map-wordresc.hpp"

#define WR_TILE_WIDTH           16
#define WR_TILE_HEIGHT          16

/// Map code to write for locations with no tile set.
#define WR_DEFAULT_BGTILE       0xFF

/// This is the largest valid tile code in the background layer.
#define WR_MAX_VALID_TILECODE   240

// Internal codes for various items
#define WR_CODE_GRUZZLE  1
#define WR_CODE_SLIME    2
#define WR_CODE_BOOK     3
#define WR_CODE_LETTER   4
#define WR_CODE_LETTER1  4 // same as WR_CODE_LETTER
#define WR_CODE_LETTER2  5
#define WR_CODE_LETTER3  6
#define WR_CODE_LETTER4  7
#define WR_CODE_LETTER5  8
#define WR_CODE_LETTER6  9
#define WR_CODE_LETTER7  10

/// Fixed number of letters in each map (to spell a word)
#define WR_NUM_LETTERS   7

// Values used when writing items (also in isinstance)
#define INDEX_GRUZZLE 0
#define INDEX_UNKNOWN 1
#define INDEX_SLIME   2
#define INDEX_BOOK    3
#define INDEX_LETTER  4
#define INDEX_ANIM    5
#define INDEX_END     6
#define INDEX_SIZE    7

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

/// Convert a map code into an image.
ImagePtr imageFromWRCode(unsigned int code, VC_TILESET tileset)
	throw ()
{
	if (tileset.size() <= 0) return ImagePtr();
	const Tileset::VC_ENTRYPTR& images = tileset[0]->getItems();
	if (code >= images.size()) return ImagePtr(); // out of range
	return tileset[0]->openImage(images[code]);
}

/// Convert an internal item code (WR_CODE_*) into an image.
ImagePtr imageFromWRItemCode(unsigned int code, VC_TILESET tileset)
	throw ()
{
	int t;
	switch (code) {
		case WR_CODE_GRUZZLE: t = 1; code = 25; break; // TODO: Use correct image
		case WR_CODE_SLIME:   t = 0; code = 238; break;
		case WR_CODE_BOOK:    t = 0; code = 239; break;
		case WR_CODE_LETTER1:
		case WR_CODE_LETTER2:
		case WR_CODE_LETTER3:
		case WR_CODE_LETTER4:
		case WR_CODE_LETTER5:
		case WR_CODE_LETTER6:
		case WR_CODE_LETTER7:
			t = 1;  // TODO: Use correct image
			code -= WR_CODE_LETTER;
			break;
		default: return ImagePtr();
	}
	if (tileset.size() <= t) return ImagePtr();
	const Tileset::VC_ENTRYPTR& images = tileset[t]->getItems();
	if (code >= images.size()) return ImagePtr(); // out of range
	return tileset[t]->openImage(images[code]);
}

std::string WordRescueMapType::getMapCode() const
	throw ()
{
	return "map-wordresc";
}

std::string WordRescueMapType::getFriendlyName() const
	throw ()
{
	return "Word Rescue level";
}

std::vector<std::string> WordRescueMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("s0");
	vcExtensions.push_back("s1");
	vcExtensions.push_back("s2");
	vcExtensions.push_back("s3");
	vcExtensions.push_back("s4");
	vcExtensions.push_back("s5");
	vcExtensions.push_back("s6");
	vcExtensions.push_back("s7");
	vcExtensions.push_back("s8");
	vcExtensions.push_back("s9");
	vcExtensions.push_back("s10");
	vcExtensions.push_back("s11");
	vcExtensions.push_back("s12");
	vcExtensions.push_back("s13");
	vcExtensions.push_back("s14");
	vcExtensions.push_back("s15");
	vcExtensions.push_back("s16");
	vcExtensions.push_back("s17");
	vcExtensions.push_back("s18");
	vcExtensions.push_back("s19");
	return vcExtensions;
}

std::vector<std::string> WordRescueMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Word Rescue");
	return vcGames;
}

MapType::Certainty WordRescueMapType::isInstance(istream_sptr psMap) const
	throw (std::ios::failure)
{
	psMap->seekg(0, std::ios::end);
	io::stream_offset lenMap = psMap->tellg();

#define WR_MIN_HEADER_SIZE (2*15 + 4*7) // includes INDEX_LETTER

	// Make sure file is large enough for the header
	// TESTED BY: fmt_map_wordresc_isinstance_c01
	if (lenMap < WR_MIN_HEADER_SIZE) return MapType::DefinitelyNo;

	uint16_t mapWidth, mapHeight;
	psMap->seekg(0, std::ios::beg);
	psMap
		>> u16le(mapWidth)
		>> u16le(mapHeight)
	;
	psMap->seekg(2*7, std::ios::cur);

	// Check the items are each within range
	int minSize = WR_MIN_HEADER_SIZE;
	for (int i = 0; i < INDEX_SIZE; i++) {
		uint16_t count;
		if (i == INDEX_LETTER) {
			psMap->seekg(WR_NUM_LETTERS * 4, std::ios::cur);
			continue; // hard coded, included above
		}

		psMap >> u16le(count);
		minSize += count * 4;
		// Don't need to count the u16le in minSize, it's in WR_MIN_HEADER_SIZE

		// Make sure the item count is within range
		// TESTED BY: fmt_map_wordresc_isinstance_c02
		if (lenMap < minSize) return MapType::DefinitelyNo;
		psMap->seekg(count * 4, std::ios::cur);
	}

	// Read in the layer and make sure all the tile codes are within range
	for (int i = 0; i < mapWidth * mapHeight; ) {
		uint8_t num, code;
		minSize += 2;
		// Make sure the background layer isn't cut off
		// TESTED BY: fmt_map_wordresc_isinstance_c03
		if (lenMap < minSize) return MapType::DefinitelyNo;

		psMap >> u8(num) >> u8(code);
		i += num;

		// Ignore the default tile (otherwise it would be out of range)
		if (code == WR_DEFAULT_BGTILE) continue;

		// Make sure the tile values are within range
		// TESTED BY: fmt_map_wordresc_isinstance_c04
		if (code > WR_MAX_VALID_TILECODE) return MapType::DefinitelyNo;
	}

	// TESTED BY: fmt_map_wordresc_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr WordRescueMapType::create(SuppData& suppData) const
	throw (std::ios::failure)
{
	// TODO: Implement
	throw std::ios::failure("Not implemented yet!");
}

MapPtr WordRescueMapType::open(istream_sptr input, SuppData& suppData) const
	throw (std::ios::failure)
{
	input->seekg(0, std::ios::beg);

	uint16_t mapWidth, mapHeight;
	uint16_t bgColour; // EGA 0-15
	uint16_t tileset; // 3 == suburban, 2 == medieval (backX.wr)
	uint16_t backdrop; // dropX.wr, 0 == none
	uint16_t startX, startY, endX, endY;
	input
		>> u16le(mapWidth)
		>> u16le(mapHeight)
		>> u16le(bgColour)
		>> u16le(tileset)
		>> u16le(backdrop)
		>> u16le(startX)
		>> u16le(startY)
		>> u16le(endX)
		>> u16le(endY)
	;

	Map2D::Layer::ItemPtrVectorPtr items(new Map2D::Layer::ItemPtrVector());

	uint16_t gruzzleCount;
	input >> u16le(gruzzleCount);
	items->reserve(items->size() + gruzzleCount);
	for (int i = 0; i < gruzzleCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_GRUZZLE;
		items->push_back(t);
	}

	uint16_t unknownCount;
	input >> u16le(unknownCount);
	input->seekg(unknownCount * 4, std::ios::cur);

	uint16_t slimeCount;
	input >> u16le(slimeCount);
	items->reserve(items->size() + slimeCount);
	for (int i = 0; i < slimeCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_SLIME;
		items->push_back(t);
	}

	uint16_t bookCount;
	input >> u16le(bookCount);
	items->reserve(items->size() + bookCount + 7);
	for (int i = 0; i < bookCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_BOOK;
		items->push_back(t);
	}

	for (int i = 0; i < WR_NUM_LETTERS; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_LETTER + i;
		items->push_back(t);
	}

	uint16_t animCount;
	input >> u16le(animCount);
	input->seekg(animCount * 4, std::ios::cur);
	// TODO: Figure out something with animated tiles

	// Skip over trailing 0x0000
	input->seekg(2, std::ios::cur);

	Map2D::LayerPtr itemLayer(new Map2D::Layer(
		"Items",
		Map2D::Layer::NoCaps,
		0, 0,
		0, 0,
		items,
		imageFromWRItemCode
	));

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(mapWidth * mapHeight);
	for (int i = 0; i < mapWidth * mapHeight; ) {
		uint8_t num, code;
		input >> u8(num) >> u8(code);
		if (code == WR_DEFAULT_BGTILE) {
			i += num;
		} else {
			while (num-- > 0) {
				Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
				t->x = i % mapWidth;
				t->y = i / mapWidth;
				t->code = code;
				tiles->push_back(t);
				i++;
			}
		}
	}
	Map2D::LayerPtr bgLayer(new Map2D::Layer(
		"Background",
		Map2D::Layer::NoCaps,
		0, 0,
		0, 0,
		tiles,
		imageFromWRCode
	));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(itemLayer);

	Map2DPtr map(new Map2D(
		Map2D::HasGlobalSize | Map2D::HasGlobalTileSize,
		288, 152, // viewport
		mapWidth, mapHeight,
		WR_TILE_WIDTH, WR_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long WordRescueMapType::write(MapPtr map, ostream_sptr output, SuppData& suppData) const
	throw (std::ios::failure)
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw std::ios::failure("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw std::ios::failure("Incorrect layer count for this format.");

	int mapWidth, mapHeight;
	if (!map2d->getCaps() & Map2D::HasGlobalSize)
		throw std::ios::failure("Cannot write this type of map as this format.");
	map2d->getMapSize(&mapWidth, &mapHeight);

	unsigned long lenWritten = 0;

	uint16_t bgColour = 0;
	uint16_t tileset = 0;
	uint16_t backdrop = 0;
	uint16_t startX = 0;
	uint16_t startY = 0;
	uint16_t endX = 0;
	uint16_t endY = 0;
	output
		<< u16le(mapWidth)
		<< u16le(mapHeight)
		<< u16le(bgColour)
		<< u16le(tileset)
		<< u16le(backdrop)
		<< u16le(startX)
		<< u16le(startY)
		<< u16le(endX)
		<< u16le(endY)
	;
	lenWritten += 18;

	typedef std::pair<uint16_t, uint16_t> point;
	std::vector<point> itemLocations[INDEX_SIZE];

	// Prefill the letter vector with the fixed number of letters
	for (int i = 0; i < WR_NUM_LETTERS; i++) itemLocations[INDEX_LETTER].push_back(point(0, 0));

	Map2D::LayerPtr layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
			throw std::ios::failure("Layer has tiles outside map boundary!");
		}
		switch ((*i)->code) {
			case WR_CODE_GRUZZLE: itemLocations[INDEX_GRUZZLE].push_back(point((*i)->x, (*i)->y)); break;
			case WR_CODE_SLIME:   itemLocations[INDEX_SLIME].push_back(point((*i)->x, (*i)->y)); break;
			case WR_CODE_BOOK:    itemLocations[INDEX_BOOK].push_back(point((*i)->x, (*i)->y)); break;
			case WR_CODE_LETTER1: itemLocations[INDEX_LETTER][0] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER2: itemLocations[INDEX_LETTER][1] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER3: itemLocations[INDEX_LETTER][2] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER4: itemLocations[INDEX_LETTER][3] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER5: itemLocations[INDEX_LETTER][4] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER6: itemLocations[INDEX_LETTER][5] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER7: itemLocations[INDEX_LETTER][6] = point((*i)->x, (*i)->y); break;
		}
	}

	// Write out all the gruzzles, slime buckets and book positions
	for (int i = 0; i < INDEX_SIZE; i++) {

		// Write the number of items first, except for letters which are fixed at 7
		if (i != INDEX_LETTER) {
			uint16_t len = itemLocations[i].size();
			output << u16le(len);
			lenWritten += 2;
		}

		// Write the X and Y coordinates for each item
		for (std::vector<point>::const_iterator j = itemLocations[i].begin();
			j != itemLocations[i].end(); j++
		) {
			output
				<< u16le(j->first)
				<< u16le(j->second)
			;
			lenWritten += 4;
		}
	}

	// Write the background layer
	unsigned long lenTiles = mapWidth * mapHeight;
	uint8_t *tiles = new uint8_t[lenTiles];
	boost::scoped_array<uint8_t> stiles(tiles);
	// Set the default background tile
	memset(tiles, WR_DEFAULT_BGTILE, lenTiles);
	layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr bgitems = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = bgitems->begin();
		i != bgitems->end();
		i++
	) {
		if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
			throw std::ios::failure("Layer has tiles outside map boundary!");
		}
		tiles[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	// RLE encode the data
	uint8_t lastCount = 0;
	uint8_t lastCode = tiles[0];
	for (int i = 0; i < lenTiles; i++) {
		if (tiles[i] == lastCode) {
			if (lastCount == 0xFF) {
				output
					<< u8(lastCount)
					<< u8(lastCode)
				;
				lenWritten += 2;
				lastCount = 1;
			} else {
				lastCount++;
			}
		} else {
			output
				<< u8(lastCount)
				<< u8(lastCode)
			;
			lenWritten += 2;
			lastCode = tiles[i];
			lastCount = 1;
		}
	}

	return lenWritten;
}

SuppFilenames WordRescueMapType::getRequiredSupps(
	const std::string& filenameMap) const
	throw ()
{
	SuppFilenames supps;
	return supps;
}


} // namespace gamemaps
} // namespace camoto
