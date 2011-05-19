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
#include <boost/scoped_array.hpp>
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

struct XargonObject: virtual public Map2D::Layer::Item {
	// uint8_t code;
	// uint16_t x
	// uint16_t y
	uint16_t spdHoriz;
	uint16_t spdVert;
	uint16_t width;
	uint16_t height;
	uint16_t subType;
	uint16_t subState;
	uint16_t stateCount;
	uint16_t link;
	uint16_t flags;
	uint32_t pointer;
	uint16_t info;
	uint16_t zapHold;
};

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

	// TESTED BY: fmt_map_xargon_isinstance_c02
	if (lenMap == offStrings) return MapType::DefinitelyYes; // exact size w/ no strings

	// TESTED BY: fmt_map_xargon_isinstance_c03
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
	input->seekg(0, std::ios::end);
	io::stream_offset lenMap = input->tellg();

	if (lenMap < XR_OFFSET_OBJLAYER + 2) {
		throw std::ios::failure("Map file has been truncated! (background section cut)");
	}

	// Read the background layer
	input->seekg(0, std::ios::beg);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(XR_MAP_WIDTH * XR_MAP_HEIGHT);
	for (int x = 0; x < XR_MAP_WIDTH; x++) {
		for (int y = 0; y < XR_MAP_HEIGHT; y++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->x = x;
			t->y = y;
			input >> u16le(t->code);
			tiles->push_back(t);
		}
	}
	lenMap -= XR_MAP_WIDTH * XR_MAP_HEIGHT * 2;

	Map2D::LayerPtr bgLayer(new Map2D::Layer(
		"Background",
		Map2D::Layer::HasOwnTileSize,
		0, 0,   // Layer size unused
		XR_TILE_SIZE, XR_TILE_SIZE,
		tiles,
		imageFromTileCode
	));

	// Read the object layer
	uint16_t numObjects;
	input >> u16le(numObjects);
	lenMap -= 2;
	if (lenMap < numObjects * XR_OBJ_ENTRY_LEN) {
		throw std::ios::failure("Map file has been truncated! (objects section cut)");
	}
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
	lenMap -= XR_OBJ_ENTRY_LEN * numObjects;

	Map2D::LayerPtr objLayer(new Map2D::Layer(
		"Objects",
		Map2D::Layer::HasOwnTileSize,
		0, 0, // Layer size unused
		1, 1,
		objects,
		imageFromObjectCode
	));

	// Make sure we read in all the objects correctly
	assert(input->tellg() == XR_OFFSET_OBJLAYER + 2 + numObjects * XR_OBJ_ENTRY_LEN);

	// Skip over the savegame data
	if (lenMap < XR_LEN_SAVEDATA) {
		throw std::ios::failure("Map file has been truncated! (savedata section cut)");
	}
	input->seekg(XR_LEN_SAVEDATA, std::ios::cur);
	lenMap -= XR_LEN_SAVEDATA;

	// Read the text elements
	while (lenMap > 2) {
		//Map2DLayer::TextPtr next(new Map2DLayer::Text());
		std::string value;
		uint16_t lenStr;
		input >> u16le(lenStr);
		lenStr++; // include terminating null
		lenMap -= 2;
		if (lenMap < lenStr) throw std::ios::failure("Map file has been truncated! (text section cut)");
		input >> null_padded(value, lenStr, false);
		lenMap -= lenStr;
		std::cout << "Found string: " << value << "\n";
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
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw std::ios::failure("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw std::ios::failure("Incorrect layer count for this format.");

	unsigned long lenWritten = 0;

	// Write the background layer
	uint16_t *bg = new uint16_t[XR_MAP_WIDTH * XR_MAP_HEIGHT];
	boost::scoped_array<uint16_t> sbg(bg);
	// Set the default background tile
	memset(bg, 0x00, XR_MAP_WIDTH * XR_MAP_HEIGHT * 2); // 2 == sizeof(uint16le)

	Map2D::LayerPtr layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		assert(((*i)->x < XR_MAP_WIDTH) && ((*i)->y < XR_MAP_HEIGHT));
		bg[(*i)->x * XR_MAP_HEIGHT + (*i)->y] = (*i)->code;
	}

	for (int i = 0; i < XR_MAP_WIDTH * XR_MAP_HEIGHT; i++) {
		output << u16le(bg[i]);
	}
	lenWritten += XR_MAP_WIDTH * XR_MAP_HEIGHT * 2;

	// Write the object layer
	layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr objects = layer->getAllItems();

	uint16_t numObjects = objects->size();
	output << u16le(numObjects);
	lenWritten += 2;
	for (Map2D::Layer::ItemPtrVector::const_iterator i = objects->begin();
		i != objects->end();
		i++
	) {
		XargonObject *obj = dynamic_cast<XargonObject *>(i->get());
		if (!obj) continue;

		output
			<< u8(obj->code)
			<< u16le(obj->x)
			<< u16le(obj->y)
			<< u16le(obj->spdHoriz)
			<< u16le(obj->spdVert)
			<< u16le(obj->width)
			<< u16le(obj->height)
			<< u16le(obj->subType)
			<< u16le(obj->subState)
			<< u16le(obj->stateCount)
			<< u16le(obj->link)
			<< u16le(obj->flags)
			<< u32le(obj->pointer)
			<< u16le(obj->info)
			<< u16le(obj->zapHold)
		;
		lenWritten += 31;
	}

	// Write out savedata
	char blank[XR_LEN_SAVEDATA];
	memset(blank, 0x00, XR_LEN_SAVEDATA);
	output->write(blank, XR_LEN_SAVEDATA);

	// TODO: Write out text strings

	return lenWritten;
}


} // namespace gamemaps
} // namespace camoto
