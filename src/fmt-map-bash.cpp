/**
 * @file   fmt-map-bash.cpp
 * @brief  MapType and Map2D implementation for Monster Bash levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Monster_Bash
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

#include "fmt-map-bash.hpp"
#include <camoto/iostream_helpers.hpp>

/// Width of map tiles
#define MB_TILE_WIDTH           16

/// Height of map tiles
#define MB_TILE_HEIGHT          16

/// Width of map view during gameplay, in pixels
#define MB_VIEWPORT_WIDTH      320

/// Height of map view during gameplay, in pixels
#define MB_VIEWPORT_HEIGHT     200

/// Map code to write for locations with no tile set.
#define MB_DEFAULT_FGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define MB_MAX_VALID_TILECODE 0x6C

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

BashForegroundLayer::BashForegroundLayer(ItemPtrVectorPtr& items)
	throw () :
		Map2D::Layer(
			"Foreground",
			Map2D::Layer::NoCaps,
			0, 0,
			0, 0,
			items
		)
{
}

ImagePtr BashForegroundLayer::imageFromCode(unsigned int code,
	VC_TILESET& tileset)
	throw ()
{
	if (tileset.size() < 3) return ImagePtr(); // no tileset?!
	unsigned int t = 1 + ((code >> 7) & 1);
	code &= 0x7F;
	const Tileset::VC_ENTRYPTR& images = tileset[t]->getItems();
	if (code >= images.size()) return ImagePtr(); // out of range
	return tileset[t]->openImage(images[code]);
}


BashBackgroundLayer::BashBackgroundLayer(ItemPtrVectorPtr& items)
	throw () :
		Map2D::Layer(
			"Background",
			Map2D::Layer::NoCaps,
			0, 0,
			0, 0,
			items
		)
{
}

ImagePtr BashBackgroundLayer::imageFromCode(unsigned int code,
	VC_TILESET& tileset)
	throw ()
{
	if (tileset.size() < 1) return ImagePtr(); // no tileset?!
	code = code & 0x1FF;
	const Tileset::VC_ENTRYPTR& images = tileset[0]->getItems();
	if (code >= images.size()) return ImagePtr(); // out of range
	return tileset[0]->openImage(images[code]);
}


std::string BashMapType::getMapCode() const
	throw ()
{
	return "map-bash";
}

std::string BashMapType::getFriendlyName() const
	throw ()
{
	return "Monster Bash level";
}

std::vector<std::string> BashMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("mif");
	return vcExtensions;
}

std::vector<std::string> BashMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Monster Bash");
	return vcGames;
}

MapType::Certainty BashMapType::isInstance(stream::input_sptr psMap) const
	throw (stream::error)
{
	return MapType::Unsure;
}

MapPtr BashMapType::create(SuppData& suppData) const
	throw (stream::error)
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr BashMapType::open(stream::input_sptr input, SuppData& suppData) const
	throw (stream::error)
{
	stream::input_sptr bg = suppData[SuppItem::Layer1];
	stream::input_sptr fg = suppData[SuppItem::Layer2];
	assert(bg);
	assert(fg);

	stream::pos lenBG = bg->size();
	bg->seekg(0, stream::start);

	// Read the background layer
	uint16_t unknown, mapWidth, mapPixelWidth, mapPixelHeight;
	bg
		>> u16le(unknown)
		>> u16le(mapWidth)
		>> u16le(mapPixelWidth)
		>> u16le(mapPixelHeight)
	;
	lenBG -= 8;

	if (lenBG < 2) throw stream::error("Background layer file too short");

	mapWidth >>= 1; // convert from # of bytes to # of ints (tiles)
	unsigned int mapHeight = mapPixelHeight / MB_TILE_HEIGHT;

	Map2D::Layer::ItemPtrVectorPtr bgtiles(new Map2D::Layer::ItemPtrVector());
	bgtiles->reserve(mapWidth * mapHeight);
	for (unsigned int y = 0; y < mapHeight; y++) {
		for (unsigned int x = 0; x < mapWidth; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->x = x;
			t->y = y;
			uint16_t code;
			bg >> u16le(code);
			lenBG -= 2;
			t->code = code;
			bgtiles->push_back(t);
			if (lenBG < 2) break;
		}
		if (lenBG < 2) break;
	}

	Map2D::LayerPtr bgLayer(new BashBackgroundLayer(bgtiles));

	// Read the foreground layer
	stream::pos lenFG = fg->size();
	fg->seekg(2, stream::start); // skip width field
	lenFG -= 2;

	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
	fgtiles->reserve(mapWidth * mapHeight);
	for (unsigned int y = 0; y < mapHeight; y++) {
		for (unsigned int x = 0; x < mapWidth; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->x = x;
			t->y = y;
			uint8_t code;
			fg >> u8(code);
			lenFG -= 2;
			t->code = code;
			if (code != MB_DEFAULT_FGTILE) fgtiles->push_back(t);
			if (lenFG < 2) break;
		}
		if (lenFG < 2) break;
	}

	Map2D::LayerPtr fgLayer(new BashForegroundLayer(fgtiles));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);

	Map2DPtr map(new Map2D(
		Map::AttributePtrVectorPtr(),
		Map2D::HasViewport,
		MB_VIEWPORT_WIDTH, MB_VIEWPORT_HEIGHT,
		mapWidth, mapHeight,
		MB_TILE_WIDTH, MB_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long BashMapType::write(MapPtr map, stream::output_sptr output, SuppData& suppData) const
	throw (stream::error)
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw stream::error("Incorrect layer count for this format.");

	unsigned long lenWritten = 0;

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	stream::output_sptr bg = suppData[SuppItem::Layer1];
	stream::output_sptr fg = suppData[SuppItem::Layer2];
	assert(bg);
	assert(fg);

	// Write the background layer
	{
		unsigned int lenBG = mapWidth * mapHeight;
		boost::shared_array<uint16_t> bgdata(new uint16_t[lenBG]);
		memset(bgdata.get(), 0, lenBG); // default background tile
		Map2D::LayerPtr layer = map2d->getLayer(0);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			i != items->end();
			i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			bgdata[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
		}

		uint16_t mapWidthBytes = mapWidth * 2; // 2 == sizeof(uint16_t)
		uint16_t mapPixelWidth = mapWidth * MB_TILE_WIDTH;
		uint16_t mapPixelHeight = mapHeight * MB_TILE_HEIGHT;
		bg
			<< u16le(0) // unknown
			<< u16le(mapWidthBytes)
			<< u16le(mapPixelWidth)
			<< u16le(mapPixelHeight)
		;
		uint16_t *pbg = bgdata.get();
		for (unsigned int i = 0; i < lenBG; i++) {
			bg << u16le(*pbg++);
		}
	}

	// Write the foreground layer
	{
		unsigned int lenFG = mapWidth * mapHeight;
		boost::shared_array<uint8_t> fgdata(new uint8_t[lenFG]);
		memset(fgdata.get(), 0, lenFG); // default background tile
		Map2D::LayerPtr layer = map2d->getLayer(1);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			i != items->end();
			i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			fgdata[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
		}

		uint16_t mapWidthBytes = mapWidth * 2; // 2 == sizeof(uint16_t)
		fg
			<< u16le(mapWidthBytes)
		;
		uint8_t *pfg = fgdata.get();
		for (unsigned int i = 0; i < lenFG; i++) {
			fg << u16le(*pfg++);
		}
	}

	return lenWritten;
}

SuppFilenames BashMapType::getRequiredSupps(
	const std::string& filenameMap) const
	throw ()
{
	SuppFilenames supps;
	std::string baseName = filenameMap.substr(0, filenameMap.length() - 3);
	supps[SuppItem::Layer1] = baseName + "mbg";
	supps[SuppItem::Layer2] = baseName + "mfg";
	return supps;
}

} // namespace gamemaps
} // namespace camoto
