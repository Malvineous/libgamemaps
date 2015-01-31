/**
 * @file  fmt-map-got.cpp
 * @brief MapType and Map2D implementation for God of Thunder levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/God_of_Thunder_Level_Format
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
#include "fmt-map-got.hpp"

/// Length of entire map, in bytes
#define GOT_SCR_LEN             512

/// Length of background layer, in bytes
#define GOT_SCR_LEN_BG          240

/// Width of each cell
#define GOT_TILE_WIDTH           16

/// Height of each cell
#define GOT_TILE_HEIGHT          16

/// Width of map, in cells
#define GOT_SCR_WIDTH            20

/// Height of map, in cells
#define GOT_SCR_HEIGHT           12

/// Map code to write for locations with no tile set
#define GOT_DEFAULT_BGTILE     0xB0 // grass

/// This is the largest valid tile code in the background layer
#define GOT_MAX_VALID_BG_TILECODE  229 // number of tiles in tileset

/// This is the largest valid tile code in the object layer
#define GOT_MAX_VALID_OBJ_TILECODE  32 // number of tiles in tileset

/// This is the largest valid tile code in the object layer
#define GOT_MAX_VALID_ACTOR_TILECODE  76 // number of tiles in tileset

/// Offset of the actor data
#define GOT_ACTOR_OFFSET (GOT_SCR_LEN_BG + 2)

/// Offset of the actor data
#define GOT_OBJ_OFFSET (GOT_ACTOR_OFFSET + 16 * 2 + 16*3)

/// Maximum number of actors in a level
#define GOT_NUM_ACTORS 16

/// Maximum number of objects in a level
#define GOT_NUM_OBJECTS 30

/// Total number of screens in the map file
#define GOT_MAP_NUMSCREENS 120

/// Number of screens to draw in the horizontal direction
#define GOT_MAP_SCREENCOUNT_HORIZ 10

/// Number of screens to draw in the vertical direction
#define GOT_MAP_SCREENCOUNT_VERT (GOT_MAP_NUMSCREENS / GOT_MAP_SCREENCOUNT_HORIZ)

/// Maximum number of songs to list in available tiles
#define GOT_MAX_SONGS 10

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_GOTBackground: virtual public GenericMap2D::Layer
{
	public:
		Layer_GOTBackground(ItemPtrVectorPtr& items,
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

class Layer_GOTFlags: virtual public GenericMap2D::Layer
{
	public:
		Layer_GOTFlags(const std::string& name, ItemPtrVectorPtr& items,
			ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					name,
					Map2D::Layer::HasOwnTileSize | Map2D::Layer::UseImageDims,
					0, 0,
					GOT_SCR_WIDTH * GOT_TILE_WIDTH, GOT_SCR_HEIGHT * GOT_TILE_HEIGHT,
					items, validItems
				)
		{
		}

		virtual Map2D::Layer::ImageType imageFromCode(
			const Map2D::Layer::ItemPtr& item, const TilesetCollectionPtr& tileset,
			ImagePtr *out) const
		{
			if ((item->code >= 256) && (item->code < 256 + GOT_MAX_SONGS)) {
				return (Map2D::Layer::ImageType)(Map2D::Layer::Digit0 + (item->code - 256));
			}

			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = t->second->openImage(images[item->code]);
			return Map2D::Layer::Supplied;
		}

};

class Layer_GOTActor: virtual public GenericMap2D::Layer
{
	public:
		Layer_GOTActor(ItemPtrVectorPtr& items,
			ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Actors",
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
			TilesetCollection::const_iterator t = tileset->find(ForegroundTileset1);
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			const Tileset::VC_ENTRYPTR& ts = t->second->getItems();
			TilesetPtr tsub = t->second->openTileset(ts[item->code]);
			if (!tsub) {
				std::cerr << "[fmt-map-got] Unable to open subtileset #"
					<< item->code << std::endl;
				return Map2D::Layer::Unknown;
			}

			const Tileset::VC_ENTRYPTR& images = tsub->getItems();
			if (item->code >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = tsub->openImage(images[0]);
			return Map2D::Layer::Supplied;
		}

};

class Layer_GOTObject: virtual public GenericMap2D::Layer
{
	public:
		Layer_GOTObject(ItemPtrVectorPtr& items,
			ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Objects",
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
			TilesetCollection::const_iterator t = tileset->find(ForegroundTileset2);
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = t->second->openImage(images[item->code]);
			return Map2D::Layer::Supplied;
		}

};


std::string MapType_GOT::getMapCode() const
{
	return "map-got";
}

std::string MapType_GOT::getFriendlyName() const
{
	return "God of Thunder level";
}

std::vector<std::string> MapType_GOT::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	return vcExtensions;
}

std::vector<std::string> MapType_GOT::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("God of Thunder");
	return vcGames;
}

MapType::Certainty MapType_GOT::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

	// Make sure there's enough data
	// TESTED BY: fmt_map_got_isinstance_c01
	if (lenMap != GOT_SCR_LEN * GOT_MAP_NUMSCREENS) return MapType::DefinitelyNo;

	psMap->seekg(0, stream::start);
	uint8_t bg[GOT_SCR_LEN_BG];
	psMap->read(bg, GOT_SCR_LEN_BG);
	for (int i = 0; i < GOT_SCR_LEN_BG; i++) {
		// Map code out of range
		// TESTED BY: fmt_map_got_isinstance_c02
		if (bg[i] > GOT_MAX_VALID_BG_TILECODE) return MapType::DefinitelyNo;
	}

	// TESTED BY: fmt_map_got_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr MapType_GOT::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr MapType_GOT::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);

	// Read the whole map since it's so small
	uint8_t *mapData = new uint8_t[GOT_SCR_LEN];
	boost::scoped_array<uint8_t> scoped_mapData(mapData);

	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr tilesDefault(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr tilesMus(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr tilesActor(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr tilesObj(new Map2D::Layer::ItemPtrVector());

	for (unsigned int s = 0; s < GOT_MAP_NUMSCREENS; s++) {
		unsigned int originX = s % GOT_MAP_SCREENCOUNT_HORIZ;
		unsigned int originY = s / GOT_MAP_SCREENCOUNT_HORIZ;

		input->read((char *)mapData, GOT_SCR_LEN);

		// Process the background layer
		tiles->reserve(GOT_SCR_LEN_BG);
		for (unsigned int i = 0; i < GOT_SCR_LEN_BG; i++) {
			// The default tile actually has an image, so don't exclude it
			//if (bg[i] == GOT_DEFAULT_BGTILE) continue;

			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = originX * GOT_SCR_WIDTH + i % GOT_SCR_WIDTH;
			t->y = originY * GOT_SCR_HEIGHT + i / GOT_SCR_WIDTH;
			t->code = mapData[i];
			tiles->push_back(t);
		}

		// Get the default BG tile for this screen
		{
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = originX;
			t->y = originY;
			t->code = mapData[GOT_SCR_LEN_BG];
			tilesDefault->push_back(t);
		}

		// Get the default song for this screen
		{
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = originX;
			t->y = originY;
			t->code = 256 + mapData[GOT_SCR_LEN_BG + 1];
			tilesMus->push_back(t);
		}

		// Process the actor layer
		for (unsigned int i = 0; i < GOT_NUM_ACTORS; i++) {
			if (mapData[GOT_ACTOR_OFFSET + i] == 0) continue;

			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = originX * GOT_SCR_WIDTH + mapData[GOT_ACTOR_OFFSET + 16 + i] % GOT_SCR_WIDTH;
			t->y = originY * GOT_SCR_HEIGHT + mapData[GOT_ACTOR_OFFSET + 16 + i] / GOT_SCR_WIDTH;
			t->code = mapData[GOT_ACTOR_OFFSET + i] - 1;
			tilesActor->push_back(t);
		}

		// Process the object layer
		for (unsigned int i = 0; i < GOT_NUM_OBJECTS; i++) {
			if (mapData[GOT_OBJ_OFFSET + i] == 0) continue;

			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = originX * GOT_SCR_WIDTH
				+ (mapData[GOT_OBJ_OFFSET + 30 + i * 2]
					| (mapData[GOT_OBJ_OFFSET + 30 + i * 2 + 1] << 8));
			t->y = originY * GOT_SCR_HEIGHT
				+ (mapData[GOT_OBJ_OFFSET + 90 + i * 2]
					| (mapData[GOT_OBJ_OFFSET + 90 + i * 2 + 1] << 8));
			t->code = mapData[GOT_OBJ_OFFSET + i] - 1;
			tilesObj->push_back(t);
		}

	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= GOT_MAX_VALID_BG_TILECODE; i++) {
		// The default tile actually has an image, so don't exclude it
		//if (i == GOT_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}
	Map2D::Layer::ItemPtrVectorPtr validActorItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 1; i <= GOT_MAX_VALID_ACTOR_TILECODE; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validActorItems->push_back(t);
	}
	Map2D::Layer::ItemPtrVectorPtr validObjItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 1; i <= GOT_MAX_VALID_OBJ_TILECODE; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validObjItems->push_back(t);
	}
	Map2D::Layer::ItemPtrVectorPtr validMusItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i < GOT_MAX_SONGS; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = 256 + i;
		validMusItems->push_back(t);
	}

	// Create the map structures
	Map2D::LayerPtr musLayer(new Layer_GOTFlags("Music", tilesMus, validMusItems));
	Map2D::LayerPtr defaultBGLayer(new Layer_GOTFlags("Default BG", tilesDefault, validBGItems));
	Map2D::LayerPtr bgLayer(new Layer_GOTBackground(tiles, validBGItems));
	Map2D::LayerPtr actorLayer(new Layer_GOTActor(tilesActor, validActorItems));
	Map2D::LayerPtr objLayer(new Layer_GOTObject(tilesObj, validObjItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(musLayer);
	layers.push_back(defaultBGLayer);
	layers.push_back(bgLayer);
	layers.push_back(actorLayer);
	layers.push_back(objLayer);

	Map2DPtr map(new GenericMap2D(
		Map::Attributes(), Map::GraphicsFilenames(),
		Map2D::HasViewport,
		320, 192, // viewport size
		GOT_SCR_WIDTH * GOT_MAP_SCREENCOUNT_HORIZ,
		GOT_SCR_HEIGHT * GOT_MAP_SCREENCOUNT_VERT,
		GOT_TILE_WIDTH, GOT_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void MapType_GOT::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 5)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);
	if (
		(mapWidth != GOT_SCR_WIDTH * GOT_MAP_SCREENCOUNT_HORIZ)
		|| (mapHeight != GOT_SCR_HEIGHT * GOT_MAP_SCREENCOUNT_VERT)
	) {
		throw stream::error("Incorrect map size for this format.");
	}

	uint8_t *bg = new uint8_t[mapWidth * mapHeight];
	boost::scoped_array<uint8_t> scoped_bg(bg);
	memset(bg, GOT_DEFAULT_BGTILE, mapWidth * mapHeight);
	{
		// Prepare the background layer
		Map2D::LayerPtr layer = map2d->getLayer(2);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = items->begin(); i != items->end(); i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			unsigned int screenX = (*i)->x / GOT_SCR_WIDTH;
			unsigned int screenY = (*i)->y / GOT_SCR_HEIGHT;
			unsigned int s = screenY * GOT_MAP_SCREENCOUNT_HORIZ + screenX;
			unsigned int originX = screenX * GOT_SCR_WIDTH;
			unsigned int originY = screenY * GOT_SCR_HEIGHT;
			uint8_t *screen = bg + s * GOT_SCR_LEN_BG;
			screen[((*i)->y - originY) * GOT_SCR_WIDTH + ((*i)->x - originX)] = (*i)->code;
		}
	}

	uint8_t defaultBG[GOT_MAP_NUMSCREENS];
	memset(defaultBG, GOT_DEFAULT_BGTILE, GOT_MAP_NUMSCREENS);
	{
		// Prepare the default background tile
		Map2D::LayerPtr layer = map2d->getLayer(1);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = items->begin(); i != items->end(); i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			defaultBG[(*i)->y * GOT_MAP_SCREENCOUNT_HORIZ + (*i)->x] = (*i)->code;
		}
	}

	uint8_t mus[GOT_MAP_NUMSCREENS];
	memset(mus, 0, GOT_MAP_NUMSCREENS);
	{
		// Prepare the song flag
		Map2D::LayerPtr layer = map2d->getLayer(0);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = items->begin(); i != items->end(); i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			if ((*i)->code < 256) {
				throw stream::error("Music layer has tiles that are out of range");
			}
			mus[(*i)->y * GOT_MAP_SCREENCOUNT_HORIZ + (*i)->x] = (*i)->code - 256;
		}
	}

	struct {
		unsigned int num;
		struct {
			uint8_t id;
			uint8_t pos;
		} entry[GOT_NUM_ACTORS];
	} actor[GOT_MAP_NUMSCREENS];
	memset(actor, 0, sizeof(actor));
	{
		// Prepare the actor layer
		Map2D::LayerPtr layer = map2d->getLayer(3);
		const Map2D::Layer::ItemPtrVectorPtr actorItems = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = actorItems->begin(); i != actorItems->end(); i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			unsigned int screenX = (*i)->x / GOT_SCR_WIDTH;
			unsigned int screenY = (*i)->y / GOT_SCR_HEIGHT;
			unsigned int s = screenY * GOT_MAP_SCREENCOUNT_HORIZ + screenX;
			unsigned int originX = screenX * GOT_SCR_WIDTH;
			unsigned int originY = screenY * GOT_SCR_HEIGHT;
			actor[s].entry[actor[s].num].id = (*i)->code + 1;
			actor[s].entry[actor[s].num].pos = ((*i)->y - originY) * GOT_SCR_WIDTH
				+ ((*i)->x - originX);
			actor[s].num++;
		}
	}

	struct {
		unsigned int num;
		struct {
			uint8_t id;
			uint16_t x;
			uint16_t y;
		} entry[GOT_NUM_OBJECTS];
	} obj[GOT_MAP_NUMSCREENS];
	memset(obj, 0, sizeof(obj));
	{
		// Prepare the object layer
		Map2D::LayerPtr layer = map2d->getLayer(4);
		const Map2D::Layer::ItemPtrVectorPtr objItems = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = objItems->begin(); i != objItems->end(); i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			unsigned int screenX = (*i)->x / GOT_SCR_WIDTH;
			unsigned int screenY = (*i)->y / GOT_SCR_HEIGHT;
			unsigned int s = screenY * GOT_MAP_SCREENCOUNT_HORIZ + screenX;
			unsigned int originX = screenX * GOT_SCR_WIDTH;
			unsigned int originY = screenY * GOT_SCR_HEIGHT;
			obj[s].entry[obj[s].num].id = (*i)->code + 1;
			obj[s].entry[obj[s].num].x = (*i)->x - originX;
			obj[s].entry[obj[s].num].y = (*i)->y - originY;
			obj[s].num++;
		}
	}

	for (unsigned int s = 0; s < GOT_MAP_NUMSCREENS; s++) {
		// Write the background layer
		output->write(bg + s * GOT_SCR_LEN_BG, GOT_SCR_LEN_BG);
		output
			<< u8(defaultBG[s])
			<< u8(mus[s])
		;
		for (int i = 0; i < GOT_NUM_ACTORS; i++) {
			output << u8(actor[s].entry[i].id);
		}
		for (int i = 0; i < GOT_NUM_ACTORS; i++) {
			output << u8(actor[s].entry[i].pos);
		}
		// Padding, this data is unknown
		output << nullPadded("", 16*3);

		for (int i = 0; i < GOT_NUM_OBJECTS; i++) {
			output << u8(obj[s].entry[i].id);
		}
		for (int i = 0; i < GOT_NUM_OBJECTS; i++) {
			output << u16le(obj[s].entry[i].x);
		}
		for (int i = 0; i < GOT_NUM_OBJECTS; i++) {
			output << u16le(obj[s].entry[i].y);
		}

		// TEMP: Pad file to 512 bytes until the format of this data is known
		output << nullPadded("", 40);
	}

	output->flush();
	return;
}

SuppFilenames MapType_GOT::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
