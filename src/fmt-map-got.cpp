/**
 * @file   fmt-map-got.cpp
 * @brief  MapType and Map2D implementation for God of Thunder levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/God_of_Thunder_Level_Format
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

#include <boost/scoped_array.hpp>
#include <camoto/iostream_helpers.hpp>
#include "map2d-generic.hpp"
#include "fmt-map-got.hpp"

/// Length of background layer, in bytes
#define GOT_MAP_LEN_BG         240

/// Width of each cell
#define GOT_TILE_WIDTH           16

/// Height of each cell
#define GOT_TILE_HEIGHT          16

/// Width of map, in cells
#define GOT_MAP_WIDTH            20

/// Height of map, in cells
#define GOT_MAP_HEIGHT           12

/// Map code to write for locations with no tile set
#define GOT_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer
#define GOT_MAX_VALID_TILECODE  229 // number of tiles in tileset

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class GOTBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		GOTBackgroundLayer(ItemPtrVectorPtr& items,
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

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = t->second->openImage(images[item->code]);
			return Map2D::Layer::Supplied;
		}

};


std::string GOTMapType::getMapCode() const
{
	return "map-got";
}

std::string GOTMapType::getFriendlyName() const
{
	return "God of Thunder level";
}

std::vector<std::string> GOTMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	return vcExtensions;
}

std::vector<std::string> GOTMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("God of Thunder");
	return vcGames;
}

MapType::Certainty GOTMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// Make sure there's enough data
	// TESTED BY: fmt_map_got_isinstance_c01
	if (lenMap != 512) return MapType::DefinitelyNo;

	psMap->seekg(0, stream::start);
	uint8_t bg[240];
	psMap->read(bg, 240);
	for (int i = 0; i < 240; i++) {
		// Map code out of range
		// TESTED BY: fmt_map_got_isinstance_c02
		if (bg[i] > GOT_MAX_VALID_TILECODE) return MapType::DefinitelyNo;
	}

	// TESTED BY: fmt_map_got_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr GOTMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr GOTMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);

	// Read the background layer
	uint8_t *bg = new uint8_t[GOT_MAP_LEN_BG];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	input->read((char *)bg, GOT_MAP_LEN_BG);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(GOT_MAP_LEN_BG);
	for (unsigned int i = 0; i < GOT_MAP_LEN_BG; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (bg[i] == GOT_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % GOT_MAP_WIDTH;
		t->y = i / GOT_MAP_WIDTH;
		t->code = bg[i];
		tiles->push_back(t);
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= GOT_MAX_VALID_TILECODE; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (i == GOT_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}

	// Create the map structures
	Map2D::LayerPtr bgLayer(new GOTBackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new GenericMap2D(
		Map::AttributePtrVectorPtr(), NO_GFX_CALLBACK,
		Map2D::HasViewport,
		320, 192, // viewport size
		GOT_MAP_WIDTH, GOT_MAP_HEIGHT,
		GOT_TILE_WIDTH, GOT_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void GOTMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);
	if ((mapWidth != GOT_MAP_WIDTH) || (mapHeight != GOT_MAP_HEIGHT))
		throw stream::error("Incorrect map size for this format.");

	Map2D::LayerPtr layer = map2d->getLayer(0);

	// Write the background layer
	uint8_t *bg = new uint8_t[GOT_MAP_LEN_BG];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	memset(bg, GOT_DEFAULT_BGTILE, GOT_MAP_LEN_BG); // default background tile
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

	output->write((char *)bg, GOT_MAP_LEN_BG);

	// TEMP: Pad file to 512 bytes until the format of this data is known
	output << nullPadded("", 256+16);

	output->flush();
	return;
}

SuppFilenames GOTMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
