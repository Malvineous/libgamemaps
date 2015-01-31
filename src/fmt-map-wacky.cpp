/**
 * @file  fmt-map-wacky.cpp
 * @brief MapType and Map2D implementation for Wacky Wheels levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Wacky_Wheels
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

#define _USE_MATH_DEFINES
#include <math.h>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp>
#include "map2d-generic.hpp"
#include "fmt-map-wacky.hpp"

#define WW_MAP_WIDTH            64
#define WW_MAP_HEIGHT           64
#define WW_TILE_WIDTH           32
#define WW_TILE_HEIGHT          32

#define WW_LAYER_OFF_BG         0
#define WW_LAYER_LEN_BG         (WW_MAP_WIDTH * WW_MAP_HEIGHT)
#define WW_FILESIZE             (WW_LAYER_OFF_BG + WW_LAYER_LEN_BG)

/// Map code to write for locations with no tile set.
#define WW_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define WW_MAX_VALID_TILECODE 0x6C

/// After this many tiles, go to the next tileset.
#define WW_TILES_PER_TILESET    54

/// Angle value equivalent to 0
#define WW_ANGLE_MAX          1920

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_WackyBackground: virtual public GenericMap2D::Layer
{
	public:
		Layer_WackyBackground(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Surface",
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
			unsigned int ti = item->code / WW_TILES_PER_TILESET;
			unsigned int index = item->code % WW_TILES_PER_TILESET;

			TilesetCollection::const_iterator t =
				tileset->find((ImagePurpose)(BackgroundTileset1 + ti));
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = t->second->openImage(images[index]);
			return Map2D::Layer::Supplied;
		}
};


std::string MapType_Wacky::getMapCode() const
{
	return "map-wacky";
}

std::string MapType_Wacky::getFriendlyName() const
{
	return "Wacky Wheels level";
}

std::vector<std::string> MapType_Wacky::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("m");
	return vcExtensions;
}

std::vector<std::string> MapType_Wacky::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Wacky Wheels");
	return vcGames;
}

MapType::Certainty MapType_Wacky::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// TESTED BY: fmt_map_wacky_isinstance_c01
	if (lenMap != WW_FILESIZE) return MapType::DefinitelyNo; // wrong size

	// Read in the layer and make sure all the tile codes are within range
	uint8_t bg[WW_LAYER_LEN_BG];
	psMap->seekg(WW_LAYER_OFF_BG, stream::start);
	// This will throw an exception on a short read which is ok, because we
	// wouldn't be here if the file was too small anyway (isinstance_c01)
	psMap->read(bg, WW_LAYER_LEN_BG);

	for (unsigned int i = 0; i < WW_LAYER_LEN_BG; i++) {
		// TESTED BY: fmt_map_wacky_isinstance_c02
		if (bg[i] > WW_MAX_VALID_TILECODE) return MapType::DefinitelyNo; // invalid tile
	}

	// TESTED BY: fmt_map_wacky_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr MapType_Wacky::create(SuppData& suppData) const
{
	/// @todo Implement MapType_Wacky::create()
	throw stream::error("Not implemented yet!");
}

MapPtr MapType_Wacky::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);

	// Read the background layer
	uint8_t bg[WW_LAYER_LEN_BG];
	input->read((char *)bg, WW_LAYER_LEN_BG);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(WW_MAP_WIDTH * WW_MAP_HEIGHT);
	for (unsigned int i = 0; i < WW_LAYER_LEN_BG; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = i % WW_MAP_WIDTH;
		t->y = i / WW_MAP_WIDTH;
		t->code = bg[i];
		tiles->push_back(t);
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= WW_MAX_VALID_TILECODE; i++) {
		// The default tile actually has an image, so don't exclude it
		if (i == WW_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}

	// Create the map structures
	Map2D::LayerPtr bgLayer(new Layer_WackyBackground(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	// Read the computer player paths
	stream::input_sptr rd = suppData[SuppItem::Layer1];
	assert(rd);
	rd->seekg(0, stream::start);
	uint16_t numPoints;
	rd >> u16le(numPoints);

	Map2D::PathPtrVectorPtr paths(new Map2D::PathPtrVector());
	Map2D::PathPtr pathptr(new Map2D::Path());
	unsigned int startX, startY;
	rd
		>> u16le(startX)
		>> u16le(startY)
	;
	pathptr->start.push_back(Map2D::Path::point(startX, startY));
	for (unsigned int i = 0; i < numPoints; i++) {
		uint16_t nextX, nextY;
		if (i > 0) rd->seekg(4, stream::cur);
		rd
			>> u16le(nextX)
			>> u16le(nextY)
		;
		rd->seekg(6, stream::cur);
		pathptr->points.push_back(Map2D::Path::point(nextX - startX, nextY - startY));
	}
	pathptr->fixed = false;
	pathptr->forceClosed = false;
	pathptr->maxPoints = 0; // no limit
	paths->push_back(pathptr);

	Map2DPtr map(new GenericMap2D(
		Map::Attributes(), Map::GraphicsFilenames(),
		Map2D::HasPaths | Map2D::FixedPathCount,
		0, 0,
		WW_MAP_WIDTH, WW_MAP_HEIGHT,
		WW_TILE_WIDTH, WW_TILE_HEIGHT,
		layers, paths
	));

	return map;
}

void MapType_Wacky::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw stream::error("Incorrect layer count for this format.");

	Map2D::PathPtrVectorPtr paths = map2d->getPaths();
	if (paths->size() != 1) throw stream::error("Incorrect path count for this format.");

	Map2D::PathPtr path = paths->at(0);
	if (path->start.size() != 1) throw stream::error("Path has no starting point!");

	if (suppData.find(SuppItem::Layer1) == suppData.end()) {
		throw stream::error("No SuppItem::Layer1 specified (need *.rd file)");
	}

	// Write the background layer
	uint8_t bg[WW_LAYER_LEN_BG];
	memset(bg, WW_DEFAULT_BGTILE, WW_LAYER_LEN_BG); // default background tile
	Map2D::LayerPtr layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > WW_MAP_WIDTH) || ((*i)->y > WW_MAP_HEIGHT)) {
			throw stream::error("Layer has tiles outside map boundary!");
		}
		bg[(*i)->y * WW_MAP_WIDTH + (*i)->x] = (*i)->code;
	}

	output->write((char *)bg, WW_LAYER_LEN_BG);

	stream::output_sptr rd = suppData[SuppItem::Layer1];
	rd->seekp(0, stream::start);

	unsigned int firstX = path->start[0].first;
	unsigned int firstY = path->start[0].second;
	unsigned int nextX = firstX;
	unsigned int nextY = firstY;
	uint16_t count = path->points.size();
	rd << u16le(count);
	for (Map2D::Path::point_vector::const_iterator i = path->points.begin();
		i != path->points.end(); i++
	) {
		unsigned int lastX = nextX;
		unsigned int lastY = nextY;
		rd
			<< u16le(lastX)
			<< u16le(lastY)
		;
		nextX = firstX + i->first;
		nextY = firstY + i->second;
		int deltaX = nextX - lastX;
		int deltaY = nextY - lastY;
		unsigned int angle = WW_ANGLE_MAX + atan2((double)deltaY, (double)deltaX) * (WW_ANGLE_MAX/2) / M_PI;
		angle %= WW_ANGLE_MAX;
		unsigned int image = (int)(angle / 240.0 + 6.5) % 8;
		unsigned int dist = sqrt((double)(deltaX*deltaX + deltaY*deltaY));
		rd
			<< u16le(nextX)
			<< u16le(nextY)
			<< u16le(angle)
			<< u16le(image)
			<< u16le(dist)
		;
	}

	rd->truncate(2 + count * 14); // implicit flush

	return;
}

SuppFilenames MapType_Wacky::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	std::string baseName = filename.substr(0, filename.length() - 1);
	supps[SuppItem::Layer1] = baseName + "rd";
	return supps;
}


} // namespace gamemaps
} // namespace camoto
