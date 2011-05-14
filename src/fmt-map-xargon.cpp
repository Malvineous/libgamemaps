/**
 * @file   fmt-map-xargon.cpp
 * @brief  MapType and Map2D implementation for Xargon maps.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Xargon_Map_Format
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

#include "fmt-map-xargon.hpp"
#include <camoto/iostream_helpers.hpp>
#include <camoto/debug.hpp>

#define XR_OBJ_ENTRY_LEN        31
#define XR_MAP_WIDTH            128
#define XR_MAP_HEIGHT           64
#define XR_OFFSET_OBJLAYER      (XR_MAP_WIDTH * XR_MAP_HEIGHT * 2)
#define XR_LEN_SAVEDATA         97

/// Tile dimensions in background layer
#define XR_TILE_SIZE 16

/// Maximum number of strings in the stringdata section
#define XR_SAFETY_MAX_STRINGS   512

namespace camoto {
namespace gamemaps {

std::string XargonMapType::getMapCode() const
	throw ()
{
	return "map-xargon";
}

std::string XargonMapType::getFriendlyName() const
	throw ()
{
	return "Xargon map";
}

std::vector<std::string> XargonMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("xr0"); // TODO: confirm
	vcExtensions.push_back("xr1");
	vcExtensions.push_back("xr2");
	vcExtensions.push_back("xr3");
	return vcExtensions;
}

std::vector<std::string> XargonMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Xargon");
	return vcGames;
}

MapType::Certainty XargonMapType::isInstance(istream_sptr psMap) const
	throw (std::ios::failure)
{
	psMap->seekg(0, std::ios::end);
	io::stream_offset lenMap = psMap->tellg();

	// TESTED BY: fmt_map_xargon_isinstance_c01
	if (lenMap < XR_OFFSET_OBJLAYER + 2) return MapType::DefinitelyNo; // too short

	psMap->seekg(XR_OFFSET_OBJLAYER, std::ios::beg);
	uint16_t numObjects;
	psMap >> u16le(numObjects);

	io::stream_offset offStrings = XR_OFFSET_OBJLAYER + 2 +
		numObjects * XR_OBJ_ENTRY_LEN + XR_LEN_SAVEDATA;
	if (lenMap < offStrings + 2) return MapType::DefinitelyNo; // too short
	psMap->seekg(offStrings, std::ios::beg);

	int i;
	for (i = 0; i < XR_SAFETY_MAX_STRINGS; i++) {
		uint16_t lenStr;
		psMap >> u16le(lenStr);
		offStrings += lenStr + 2 + 1; // +2 for uint16le, +1 for terminating null
		if (lenMap < offStrings + 2) return MapType::DefinitelyNo; // too short

		psMap->seekg(offStrings, std::ios::beg);
		if (offStrings == lenMap) break; // reached EOF
	}
	if (i == XR_SAFETY_MAX_STRINGS) return MapType::DefinitelyNo; // too many strings

	// TESTED BY: fmt_map_xargon_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr XargonMapType::create(SuppData& suppData) const
	throw (std::ios::failure)
{
	// TODO: Implement
	throw std::ios::failure("Not implemented yet!");
}

MapPtr XargonMapType::open(istream_sptr input, SuppData& suppData) const
	throw (std::ios::failure)
{
	// Read the background layer
	input->seekg(0, std::ios::beg);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(XR_MAP_WIDTH * XR_MAP_HEIGHT);
	for (int x = 0; x < XR_MAP_WIDTH; x++) {
		for (int y = 0; y < XR_MAP_HEIGHT; y++) {
		//for (int x = 0; x < XR_MAP_WIDTH; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->x = x;
			t->y = y;
			input >> u16le(t->code);
			tiles->push_back(t);
		}
	}

	if (input->tellg() != XR_OFFSET_OBJLAYER) {
		throw std::ios::failure("Map file has been truncated!  No object layer present.");
	}

	Map2D::LayerPtr bgLayer(new Map2D::Layer(
		"Background",
		Map2D::Layer::HasOwnTileSize,
		0, 0,   // Layer size unused
		XR_TILE_SIZE, XR_TILE_SIZE,
		tiles
	));

	// Read the object layer
	uint16_t numObjects;
	input >> u16le(numObjects);
	Map2D::Layer::ItemPtrVectorPtr objects(new Map2D::Layer::ItemPtrVector());
	objects->reserve(numObjects);
	for (int i = 0; i < numObjects; i++) {
		XargonObject *obj = new XargonObject;
		Map2D::Layer::ItemPtr objItem(obj);
		uint8_t code;
		input
			>> u8(code)
			>> u16le(obj->x)
			>> u16le(obj->y)
			>> u16le(obj->spdHoriz)
			>> u16le(obj->spdVert)
			>> u16le(obj->width)
			>> u16le(obj->height)
			>> u16le(obj->subType)
			>> u16le(obj->subState)
			>> u16le(obj->stateCount)
			>> u16le(obj->link)
			>> u16le(obj->flags)
			>> u32le(obj->pointer)
			>> u16le(obj->info)
			>> u16le(obj->zapHold)
		;
		obj->code = code; // u8 to u16 conversion
		objects->push_back(objItem);
	}

	Map2D::LayerPtr objLayer(new Map2D::Layer(
		"Objects",
		Map2D::Layer::HasOwnTileSize,
		0, 0, // Layer size unused
		1, 1,
		objects
	));

	// Make sure we read in all the objects correctly
	assert(input->tellg() == XR_OFFSET_OBJLAYER + 2 + numObjects * XR_OBJ_ENTRY_LEN);

	// Skip over the savegame data
	input->seekg(XR_LEN_SAVEDATA, std::ios::cur);

	// Read the text elements
	while (!input->eof()) {
		//Map2DLayer::TextPtr next(new Map2DLayer::Text());
		std::string value;
		uint16_t lenStr;
		try {
			input >> u16le(lenStr);
			input >> null_padded(value, lenStr + 1, false);
			std::cout << "Found string: " << value << "\n";
		} catch (std::ios::failure) {
			// EOF
			break;
		}
	}

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(objLayer);

	Map2DPtr map(new Map2D(
		Map2D::HasViewport | Map2D::HasGlobalSize,
		20 * XR_TILE_SIZE, 10 * XR_TILE_SIZE, // viewport size
		XR_MAP_WIDTH * XR_TILE_SIZE, XR_MAP_HEIGHT * XR_TILE_SIZE,
		0, 0,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long XargonMapType::write(MapPtr map, ostream_sptr output, SuppData& suppData) const
	throw (std::ios::failure)
{
	throw std::ios::failure("Not implemented yet");
}


} // namespace gamemaps
} // namespace camoto
