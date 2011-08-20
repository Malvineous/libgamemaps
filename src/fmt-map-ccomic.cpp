/**
 * @file   fmt-map-ccomic.cpp
 * @brief  MapType and Map2D implementation for Captain Comic levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Captain_Comic_Map_Format
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

#include <boost/shared_array.hpp>
#include <camoto/gamemaps/map2d.hpp>
#include <camoto/iostream_helpers.hpp>
#include "fmt-map-ccomic.hpp"

#define CC_TILE_WIDTH           16
#define CC_TILE_HEIGHT          16

/// Map code to write for locations with no tile set.
#define CC_DEFAULT_BGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define CC_MAX_VALID_TILECODE   87 // number of tiles in tileset

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

/// Convert a map code into an image.
ImagePtr imageFromCComicCode(unsigned int code, VC_TILESET tileset)
	throw ()
{
	if (tileset.size() < 1) return ImagePtr(); // no tileset?!
	const Tileset::VC_ENTRYPTR& images = tileset[0]->getItems();
	if (code >= images.size()) return ImagePtr(); // out of range
	return tileset[0]->openImage(images[code]);
}

std::string CComicMapType::getMapCode() const
	throw ()
{
	return "map-ccomic";
}

std::string CComicMapType::getFriendlyName() const
	throw ()
{
	return "Captain Comic level";
}

std::vector<std::string> CComicMapType::getFileExtensions() const
	throw ()
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("pt");
	return vcExtensions;
}

std::vector<std::string> CComicMapType::getGameList() const
	throw ()
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Captain Comic");
	return vcGames;
}

MapType::Certainty CComicMapType::isInstance(istream_sptr psMap) const
	throw (std::ios::failure)
{
	psMap->seekg(0, std::ios::end);
	io::stream_offset lenMap = psMap->tellg();

	// Make sure there's enough data to read the map dimensions
	// TESTED BY: fmt_map_ccomic_isinstance_c01
	if (lenMap < 4) return MapType::DefinitelyNo;

	psMap->seekg(0, std::ios::beg);
	int width, height;
	psMap >> u16le(width) >> u16le(height);

	// Make sure the dimensions cover the entire file
	// TESTED BY: fmt_map_ccomic_isinstance_c02
	int mapLen = width * height;
	if (lenMap != mapLen + 4) return MapType::DefinitelyNo;

	// Read in the map and make sure all the tile codes are within range
	uint8_t *bg = new uint8_t[mapLen];
	psMap->read((char *)bg, mapLen);
	if (psMap->gcount() != mapLen) return MapType::DefinitelyNo; // read error
	for (int i = 0; i < mapLen; i++) {
		// Make sure each tile is within range
		// TESTED BY: fmt_map_ccomic_isinstance_c03
		if (bg[i] > CC_MAX_VALID_TILECODE) {
			delete[] bg;
			return MapType::DefinitelyNo;
		}
	}
	delete[] bg;

	// TESTED BY: fmt_map_ccomic_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr CComicMapType::create(SuppData& suppData) const
	throw (std::ios::failure)
{
	// TODO: Implement
	throw std::ios::failure("Not implemented yet!");
}

MapPtr CComicMapType::open(istream_sptr input, SuppData& suppData) const
	throw (std::ios::failure)
{
	input->seekg(0, std::ios::beg);
	int width, height;
	input >> u16le(width) >> u16le(height);
	int mapLen = width * height;

	// Read the background layer
	uint8_t *bg = new uint8_t[mapLen];
	input->read((char *)bg, mapLen);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(mapLen);
	for (int i = 0; i < mapLen; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (bg[i] == CC_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->x = i % width;
		t->y = i / width;
		t->code = bg[i];
		tiles->push_back(t);
	}
	delete[] bg;

	Map2D::LayerPtr bgLayer(new Map2D::Layer(
		"Background",
		Map2D::Layer::HasOwnSize | Map2D::Layer::HasOwnTileSize,
		width, height,
		CC_TILE_WIDTH, CC_TILE_HEIGHT,
		tiles,
		imageFromCComicCode
	));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);

	Map2DPtr map(new Map2D(
		Map2D::HasViewport | Map2D::HasPaths | Map2D::FixedPaths,
		193, 160, // viewport size
		0, 0,
		0, 0,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

unsigned long CComicMapType::write(MapPtr map, ostream_sptr output, SuppData& suppData) const
	throw (std::ios::failure)
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw std::ios::failure("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 1)
		throw std::ios::failure("Incorrect layer count for this format.");

	unsigned long lenWritten = 0;

	Map2D::LayerPtr layer = map2d->getLayer(0);
	int width, height;
	layer->getLayerSize(&width, &height);

	// Write the background layer
	output << u16le(width) << u16le(height);
	int mapLen = width * height;

	uint8_t *bg = new uint8_t[mapLen];
	memset(bg, CC_DEFAULT_BGTILE, mapLen); // default background tile
	const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
		i != items->end();
		i++
	) {
		if (((*i)->x > width) || ((*i)->y > height)) {
			delete[] bg;
			throw std::ios::failure("Layer has tiles outside map boundary!");
		}
		bg[(*i)->y * width + (*i)->x] = (*i)->code;
	}

	output->write((char *)bg, mapLen);
	lenWritten += mapLen;
	delete[] bg;

	return lenWritten;
}


} // namespace gamemaps
} // namespace camoto