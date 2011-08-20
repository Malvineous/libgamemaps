/**
 * @file   fmt-map-cosmo.cpp
 * @brief  MapType and Map2D implementation for Cosmo levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Cosmo_Level_Format
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
#include "fmt-map-cosmo.hpp"

/// Width of each tile in pixels
#define CCA_TILE_WIDTH 8

/// Height of each tile in pixels
#define CCA_TILE_HEIGHT 8

/// Width of map view during gameplay, in pixels
#define CCA_VIEWPORT_WIDTH 304

/// Height of map view during gameplay, in pixels
#define CCA_VIEWPORT_HEIGHT 144

/// Maximum width of a valid level, in tiles (TODO: See if the game has a maximum value)
#define CCA_MAX_WIDTH 512

/// Maximum number of actors in a valid level (TODO: See if the game has a maximum value)
#define CCA_MAX_ACTORS 512

/// Length of the map data, in bytes
#define CCA_LAYER_LEN_BG 65528

/// Number of tiles in the map
#define CCA_NUM_TILES_BG (CCA_LAYER_LEN_BG / 2)

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

/// Convert an actor code into an image.
ImagePtr imageFromCCAActorCode(unsigned int code, VC_TILESET tileset)
	throw ()
{
	// TODO
	if (tileset.size() < 1) return ImagePtr(); // no tileset?!
	const Tileset::VC_ENTRYPTR& images = tileset[0]->getItems();
	if (images.size() < code) return ImagePtr(); // out of range
	return tileset[0]->openImage(images[code]);
}

/// Convert a map tile code into an image.
ImagePtr imageFromCCATileCode(unsigned int code, VC_TILESET tileset)
	throw ()
{
	if (tileset.size() < 2) return ImagePtr(); // no tileset?!
	int i = code >> 3; // divide by 8
	int t = 0;
	if (i >= 2000) {
		i -= 2000;
		i /= 5;
		t++; // masked tile
		if (i >= 1000) {
			// out of range!
			return ImagePtr();
		}
	}
	const Tileset::VC_ENTRYPTR& images = tileset[t]->getItems();
	if (i >= images.size()) return ImagePtr(); // out of range
	return tileset[t]->openImage(images[i]);
}

std::string CosmoMapType::getMapCode() const
	throw ()
{
	return "map-cosmo";
}

std::string CosmoMapType::getFriendlyName() const
	throw ()
{
	return "Cosmo's Cosmic Adventures level";
}

std::vector<std::string> CosmoMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("mni");
	return vcExtensions;
}

std::vector<std::string> CosmoMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Cosmo's Cosmic Adventures");
	return vcGames;
}

MapType::Certainty CosmoMapType::isInstance(istream_sptr psMap) const
	throw (std::ios::failure)
{
	psMap->seekg(0, std::ios::end);
	io::stream_offset lenMap = psMap->tellg();

	// TESTED BY: fmt_map_cosmo_isinstance_c01/c01a
	if (lenMap < 6 + CCA_LAYER_LEN_BG) return MapType::DefinitelyNo; // too short

	uint16_t mapWidth;
	psMap->seekg(2, std::ios::beg);
	psMap >> u16le(mapWidth);

	// TESTED BY: fmt_map_cosmo_isinstance_c02
	if (mapWidth > CCA_MAX_WIDTH) return MapType::DefinitelyNo; // map too wide

	uint16_t numActorInts;
	psMap >> u16le(numActorInts);

	// TESTED BY: fmt_map_cosmo_isinstance_c03
	if (numActorInts > (CCA_MAX_ACTORS * 3)) return MapType::DefinitelyNo; // too many actors

	// TESTED BY: fmt_map_cosmo_isinstance_c04
	if (6 + numActorInts * 3 > lenMap) {
		// This doesn't count the BG layer, because it seems to be possible for
		// it to be an arbitrary size - missing tiles are just left as blanks
		return MapType::DefinitelyNo; // file too small
	}

	// TODO: Read map data and confirm each uint16le is < 56000

	// TESTED BY: fmt_map_cosmo_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr CosmoMapType::create(SuppData& suppData) const
	throw (std::ios::failure)
{
	// TODO: Implement
	throw std::ios::failure("Not implemented yet!");
}

MapPtr CosmoMapType::open(istream_sptr input, SuppData& suppData) const
	throw (std::ios::failure)
{
	input->seekg(0, std::ios::end);
	io::stream_offset lenMap = input->tellg();
	input->seekg(0, std::ios::beg);

	uint16_t flags, mapWidth, numActorInts;
	input
		>> u16le(flags)
		>> u16le(mapWidth)
		>> u16le(numActorInts)
	;
	lenMap -= 6;

	// Read in the actor layer
	int numActors = numActorInts / 3;
	if (lenMap < numActors * 6) throw std::ios::failure("Map file has been truncated!");
	Map2D::Layer::ItemPtrVectorPtr actors(new Map2D::Layer::ItemPtrVector());
	actors->reserve(numActors);
	for (int i = 0; i < numActors; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		input
			>> u16le(t->code)
			>> u16le(t->x)
			>> u16le(t->y)
		;
		actors->push_back(t);
	}
	lenMap -= 6 * numActors;
	Map2D::LayerPtr actorLayer(new Map2D::Layer(
		"Actors",
		Map2D::Layer::NoCaps,
		0, 0,
		0, 0,
		actors,
		imageFromCCAActorCode
	));

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(CCA_NUM_TILES_BG);

	for (int i = 0; (i < CCA_NUM_TILES_BG) && (lenMap >= 2); i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->x = i % mapWidth;
		t->y = i / mapWidth;
		input >> u16le(t->code);
		// Don't push zero codes (these are transparent/no-tile)
		if (t->code != 0) tiles->push_back(t);
		lenMap -= 2;
	}
	Map2D::LayerPtr bgLayer(new Map2D::Layer(
		"Background",
		Map2D::Layer::NoCaps,
		0, 0,
		0, 0,
		tiles,
		imageFromCCATileCode
	));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(actorLayer);

	Map2DPtr map(new Map2D(
		Map2D::HasViewport | Map2D::HasGlobalSize | Map2D::HasGlobalTileSize,
		CCA_VIEWPORT_WIDTH, CCA_VIEWPORT_HEIGHT,
		mapWidth, 32768 / mapWidth,
		CCA_TILE_WIDTH, CCA_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long CosmoMapType::write(MapPtr map, ostream_sptr output, SuppData& suppData) const
	throw (std::ios::failure)
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw std::ios::failure("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw std::ios::failure("Incorrect layer count for this format.");

	unsigned long lenWritten = 0;

	int mapWidth, mapHeight;
	if (!map2d->getCaps() & Map2D::HasGlobalSize)
		throw std::ios::failure("Cannot write this type of map as this format.");
	map2d->getMapSize(&mapWidth, &mapHeight);

	uint16_t flags = 0;
	output
		<< u16le(flags)
		<< u16le(mapWidth)
	;
	lenWritten += 4;

	// Write the actor layer
	Map2D::LayerPtr layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr actors = layer->getAllItems();

	uint16_t numActorInts = actors->size() * 3;
	output << u16le(numActorInts);
	lenWritten += 2;
	for (Map2D::Layer::ItemPtrVector::const_iterator i = actors->begin();
		i != actors->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		output
			<< u16le((*i)->code)
			<< u16le((*i)->x)
			<< u16le((*i)->y)
		;
		lenWritten += 6;
	}

	// Write the background layer
	unsigned long lenBG = mapWidth * mapHeight;
	uint16_t *bg = new uint16_t[lenBG];
	boost::scoped_array<uint16_t> sbg(bg);
	// Set the default background tile
	memset(bg, 0x00, lenBG * 2); // 2 == sizeof(uint16le)

	layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		bg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	for (int i = 0; i < mapWidth * mapHeight; i++) {
		output << u16le(bg[i]);
	}
	lenWritten += mapWidth * mapHeight * 2;

	return lenWritten;
}


} // namespace gamemaps
} // namespace camoto