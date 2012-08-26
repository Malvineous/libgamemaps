/**
 * @file   fmt-map-ccaves.cpp
 * @brief  MapType and Map2D implementation for Crystal Caves levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Crystal_Caves
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
#include <camoto/iostream_helpers.hpp>
#include "map2d-generic.hpp"
#include "fmt-map-ccaves.hpp"

#define CC_MAP_WIDTH            40
#define CC_TILE_WIDTH           16
#define CC_TILE_HEIGHT          16

/// This is the largest number of rows ever expected to be seen.
#define CC_MAX_MAP_HEIGHT      100

/// This is the largest valid tile code in the background layer.
#define CC_MAX_VALID_TILECODE   0xfb

/// Width of map view during gameplay, in pixels
#define CC_VIEWPORT_WIDTH       320

/// Height of map view during gameplay, in pixels
#define CC_VIEWPORT_HEIGHT      192

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

CCavesBackgroundLayer::CCavesBackgroundLayer(ItemPtrVectorPtr& items,
	ItemPtrVectorPtr& validItems)
	:	GenericMap2D::Layer(
			"Background",
			Map2D::Layer::NoCaps,
			0, 0,
			0, 0,
			items, validItems
		)
{
}

ImagePtr CCavesBackgroundLayer::imageFromCode(unsigned int code,
	VC_TILESET& tileset)
{
	// TODO
	return ImagePtr();
}


std::string CCavesMapType::getMapCode() const
{
	return "map-ccaves";
}

std::string CCavesMapType::getFriendlyName() const
{
	return "Crystal Caves level";
}

std::vector<std::string> CCavesMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("ccl");
	return vcExtensions;
}

std::vector<std::string> CCavesMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Crystal Caves");
	return vcGames;
}

MapType::Certainty CCavesMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	psMap->seekg(0, stream::start);

	// TESTED BY: fmt_map_ccaves_isinstance_c01
	if (lenMap < CC_MAP_WIDTH + 1) return MapType::DefinitelyNo; // too small

	uint8_t row[CC_MAP_WIDTH];
	unsigned int y;
	for (y = 0; (y < CC_MAX_MAP_HEIGHT) && lenMap; y++) {
		uint8_t lenRow;
		psMap >> u8(lenRow);
		lenMap--;

		// Incorrect row length
		// TESTED BY: fmt_map_ccaves_isinstance_c02
		if (lenRow != CC_MAP_WIDTH) return MapType::DefinitelyNo;

		// Incomplete row
		// TESTED BY: fmt_map_ccaves_isinstance_c03
		if (lenMap < CC_MAP_WIDTH) return MapType::DefinitelyNo;

		// Ensure the row data is valid
		psMap->read((char *)row, CC_MAP_WIDTH);
		for (unsigned int x = 0; x < CC_MAP_WIDTH; x++) {
			// TESTED BY: fmt_map_ccaves_isinstance_c04
			if (row[x] > CC_MAX_VALID_TILECODE) return MapType::DefinitelyNo; // invalid tile
		}

		lenMap -= CC_MAP_WIDTH;
	}

	// TESTED BY: fmt_map_ccaves_isinstance_c05
	if (y == CC_MAX_MAP_HEIGHT) return MapType::DefinitelyNo; // map too tall

	// TESTED BY: fmt_map_ccaves_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr CCavesMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr CCavesMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	stream::pos lenMap = input->size();

	input->seekg(0, stream::start);

	// Read the background layer
	uint8_t *bg = new uint8_t[lenMap];
	boost::scoped_array<uint8_t> sbg(bg);
	input->read((char *)bg, lenMap);

	unsigned int height = lenMap / (CC_MAP_WIDTH + 1);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(CC_MAP_WIDTH * height);
	for (unsigned int y = 0; y < height; y++) {
		bg++; // skip row length byte
		for (unsigned int x = 0; x < CC_MAP_WIDTH; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->x = x;
			t->y = y;
			t->code = *bg++;
			if (t->code != 0x20) tiles->push_back(t);
		}
	}

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr bgLayer(new CCavesBackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new GenericMap2D(
		Map::AttributePtrVectorPtr(),
		Map2D::HasViewport,
		CC_VIEWPORT_WIDTH, CC_VIEWPORT_HEIGHT,
		CC_MAP_WIDTH, height,
		CC_TILE_WIDTH, CC_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void CCavesMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	// Write the background layer
	unsigned long lenBG = mapWidth * mapHeight;
	uint8_t *bg = new uint8_t[lenBG];
	boost::scoped_array<uint8_t> sbg(bg);
	// Set the default background tile
	memset(bg, 0x20, lenBG);

	Map2D::LayerPtr layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		bg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	for (unsigned int y = 0; y < mapHeight; y++) {
		output << u8(mapWidth);
		output->write((char *)bg, mapWidth);
		bg += mapWidth;
	}
	output->flush();
	return;
}

SuppFilenames CCavesMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
