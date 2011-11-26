/**
 * @file   fmt-map-xargon.cpp
 * @brief  MapType and Map2D implementation for Jill of the Jungle and Xargon.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Jill_of_the_Jungle_Map_Format
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

#include <iostream>
#include <list>
#include <boost/scoped_array.hpp>
#include <camoto/gamemaps/map2d.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/stream_string.hpp>
#include "fmt-map-xargon.hpp"

#define XR_OBJ_ENTRY_LEN        31
#define XR_MAP_WIDTH            128
#define XR_MAP_HEIGHT           64
#define XR_OFFSET_OBJLAYER      (XR_MAP_WIDTH * XR_MAP_HEIGHT * 2)

#define XR_LEN_SAVEDATA         97 ///< Length of savedata section for Xargon
#define JILL_LEN_SAVEDATA       70 ///< Length of savedata section for Jill

/// Tile dimensions in background layer
#define XR_TILE_WIDTH   16
#define XR_TILE_HEIGHT  16

/// Maximum number of strings in the stringdata section
#define XR_SAFETY_MAX_STRINGS   512

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

SweeneyBackgroundLayer::SweeneyBackgroundLayer(ItemPtrVectorPtr& items,
	SweeneyMapType::image_map_sptr imgMap, ItemPtrVectorPtr& validItems)
	throw () :
		Map2D::Layer(
			"Background",
			Map2D::Layer::HasPalette,
			0, 0,   // Layer size unused
			0, 0,
			items, validItems
		),
		imgMap(imgMap)
{
}

SweeneyBackgroundLayer::~SweeneyBackgroundLayer()
	throw ()
{
}

ImagePtr SweeneyBackgroundLayer::imageFromCode(unsigned int code,
	VC_TILESET& tileset)
	throw ()
{
	if (tileset.size() < 1) return ImagePtr(); // no tileset?!
	try {
		const Tileset::VC_ENTRYPTR& tilesets = tileset[0]->getItems();

		uint16_t v = (*this->imgMap)[code & 0x0FFF];
		uint8_t t = ((v >> 8) & 0xFF);
		uint8_t i = v & 0xFF;
		if (tilesets[t]->attr & Tileset::EmptySlot) {
			std::cerr << "[SweeneyBackgroundLayer] Tried to open tileset 0x"
				<< std::hex << (int)t << std::dec << " but it's an empty slot!"
				<< std::endl;
			return ImagePtr();
		}
		TilesetPtr tls = tileset[0]->openTileset(tilesets[t]);
		const Tileset::VC_ENTRYPTR& images = tls->getItems();
		if (i >= images.size()) return ImagePtr(); // out of range
		if (images[i]->attr & Tileset::EmptySlot) {
			std::cerr << "[SweeneyBackgroundLayer] Tried to open image " << t << "."
				<< i << " but it's an empty slot!" << std::endl;
			return ImagePtr();
		}
		return tls->openImage(images[i]);
	} catch (...) {
		std::cerr << "[SweeneyBackgroundLayer] Exception trying to open image for "
			"tile code " << code << std::endl;
		return ImagePtr();
	}
}

PaletteTablePtr SweeneyBackgroundLayer::getPalette(VC_TILESET& tileset)
	throw ()
{
	// Try (Xargon) to load the palette from tile 0.5.0
	const Tileset::VC_ENTRYPTR& tilesets = tileset[0]->getItems();
	if (tilesets.size() > 5) {
		TilesetPtr tls = tileset[0]->openTileset(tilesets[5]);
		const Tileset::VC_ENTRYPTR& images = tls->getItems();
		if (images.size() > 0) {
			ImagePtr img = tls->openImage(images[0]);
			if (img->getCaps() & Image::HasPalette) {
				return img->getPalette();
			}
		}
	}

	// Otherwise (Jill) use the tileset's palette
	if (tileset[0]->getCaps() & Tileset::HasPalette) {
		return tileset[0]->getPalette();
	}

	std::cerr << "[SweeneyBackgroundLayer] Couldn't load palette from tile "
		"0.5.0 and no palette was specified in the XML file" << std::endl;
	return PaletteTablePtr();
}

SweeneyObjectLayer::SweeneyObjectLayer(ItemPtrVectorPtr& items,
	SweeneyMapType::image_map_sptr imgMap, ItemPtrVectorPtr& validItems)
	throw () :
		Map2D::Layer(
			"Objects",
			Map2D::Layer::HasOwnTileSize | Map2D::Layer::HasPalette,
			0, 0, // Layer size unused
			1, 1,
			items, validItems
		),
		imgMap(imgMap)
{
}

SweeneyObjectLayer::~SweeneyObjectLayer()
	throw ()
{
}

ImagePtr SweeneyObjectLayer::imageFromCode(unsigned int code,
	VC_TILESET& tileset)
	throw ()
{
	return ImagePtr(); // unknown map code
}

PaletteTablePtr SweeneyObjectLayer::getPalette(VC_TILESET& tileset)
	throw ()
{
	// Try (Xargon) to load the palette from tile 0.5.0
	const Tileset::VC_ENTRYPTR& tilesets = tileset[0]->getItems();
	if (tilesets.size() > 5) {
		TilesetPtr tls = tileset[0]->openTileset(tilesets[5]);
		const Tileset::VC_ENTRYPTR& images = tls->getItems();
		if (images.size() > 0) {
			ImagePtr img = tls->openImage(images[0]);
			if (img->getCaps() & Image::HasPalette) {
				return img->getPalette();
			}
		}
	}

	// Otherwise (Jill) use the tileset's palette
	if (tileset[0]->getCaps() & Tileset::HasPalette) {
		return tileset[0]->getPalette();
	}

	std::cerr << "[SweeneyObjectLayer] Couldn't load palette from tile "
		"0.5.0 and no palette was specified in the XML file" << std::endl;
	return PaletteTablePtr();
}


//
// JillMapType
//

JillMapType::JillMapType()
	throw ()
{
	this->lenSavedata = JILL_LEN_SAVEDATA;
}

std::string JillMapType::getMapCode() const
	throw ()
{
	return "map-jill";
}

std::string JillMapType::getFriendlyName() const
	throw ()
{
	return "Jill of the Jungle map";
}

std::vector<std::string> JillMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("jn1");
	vcExtensions.push_back("jn2");
	vcExtensions.push_back("jn3");
	return vcExtensions;
}

std::vector<std::string> JillMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Jill of the Jungle");
	return vcGames;
}

SuppFilenames JillMapType::getRequiredSupps(
	const std::string& filenameMap) const
	throw ()
{
	SuppFilenames supps;
	supps[SuppItem::Extra1] = "jill.dma";
	return supps;
}


//
// XargonMapType
//

XargonMapType::XargonMapType()
	throw ()
{
	this->lenSavedata = XR_LEN_SAVEDATA;
}

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
	vcExtensions.push_back("xr0");
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

SuppFilenames XargonMapType::getRequiredSupps(
	const std::string& filenameMap) const
	throw ()
{
	// Take the extension from the file being opened and use the corresponding
	// tiles file, i.e. "blah.xr1" -> "tiles.xr1".  There are no ".xr0" levels.
	SuppFilenames supps;
	std::string ext = filenameMap.substr(filenameMap.find_last_of('.'));
	supps[SuppItem::Extra1] = "tiles" + ext;
	return supps;
}


//
// SweeneyMapType
//

MapType::Certainty SweeneyMapType::isInstance(stream::input_sptr psMap) const
	throw (stream::error)
{
	stream::pos lenMap = psMap->size();

	// TESTED BY: fmt_map_xargon_isinstance_c01
	if (lenMap < XR_OFFSET_OBJLAYER + 2) return MapType::DefinitelyNo; // too short

	psMap->seekg(XR_OFFSET_OBJLAYER, stream::start);
	uint16_t numObjects;
	psMap >> u16le(numObjects);

	stream::pos offStrings = XR_OFFSET_OBJLAYER + 2 +
		numObjects * XR_OBJ_ENTRY_LEN + this->lenSavedata;

	// TESTED BY: fmt_map_xargon_isinstance_c02
	if (lenMap == offStrings) return MapType::DefinitelyYes; // exact size w/ no strings

	// TESTED BY: fmt_map_xargon_isinstance_c03
	if (lenMap < offStrings + 3) return MapType::DefinitelyNo; // too short
	psMap->seekg(offStrings, stream::start);

	unsigned int i;
	for (i = 0; i < XR_SAFETY_MAX_STRINGS; i++) {
		uint16_t lenStr;
		psMap >> u16le(lenStr);
		offStrings += lenStr + 2 + 1; // +2 for uint16le, +1 for terminating null
		if (offStrings == lenMap) break; // reached EOF

		// Make sure the next string's length field isn't cut
		if (lenMap < offStrings + 2) return MapType::DefinitelyNo;
		psMap->seekg(offStrings, stream::start);
	}
	if (i == XR_SAFETY_MAX_STRINGS) return MapType::DefinitelyNo; // too many strings

	// TESTED BY: fmt_map_xargon_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr SweeneyMapType::create(SuppData& suppData) const
	throw (stream::error)
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr SweeneyMapType::open(stream::input_sptr input, SuppData& suppData) const
	throw (stream::error)
{
	// Read the tile properties from the suppdata
	stream::input_sptr dma = suppData[SuppItem::Extra1];
	assert(dma);

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtr v;

	image_map_sptr imgMap;
	imgMap.reset(new image_map());
	stream::len len = dma->size();
	do {
		uint16_t mapCode, flags;
		uint8_t tileset, tile, namelen;
		dma
			>> u16le(mapCode)
			>> u8(tile)
			>> u8(tileset)
			>> u16le(flags)
			>> u8(namelen)
		;

		// Add to list of valid tiles
		v.reset(new Map2D::Layer::Item());
		v->code = mapCode;
		validBGItems->push_back(v);

		// Add to image map
		(*imgMap)[mapCode] = ((tileset & 0x3F) << 8) | tile;

		// Skip name
		dma->seekg(namelen, stream::cur);
		len -= 7 + namelen;
	} while (len); // TODO: Make this so it can't get stuck in an infinite loop

	// Read the map
	stream::pos lenMap = input->size();
	if (lenMap < XR_OFFSET_OBJLAYER + 2) {
		throw stream::error("Map file has been truncated! (background section cut)");
	}

	// Read the background layer
	input->seekg(0, stream::start);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(XR_MAP_WIDTH * XR_MAP_HEIGHT);
	for (unsigned int x = 0; x < XR_MAP_WIDTH; x++) {
		for (unsigned int y = 0; y < XR_MAP_HEIGHT; y++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->x = x;
			t->y = y;
			input >> u16le(t->code);
			tiles->push_back(t);
		}
	}
	lenMap -= XR_MAP_WIDTH * XR_MAP_HEIGHT * 2;

	Map2D::LayerPtr bgLayer(new SweeneyBackgroundLayer(tiles, imgMap, validBGItems));

	// Read the object layer
	uint16_t numObjects;
	input >> u16le(numObjects);
	lenMap -= 2;
	if (lenMap < numObjects * XR_OBJ_ENTRY_LEN) {
		throw stream::error("Map file has been truncated! (objects section cut)");
	}

	// Read all the text strings at the end of the map first
	stream::pos offStrings = numObjects * XR_OBJ_ENTRY_LEN + this->lenSavedata;
	input->seekg(offStrings, stream::cur);
	offStrings = lenMap - offStrings; // offStrings is now the amount of string data left to read
		std::cout << "read all strings at offset " << offStrings << " / " << lenMap << std::endl;

	std::list<std::string> mapStrings;
	try {
		// Read the text elements, if any are present
		while (offStrings >= 2) {
		std::cout << "read all strings at offset " << offStrings << " / " << lenMap << std::endl;
			uint16_t lenStr;
			input >> u16le(lenStr);
			lenStr++; // include terminating null
			offStrings -= 2;
			if (offStrings < lenStr) {
				throw stream::error("Map file has been truncated! (text section cut)");
			}
			std::string next;
			input >> null_padded(next, lenStr, false);
			mapStrings.push_back(next);
			offStrings -= lenStr;
		}
	} catch (const stream::incomplete_read& e) {
		throw stream::error("Map file has been truncated! (strings section cut)");
	}

	// Go back to the start of the object layer
	input->seekg(XR_OFFSET_OBJLAYER + 2, stream::start);

	Map2D::Layer::ItemPtrVectorPtr objects(new Map2D::Layer::ItemPtrVector());
	objects->reserve(numObjects);
	for (unsigned int i = 0; i < numObjects; i++) {
		uint8_t code;
		uint16_t x, y;
		uint16_t spdHoriz, spdVert;
		uint16_t width, height;
		uint16_t subType;
		uint16_t subState;
		uint16_t stateCount;
		uint16_t link;
		uint16_t flags;
		uint32_t pointer;
		uint16_t info;
		uint16_t zapHold;
		input
			>> u8(code)
			>> u16le(x)
			>> u16le(y)
			>> u16le(spdHoriz)
			>> u16le(spdVert)
			>> u16le(width)
			>> u16le(height)
			>> u16le(subType)
			>> u16le(subState)
			>> u16le(stateCount)
			>> u16le(link)
			>> u16le(flags)
			>> u32le(pointer)
			>> u16le(info)
			>> u16le(zapHold)
		;

		//SweeneyObject *obj = new SweeneyObject;
		Map2D::Layer::Item *obj;
		if (pointer) {
			// This one refers to a text entry
			Map2D::Layer::Item::Text *t_obj = new Map2D::Layer::Item::Text();
			t_obj->font = 0; ///< @todo Correct font
			if (!mapStrings.empty()) {
				t_obj->content = mapStrings.front();
				mapStrings.pop_front();
			}
			obj = t_obj;
		} else if (spdHoriz || spdVert) {
			Map2D::Layer::Item::Movable *t_obj = new Map2D::Layer::Item::Movable();
			t_obj->speedX = spdHoriz; ///< @todo Correct calculation
			t_obj->speedY = spdVert;  ///< @todo Correct calculation
			t_obj->distLeft = 0;
			t_obj->distRight = 0;
			t_obj->distUp = 0;
			t_obj->distDown = 0;
			obj = t_obj;
		} else {
			obj = new Map2D::Layer::Item();
		}

		obj->x = x;
		obj->y = y;
		obj->code = code | (subType << 8);

		Map2D::Layer::ItemPtr objItem(obj);
		objects->push_back(objItem);
	}
	lenMap -= XR_OBJ_ENTRY_LEN * numObjects;

	Map2D::Layer::ItemPtrVectorPtr validObjItems(new Map2D::Layer::ItemPtrVector());
	v.reset(new Map2D::Layer::Item());
	v->code = 0x33; // Clouds
	validObjItems->push_back(v);

	Map2D::Layer::Item::Text *txt = new Map2D::Layer::Item::Text();
	v.reset(txt);
	txt->font = 0;
	txt->content = "Small text";
	validObjItems->push_back(v);

	txt = new Map2D::Layer::Item::Text();
	v.reset(txt);
	txt->font = 0;
	txt->content = "Large text";
	validObjItems->push_back(v);

	Map2D::LayerPtr objLayer(new SweeneyObjectLayer(objects, imgMap, validObjItems));

	// Make sure we read in all the objects correctly
	assert(input->tellg() == (unsigned)(XR_OFFSET_OBJLAYER + 2 + numObjects * XR_OBJ_ENTRY_LEN));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(objLayer);

	Map2DPtr map(new Map2D(
		Map::AttributePtrVectorPtr(),
		Map2D::HasViewport,
		20 * XR_TILE_WIDTH, 10 * XR_TILE_HEIGHT, // viewport size
		XR_MAP_WIDTH, XR_MAP_HEIGHT,
		XR_TILE_WIDTH, XR_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long SweeneyMapType::write(MapPtr map, stream::output_sptr output, SuppData& suppData) const
	throw (stream::error)
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw stream::error("Incorrect layer count for this format.");

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

	for (unsigned int i = 0; i < XR_MAP_WIDTH * XR_MAP_HEIGHT; i++) {
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
		Map2D::Layer::Item *obj = i->get();
		Map2D::Layer::Item::Text *obj_text = dynamic_cast<Map2D::Layer::Item::Text *>(obj);
		Map2D::Layer::Item::Movable *obj_movable = dynamic_cast<Map2D::Layer::Item::Movable *>(obj);
		uint8_t code = obj->code & 0xFF;
		uint16_t x = obj->x;
		uint16_t y = obj->y;
		uint16_t spdHoriz;
		uint16_t spdVert;
		if (obj_movable) {
			spdHoriz = obj_movable->speedX;
			spdVert = obj_movable->speedY;
		} else {
			spdHoriz = 0;
			spdVert = 0;
		}
		uint16_t width = 16; ///< @todo How to get size when we don't have tileset?
		uint16_t height = 16; ///< @todo Throw exception when there is no tileset maybe?  Or hardcode object sizes?
		uint16_t subType = obj->code >> 8;
		uint16_t subState = 0;
		uint16_t stateCount = 0;
		uint16_t link = 0;
		uint16_t flags = 0;
		uint32_t pointer;
		if (obj_text) {
			pointer = 1; // non-zero means text is present
		} else {
			pointer = 0;
		}
		uint16_t info = 0;
		uint16_t zapHold = 0;

		output
			<< u8(code)
			<< u16le(x)
			<< u16le(y)
			<< u16le(spdHoriz)
			<< u16le(spdVert)
			<< u16le(width)
			<< u16le(height)
			<< u16le(subType)
			<< u16le(subState)
			<< u16le(stateCount)
			<< u16le(link)
			<< u16le(flags)
			<< u32le(pointer)
			<< u16le(info)
			<< u16le(zapHold)
		;
		lenWritten += 31;
	}

	// Write out savedata
	for (unsigned int i = 0; i < this->lenSavedata; i++) {
		output->write("\0", 1);
	}

	// Write out text strings
	for (Map2D::Layer::ItemPtrVector::const_iterator i = objects->begin();
		i != objects->end();
		i++
	) {
		Map2D::Layer::Item::Text *t = dynamic_cast<Map2D::Layer::Item::Text *>(i->get());
		if (t) {
			unsigned int len = t->content.length();
			if (len > 255) throw stream::error("Cannot write a text element longer than 255 characters.");
			output << u8(len);
			output->write(t->content);
		}
	}

	return lenWritten;
}


} // namespace gamemaps
} // namespace camoto
