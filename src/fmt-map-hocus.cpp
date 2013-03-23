/**
 * @file   fmt-map-hocus.cpp
 * @brief  MapType and Map2D implementation for Hocus Pocus.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Hocus_Pocus
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
#include "map2d-generic.hpp"
#include <camoto/iostream_helpers.hpp>
#include "fmt-map-hocus.hpp"

/// Width of each tile in pixels
#define HP_TILE_WIDTH 16

/// Height of each tile in pixels
#define HP_TILE_HEIGHT 16

/// Width of each map in tiles
#define HP_MAP_WIDTH 240

/// Height of each map in tiles
#define HP_MAP_HEIGHT 60

/// Number of grid cells in the map
#define HP_MAP_SIZE  (HP_MAP_WIDTH * HP_MAP_HEIGHT)

/// Width of map view during gameplay, in pixels
#define HP_VIEWPORT_WIDTH 320

/// Height of map view during gameplay, in pixels
#define HP_VIEWPORT_HEIGHT 160

/// Map code used for 'no tile' in background layer
#define HP_DEFAULT_TILE_BG 0xFF

/// Map code used for 'no tile' in foreground layer
#define HP_DEFAULT_TILE_FG 0xFF

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class HocusBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		HocusBackgroundLayer(const std::string& name, ItemPtrVectorPtr& items,
			ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					name,
					Map2D::Layer::NoCaps,
					0, 0,
					0, 0,
					items, validItems
				)
		{
		}

		virtual gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset)
		{
			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[item->code]);
		}
};


std::string HocusMapType::getMapCode() const
{
	return "map-hocus";
}

std::string HocusMapType::getFriendlyName() const
{
	return "Hocus Pocus level";
}

std::vector<std::string> HocusMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("");
	return vcExtensions;
}

std::vector<std::string> HocusMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Hocus Pocus");
	return vcGames;
}

MapType::Certainty HocusMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();
	// TESTED BY: fmt_map_hocus_isinstance_c01
	if (lenMap != 14400) return MapType::DefinitelyNo; // wrong size

	// TESTED BY: fmt_map_hocus_isinstance_c00
	return MapType::PossiblyYes;
}

MapPtr HocusMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr HocusMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);

	uint8_t code;

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr bgtiles(new Map2D::Layer::ItemPtrVector());
	bgtiles->reserve(HP_MAP_WIDTH * HP_MAP_HEIGHT);

	for (unsigned int y = 0; y < HP_MAP_HEIGHT; y++) {
		for (unsigned int x = 0; x < HP_MAP_WIDTH; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = x;
			t->y = y;
			input >> u8(code);
			t->code = code;
			if (t->code != HP_DEFAULT_TILE_BG) bgtiles->push_back(t);
		}
	}

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr bgLayer(new HocusBackgroundLayer("Background", bgtiles, validBGItems));

	stream::input_sptr layerFile = suppData[SuppItem::Layer1];
	assert(layerFile);

	// Read the foreground layer
	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
	fgtiles->reserve(HP_MAP_WIDTH * HP_MAP_HEIGHT);

	for (unsigned int y = 0; y < HP_MAP_HEIGHT; y++) {
		for (unsigned int x = 0; x < HP_MAP_WIDTH; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = x;
			t->y = y;
			layerFile >> u8(code);
			t->code = code;
			if (t->code != HP_DEFAULT_TILE_BG) fgtiles->push_back(t);
		}
	}

	Map2D::Layer::ItemPtrVectorPtr validFGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr fgLayer(new HocusBackgroundLayer("Foreground", fgtiles, validFGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);
	//layers.push_back(actorLayer);

	Map2DPtr map(new GenericMap2D(
		Map::AttributePtrVectorPtr(), NO_GFX_CALLBACK,
		Map2D::HasViewport,
		HP_VIEWPORT_WIDTH, HP_VIEWPORT_HEIGHT,
		HP_MAP_WIDTH, HP_MAP_HEIGHT,
		HP_TILE_WIDTH, HP_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void HocusMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	Map2D::LayerPtr layer;

	// Write the background layer
	boost::scoped_array<uint8_t> tiles(new uint8_t[HP_MAP_SIZE]);

	// Set the default background tile
	memset(tiles.get(), HP_DEFAULT_TILE_BG, HP_MAP_SIZE);

	layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr itemsBG = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = itemsBG->begin();
		i != itemsBG->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		tiles[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	output->write((char *)tiles.get(), HP_MAP_SIZE);
	output->flush();
	assert(output->tellp() == 14400);

	// Write the foreground layer
	stream::output_sptr layerFile = suppData[SuppItem::Layer1];
	assert(layerFile);

	// Set the default background tile
	memset(tiles.get(), HP_DEFAULT_TILE_FG, HP_MAP_SIZE);

	layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr itemsFG = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = itemsFG->begin();
		i != itemsFG->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		tiles[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	layerFile->seekp(0, stream::start);
	layerFile->write((char *)tiles.get(), HP_MAP_SIZE);
	layerFile->flush();
	assert(layerFile->tellp() == 14400);

	return;
}

SuppFilenames HocusMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
