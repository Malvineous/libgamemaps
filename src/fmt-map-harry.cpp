/**
 * @file   fmt-map-harry.cpp
 * @brief  MapType and Map2D implementation for Halloween Harry/Alien Carnage.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/GMF_Format_(Halloween_Harry)
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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
#include "fmt-map-harry.hpp"

/// Width of each tile in pixels
#define HH_TILE_WIDTH 16

/// Height of each tile in pixels
#define HH_TILE_HEIGHT 16

/// Width of map view during gameplay, in pixels
#define HH_VIEWPORT_WIDTH 288

/// Height of map view during gameplay, in pixels
#define HH_VIEWPORT_HEIGHT 144

/// Number of bytes for each actor struct
#define HH_ACTOR_LEN  128

/// Map code used for 'no tile'
#define HH_DEFAULT_TILE 0xFE

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class HarryActorLayer: virtual public GenericMap2D::Layer
{
	public:
		HarryActorLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
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
			TilesetCollection::const_iterator t = tileset->find(SpriteTileset);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[item->code]);
		}
};

class HarryBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		HarryBackgroundLayer(const std::string& name, ItemPtrVectorPtr& items,
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
			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[item->code]);
		}
};


std::string HarryMapType::getMapCode() const
{
	return "map-harry";
}

std::string HarryMapType::getFriendlyName() const
{
	return "Halloween Harry level";
}

std::vector<std::string> HarryMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("gmf");
	return vcExtensions;
}

std::vector<std::string> HarryMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Alien Carnage");
	vcGames.push_back("Halloween Harry");
	return vcGames;
}

MapType::Certainty HarryMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();
	// TESTED BY: fmt_map_harry_isinstance_c01
	if (lenMap < 29 + 768 + 256 + 10 + 2 + 4) return MapType::DefinitelyNo; // too short

	psMap->seekg(0, stream::start);

	// Check the signature
	char sig[0x12];
	psMap->read(sig, 0x12);
	// TESTED BY: fmt_map_harry_isinstance_c02
	if (strncmp(sig, "\x11SubZero Game File", 0x12) != 0) return MapType::DefinitelyNo;
	lenMap -= 0x12;

	// Skip flags
	psMap->seekg(11, stream::cur);
	lenMap -= 11;

	// Check palette is within range
	char pal[768];
	psMap->read(pal, 768);
	for (int i = 0; i < 768; i++) {
		// TESTED BY: fmt_map_harry_isinstance_c03
		if (pal[i] > 0x40) return MapType::DefinitelyNo;
	}
	lenMap -= 768;

	// Check tile flags are within range
	char tileFlags[256];
	psMap->read(tileFlags, 256);
	for (int i = 0; i < 256; i++) {
		// TESTED BY: fmt_map_harry_isinstance_c04
		if (tileFlags[i] > 0x01) return MapType::DefinitelyNo;
	}
	lenMap -= 256;

	// Skip unknown block
	psMap->seekg(10, stream::cur);
	lenMap -= 10;

	// isinstance_c01 should have prevented this
	assert(lenMap >= 6);

	uint16_t numActors;
	psMap >> u16le(numActors);
	lenMap -= 2;

	// TESTED BY: fmt_map_harry_isinstance_c05
	if (lenMap < (unsigned)(numActors * HH_ACTOR_LEN + 4)) return MapType::DefinitelyNo;

	psMap->seekg(numActors * HH_ACTOR_LEN, stream::cur);
	lenMap -= numActors * HH_ACTOR_LEN;

	assert(lenMap >= 4);
	uint16_t mapWidth, mapHeight;
	psMap >> u16le(mapWidth) >> u16le(mapHeight);
	lenMap -= 4;

	// TESTED BY: fmt_map_harry_isinstance_c06
	if (lenMap != (unsigned)(mapWidth * mapHeight * 2)) return MapType::DefinitelyNo;

	// TESTED BY: fmt_map_harry_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr HarryMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr HarryMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);

	Map::AttributePtrVectorPtr attributes(new Map::AttributePtrVector());

	// Skip signature and flags
	input->seekg(0x12 + 4, stream::cur);

	uint16_t startX, startY;
	input
		>> u16le(startX)
		>> u16le(startY)
	;

	input->seekg(2, stream::cur);

	uint8_t mapFlags;
	input >> u8(mapFlags);

	Map::AttributePtr attrParallax(new Map::Attribute);
	attrParallax->type = Map::Attribute::Enum;
	attrParallax->name = "Background";
	attrParallax->desc = "How to position the background layer as the player "
		"moves (parallax is only visible in-game).";
	attrParallax->enumValue = mapFlags == 0 ? 0 : 1;
	attrParallax->enumValueNames.push_back("Fixed");
	attrParallax->enumValueNames.push_back("Parallax");
	attributes->push_back(attrParallax);

	// TODO: Load palette
	input->seekg(768, stream::cur);

	// TODO: Load tile flags
	input->seekg(256, stream::cur);

	// Skip unknown block
	input->seekg(10, stream::cur);

	uint8_t code;

	// Read in the actor layer
	uint16_t numActors;
	input >> u16le(numActors);
	Map2D::Layer::ItemPtrVectorPtr actors(new Map2D::Layer::ItemPtrVector());
	actors->reserve(numActors + 1);

	// Create a fake object with the player's starting location
	{
		Map2D::Layer::ItemPtr p(new Map2D::Layer::Item);
		p->type = Map2D::Layer::Item::Player;
		p->x = startX;
		p->y = startY;
		p->code = 0; // unused
		p->playerNumber = 0; // player 1
		p->playerFacingLeft = false; // fixed?
		actors->push_back(p);
	}

	// Read the real objects
	for (unsigned int i = 0; i < numActors; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		input
			>> u8(code)
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->type = Map2D::Layer::Item::Default;
		// TEMP
		t->x /= 16;
		t->y /= 16;
		// ENDTEMP
		t->code = code;
		actors->push_back(t);
		input->seekg(128-1-2-2, stream::cur);
	}

	Map2D::Layer::ItemPtrVectorPtr validActorItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr actorLayer(new HarryActorLayer(actors, validActorItems));

	uint16_t mapWidth, mapHeight;
	input >> u16le(mapWidth) >> u16le(mapHeight);

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr bgtiles(new Map2D::Layer::ItemPtrVector());
	bgtiles->reserve(mapWidth * mapHeight);

	for (unsigned int y = 0; y < mapHeight; y++) {
		for (unsigned int x = 0; x < mapWidth; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = x;
			t->y = y;
			input >> u8(code);
			t->code = code;
			if (t->code != HH_DEFAULT_TILE) bgtiles->push_back(t);
		}
	}

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr bgLayer(new HarryBackgroundLayer("Background", bgtiles, validBGItems));

	// Read the foreground layer
	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
	fgtiles->reserve(mapWidth * mapHeight);
	for (unsigned int y = 0; y < mapHeight; y++) {
		for (unsigned int x = 0; x < mapWidth; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = x;
			t->y = y;
			input >> u8(code);
			t->code = code;
			if (t->code != HH_DEFAULT_TILE) fgtiles->push_back(t);
		}
	}

	Map2D::Layer::ItemPtrVectorPtr validFGItems(new Map2D::Layer::ItemPtrVector());
	Map2D::LayerPtr fgLayer(new HarryBackgroundLayer("Foreground", fgtiles, validFGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);
	layers.push_back(actorLayer);

	Map2DPtr map(new GenericMap2D(
		attributes, NO_GFX_CALLBACK,
		Map2D::HasViewport,
		HH_VIEWPORT_WIDTH, HH_VIEWPORT_HEIGHT,
		mapWidth, mapHeight,
		HH_TILE_WIDTH, HH_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void HarryMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 3)
		throw stream::error("Incorrect layer count for this format.");

	Map::AttributePtrVectorPtr attributes = map->getAttributes();
	if (attributes->size() != 1) {
		throw stream::error("Cannot write map as there is an incorrect number "
			"of attributes set.");
	}

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	Map::Attribute *attrParallax = attributes->at(0).get();
	if (attrParallax->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (parallax != enum)");
	}
	uint8_t mapFlags = attrParallax->enumValue;

	// Find the player-start-point objects
	uint16_t startX = 0, startY = 0;
	bool setPlayer = false;
	Map2D::LayerPtr layer = map2d->getLayer(2);
	const Map2D::Layer::ItemPtrVectorPtr actors = layer->getAllItems();
	uint16_t numActors = actors->size();
	for (Map2D::Layer::ItemPtrVector::const_iterator i =
		actors->begin(); i != actors->end(); i++
	) {
		if ((*i)->type & Map2D::Layer::Item::Player) {
			// This is the player starting location
			if (setPlayer) {
				// We've already set the player position
				throw stream::error("This map format can only have one player.");
			}
			if ((*i)->playerNumber == 0) {
				startX = (*i)->x;
				startY = (*i)->y;
				setPlayer = true;
			}
			numActors--;
		}
	}

	output
		<< nullPadded("\x11SubZero Game File", 0x12)
		<< u32le(0)
		<< u16le(startX)
		<< u16le(startY)
		<< u16le(0)
		<< u8(mapFlags)
	;

	// TODO: Write the palette
	char pal[768];
	memset(pal, 0x00, 768);
	output->write(pal, 768);

	// TODO: Write the tile flags
	char tileFlags[256];
	memset(tileFlags, 0x00, 256);
	output->write(tileFlags, 256);

	// Unknown data
	char unk[10];
	memset(unk, 0x00, 10);
	output->write(unk, 10);

	// Write the actor layer
	output << u16le(numActors);
	for (Map2D::Layer::ItemPtrVector::const_iterator i = actors->begin();
		i != actors->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));

		// Don't write player start points here
		if ((*i)->type & Map2D::Layer::Item::Player) continue;

		output
			<< u8((*i)->code)
			<< u16le((*i)->x)
			<< u16le((*i)->y)
		;
		char pad[128-5];
		memset(pad, 0x00, 128-5);
		output->write(pad, 128-5);
	}

	output
		<< u16le(mapWidth)
		<< u16le(mapHeight)
	;

	// Write the background layer
	unsigned long lenTiles = mapWidth * mapHeight;
	uint8_t *tiles = new uint8_t[lenTiles];
	boost::scoped_array<uint8_t> stiles(tiles);
	// Set the default background tile
	memset(tiles, HH_DEFAULT_TILE, lenTiles);

	layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr itemsBG = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = itemsBG->begin();
		i != itemsBG->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		tiles[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}
	output->write((char *)tiles, lenTiles);

	// Write the foreground layer
	memset(tiles, HH_DEFAULT_TILE, lenTiles);
	layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr itemsFG = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = itemsFG->begin();
		i != itemsFG->end();
		i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		tiles[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}
	output->write((char *)tiles, lenTiles);

	output->flush();
	return;
}

SuppFilenames HarryMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
