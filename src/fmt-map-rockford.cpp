/**
 * @file   fmt-map-rockford.cpp
 * @brief  MapType and Map2D implementation for Rockford levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Rockford
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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
#include "fmt-map-rockford.hpp"

#define ROCKFORD_TILE_WIDTH  16
#define ROCKFORD_TILE_HEIGHT 16

/// Width of a map, in tiles.
#define ROCKFORD_MAP_WIDTH   40

/// Height of a map, in tiles.
#define ROCKFORD_MAP_HEIGHT  22

/// Map code to write for locations with no tile set.
#define ROCKFORD_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define ROCKFORD_MAX_VALID_TILECODE   (10*20) // number of tiles in tileset

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class RockfordBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		RockfordBackgroundLayer(ItemPtrVectorPtr& items,
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

		virtual Map2D::Layer::ImageType imageFromCode(
			const Map2D::Layer::ItemPtr& item, const TilesetCollectionPtr& tileset,
			ImagePtr *out) const
		{
			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			unsigned int index = item->code;

			// Special case for one image!
			if (index == 3) index++;

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = t->second->openImage(images[index]);
			return Map2D::Layer::Supplied;
		}
};


std::string RockfordMapType::getMapCode() const
{
	return "map-rockford";
}

std::string RockfordMapType::getFriendlyName() const
{
	return "Rockford level";
}

std::vector<std::string> RockfordMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("bin");
	return vcExtensions;
}

std::vector<std::string> RockfordMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Rockford");
	return vcGames;
}

MapType::Certainty RockfordMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_rockford_isinstance_c01
	if (lenMap != ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT) return MapType::DefinitelyNo;

	psMap->seekg(0, stream::start);

	// Read in the map and make sure all the tile codes are within range
	uint8_t *bg = new uint8_t[ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	stream::len r = psMap->try_read(bg, ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT);
	if (r != ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT) return MapType::DefinitelyNo; // read error
	for (unsigned int i = 0; i < ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT; i++) {
		// Make sure each tile is within range
		// TESTED BY: fmt_map_rockford_isinstance_c03
		if (bg[i] > ROCKFORD_MAX_VALID_TILECODE) {
			return MapType::DefinitelyNo;
		}
	}

	// TESTED BY: fmt_map_rockford_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr RockfordMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr RockfordMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);

	// Read the background layer
	uint8_t *bg = new uint8_t[ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	input->read(bg, ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT);
	for (unsigned int i = 0; i < ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (bg[i] == ROCKFORD_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % ROCKFORD_MAP_WIDTH;
		t->y = i / ROCKFORD_MAP_WIDTH;
		t->code = bg[i];
		tiles->push_back(t);
	}

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	unsigned int validItems[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0A, 0x0B, 0x0C,
		0x10,
		0x28, 0x2C, 0x2D, 0x2E,
		0x30, 0x34, 0x35, 0x36, 0x37,
		0x38,
		0x53,
		0x70, 0x74, 0x7C,
		0x80, 0x82, 0x84, 0x88,
		0xC4,
	};
	for (unsigned int i = 0; i < sizeof(validItems) / sizeof(unsigned int); i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}
	Map2D::LayerPtr bgLayer(new RockfordBackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new GenericMap2D(
		Map::AttributePtrVectorPtr(), NO_GFX_CALLBACK,
		Map2D::HasViewport,
		320, 176, // viewport size
		ROCKFORD_MAP_WIDTH, ROCKFORD_MAP_HEIGHT,
		ROCKFORD_TILE_WIDTH, ROCKFORD_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void RockfordMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);
	if ((mapWidth != ROCKFORD_MAP_WIDTH) || (mapHeight != ROCKFORD_MAP_HEIGHT)) {
		throw stream::error("Incorrect layer size for this format.");
	}

	Map2D::LayerPtr layer = map2d->getLayer(0);

	// Write the background layer
	uint8_t *bg = new uint8_t[ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	memset(bg, ROCKFORD_DEFAULT_BGTILE, ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
			throw stream::error("Layer has tiles outside map boundary!");
		}
		bg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	output->write(bg, ROCKFORD_MAP_WIDTH * ROCKFORD_MAP_HEIGHT);
	output->flush();
	return;
}

SuppFilenames RockfordMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
