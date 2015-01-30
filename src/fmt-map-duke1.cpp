/**
 * @file  fmt-map-duke1.cpp
 * @brief MapType and Map2D implementation for Duke Nukem 1 levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Duke_1_Level_Format
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

#include <camoto/iostream_helpers.hpp>
#include "map2d-generic.hpp"
#include "fmt-map-duke1.hpp"

#define DN1_MAP_WIDTH            128
#define DN1_MAP_HEIGHT           90
#define DN1_TILE_WIDTH           16
#define DN1_TILE_HEIGHT          16

#define DN1_LAYER_LEN            (DN1_MAP_WIDTH * DN1_MAP_HEIGHT)
#define DN1_FILESIZE             (DN1_LAYER_LEN * 2)

/// Map code to write for locations with no tile set.
#define DN1_DEFAULT_BGTILE     0x0000

/// This is the largest valid tile code in the background layer.
#define DN1_MAX_VALID_TILECODE 0xF000

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;


class Duke1BackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		Duke1BackgroundLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
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
			unsigned int index = item->code / 32;
			unsigned int ts_num = index / 48;
			unsigned int ts_index = index % 48;

			TilesetCollection::const_iterator t =
				tileset->find((ImagePurpose)((unsigned int)BackgroundTileset1 + ts_num));
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (ts_index >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = t->second->openImage(images[ts_index]);
			return Map2D::Layer::Supplied;
		}
};


std::string Duke1MapType::getMapCode() const
{
	return "map-duke1";
}

std::string Duke1MapType::getFriendlyName() const
{
	return "Duke Nukem 1 level";
}

std::vector<std::string> Duke1MapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dn1");
	vcExtensions.push_back("dn2");
	vcExtensions.push_back("dn3");
	return vcExtensions;
}

std::vector<std::string> Duke1MapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Duke Nukem 1");
	return vcGames;
}

MapType::Certainty Duke1MapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// TESTED BY: fmt_map_duke1_isinstance_c01
	if (lenMap != DN1_FILESIZE) return MapType::DefinitelyNo; // wrong size

	// Read in the layer and make sure all the tile codes are within range
	psMap->seekg(0, stream::start);
	for (unsigned int i = 0; i < DN1_LAYER_LEN; i++) {
		uint16_t tile;
		psMap >> u16le(tile);
		// TESTED BY: fmt_map_duke1_isinstance_c02
		if (tile > DN1_MAX_VALID_TILECODE) return MapType::DefinitelyNo; // invalid tile
	}

	// TESTED BY: fmt_map_duke1_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr Duke1MapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr Duke1MapType::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(DN1_LAYER_LEN);
	for (unsigned int i = 0; i < DN1_LAYER_LEN; i++) {
		uint16_t tileCode;
		input >> u16le(tileCode);
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % DN1_MAP_WIDTH;
		t->y = i / DN1_MAP_WIDTH;
		t->code = tileCode;
		if (t->code != DN1_DEFAULT_BGTILE) tiles->push_back(t);
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());

	// Create the map structures
	Map2D::LayerPtr bgLayer(new Duke1BackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new GenericMap2D(
		Map::Attributes(), Map::GraphicsFilenames(),
		Map2D::HasViewport,
		13 * DN1_TILE_WIDTH, 10 * DN1_TILE_HEIGHT, // viewport size
		DN1_MAP_WIDTH, DN1_MAP_HEIGHT,
		DN1_TILE_WIDTH, DN1_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void Duke1MapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	// Write the background layer
	uint16_t bg[DN1_LAYER_LEN];
	for (unsigned int i = 0; i < DN1_LAYER_LEN; i++) {
		bg[i] = DN1_DEFAULT_BGTILE;
	}
	Map2D::LayerPtr layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > DN1_MAP_WIDTH) || ((*i)->y > DN1_MAP_HEIGHT)) {
			throw stream::error("Layer has tiles outside map boundary!");
		}
		bg[(*i)->y * DN1_MAP_WIDTH + (*i)->x] = (*i)->code;
	}
	for (unsigned int i = 0; i < DN1_LAYER_LEN; i++) {
		output << u16le(bg[i]);
	}

	output->flush();
	return;
}

SuppFilenames Duke1MapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
