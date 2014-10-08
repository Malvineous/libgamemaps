/**
 * @file   fmt-map-darkages.cpp
 * @brief  MapType and Map2D implementation for Dark Ages levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Dark_Ages_Map_Format
 *
 * Copyright (C) 2010-2014 Adam Nielsen <malvineous@shikadi.net>
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
#include "fmt-map-darkages.hpp"

#define DA_TILE_WIDTH           16
#define DA_TILE_HEIGHT          16

#define DA_MAP_WIDTH           128 ///< Width of map, in tiles
#define DA_MAP_HEIGHT            9 ///< Height of map, in tiles

/// Map code to write for locations with no tile set.
#define DA_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define DA_MAX_VALID_TILECODE  255 // number of tiles in tileset

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class DarkAgesBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		DarkAgesBackgroundLayer(ItemPtrVectorPtr& items,
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

			unsigned int tilesetIndex, imageIndex;
			switch (item->code) {
				// This mapping was borrowed from Frenkel's DAVIEW.BAS
				case 100: tilesetIndex = 3; imageIndex = 1; break;
				case 101: tilesetIndex = 3; imageIndex = 8; break;
				case 102: tilesetIndex = 3; imageIndex = 18; break;
				case 103: tilesetIndex = 3; imageIndex = 24; break;
				case 104: tilesetIndex = 3; imageIndex = 34; break;
				case 105: tilesetIndex = 3; imageIndex = 38; break;
				case 106: tilesetIndex = 3; imageIndex = 42; break;
				case 107: tilesetIndex = 3; imageIndex = 48; break;
				case 108: tilesetIndex = 4; imageIndex = 0; break;
				case 109: tilesetIndex = 4; imageIndex = 30; break;
				case 110: tilesetIndex = 4; imageIndex = 35; break;
				case 111: tilesetIndex = 4; imageIndex = 36; break;
				case 112: tilesetIndex = 4; imageIndex = 39; break;
				case 113: tilesetIndex = 4; imageIndex = 42; break;
				case 114: tilesetIndex = 4; imageIndex = 46; break;
				case 115: tilesetIndex = 5; imageIndex = 0; break;
				case 116: tilesetIndex = 5; imageIndex = 1; break;
				case 117: tilesetIndex = 5; imageIndex = 2; break;
				case 118: tilesetIndex = 5; imageIndex = 3; break;
				case 119: tilesetIndex = 5; imageIndex = 4; break;
				case 120: tilesetIndex = 3; imageIndex = 4; break;
				case 121: tilesetIndex = 3; imageIndex = 5; break;
				case 122: tilesetIndex = 3; imageIndex = 6; break;
				case 123: tilesetIndex = 3; imageIndex = 7; break;
				case 124: tilesetIndex = 5; imageIndex = 12; break;
				case 125: tilesetIndex = 5; imageIndex = 14; break;
				case 126: tilesetIndex = 5; imageIndex = 8; break;
				case 127: tilesetIndex = 5; imageIndex = 9; break;
				case 128: tilesetIndex = 5; imageIndex = 10; break;
				case 129: tilesetIndex = 5; imageIndex = 23; break;
				case 130: tilesetIndex = 5; imageIndex = 24; break;
				case 131: tilesetIndex = 5; imageIndex = 25; break;
				case 132: tilesetIndex = 2; imageIndex = 10; break;
				case 133: tilesetIndex = 5; imageIndex = 30; break;
				case 134: tilesetIndex = 5; imageIndex = 31; break;
				case 135: tilesetIndex = 5; imageIndex = 32; break;
				case 136: tilesetIndex = 5; imageIndex = 33; break;
				case 137: tilesetIndex = 5; imageIndex = 35; break;
				case 138: tilesetIndex = 5; imageIndex = 36; break;
				case 139: tilesetIndex = 5; imageIndex = 40; break;
				case 140: tilesetIndex = 3; imageIndex = 32; break;
				case 141: tilesetIndex = 5; imageIndex = 47; break;
				case 142: tilesetIndex = 3; imageIndex = 30; break;
				case 143: tilesetIndex = 6; imageIndex = 0; break;
				default:
					tilesetIndex = item->code / 50;
					imageIndex = item->code % 50;
					break;
			}

			const Tileset::VC_ENTRYPTR& tilesets = t->second->getItems();
			if (tilesetIndex >= tilesets.size()) return Map2D::Layer::Unknown; // out of range

			TilesetPtr tls = t->second->openTileset(tilesets[tilesetIndex]);
			const Tileset::VC_ENTRYPTR& tilesets2 = tls->getItems();

			tls = tls->openTileset(tilesets2[0]);
			const Tileset::VC_ENTRYPTR& images = tls->getItems();
			if (imageIndex >= images.size()) return Map2D::Layer::Unknown; // out of range

			*out = tls->openImage(images[imageIndex]);

			return Map2D::Layer::Supplied;
		}

};


std::string DarkAgesMapType::getMapCode() const
{
	return "map-darkages";
}

std::string DarkAgesMapType::getFriendlyName() const
{
	return "Dark Ages level";
}

std::vector<std::string> DarkAgesMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("dal"); // made up, inside file05.da[123]
	return vcExtensions;
}

std::vector<std::string> DarkAgesMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Dark Ages");
	return vcGames;
}

MapType::Certainty DarkAgesMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_darkages_isinstance_c01
	if (lenMap != 1152) return MapType::DefinitelyNo;

	// TESTED BY: fmt_map_darkages_isinstance_c00
	return MapType::PossiblyYes;
}

MapPtr DarkAgesMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr DarkAgesMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);
	const unsigned int mapLen = DA_MAP_WIDTH * DA_MAP_HEIGHT;

	// Read the background layer
	uint8_t *bg = new uint8_t[mapLen];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	input->read((char *)bg, mapLen);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(mapLen);
	for (unsigned int i = 0; i < mapLen; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (bg[i] == DA_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % DA_MAP_WIDTH;
		t->y = i / DA_MAP_WIDTH;
		t->code = bg[i];
		tiles->push_back(t);
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= DA_MAX_VALID_TILECODE; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (i == DA_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}

	// Create the map structures
	Map2D::LayerPtr bgLayer(new DarkAgesBackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new GenericMap2D(
		Map::AttributePtrVectorPtr(), NO_GFX_CALLBACK,
		Map2D::HasViewport,
		240, 144, // viewport size
		DA_MAP_WIDTH, DA_MAP_HEIGHT,
		DA_TILE_WIDTH, DA_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void DarkAgesMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	Map2D::LayerPtr layer = map2d->getLayer(0);

	// Write the background layer
	const unsigned int mapLen = DA_MAP_WIDTH * DA_MAP_HEIGHT;

	uint8_t *bg = new uint8_t[mapLen];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	memset(bg, DA_DEFAULT_BGTILE, mapLen); // default background tile
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > DA_MAP_WIDTH) || ((*i)->y > DA_MAP_HEIGHT)) {
			throw stream::error("Layer has tiles outside map boundary!");
		}
		bg[(*i)->y * DA_MAP_WIDTH + (*i)->x] = (*i)->code;
	}

	output->write((char *)bg, mapLen);
	output->flush();
	return;
}

SuppFilenames DarkAgesMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
