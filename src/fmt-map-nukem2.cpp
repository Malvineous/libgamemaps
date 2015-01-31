/**
 * @file  fmt-map-nukem2.cpp
 * @brief MapType and Map2D implementation for Duke Nukem II levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Duke_Nukem_II_Map_Format
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
#include "fmt-map-nukem2.hpp"

/// Width of each tile in pixels
#define DN2_TILE_WIDTH 8

/// Height of each tile in pixels
#define DN2_TILE_HEIGHT 8

/// Width of map view during gameplay, in pixels
#define DN2_VIEWPORT_WIDTH 256

/// Height of map view during gameplay, in pixels
#define DN2_VIEWPORT_HEIGHT 160

/// Length of the map data, in bytes
#define DN2_LAYER_LEN_BG 65500u

/// Number of tiles in the map
#define DN2_NUM_TILES_BG (DN2_LAYER_LEN_BG / 2)

/// Number of tiles in the solid tileset
#define DN2_NUM_SOLID_TILES 1000

/// Number of tiles in the masked tileset
#define DN2_NUM_MASKED_TILES 160

/// Map code to write for locations with no tile set
#define DN2_DEFAULT_BGTILE 0x00

// Indices into attributes array
#define ATTR_CZONE    0
#define ATTR_BACKDROP 1
#define ATTR_MUSIC    2
#define ATTR_USEALTBD 3
#define ATTR_QUAKE    4
#define ATTR_SCROLLBD 5
#define ATTR_PARALLAX 6
#define ATTR_ALTBD    7
#define ATTR_ZONEATTR 8
#define ATTR_ZONETSET 9
#define ATTR_ZONEMSET 10

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

class Layer_Nukem2Actor: virtual public GenericMap2D::Layer
{
	public:
		Layer_Nukem2Actor(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
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
			TilesetCollection::const_iterator t = tileset->find(SpriteTileset1);
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			// TODO
			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = t->second->openImage(images[item->code]);
			return Map2D::Layer::Supplied;
		}
};

class Layer_Nukem2Background: virtual public GenericMap2D::Layer
{
	public:
		Layer_Nukem2Background(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
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
			const Tileset::VC_ENTRYPTR& czoneTilesets = t->second->getItems();

			unsigned int index = item->code;
			unsigned int czoneTarget = 0;
			if (czoneTarget >= czoneTilesets.size()) return Map2D::Layer::Unknown; // out of range
			TilesetPtr ts = t->second->openTileset(czoneTilesets[czoneTarget]);
			const Tileset::VC_ENTRYPTR& images = ts->getItems();
			if (index >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = ts->openImage(images[index]);
			return Map2D::Layer::Supplied;
		}
};

class Layer_Nukem2Foreground: virtual public GenericMap2D::Layer
{
	public:
		Layer_Nukem2Foreground(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Foreground",
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
			const Tileset::VC_ENTRYPTR& czoneTilesets = t->second->getItems();

			unsigned int index = item->code;
			unsigned int czoneTarget = 1;
			if (czoneTarget >= czoneTilesets.size()) return Map2D::Layer::Unknown; // out of range
			TilesetPtr ts = t->second->openTileset(czoneTilesets[czoneTarget]);
			const Tileset::VC_ENTRYPTR& images = ts->getItems();
			if (index >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = ts->openImage(images[index]);
			return Map2D::Layer::Supplied;
		}
};

class Map2D_Nukem2: virtual public GenericMap2D
{
	public:
		Map2D_Nukem2(const Attributes& attributes,
			unsigned int width, LayerPtrVector& layers)
			:	GenericMap2D(
					attributes, GraphicsFilenames(),
					Map2D::HasViewport,
					DN2_VIEWPORT_WIDTH, DN2_VIEWPORT_HEIGHT,
					width, DN2_NUM_TILES_BG / width,
					DN2_TILE_WIDTH, DN2_TILE_HEIGHT,
					layers, Map2D::PathPtrVectorPtr()
				)
		{
			// Populate the graphics filenames
			assert(this->attributes.size() > 0);

			Map::GraphicsFilename gf;
			gf.type = "tls-nukem2-czone";
			gf.filename = this->attributes[ATTR_CZONE].filenameValue;
			if (!gf.filename.empty()) {
				this->graphicsFilenames[BackgroundTileset1] = gf;
			}

			gf.type = "img-nukem2-backdrop";
			gf.filename = this->attributes[ATTR_BACKDROP].filenameValue;
			if (!gf.filename.empty()) {
				this->graphicsFilenames[BackgroundImage] = gf;
			}
		}

		Map2D::ImageAttachment getBackgroundImage(
			const TilesetCollectionPtr& tileset, ImagePtr *outImage,
			PaletteEntry *outColour) const
		{
			TilesetCollection::const_iterator t = tileset->find(BackgroundImage);
			if (t != tileset->end()) {
				const Tileset::VC_ENTRYPTR& images = t->second->getItems();
				if (images.size() > 0) {
					// Just open the first image, it will have been whatever was supplied
					// by this->graphicsFilenames[BackgroundImage]
					*outImage = t->second->openImage(images[0]);
					return Map2D::SingleImageCentred;
				}
			}

			return Map2D::NoBackground;
		}
};


std::string MapType_Nukem2::getMapCode() const
{
	return "map-nukem2";
}

std::string MapType_Nukem2::getFriendlyName() const
{
	return "Duke Nukem II level";
}

std::vector<std::string> MapType_Nukem2::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("mni");
	return vcExtensions;
}

std::vector<std::string> MapType_Nukem2::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Duke Nukem II");
	return vcGames;
}

MapType::Certainty MapType_Nukem2::isInstance(stream::input_sptr psMap) const
{
	stream::len lenMap = psMap->size();

	// TESTED BY: fmt_map_nukem2_isinstance_c01
	if (lenMap < 2+13+13+13+1+1+2+2 + 2+DN2_LAYER_LEN_BG) return MapType::DefinitelyNo; // too short

	psMap->seekg(0, stream::start);
	uint16_t bgOffset;
	std::string zoneFile, backFile, musFile;
	psMap
		>> u16le(bgOffset)
	;

	// TESTED BY: fmt_map_nukem2_isinstance_c02
	if (bgOffset > lenMap - (2+DN2_LAYER_LEN_BG)) return MapType::DefinitelyNo; // offset wrong

	psMap->seekg(13 * 3 + 4, stream::cur);

	uint16_t numActorInts;
	psMap >> u16le(numActorInts);

	// TESTED BY: fmt_map_nukem2_isinstance_c03
	if (2+13*3+6 + numActorInts * 2 + 2+DN2_LAYER_LEN_BG > lenMap) return MapType::DefinitelyNo; // too many actors

	psMap->seekg(bgOffset + 2+DN2_LAYER_LEN_BG, stream::start);
	uint16_t lenExtra;
	psMap >> u16le(lenExtra);
	// TESTED BY: fmt_map_nukem2_isinstance_c04
	if (bgOffset + 2+DN2_LAYER_LEN_BG + lenExtra+2 > lenMap) return MapType::DefinitelyNo; // extra data too long

	// TESTED BY: fmt_map_nukem2_isinstance_c00
	if (bgOffset + 2+DN2_LAYER_LEN_BG + lenExtra+2 + 13*3 == lenMap) return MapType::DefinitelyYes;

	// TESTED BY: fmt_map_nukem2_isinstance_c05
	return MapType::PossiblyYes;
}

MapPtr MapType_Nukem2::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr MapType_Nukem2::open(stream::input_sptr input, SuppData& suppData) const
{
	stream::pos lenMap = input->size();
	input->seekg(0, stream::start);

	uint16_t bgOffset, unk, numActorInts;
	std::string zoneFile, backFile, musFile;
	uint8_t flags, altBack;
	input
		>> u16le(bgOffset)
	;

	Map::Attributes attributes;
	Map::Attribute attr;

	attr.type = Map::Attribute::Filename;
	attr.name = "CZone tileset";
	attr.desc = "Filename of the tileset to use for drawing the foreground and background layers";
	input >> nullPadded(attr.filenameValue, 13);
	// Trim off the padding spaces
	attr.filenameValue = attr.filenameValue.substr(0, attr.filenameValue.find_last_not_of(' ') + 1);
	attr.filenameValidExtension = "mni";
	assert(attributes.size() == ATTR_CZONE); // make sure compile-time index is correct
	attributes.push_back(attr);

	attr.type = Map::Attribute::Filename;
	attr.name = "Backdrop";
	attr.desc = "Filename of the backdrop to draw behind the map";
	input >> nullPadded(attr.filenameValue, 13);
	// Trim off the padding spaces
	attr.filenameValue = attr.filenameValue.substr(0, attr.filenameValue.find_last_not_of(' ') + 1);
	attr.filenameValidExtension = "mni";
	assert(attributes.size() == ATTR_BACKDROP); // make sure compile-time index is correct
	attributes.push_back(attr);

	attr.type = Map::Attribute::Filename;
	attr.name = "Music";
	attr.desc = "File to play as background music";
	input >> nullPadded(attr.filenameValue, 13);
	// Trim off the padding spaces
	attr.filenameValue = attr.filenameValue.substr(0, attr.filenameValue.find_last_not_of(' ') + 1);
	attr.filenameValidExtension = "imf";
	assert(attributes.size() == ATTR_MUSIC); // make sure compile-time index is correct
	attributes.push_back(attr);

	input
		>> u8(flags)
		>> u8(altBack)
		>> u16le(unk)
		>> u16le(numActorInts)
	;
	lenMap -= 2+13+13+13+1+1+2+2;

	attr.type = Map::Attribute::Enum;
	attr.name = "Use alt backdrop?";
	attr.desc = "When should the alternate backdrop file be used?";
	attr.enumValue = (flags >> 6) & 3;
	attr.enumValueNames.push_back("Never");
	attr.enumValueNames.push_back("After destroying force field");
	attr.enumValueNames.push_back("After teleporting");
	attr.enumValueNames.push_back("Both? (this value has an unknown/untested effect)");
	assert(attributes.size() == ATTR_USEALTBD); // make sure compile-time index is correct
	attributes.push_back(attr);

	attr.type = Map::Attribute::Enum;
	attr.name = "Earthquake";
	attr.desc = "Should the level shake like there is an earthquake?";
	attr.enumValue = (flags >> 5) & 1;
	attr.enumValueNames.push_back("No");
	attr.enumValueNames.push_back("Yes");
	assert(attributes.size() == ATTR_QUAKE); // make sure compile-time index is correct
	attributes.push_back(attr);

	attr.type = Map::Attribute::Enum;
	attr.name = "Backdrop movement";
	attr.desc = "Should the backdrop move when the player is stationary?";
	attr.enumValue = (flags >> 3) & 3;
	attr.enumValueNames.push_back("No");
	attr.enumValueNames.push_back("Scroll left");
	attr.enumValueNames.push_back("Scroll up");
	attr.enumValueNames.push_back("3 (this value has an unknown/untested effect)");
	assert(attributes.size() == ATTR_SCROLLBD); // make sure compile-time index is correct
	attributes.push_back(attr);

	attr.type = Map::Attribute::Enum;
	attr.name = "Parallax";
	attr.desc = "How should the backdrop scroll when the player moves?";
	attr.enumValue = (flags >> 0) & 3;
	attr.enumValueNames.push_back("Fixed - no movement");
	attr.enumValueNames.push_back("Horizontal and vertical movement");
	attr.enumValueNames.push_back("Horizontal movement only");
	attr.enumValueNames.push_back("3 (this value has an unknown/untested effect)");
	assert(attributes.size() == ATTR_PARALLAX); // make sure compile-time index is correct
	attributes.push_back(attr);

	attr.type = Map::Attribute::Integer;
	attr.name = "Alt backdrop";
	attr.desc = "Number of alternate backdrop file (DROPx.MNI)";
	attr.integerValue = altBack;
	attr.integerMinValue = 1;
	attr.integerMaxValue = 24;
	assert(attributes.size() == ATTR_ALTBD); // make sure compile-time index is correct
	attributes.push_back(attr);

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
	Map2D::LayerPtr actorLayer(new Layer_Nukem2Actor(actors, validActorItems));

	input->seekg(bgOffset, stream::start);
	uint16_t mapWidth;
	input
		>> u16le(mapWidth)
	;

	// Read the main layer
	Map2D::Layer::ItemPtrVectorPtr tilesBG(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr tilesFG(new Map2D::Layer::ItemPtrVector());
	unsigned int tileValues[DN2_NUM_TILES_BG];
	memset(tileValues, 0, sizeof(tileValues));

	unsigned int *v = tileValues;
	for (unsigned int i = 0; (i < DN2_NUM_TILES_BG) && (lenMap >= 2); i++) {
		input >> u16le(*v++);
		lenMap -= 2;
	}

	uint16_t lenExtra;
	input >> u16le(lenExtra);
	unsigned int extraValues[DN2_NUM_TILES_BG];
	memset(extraValues, 0, sizeof(extraValues));
	unsigned int *ev = extraValues;
	unsigned int *ev_end = extraValues + DN2_NUM_TILES_BG;
	for (unsigned int i = 0; i < lenExtra; i++) {
		uint8_t code;
		input >> u8(code);
		lenMap--;
		if (code & 0x80) {
			// Multiple bytes concatenated together
			// code == 0xFF for one byte, 0xFE for two bytes, etc.
			unsigned int len = 0x100 - code;
			while (len--) {
				input >> u8(code);
				lenMap--;
				i++;
				if (ev + 4 >= ev_end) break;
				*ev++ = (code << 5) & 0x60;
				*ev++ = (code << 3) & 0x60;
				*ev++ = (code << 1) & 0x60;
				*ev++ = (code >> 1) & 0x60;
			}
		} else {
			unsigned int len = code;
			input >> u8(code);
			lenMap--;
			i++;
			if (code == 0x00) {
				ev += len * 4; // faster
			} else {
				while (len--) {
					if (ev + 4 >= ev_end) break;
					*ev++ = (code << 5) & 0x60;
					*ev++ = (code << 3) & 0x60;
					*ev++ = (code << 1) & 0x60;
					*ev++ = (code >> 1) & 0x60;
				}
			}
		}
		if (ev + 4 > ev_end) {
			// This would read past the end of the array, so skip it
			lenMap -= lenExtra - i - 1;
			input->seekg(lenExtra - i - 1, stream::cur);
			break;
		}
	}

	v = tileValues;
	ev = extraValues;
	for (unsigned int i = 0; i < DN2_NUM_TILES_BG; i++) {
		if (*v & 0x8000) {
			// This cell has a foreground and background tile
			{
				Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
				t->type = Map2D::Layer::Item::Default;
				t->x = i % mapWidth;
				t->y = i / mapWidth;
				t->code = *v & 0x3FF;
				if (t->code != DN2_DEFAULT_BGTILE) tilesBG->push_back(t);
			}

			{
				Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
				t->type = Map2D::Layer::Item::Default;
				t->x = i % mapWidth;
				t->y = i / mapWidth;
				t->code = ((*v >> 10) & 0x1F) | *ev;
				tilesFG->push_back(t);
			}

		} else if (*v < DN2_NUM_SOLID_TILES * DN2_TILE_WIDTH) {
			// Background only tile

			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = i % mapWidth;
			t->y = i / mapWidth;
			t->code = *v >> 3;
			if (t->code != DN2_DEFAULT_BGTILE) tilesBG->push_back(t);
		} else {
			// Foreground only tile

			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = i % mapWidth;
			t->y = i / mapWidth;
			t->code = ((*v >> 3) - DN2_NUM_SOLID_TILES) / 5;
			tilesFG->push_back(t);
		}
		v++;
		ev++;
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i < DN2_NUM_SOLID_TILES; i++) {
		// Background tiles first
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}
	Map2D::Layer::ItemPtrVectorPtr validFGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i < DN2_NUM_MASKED_TILES; i++) {
		// Then foreground tiles
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validFGItems->push_back(t);
	}

	// Trailing filenames
	attr.type = Map::Attribute::Filename;
	attr.name = "Zone attribute (unused)";
	attr.desc = "Filename of the zone tile attributes (unused)";
	input >> nullPadded(attr.filenameValue, 13);
	// Trim off the padding spaces
	attr.filenameValue = attr.filenameValue.substr(0, attr.filenameValue.find_last_not_of(' ') + 1);
	attr.filenameValidExtension = "mni";
	assert(attributes.size() == ATTR_ZONEATTR); // make sure compile-time index is correct
	attributes.push_back(attr);

	attr.type = Map::Attribute::Filename;
	attr.name = "Zone tileset (unused)";
	attr.desc = "Filename of the zone solid tileset (unused)";
	input >> nullPadded(attr.filenameValue, 13);
	// Trim off the padding spaces
	attr.filenameValue = attr.filenameValue.substr(0, attr.filenameValue.find_last_not_of(' ') + 1);
	attr.filenameValidExtension = "mni";
	assert(attributes.size() == ATTR_ZONETSET); // make sure compile-time index is correct
	attributes.push_back(attr);

	attr.type = Map::Attribute::Filename;
	attr.name = "Zone masked tileset (unused)";
	attr.desc = "Filename of the zone masked tileset (unused)";
	input >> nullPadded(attr.filenameValue, 13);
	// Trim off the padding spaces
	attr.filenameValue = attr.filenameValue.substr(0, attr.filenameValue.find_last_not_of(' ') + 1);
	attr.filenameValidExtension = "mni";
	assert(attributes.size() == ATTR_ZONEMSET); // make sure compile-time index is correct
	attributes.push_back(attr);

	// Create the map structures
	Map2D::LayerPtr bgLayer(new Layer_Nukem2Background(tilesBG, validBGItems));
	Map2D::LayerPtr fgLayer(new Layer_Nukem2Foreground(tilesFG, validFGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);
	layers.push_back(actorLayer);

	Map2DPtr map(new Map2D_Nukem2(attributes, mapWidth, layers));

	return map;
}

void MapType_Nukem2::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 3)
		throw stream::error("Incorrect layer count for this format.");

	// Figure out where the main data will start
	Map2D::LayerPtr layerA = map2d->getLayer(2);
	const Map2D::Layer::ItemPtrVectorPtr actors = layerA->getAllItems();
	stream::pos offBG = 2+13+13+13+1+1+2+2+6*actors->size();

	output
		<< u16le(offBG)
	;
	// CZone
	Map::Attribute& attr0 = map->attributes[ATTR_CZONE];
	std::string val = attr0.filenameValue;
	int padamt = 12 - val.length();
	val += std::string(padamt, ' '); // pad with spaces
	output << nullPadded(val, 13);

	// Backdrop
	Map::Attribute& attr1 = map->attributes[ATTR_BACKDROP];
	val = attr1.filenameValue;
	padamt = 12 - val.length();
	val += std::string(padamt, ' '); // pad with spaces
	output << nullPadded(val, 13);

	// Song
	Map::Attribute& attr2 = map->attributes[ATTR_MUSIC];
	val = attr2.filenameValue;
	padamt = 12 - val.length();
	val += std::string(padamt, ' '); // pad with spaces
	output << nullPadded(val, 13);

	uint8_t flags = 0;

	Map::Attribute& attr3 = map->attributes[ATTR_USEALTBD];
	flags |= attr3.enumValue << 6;

	Map::Attribute& attr4 = map->attributes[ATTR_QUAKE];
	flags |= attr4.enumValue << 5;

	Map::Attribute& attr5 = map->attributes[ATTR_SCROLLBD];
	flags |= attr5.enumValue << 3;

	Map::Attribute& attr6 = map->attributes[ATTR_PARALLAX];
	flags |= attr6.enumValue << 0;

	output << u8(flags);

	Map::Attribute& attr7 = map->attributes[ATTR_ALTBD];
	output << u8(attr7.integerValue);

	output << u16le(0);

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	// Write the actor layer
	uint16_t numActorInts = actors->size() * 3;
	output << u16le(numActorInts);
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = actors->begin(); i != actors->end(); i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		output
			<< u16le((*i)->code)
			<< u16le((*i)->x)
			<< u16le((*i)->y)
		;
	}

	// Write the background layer
	uint16_t *bg = new uint16_t[DN2_NUM_TILES_BG];
	boost::scoped_array<uint16_t> sbg(bg);
	// Set the default background tile
	memset(bg, 0x00, DN2_NUM_TILES_BG * sizeof(uint16_t));

	uint16_t *fg = new uint16_t[DN2_NUM_TILES_BG];
	boost::scoped_array<uint16_t> sfg(fg);
	// Set the default foreground tile
	memset(fg, 0xFF, DN2_NUM_TILES_BG * sizeof(uint16_t));

	uint16_t *extra = new uint16_t[DN2_NUM_TILES_BG];
	boost::scoped_array<uint16_t> sextra(extra);
	// Set the default extra bits
	memset(extra, 0x00, DN2_NUM_TILES_BG * sizeof(uint16_t));

	Map2D::LayerPtr layerBG = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr items = layerBG->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = items->begin(); i != items->end(); i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		bg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	Map2D::LayerPtr layerFG = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr itemsFG = layerFG->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = itemsFG->begin(); i != itemsFG->end(); i++
	) {
		assert(((*i)->x < mapWidth) && ((*i)->y < mapHeight));
		fg[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	output << u16le(mapWidth);

	assert(mapWidth * mapHeight < DN2_NUM_TILES_BG);
	for (unsigned int i = 0; i < DN2_NUM_TILES_BG; i++) {
		if (fg[i] == (uint16_t)-1) {
			// BG tile only
			output << u16le(bg[i] * 8);
		} else if (bg[i] == 0x00) {
			// FG tile only
			output << u16le((fg[i] * 5 + DN2_NUM_SOLID_TILES) * 8);
		} else {
			// BG and FG tile
			uint16_t code = 0x8000 | bg[i] | ((fg[i] & 0x1F) << 10);
			if (fg[i] & 0x60) {
				// Need to save these extra bits
				extra[i] = fg[i] & 0x60;
			}
			output << u16le(code);
		}
	}

	std::vector<int> rawExtra;
	for (unsigned int i = 0; i < DN2_NUM_TILES_BG / 4; i++) {
		rawExtra.push_back(
			  (extra[i * 4 + 0] >> 5)
			| (extra[i * 4 + 1] >> 3)
			| (extra[i * 4 + 2] >> 1)
			| (extra[i * 4 + 3] << 1)
		);
	}

	std::vector<int> rleExtra;
	std::vector<int> diffCount;
	std::vector<int>::iterator i = rawExtra.begin();
	assert(i != rawExtra.end());
	int lastByte = *i;
	int lastByteCount = 1;
	i++;
	for (; i != rawExtra.end(); i++) {
		if (lastByte == *i) {
			// Write out the different bytes so this byte will get written as at
			// least a duplicate
			while (!diffCount.empty()) {
				std::vector<int>::iterator end;
				int len;
				if (diffCount.size() > 0x7F) { // 0x80 length freezes the game
					len = 0x7F;
					end = diffCount.begin() + 0x7F;
				} else {
					len = diffCount.size();
					end = diffCount.end();
				}
				rleExtra.push_back(0x100 - len);
				rleExtra.insert(rleExtra.end(), diffCount.begin(), end);
				diffCount.erase(diffCount.begin(), end);
			}
			assert(diffCount.empty());
			lastByteCount++;
		} else {
			// This byte is different to the last
			if (lastByteCount > 1) {
				while (lastByteCount > 0) {
					int amt = std::min(0x7F, lastByteCount);
					rleExtra.push_back(amt);
					rleExtra.push_back(lastByte);
					lastByteCount -= amt;
				}
				// proceed with this new, different byte becoming lastByte
			} else {
				diffCount.push_back(lastByte);
			}
			lastByte = *i;
			lastByteCount = 1;
		}
	}
	while (!diffCount.empty()) {
		std::vector<int>::iterator end;
		int len;
		if (diffCount.size() > 0x80) {
			len = 0x80;
			end = diffCount.begin() + 0x80;
		} else {
			len = diffCount.size();
			end = diffCount.end();
		}
		rleExtra.push_back(0x100 - len);
		rleExtra.insert(rleExtra.end(), diffCount.begin(), end);
		diffCount.erase(diffCount.begin(), end);
	}
	if ((lastByte != 0x00) && (lastByteCount > 0)) {
		// Need to write out trailing repeated data
		while (lastByteCount > 0) {
			int amt = std::min(0x7F, lastByteCount);
			rleExtra.push_back(amt);
			rleExtra.push_back(lastByte);
			lastByteCount -= amt;
		}
	}
	// Last two bytes are always 0x00
	rleExtra.push_back(0x00);
	rleExtra.push_back(0x00);

	output << u16le(rleExtra.size());
	for (std::vector<int>::iterator i = rleExtra.begin(); i != rleExtra.end(); i++) {
		output << u8(*i);
	}

	// Zone attribute filename (null-padded, not space-padded)
	Map::Attribute& attr8 = map->attributes[ATTR_ZONEATTR];
	output << nullPadded(attr8.filenameValue, 13);

	// Zone solid tileset filename (null-padded, not space-padded)
	Map::Attribute& attr9 = map->attributes[ATTR_ZONETSET];
	output << nullPadded(attr9.filenameValue, 13);

	// Zone masked tileset filename (null-padded, not space-padded)
	Map::Attribute& attr10 = map->attributes[ATTR_ZONEMSET];
	output << nullPadded(attr10.filenameValue, 13);

	output->flush();
	return;
}

SuppFilenames MapType_Nukem2::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}

} // namespace gamemaps
} // namespace camoto
