/**
 * @file   fmt-map-ccomic.cpp
 * @brief  MapType and Map2D implementation for Captain Comic levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Captain_Comic_Map_Format
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

#include <boost/shared_array.hpp>
#include <camoto/gamemaps/map2d.hpp>
#include <camoto/iostream_helpers.hpp>
#include "fmt-map-ccomic.hpp"

#define CC_TILE_WIDTH           16
#define CC_TILE_HEIGHT          16

/// Map code to write for locations with no tile set.
#define CC_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define CC_MAX_VALID_TILECODE   87 // number of tiles in tileset

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

CComicBackgroundLayer::CComicBackgroundLayer(ItemPtrVectorPtr& items,
	ItemPtrVectorPtr& validItems)
	throw () :
		Map2D::Layer(
			"Background",
			Map2D::Layer::NoCaps,
			0, 0,
			0, 0,
			items, validItems
		)
{
}

ImagePtr CComicBackgroundLayer::imageFromCode(unsigned int code,
	VC_TILESET& tileset)
	throw ()
{
	if (tileset.size() < 1) return ImagePtr(); // no tileset?!
	const Tileset::VC_ENTRYPTR& images = tileset[0]->getItems();
	if (code >= images.size()) return ImagePtr(); // out of range
	return tileset[0]->openImage(images[code]);
}

std::string CComicMapType::getMapCode() const
	throw ()
{
	return "map-ccomic";
}

std::string CComicMapType::getFriendlyName() const
	throw ()
{
	return "Captain Comic level";
}

std::vector<std::string> CComicMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("pt");
	return vcExtensions;
}

std::vector<std::string> CComicMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Captain Comic");
	return vcGames;
}

MapType::Certainty CComicMapType::isInstance(stream::input_sptr psMap) const
	throw (stream::error)
{
	stream::pos lenMap = psMap->size();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_ccomic_isinstance_c01
	if (lenMap < 4) return MapType::DefinitelyNo;

	psMap->seekg(0, stream::start);
	unsigned int width, height;
	psMap >> u16le(width) >> u16le(height);

	// Make sure the dimensions cover the entire file
	// TESTED BY: fmt_map_ccomic_isinstance_c02
	unsigned int mapLen = width * height;
	if (lenMap != mapLen + 4) return MapType::DefinitelyNo;

	// Read in the map and make sure all the tile codes are within range
	uint8_t *bg = new uint8_t[mapLen];
	stream::len r = psMap->try_read(bg, mapLen);
	if (r != mapLen) return MapType::DefinitelyNo; // read error
	for (unsigned int i = 0; i < mapLen; i++) {
		// Make sure each tile is within range
		// TESTED BY: fmt_map_ccomic_isinstance_c03
		if (bg[i] > CC_MAX_VALID_TILECODE) {
			delete[] bg;
			return MapType::DefinitelyNo;
		}
	}
	delete[] bg;

	// TESTED BY: fmt_map_ccomic_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr CComicMapType::create(SuppData& suppData) const
	throw (stream::error)
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr CComicMapType::open(stream::input_sptr input, SuppData& suppData) const
	throw (stream::error)
{
	input->seekg(0, stream::start);
	unsigned int width, height;
	input >> u16le(width) >> u16le(height);
	unsigned int mapLen = width * height;

	// Read the background layer
	uint8_t *bg = new uint8_t[mapLen];
	input->read((char *)bg, mapLen);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(mapLen);
	for (unsigned int i = 0; i < mapLen; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (bg[i] == CC_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->x = i % width;
		t->y = i / width;
		t->code = bg[i];
		tiles->push_back(t);
	}
	delete[] bg;

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr bgLayer(new CComicBackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new Map2D(
		Map::AttributePtrVectorPtr(),
		Map2D::HasViewport,
		193, 160, // viewport size
		width, height,
		CC_TILE_WIDTH, CC_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long CComicMapType::write(MapPtr map, stream::output_sptr output, SuppData& suppData) const
	throw (stream::error)
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	unsigned long lenWritten = 0;

	Map2D::LayerPtr layer = map2d->getLayer(0);

	// Write the background layer
	output << u16le(mapWidth) << u16le(mapHeight);
	unsigned int mapLen = mapWidth * mapHeight;

	uint8_t *bg = new uint8_t[mapLen];
	memset(bg, CC_DEFAULT_BGTILE, mapLen); // default background tile
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
			delete[] bg;
			throw stream::error("Layer has tiles outside map boundary!");
		}
		bg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	output->write((char *)bg, mapLen);
	lenWritten += mapLen;
	delete[] bg;

	return lenWritten;
}


} // namespace gamemaps
} // namespace camoto
