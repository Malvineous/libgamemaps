/**
 * @file   fmt-map-wacky.cpp
 * @brief  MapType and Map2D implementation for Wacky Wheels levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Wacky_Wheels
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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/progress.hpp>
#include <boost/shared_array.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <exception>
#include <string.h>

#include "fmt-map-wacky.hpp"
#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>

#define WW_MAP_WIDTH            64
#define WW_MAP_HEIGHT           64
#define WW_TILE_WIDTH           16
#define WW_TILE_HEIGHT          16

#define WW_LAYER_OFF_BG         0
#define WW_LAYER_LEN_BG         (WW_MAP_WIDTH * WW_MAP_HEIGHT)
#define WW_FILESIZE             (WW_LAYER_LEN_BG)

/// Map code to write for locations with no tile set.
#define WW_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define WW_MAX_VALID_TILECODE 0x6C

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

/// Convert a map code into an image.
ImagePtr imageFromWWCode(unsigned int code, VC_TILESET tileset)
	throw ()
{
	// TODO
	return ImagePtr();
}

std::string WackyMapType::getMapCode() const
	throw ()
{
	return "map-wacky";
}

std::string WackyMapType::getFriendlyName() const
	throw ()
{
	return "Wacky Wheels level";
}

std::vector<std::string> WackyMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("m");
	return vcExtensions;
}

std::vector<std::string> WackyMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Wacky Wheels");
	return vcGames;
}

MapType::Certainty WackyMapType::isInstance(istream_sptr psMap) const
	throw (std::ios::failure)
{
	psMap->seekg(0, std::ios::end);
	io::stream_offset lenMap = psMap->tellg();

	// TESTED BY: fmt_map_wacky_isinstance_c01
	if (lenMap != WW_FILESIZE) return MapType::DefinitelyNo; // wrong size

	// Read in the layer and make sure all the tile codes are within range
	uint8_t bg[WW_LAYER_LEN_BG];
	psMap->seekg(WW_LAYER_OFF_BG, std::ios::beg);
	psMap->read((char *)bg, WW_LAYER_LEN_BG);
	if (psMap->gcount() != WW_LAYER_LEN_BG) throw std::ios::failure("short read");
	for (int i = 0; i < WW_LAYER_LEN_BG; i++) {
		// TESTED BY: fmt_map_wacky_isinstance_c02
		if (bg[i] > WW_MAX_VALID_TILECODE) return MapType::DefinitelyNo; // invalid tile
	}

	// TESTED BY: fmt_map_wacky_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr WackyMapType::create(SuppData& suppData) const
	throw (std::ios::failure)
{
	// TODO: Implement
	throw std::ios::failure("Not implemented yet!");
}

MapPtr WackyMapType::open(istream_sptr input, SuppData& suppData) const
	throw (std::ios::failure)
{
	input->seekg(0, std::ios::beg);

	// Read the background layer
	uint8_t bg[WW_LAYER_LEN_BG];
	input->read((char *)bg, WW_LAYER_LEN_BG);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(WW_MAP_WIDTH * WW_MAP_HEIGHT);
	for (int i = 0; i < WW_LAYER_LEN_BG; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->x = i % WW_MAP_WIDTH;
		t->y = i / WW_MAP_WIDTH;
		t->code = bg[i];
		tiles->push_back(t);
	}
	Map2D::LayerPtr bgLayer(new Map2D::Layer(
		"Background",
		Map2D::Layer::NoCaps,
		0, 0,
		0, 0,
		tiles,
		imageFromWWCode
	));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new Map2D(
		Map2D::HasGlobalSize | Map2D::HasGlobalTileSize,
		0, 0,
		WW_MAP_WIDTH, WW_MAP_HEIGHT,
		WW_TILE_WIDTH, WW_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long WackyMapType::write(MapPtr map, ostream_sptr output, SuppData& suppData) const
	throw (std::ios::failure)
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw std::ios::failure("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw std::ios::failure("Incorrect layer count for this format.");

	unsigned long lenWritten = 0;

	// Write the background layer
	uint8_t bg[WW_LAYER_LEN_BG];
	memset(bg, WW_DEFAULT_BGTILE, WW_LAYER_LEN_BG); // default background tile
	Map2D::LayerPtr layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > WW_MAP_WIDTH) || ((*i)->y > WW_MAP_HEIGHT)) {
			throw std::ios::failure("Layer has tiles outside map boundary!");
		}
		bg[(*i)->y * WW_MAP_WIDTH + (*i)->x] = (*i)->code;
	}

	output->write((char *)bg, WW_LAYER_LEN_BG);
	lenWritten += WW_LAYER_LEN_BG;

	return lenWritten;
}


} // namespace gamemaps
} // namespace camoto
