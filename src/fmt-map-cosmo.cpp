/**
 * @file   fmt-map-cosmo.cpp
 * @brief  MapType and Map2D implementation for Cosmo levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Cosmo_Level_Format
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
#include "fmt-map-cosmo.hpp"

/// Width of each tile in pixels
#define CCA_TILE_WIDTH 8

/// Height of each tile in pixels
#define CCA_TILE_HEIGHT 8

/// Width of map view during gameplay, in pixels
#define CCA_VIEWPORT_WIDTH 304

/// Height of map view during gameplay, in pixels
#define CCA_VIEWPORT_HEIGHT 144

/// Maximum width of a valid level, in tiles (TODO: See if the game has a maximum value)
#define CCA_MAX_WIDTH 512

/// Maximum number of actors in a valid level (TODO: See if the game has a maximum value)
#define CCA_MAX_ACTORS 512

/// Length of the map data, in bytes
#define CCA_LAYER_LEN_BG 65528

/// Number of tiles in the map
#define CCA_NUM_TILES_BG (CCA_LAYER_LEN_BG / 2)

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;


class CosmoActorLayer: virtual public GenericMap2D::Layer
{
	public:
		CosmoActorLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Actors",
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
			TilesetCollection::const_iterator t = tileset->find(SpriteTileset1);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			// TODO
			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[item->code]);
		}
};

class CosmoBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		CosmoBackgroundLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Background",
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
			unsigned int index = item->code >> 3; // divide by 8
			ImagePurpose purpose;
			if (index >= 2000) {
				index -= 2000;
				index /= 5;
				/*if (index >= 1000) {
					// out of range!
					return ImagePtr();
				}*/
				purpose = ForegroundTileset1;
			} else {
				purpose = BackgroundTileset1;
			}
			TilesetCollection::const_iterator t = tileset->find(purpose);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[index]);
		}
};


std::string CosmoMapType::getMapCode() const
{
	return "map-cosmo";
}

std::string CosmoMapType::getFriendlyName() const
{
	return "Cosmo's Cosmic Adventures level";
}

std::vector<std::string> CosmoMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("mni");
	return vcExtensions;
}

std::vector<std::string> CosmoMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Cosmo's Cosmic Adventures");
	return vcGames;
}

MapType::Certainty CosmoMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// TESTED BY: fmt_map_cosmo_isinstance_c01/c01a
	if (lenMap < 6 + CCA_LAYER_LEN_BG) return MapType::DefinitelyNo; // too short

	uint16_t mapWidth;
	psMap->seekg(2, stream::start);
	psMap >> u16le(mapWidth);

	// TESTED BY: fmt_map_cosmo_isinstance_c02
	if (mapWidth > CCA_MAX_WIDTH) return MapType::DefinitelyNo; // map too wide

	uint16_t numActorInts;
	psMap >> u16le(numActorInts);

	// TESTED BY: fmt_map_cosmo_isinstance_c03
	if (numActorInts > (CCA_MAX_ACTORS * 3)) return MapType::DefinitelyNo; // too many actors

	// TESTED BY: fmt_map_cosmo_isinstance_c04
	if ((unsigned)(6 + numActorInts * 3) > lenMap) {
		// This doesn't count the BG layer, because it seems to be possible for
		// it to be an arbitrary size - missing tiles are just left as blanks
		return MapType::DefinitelyNo; // file too small
	}

	// TODO: Read map data and confirm each uint16le is < 56000

	// TESTED BY: fmt_map_cosmo_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr CosmoMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr CosmoMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	stream::pos lenMap = input->size();
	input->seekg(0, stream::start);

	uint16_t flags, mapWidth, numActorInts;
	input
		>> u16le(flags)
		>> u16le(mapWidth)
		>> u16le(numActorInts)
	;
	lenMap -= 6;

	// Read in the actor layer
	unsigned int numActors = numActorInts / 3;
	if (lenMap < numActors * 6) throw stream::error("Map file has been truncated!");
	Map2D::Layer::ItemPtrVectorPtr actors(new Map2D::Layer::ItemPtrVector());
	actors->reserve(numActors);
	for (unsigned int i = 0; i < numActors; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		input
			>> u16le(t->code)
			>> u16le(t->x)
			>> u16le(t->y)
		;
		actors->push_back(t);
	}
	lenMap -= 6 * numActors;

	Map2D::Layer::ItemPtrVectorPtr validActorItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr actorLayer(new CosmoActorLayer(actors, validActorItems));

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(CCA_NUM_TILES_BG);

	for (unsigned int i = 0; (i < CCA_NUM_TILES_BG) && (lenMap >= 2); i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % mapWidth;
		t->y = i / mapWidth;
		input >> u16le(t->code);
		// Don't push zero codes (these are transparent/no-tile)
		if (t->code != 0) tiles->push_back(t);
		lenMap -= 2;
	}

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr bgLayer(new CosmoBackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(actorLayer);

	Map2DPtr map(new GenericMap2D(
		Map::AttributePtrVectorPtr(), NO_GFX_CALLBACK,
		Map2D::HasViewport,
		CCA_VIEWPORT_WIDTH, CCA_VIEWPORT_HEIGHT,
		mapWidth, 32768 / mapWidth,
		CCA_TILE_WIDTH, CCA_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void CosmoMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	uint16_t flags = 0;
	output
		<< u16le(flags)
		<< u16le(mapWidth)
	;

	// Write the actor layer
	Map2D::LayerPtr layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr actors = layer->getAllItems();

	uint16_t numActorInts = actors->size() * 3;
	output << u16le(numActorInts);
	for (Map2D::Layer::ItemPtrVector::const_iterator i = actors->begin();
		i != actors->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		output
			<< u16le((*i)->code)
			<< u16le((*i)->x)
			<< u16le((*i)->y)
		;
	}

	// Write the background layer
	unsigned long lenBG = mapWidth * mapHeight;
	uint16_t *bg = new uint16_t[lenBG];
	boost::scoped_array<uint16_t> sbg(bg);
	// Set the default background tile
	memset(bg, 0x00, lenBG * 2); // 2 == sizeof(uint16le)

	layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		bg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	for (unsigned int i = 0; i < mapWidth * mapHeight; i++) {
		output << u16le(bg[i]);
	}

	output->flush();
	return;
}

SuppFilenames CosmoMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
