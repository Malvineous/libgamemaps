/**
 * @file   fmt-map-ddave.cpp
 * @brief  MapType and Map2D implementation for Dangerous Dave levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/DDave_Map_Format
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#include "fmt-map-ddave.hpp"
#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>

#define DD_MAP_WIDTH            100
#define DD_MAP_HEIGHT           10

#define DD_LAYER_OFF_PATH       0
#define DD_LAYER_LEN_PATH       256
#define DD_LAYER_OFF_BG         (DD_LAYER_OFF_PATH + DD_LAYER_LEN_PATH)
#define DD_LAYER_LEN_BG         (DD_MAP_WIDTH * DD_MAP_HEIGHT)
#define DD_FILESIZE             (256 + 1024)

/// This is the largest valid tile code in the background layer.
#define DD_MAX_VALID_TILECODE 52

namespace camoto {
namespace gamemaps {

std::string DDaveMapType::getMapCode() const
	throw ()
{
	return "map-ddave";
}

std::string DDaveMapType::getFriendlyName() const
	throw ()
{
	return "Dangerous Dave level";
}

std::vector<std::string> DDaveMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dav");
	return vcExtensions;
}

std::vector<std::string> DDaveMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Dangerous Dave");
	return vcGames;
}

E_CERTAINTY DDaveMapType::isInstance(iostream_sptr psMap) const
	throw (std::ios::failure)
{
	psMap->seekg(0, std::ios::end);
	io::stream_offset lenMap = psMap->tellg();

	// TESTED BY: fmt_map_ddave_isinstance_c01
	if (lenMap != DD_FILESIZE) return EC_DEFINITELY_NO; // wrong size

	// Read in the layer and make sure all the tile codes are within range
	uint8_t bg[DD_LAYER_LEN_BG];
	psMap->seekg(DD_LAYER_OFF_BG, std::ios::beg);
	psMap->read((char *)bg, DD_LAYER_LEN_BG);
	if (psMap->gcount() != DD_LAYER_LEN_BG) return EC_DEFINITELY_NO; // read error
	for (int i = 0; i < DD_LAYER_LEN_BG; i++) {
		if (bg[i] > DD_MAX_VALID_TILECODE) return EC_DEFINITELY_NO; // invalid tile
	}

	// TESTED BY: fmt_map_ddave_isinstance_c00
	return EC_DEFINITELY_YES;
}

MapPtr DDaveMapType::create(MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	// TODO: Implement
	throw std::ios::failure("Not implemented yet!");
}

MapPtr DDaveMapType::open(istream_sptr input, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	input->seekg(0, std::ios::beg);

	// Read the path
	uint8_t path[DD_LAYER_LEN_PATH];
	input->read((char *)path, DD_LAYER_LEN_PATH);

	// Read the background layer
	uint8_t bg[DD_LAYER_LEN_BG];
	input->read((char *)bg, DD_LAYER_LEN_BG);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(DD_MAP_WIDTH * DD_MAP_HEIGHT);
	for (int i = 0; i < DD_LAYER_LEN_BG; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->x = i % DD_MAP_WIDTH;
		t->y = i / DD_MAP_WIDTH;
		t->code = bg[i];
		tiles->push_back(t);
	}
	Map2D::LayerPtr bgLayer(new Map2D::Layer(
		Map2D::Layer::HasOwnSize | Map2D::Layer::HasOwnTileSize,
		DD_MAP_WIDTH, DD_MAP_HEIGHT,
		16, 16,
		tiles
	));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new Map2D(
		Map2D::NoCaps,
		0, 0,
		0, 0,
		layers
	));

	return map;
}

unsigned long DDaveMapType::write(MapPtr map, ostream_sptr output, MP_SUPPDATA& suppData) const
	throw (std::ios::failure)
{
	throw std::ios::failure("Not implemented yet");
}


} // namespace gamemaps
} // namespace camoto
