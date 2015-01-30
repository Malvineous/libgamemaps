/**
 * @file  fmt-map-zone66.cpp
 * @brief MapType and Map2D implementation for Zone 66 levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Zone_66_Level_Format
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
#include "fmt-map-zone66.hpp"

/// Width of the map, in tiles
#define Z66_MAP_WIDTH  256

/// Height of the map, in tiles
#define Z66_MAP_HEIGHT 256

/// Length of the background layer, in bytes
#define Z66_MAP_BG_LEN (Z66_MAP_WIDTH * Z66_MAP_HEIGHT)

/// Width of each tile, in pixels
#define Z66_TILE_WIDTH  32

/// Height of each tile, in pixels
#define Z66_TILE_HEIGHT 32

/// Map code to write for locations with no tile set
#define Z66_DEFAULT_BGTILE 0x00

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Zone66BackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		Zone66BackgroundLayer(ItemPtrVectorPtr& items,
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


std::string Zone66MapType::getMapCode() const
{
	return "map-zone66";
}

std::string Zone66MapType::getFriendlyName() const
{
	return "Zone 66 level";
}

std::vector<std::string> Zone66MapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("z66");
	return vcExtensions;
}

std::vector<std::string> Zone66MapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Zone 66");
	return vcGames;
}

MapType::Certainty Zone66MapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_zone66_isinstance_c01
	if (lenMap != Z66_MAP_WIDTH * Z66_MAP_HEIGHT) return MapType::DefinitelyNo;

	// TESTED BY: fmt_map_zone66_isinstance_c00
	return MapType::PossiblyYes;
}

MapPtr Zone66MapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr Zone66MapType::open(stream::input_sptr input, SuppData& suppData) const
{
	// Read the background layer
	uint8_t *bg = new uint8_t[Z66_MAP_BG_LEN];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	memset(bg, Z66_DEFAULT_BGTILE, Z66_MAP_BG_LEN); // default background tile
	input->seekg(0, stream::start);
	stream::len amtRead = input->try_read(bg, Z66_MAP_BG_LEN);
	if (amtRead != Z66_MAP_BG_LEN) {
		std::cout << "Warning: Zone 66 level file was "
			<< (Z66_MAP_BG_LEN - amtRead)
			<< " bytes short - the last tiles will be left blank" << std::endl;
	}

	// Read the tile mapping table
	stream::input_sptr dataMapBG = suppData[SuppItem::Extra1];
	dataMapBG->seekg(0, stream::start);
	uint16_t lenMapBG, unknown;
	dataMapBG
		>> u16le(lenMapBG)
		>> u16le(unknown)
	;
	lenMapBG *= 2; // convert count of 16-bit numbers into number of bytes
	unsigned int mapBG[256];
	memset(mapBG, Z66_DEFAULT_BGTILE, 256);
	for (unsigned int i = 0; i < lenMapBG; i++) {
		dataMapBG >> u16le(mapBG[i]);
	}

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(Z66_MAP_BG_LEN);
	for (unsigned int i = 0; i < Z66_MAP_BG_LEN; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (bg[i] == Z66_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % Z66_MAP_WIDTH;
		t->y = i / Z66_MAP_WIDTH;
		t->code = mapBG[bg[i]];
		tiles->push_back(t);
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());

/// @todo Add all tiles instead of just ones already in the map, and rewrite the map on save
	for (unsigned int i = 0; i < 300; i++) {

		// The default tile actually has an image, so don't exclude it
		//if (i == Z66_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}

	// Create the map structures
	Map2D::LayerPtr bgLayer(new Zone66BackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new GenericMap2D(
		Map::Attributes(), Map::GraphicsFilenames(),
		Map2D::HasViewport,
		320, 200, // viewport size
		Z66_MAP_WIDTH, Z66_MAP_HEIGHT,
		Z66_TILE_WIDTH, Z66_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void Zone66MapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);
	if ((mapWidth != Z66_MAP_WIDTH) || (mapHeight != Z66_MAP_HEIGHT))
		throw stream::error("Incorrect map dimensions for this format.");

	Map2D::LayerPtr layer = map2d->getLayer(0);

	unsigned int numTileMappings = 0;
	unsigned int mapBG[256];

	// Prepare the background layer
	uint8_t *bg = new uint8_t[Z66_MAP_BG_LEN];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	memset(bg, Z66_DEFAULT_BGTILE, Z66_MAP_BG_LEN); // default background tile
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
			throw stream::error("Layer has tiles outside map boundary!");
		}
		// Look for an existing tile mapping first
		bool found = false;
		for (unsigned int m = 0; m < numTileMappings; m++) {
			if (mapBG[m] == (*i)->code) {
				bg[(*i)->y * mapWidth + (*i)->x] = m;
				found = true;
				break;
			}
		}
		if (!found) {
			// Have to add this tile to the mapping table
			if (numTileMappings >= 256) {
				throw stream::error("There are too many unique tiles in this level - "
					"Zone 66 only supports up to 256 different tiles in each level.  "
					"Please remove some tiles and try again.");
			}
			bg[(*i)->y * mapWidth + (*i)->x] = numTileMappings;
			mapBG[numTileMappings++] = (*i)->code;
			/// @todo Use the correct "destroyed" tile code
			mapBG[numTileMappings++] = (*i)->code;
		}
	}
	output->seekp(0, stream::start);
	output->write(bg, Z66_MAP_BG_LEN);
	output->flush();

	// Write the tile mapping table
	stream::output_sptr dataMapBG = suppData[SuppItem::Extra1];
	dataMapBG->seekp(0, stream::start);
	dataMapBG
		<< u16le(numTileMappings)
		<< u16le(0) /// @todo Animated tiles
	;
	for (unsigned int i = 0; i < numTileMappings; i++) {
		dataMapBG
			<< u16le(mapBG[i * 2])      // normal tile
			<< u16le(mapBG[i * 2 + 1])  // destroyed tile
		;
	}

	/// @todo Write correct values for tile points/score
	dataMapBG << nullPadded("", numTileMappings);

	/// @todo Write correct values for canDestroy flags
	dataMapBG << nullPadded("", numTileMappings);

	/// @todo Write animated tile info

	dataMapBG->flush();

	return;
}

SuppFilenames Zone66MapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	supps[SuppItem::Extra1] = filename.substr(0, filename.length() - 4) + "dat.z66";
	return supps;
}

} // namespace gamemaps
} // namespace camoto
