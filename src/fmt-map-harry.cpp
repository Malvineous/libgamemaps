/**
 * @file   fmt-map-harry.cpp
 * @brief  MapType and Map2D implementation for Halloween Harry/Alien Carnage.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GMF_Format_(Halloween_Harry)
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
#include <camoto/gamemaps/map2d.hpp>
#include <camoto/iostream_helpers.hpp>
#include "fmt-map-harry.hpp"

/// Width of each tile in pixels
#define HH_TILE_WIDTH 16

/// Height of each tile in pixels
#define HH_TILE_HEIGHT 16

/// Width of map view during gameplay, in pixels
#define HH_VIEWPORT_WIDTH 288

/// Height of map view during gameplay, in pixels
#define HH_VIEWPORT_HEIGHT 144

/// Number of bytes for each actor struct
#define HH_ACTOR_LEN  128

/// Map code used for 'no tile'
#define HH_DEFAULT_TILE 0xFE

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

/// Convert an actor code into an image.
ImagePtr imageFromHHActorCode(unsigned int code, VC_TILESET tileset)
	throw ()
{
	// TODO
	return ImagePtr();
}

/// Convert a map tile code into an image.
ImagePtr imageFromHHTileCode(unsigned int code, VC_TILESET tileset)
	throw ()
{
	if (tileset.size() < 1) return ImagePtr(); // no tileset?!
	const Tileset::VC_ENTRYPTR& images = tileset[0]->getItems();
	if (code >= images.size()) return ImagePtr(); // out of range
	return tileset[0]->openImage(images[code]);
}

std::string HarryMapType::getMapCode() const
	throw ()
{
	return "map-harry";
}

std::string HarryMapType::getFriendlyName() const
	throw ()
{
	return "Halloween Harry level";
}

std::vector<std::string> HarryMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("gmf");
	return vcExtensions;
}

std::vector<std::string> HarryMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Alien Carnage");
	vcGames.push_back("Halloween Harry");
	return vcGames;
}

MapType::Certainty HarryMapType::isInstance(istream_sptr psMap) const
	throw (std::ios::failure)
{
	psMap->seekg(0, std::ios::end);
	io::stream_offset lenMap = psMap->tellg();
	// TESTED BY: fmt_map_harry_isinstance_c01
	if (lenMap < 29 + 768 + 256 + 10 + 2 + 4) return MapType::DefinitelyNo; // too short

	psMap->seekg(0, std::ios::beg);

	// Check the signature
	char sig[0x12];
	psMap->read(sig, 0x12);
	// TESTED BY: fmt_map_harry_isinstance_c02
	if (strncmp(sig, "\x11SubZero Game File", 0x12) != 0) return MapType::DefinitelyNo;
	lenMap -= 0x12;

	// Skip flags
	psMap->seekg(11, std::ios::cur);
	lenMap -= 11;

	// Check palette is within range
	char pal[768];
	psMap->read(pal, 768);
	for (int i = 0; i < 768; i++) {
		// TESTED BY: fmt_map_harry_isinstance_c03
		if (pal[i] > 0x40) return MapType::DefinitelyNo;
	}
	lenMap -= 768;

	// Check tile flags are within range
	char tileFlags[256];
	psMap->read(tileFlags, 256);
	for (int i = 0; i < 256; i++) {
		// TESTED BY: fmt_map_harry_isinstance_c04
		if (tileFlags[i] > 0x01) return MapType::DefinitelyNo;
	}
	lenMap -= 256;

	// Skip unknown block
	psMap->seekg(10, std::ios::cur);
	lenMap -= 10;

	// isinstance_c01 should have prevented this
	assert(lenMap >= 6);

	uint16_t numActors;
	psMap >> u16le(numActors);
	lenMap -= 2;

	// TESTED BY: fmt_map_harry_isinstance_c05
	if (lenMap < numActors * HH_ACTOR_LEN + 4) return MapType::DefinitelyNo;

	psMap->seekg(numActors * HH_ACTOR_LEN, std::ios::cur);
	lenMap -= numActors * HH_ACTOR_LEN;

	assert(lenMap >= 4);
	uint16_t mapWidth, mapHeight;
	psMap >> u16le(mapWidth) >> u16le(mapHeight);
	lenMap -= 4;

	// TESTED BY: fmt_map_harry_isinstance_c06
	if (lenMap != mapWidth * mapHeight * 2) return MapType::DefinitelyNo;

	// TESTED BY: fmt_map_harry_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr HarryMapType::create(SuppData& suppData) const
	throw (std::ios::failure)
{
	// TODO: Implement
	throw std::ios::failure("Not implemented yet!");
}

MapPtr HarryMapType::open(istream_sptr input, SuppData& suppData) const
	throw (std::ios::failure)
{
	input->seekg(0, std::ios::beg);

	// Skip signature and flags
	input->seekg(0x12 + 11, std::ios::cur);

	// TODO: Load palette
	input->seekg(768, std::ios::cur);

	// TODO: Load tile flags
	input->seekg(256, std::ios::cur);

	// Skip unknown block
	input->seekg(10, std::ios::cur);

	uint8_t code;

	// Read in the actor layer
	uint16_t numActors;
	input >> u16le(numActors);
	Map2D::Layer::ItemPtrVectorPtr actors(new Map2D::Layer::ItemPtrVector());
	actors->reserve(numActors);
	for (int i = 0; i < numActors; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		input
			>> u8(code)
			>> u16le(t->x)
			>> u16le(t->y)
		;
		// TEMP
		t->x /= 16;
		t->y /= 16;
		// ENDTEMP
		t->code = code;
		actors->push_back(t);
		input->seekg(128-1-2-2, std::ios::cur);
	}
	Map2D::LayerPtr actorLayer(new Map2D::Layer(
		"Actors",
		Map2D::Layer::NoCaps,
		0, 0,
		0, 0,
		actors,
		imageFromHHActorCode, NULL
	));

	uint16_t mapWidth, mapHeight;
	input >> u16le(mapWidth) >> u16le(mapHeight);

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr bgtiles(new Map2D::Layer::ItemPtrVector());
	bgtiles->reserve(mapWidth * mapHeight);

	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->x = x;
			t->y = y;
			input >> u8(code);
			t->code = code;
			if (t->code != HH_DEFAULT_TILE) bgtiles->push_back(t);
		}
	}

	Map2D::LayerPtr bgLayer(new Map2D::Layer(
		"Background",
		Map2D::Layer::NoCaps,
		0, 0,
		0, 0,
		bgtiles,
		imageFromHHTileCode, NULL
	));

	// Read the foreground layer
	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
	fgtiles->reserve(mapWidth * mapHeight);
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->x = x;
			t->y = y;
			input >> u8(code);
			t->code = code;
			if (t->code != HH_DEFAULT_TILE) fgtiles->push_back(t);
		}
	}

	Map2D::LayerPtr fgLayer(new Map2D::Layer(
		"Foreground",
		Map2D::Layer::NoCaps,
		0, 0,
		0, 0,
		fgtiles,
		imageFromHHTileCode, NULL
	));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);
	layers.push_back(actorLayer);

	Map2DPtr map(new Map2D(
		Map::AttributePtrVectorPtr(),
		Map2D::HasViewport | Map2D::HasGlobalSize | Map2D::HasGlobalTileSize,
		HH_VIEWPORT_WIDTH, HH_VIEWPORT_HEIGHT,
		mapWidth * HH_TILE_WIDTH, mapHeight * HH_TILE_HEIGHT,
		HH_TILE_WIDTH, HH_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long HarryMapType::write(MapPtr map, ostream_sptr output, SuppData& suppData) const
	throw (std::ios::failure)
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw std::ios::failure("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 3)
		throw std::ios::failure("Incorrect layer count for this format.");

	int mapWidth, mapHeight;
	if (!map2d->getCaps() & Map2D::HasGlobalSize)
		throw std::ios::failure("Cannot write this type of map as this format.");
	map2d->getMapSize(&mapWidth, &mapHeight);

	unsigned long lenWritten = 0;

	uint16_t startX = 0, startY = 0;
	uint8_t mapFlags = 0;
	output
		<< nullPadded("\x11SubZero Game File", 0x12)
		<< u32le(0)
		<< u16le(startX)
		<< u16le(startY)
		<< u16le(0)
		<< u8(mapFlags)
	;
	lenWritten += 0x12 + 11;

	// TODO: Write the palette
	char pal[768];
	memset(pal, 0x00, 768);
	output->write(pal, 768);
	lenWritten += 768;

	// TODO: Write the tile flags
	char tileFlags[256];
	memset(tileFlags, 0x00, 256);
	output->write(tileFlags, 256);
	lenWritten += 256;

	// Unknown data
	char unk[10];
	memset(unk, 0x00, 10);
	output->write(unk, 10);
	lenWritten += 10;

	// Write the actor layer
	Map2D::LayerPtr layer = map2d->getLayer(2);
	const Map2D::Layer::ItemPtrVectorPtr actors = layer->getAllItems();

	uint16_t numActors = actors->size();
	output << u16le(numActors);
	lenWritten += 2;
	for (Map2D::Layer::ItemPtrVector::const_iterator i = actors->begin();
		i != actors->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		output
			<< u8((*i)->code)
			<< u16le((*i)->x)
			<< u16le((*i)->y)
		;
		char pad[128-5];
		memset(pad, 0x00, 128-5);
		output->write(pad, 128-5);
		lenWritten += 128;
	}

	output
		<< u16le(mapWidth)
		<< u16le(mapHeight)
	;
	lenWritten += 4;

	// Write the background layer
	unsigned long lenTiles = mapWidth * mapHeight;
	uint8_t *tiles = new uint8_t[lenTiles];
	boost::scoped_array<uint8_t> stiles(tiles);
	// Set the default background tile
	memset(tiles, HH_DEFAULT_TILE, lenTiles);

	layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr itemsBG = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = itemsBG->begin();
		i != itemsBG->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		tiles[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	output->write((char *)tiles, lenTiles);
	lenWritten += lenTiles;

	// Write the foreground layer
	memset(tiles, HH_DEFAULT_TILE, lenTiles);
	layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr itemsFG = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = itemsFG->begin();
		i != itemsFG->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		tiles[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	output->write((char *)tiles, lenTiles);
	lenWritten += lenTiles;

	return lenWritten;
}


} // namespace gamemaps
} // namespace camoto
