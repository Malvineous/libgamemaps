/**
 * @file   fmt-map-xargon.cpp
 * @brief  MapType and Map2D implementation for Jill of the Jungle and Xargon.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Jill_of_the_Jungle_Map_Format
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

#include <iostream>
#include <list>
#include <boost/scoped_array.hpp>
#include "map2d-generic.hpp"
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

class SweeneyBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		SweeneyBackgroundLayer(ItemPtrVectorPtr& items,
			SweeneyMapType::image_map_sptr imgMap, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Background",
					Map2D::Layer::HasPalette,
					0, 0,   // Layer size unused
					0, 0,
					items, validItems
				),
				imgMap(imgMap)
		{
		}

		virtual gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset) const
		{
			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& tilesets = t->second->getItems();

			uint16_t v = (*this->imgMap)[item->code & 0x0FFF];
			uint8_t ti = ((v >> 8) & 0xFF);
			uint8_t i = v & 0xFF;
			if (tilesets[ti]->getAttr() & Tileset::EmptySlot) {
				std::cerr << "[SweeneyBackgroundLayer] Tried to open tileset 0x"
					<< std::hex << (int)ti << std::dec << " but it's an empty slot!"
					<< std::endl;
				return ImagePtr();
			}
			TilesetPtr tls = t->second->openTileset(tilesets[ti]);
			const Tileset::VC_ENTRYPTR& images = tls->getItems();
			if (i >= images.size()) return ImagePtr(); // out of range
			if (images[i]->getAttr() & Tileset::EmptySlot) {
				std::cerr << "[SweeneyBackgroundLayer] Tried to open image " << ti << "."
					<< i << " but it's an empty slot!" << std::endl;
				return ImagePtr();
			}
			return tls->openImage(images[i]);
		} catch (...) {
			std::cerr << "[SweeneyBackgroundLayer] Exception trying to open image for "
				"tile code " << code << std::endl;
			return ImagePtr();
		}

		gamegraphics::PaletteTablePtr getPalette(
			const TilesetCollectionPtr& tileset) const
		{
			// Try (Xargon) to load the palette from tile 0.5.0
			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return PaletteTablePtr(); // no tileset?!
			const Tileset::VC_ENTRYPTR& tilesets = t->second->getItems();
			if (tilesets.size() > 5) {
				TilesetPtr tls = t->second->openTileset(tilesets[5]);
				const Tileset::VC_ENTRYPTR& images = tls->getItems();
				if (images.size() > 0) {
					ImagePtr img = tls->openImage(images[0]);
					if (img->getCaps() & Image::HasPalette) {
						return img->getPalette();
					}
				}
			}

			// Otherwise (Jill) use the tileset's palette
			if (t->second->getCaps() & Tileset::HasPalette) {
				return t->second->getPalette();
			}

			std::cerr << "[SweeneyBackgroundLayer] Couldn't load palette from tile "
				"0.5.0 and no palette was specified in the XML file" << std::endl;
			return PaletteTablePtr();
		}

	protected:
		SweeneyMapType::image_map_sptr imgMap;
};

class SweeneyObjectLayer: virtual public GenericMap2D::Layer
{
	public:
		SweeneyObjectLayer(ItemPtrVectorPtr& items,
			SweeneyMapType::image_map_sptr imgMap, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Objects",
					Map2D::Layer::HasOwnTileSize | Map2D::Layer::HasPalette,
					0, 0, // Layer size unused
					1, 1,
					items, validItems
				),
				imgMap(imgMap)
		{
		}

		virtual gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset) const
		{
			return ImagePtr(); // unknown map code
		}

		gamegraphics::PaletteTablePtr getPalette(
			const TilesetCollectionPtr& tileset) const
		{
			// Try (Xargon) to load the palette from tile 0.5.0
			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return PaletteTablePtr(); // no tileset?!
			const Tileset::VC_ENTRYPTR& tilesets = t->second->getItems();
			if (tilesets.size() > 5) {
				TilesetPtr tls = t->second->openTileset(tilesets[5]);
				const Tileset::VC_ENTRYPTR& images = tls->getItems();
				if (images.size() > 0) {
					ImagePtr img = tls->openImage(images[0]);
					if (img->getCaps() & Image::HasPalette) {
						return img->getPalette();
					}
				}
			}

			// Otherwise (Jill) use the tileset's palette
			if (t->second->getCaps() & Tileset::HasPalette) {
				return t->second->getPalette();
			}

			std::cerr << "[SweeneyObjectLayer] Couldn't load palette from tile "
				"0.5.0 and no palette was specified in the XML file" << std::endl;
			return PaletteTablePtr();
		}

	protected:
		SweeneyMapType::image_map_sptr imgMap;
};




//
// JillMapType
//

JillMapType::JillMapType()
{
	this->viewportWidth = 232;
	this->viewportHeight = 160;
	this->lenSavedata = JILL_LEN_SAVEDATA;
}

std::string JillMapType::getMapCode() const
{
	return "map-jill";
}

std::string JillMapType::getFriendlyName() const
{
	return "Jill of the Jungle map";
}

std::vector<std::string> JillMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("jn1");
	vcExtensions.push_back("jn2");
	vcExtensions.push_back("jn3");
	return vcExtensions;
}

std::vector<std::string> JillMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Jill of the Jungle");
	return vcGames;
}

SuppFilenames JillMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	supps[SuppItem::Extra1] = "jill.dma";
	return supps;
}


//
// XargonMapType
//

XargonMapType::XargonMapType()
{
	this->viewportWidth = 20 * XR_TILE_WIDTH;
	this->viewportHeight = 10 * XR_TILE_HEIGHT;
	this->lenSavedata = XR_LEN_SAVEDATA;
}

std::string XargonMapType::getMapCode() const
{
	return "map-xargon";
}

std::string XargonMapType::getFriendlyName() const
{
	return "Xargon map";
}

std::vector<std::string> XargonMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("xr0");
	vcExtensions.push_back("xr1");
	vcExtensions.push_back("xr2");
	vcExtensions.push_back("xr3");
	return vcExtensions;
}

std::vector<std::string> XargonMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Xargon");
	return vcGames;
}

SuppFilenames XargonMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	// Take the extension from the file being opened and use the corresponding
	// tiles file, i.e. "blah.xr1" -> "tiles.xr1".  There are no ".xr0" levels.
	SuppFilenames supps;
	std::string ext = filename.substr(filename.find_last_of('.'));
	supps[SuppItem::Extra1] = "tiles" + ext;
	return supps;
}


//
// SweeneyMapType
//

MapType::Certainty SweeneyMapType::isInstance(stream::input_sptr psMap) const
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
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr SweeneyMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	// Read the tile properties from the suppdata
	stream::input_sptr dma = suppData[SuppItem::Extra1];
	assert(dma);

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtr v;

	image_map_sptr imgMap;
	imgMap.reset(new image_map());
	stream::delta len = dma->size();
	dma->seekg(0, stream::start);
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
		v->type = Map2D::Layer::Item::Default;
		v->code = mapCode;
		validBGItems->push_back(v);

		// Add to image map
		(*imgMap)[mapCode] = ((tileset & 0x3F) << 8) | tile;

		// Skip name
		dma->seekg(namelen, stream::cur);
		len -= 7 + namelen;
	} while (len > 0);

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
			uint16_t code;
			input >> u16le(code);
			if ((code & 0x03FF) == 0) continue; // empty spot
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = x;
			t->y = y;
			t->code = code;
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
	if (offStrings > lenMap) {
		throw stream::error("Map file is missing text section entirely!");
	}
	offStrings = lenMap - offStrings; // offStrings is now the amount of string data left to read

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
			input >> fixedLength(next, lenStr);
			mapStrings.push_back(next);
			offStrings -= lenStr;
		}
	} catch (const stream::incomplete_read& e) {
		throw stream::error("Map file has been truncated! (text section cut unexpectedly)");
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
		Map2D::Layer::ItemPtr obj(new Map2D::Layer::Item);
		obj->type = Map2D::Layer::Item::Default;
		obj->x = x;
		obj->y = y;
		obj->code = code | (subType << 8);

		if (pointer) {
			// This one refers to a text entry
			obj->type |= Map2D::Layer::Item::Text;
			obj->textFont = 0; ///< @todo Correct font
			if (!mapStrings.empty()) {
				obj->textContent = mapStrings.front();
				mapStrings.pop_front();
			}
		}
		if (spdHoriz || spdVert) {
			obj->type |= Map2D::Layer::Item::Movement;
			obj->movementSpeedX = spdHoriz; ///< @todo Correct calculation
			obj->movementSpeedY = spdVert;  ///< @todo Correct calculation
			obj->movementDistLeft = 0;
			obj->movementDistRight = 0;
			obj->movementDistUp = 0;
			obj->movementDistDown = 0;
		}
		objects->push_back(obj);
	}
	lenMap -= XR_OBJ_ENTRY_LEN * numObjects;

	Map2D::Layer::ItemPtrVectorPtr validObjItems(new Map2D::Layer::ItemPtrVector());
	v.reset(new Map2D::Layer::Item());
	v->type = Map2D::Layer::Item::Default;
	v->code = 0x33; // Clouds
	validObjItems->push_back(v);

	v.reset(new Map2D::Layer::Item);
	v->type = Map2D::Layer::Item::Text;
	v->textFont = 0;
	v->textContent = "Small text";
	validObjItems->push_back(v);

	v.reset(new Map2D::Layer::Item);
	v->type = Map2D::Layer::Item::Text;
	v->textFont = 0;
	v->textContent = "Large text";
	validObjItems->push_back(v);

	Map2D::LayerPtr objLayer(new SweeneyObjectLayer(objects, imgMap, validObjItems));

	// Make sure we read in all the objects correctly
	assert(input->tellg() == (unsigned)(XR_OFFSET_OBJLAYER + 2 + numObjects * XR_OBJ_ENTRY_LEN));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(objLayer);

	Map2DPtr map(new GenericMap2D(
		Map::AttributePtrVectorPtr(), NO_GFX_CALLBACK,
		Map2D::HasViewport,
		this->viewportWidth, this->viewportHeight,
		XR_MAP_WIDTH, XR_MAP_HEIGHT,
		XR_TILE_WIDTH, XR_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void SweeneyMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw stream::error("Incorrect layer count for this format.");

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

	// Write the object layer
	layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr objects = layer->getAllItems();

	uint16_t numObjects = objects->size();
	output << u16le(numObjects);
	for (Map2D::Layer::ItemPtrVector::const_iterator i = objects->begin();
		i != objects->end();
		i++
	) {
		Map2D::Layer::Item *obj = i->get();
		uint8_t code = obj->code & 0xFF;
		uint16_t x = obj->x;
		uint16_t y = obj->y;
		uint16_t spdHoriz;
		uint16_t spdVert;
		if (obj->type & Map2D::Layer::Item::Movement) {
			spdHoriz = obj->movementSpeedX;
			spdVert = obj->movementSpeedY;
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
		if (obj->type & Map2D::Layer::Item::Text) {
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
		if ((*i)->type & Map2D::Layer::Item::Text) {
			unsigned int len = (*i)->textContent.length();
			if (len > 255) throw stream::error("Cannot write a text element longer than 255 characters.");
			output << u8(len);
			output->write((*i)->textContent);
		}
	}

	output->flush();
	return;
}


} // namespace gamemaps
} // namespace camoto
