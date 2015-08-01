/**
 * @file  fmt-map-vinyl.cpp
 * @brief MapType and Map2D implementation for Vinyl Goddess From Mars levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/VGFM_Level_Format
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
#include "fmt-map-vinyl.hpp"

#define VGFM_TILE_WIDTH             16
#define VGFM_TILE_HEIGHT            16

/// This is the largest valid tile code in the background layer.
#define VGFM_MAX_VALID_BGTILECODE  481 // number of tiles in tileset

/// This is the largest valid tile code in the foreground layer.
#define VGFM_MAX_VALID_FGTILECODE  255 // limit of 8-bit byte

/// Value used where no tile should appear in the foreground layer.
#define VGFM_DEFAULT_TILE_FG      0x00

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_VinylMap: virtual public GenericMap2D::Layer
{
	public:
		Layer_VinylMap(const std::string& name, ItemPtrVectorPtr& items,
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


std::string MapType_Vinyl::getMapCode() const
{
	return "map2d-vinyl";
}

std::string MapType_Vinyl::getFriendlyName() const
{
	return "Vinyl Goddess From Mars level";
}

std::vector<std::string> MapType_Vinyl::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("m");
	return vcExtensions;
}

std::vector<std::string> MapType_Vinyl::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Vinyl Goddess From Mars");
	return vcGames;
}

MapType::Certainty MapType_Vinyl::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_vinyl_isinstance_c01
	if (lenMap < 4) return MapType::DefinitelyNo;

	psMap->seekg(0, stream::start);
	unsigned int width, height;
	psMap >> u16le(height) >> u16le(width);

	// Make sure the dimensions cover the entire file
	// TESTED BY: fmt_map_vinyl_isinstance_c02
	unsigned int expLen = 4 + width * height * 3; // 3 = uint16 bg + uint8 fg
	if (lenMap != expLen) return MapType::DefinitelyNo;

	// Read in the map and make sure all the tile codes are within range
	for (unsigned int i = 0; i < width * height; i++) {
		// Make sure each tile is within range
		// TESTED BY: fmt_map_vinyl_isinstance_c03
		uint16_t code;
		try {
			psMap >> u16le(code);
		} catch (...) {
			return MapType::DefinitelyNo;
		}
		if (code > VGFM_MAX_VALID_BGTILECODE) {
			return MapType::DefinitelyNo;
		}
	}

	// TESTED BY: fmt_map_vinyl_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr MapType_Vinyl::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr MapType_Vinyl::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);
	unsigned int width, height;
	input >> u16le(height) >> u16le(width);
	unsigned int mapLen = width * height;

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr bgtiles(new Map2D::Layer::ItemPtrVector());
	bgtiles->reserve(mapLen);
	for (unsigned int i = 0; i < mapLen; i++) {
		uint16_t code;
		input >> u16le(code);

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % width;
		t->y = i / width;
		t->code = code;
		bgtiles->push_back(t);
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= VGFM_MAX_VALID_BGTILECODE; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}

	// Create the map structures
	Map2D::LayerPtr bgLayer(new Layer_VinylMap("Background", bgtiles, validBGItems));

	// Read the foreground layer
	uint8_t *fg = new uint8_t[mapLen];
	boost::scoped_array<uint8_t> scoped_fg(fg);
	input->read((char *)fg, mapLen);

	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
	fgtiles->reserve(mapLen);
	for (unsigned int i = 0; i < mapLen; i++) {
		if (fg[i] == VGFM_DEFAULT_TILE_FG) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % width;
		t->y = i / width;
		t->code = fg[i];
		fgtiles->push_back(t);
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validFGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= VGFM_MAX_VALID_FGTILECODE; i++) {
		if (i == VGFM_DEFAULT_TILE_FG) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validFGItems->push_back(t);
	}

	// Create the map structures
	Map2D::LayerPtr fgLayer(new Layer_VinylMap("Foreground", fgtiles, validFGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);

	Map2DPtr map(new GenericMap2D(
		Map::Attributes(), Map::GraphicsFilenames(),
		Map2D::HasViewport,
		320, 159, // viewport size
		width, height,
		VGFM_TILE_WIDTH, VGFM_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void MapType_Vinyl::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 2)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	output << u16le(mapHeight) << u16le(mapWidth);
	unsigned int mapLen = mapWidth * mapHeight;

	// Write the background layer
	{
		Map2D::LayerPtr layer = map2d->getLayer(0);
		uint16_t *bg = new uint16_t[mapLen];
		boost::scoped_array<uint16_t> scoped_bg(bg);
		memset(bg, 0, mapLen);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = items->begin(); i != items->end(); i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			bg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
		}
		for (unsigned int i = 0; i < mapLen; i++) {
			output << u16le(*bg++);
		}
	}

	// Write the foreground layer
	{
		Map2D::LayerPtr layer = map2d->getLayer(1);
		uint8_t *fg = new uint8_t[mapLen];
		boost::scoped_array<uint8_t> scoped_fg(fg);
		memset(fg, VGFM_DEFAULT_TILE_FG, mapLen);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = items->begin(); i != items->end(); i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			fg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
		}
		output->write((char *)fg, mapLen);
	}

	output->flush();
	return;
}

SuppFilenames MapType_Vinyl::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
