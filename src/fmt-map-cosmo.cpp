/**
 * @file   fmt-map-cosmo.cpp
 * @brief  MapType and Map2D implementation for Cosmo levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Cosmo_Level_Format
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

/// Number of tiles in the solid tileset
#define CCA_NUM_SOLID_TILES 2000

/// Number of tiles in the masked tileset
#define CCA_NUM_MASKED_TILES 1000

// Indices into attributes array
#define ATTR_BACKDROP 0
#define ATTR_RAIN     1
#define ATTR_SCROLL_X 2
#define ATTR_SCROLL_Y 3
#define ATTR_PAL_ANIM 4
#define ATTR_MUSIC    5

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;


class CosmoActorLayer: virtual public GenericMap2D::Layer
{
	public:
		CosmoActorLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Actors",
					Map2D::Layer::UseImageDims,
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

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();

			unsigned int index = item->code - 31;
			unsigned int num = images.size();
			if (index >= num) return Map2D::Layer::Unknown; // out of range
			while (!(images[index]->getAttr() & Tileset::SubTileset)) {
				// Some images are duplicated, but libgamegraphics reports these as
				// empty tilesets.  So if we encounter an empty one, find the next
				// available actor.
				index++;
				if (index >= num) return Map2D::Layer::Unknown; // out of range
			}
			TilesetPtr actor = t->second->openTileset(images[index]);
			const Tileset::VC_ENTRYPTR& actorFrames = actor->getItems();
			if (actorFrames.size() <= 0) return Map2D::Layer::Unknown; // out of range
			*out = actor->openImage(actorFrames[0]);
			return Map2D::Layer::Supplied;
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

		virtual Map2D::Layer::ImageType imageFromCode(
			const Map2D::Layer::ItemPtr& item, const TilesetCollectionPtr& tileset,
			ImagePtr *out) const
		{
			unsigned int index = item->code >> 3; // divide by 8
			ImagePurpose purpose;
			if (index >= CCA_NUM_SOLID_TILES) {
				index -= CCA_NUM_SOLID_TILES;
				index /= 5;
				/*if (index >= 1000) {
					// out of range!
					return Map2D::Layer::Unknown;
				}*/
				purpose = ForegroundTileset1;
			} else {
				purpose = BackgroundTileset1;
			}
			TilesetCollection::const_iterator t = tileset->find(purpose);
			if (t == tileset->end()) return Map2D::Layer::Unknown; // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return Map2D::Layer::Unknown; // out of range
			*out = t->second->openImage(images[index]);
			return Map2D::Layer::Supplied;
		}
};

class Map2D_Cosmo: virtual public GenericMap2D
{
	public:
		Map2D_Cosmo(const Attributes& attributes, unsigned int width,
			LayerPtrVector& layers)
			:	GenericMap2D(
					attributes, Map::GraphicsFilenames(),
					Map2D::HasViewport,
					CCA_VIEWPORT_WIDTH, CCA_VIEWPORT_HEIGHT,
					width, 32768 / width,
					CCA_TILE_WIDTH, CCA_TILE_HEIGHT,
					layers, Map2D::PathPtrVectorPtr()
				)
		{
			// Populate the graphics filenames
			unsigned int dropNum = this->attributes[ATTR_BACKDROP].enumValue;
			if (dropNum > 0) {
				Map::GraphicsFilename gf;
				gf.type = "img-cosmo-backdrop";
				switch (dropNum) {
					case 0: gf.filename = "bdblank.mni"; break;
					case 1: gf.filename = "bdpipe.mni"; break;
					case 2: gf.filename = "bdredsky.mni"; break;
					case 3: gf.filename = "bdrocktk.mni"; break;
					case 4: gf.filename = "bdjungle.mni"; break;
					case 5: gf.filename = "bdstar.mni"; break;
					case 6: gf.filename = "bdwierd.mni"; break;
					case 7: gf.filename = "bdcave.mni"; break;
					case 8: gf.filename = "bdice.mni"; break;
					case 9: gf.filename = "bdshrum.mni"; break;
					case 10: gf.filename = "bdtechms.mni"; break;
					case 11: gf.filename = "bdnewsky.mni"; break;
					case 12: gf.filename = "bdstar2.mni"; break;
					case 13: gf.filename = "bdstar3.mni"; break;
					case 14: gf.filename = "bdforest.mni"; break;
					case 15: gf.filename = "bdmountn.mni"; break;
					case 16: gf.filename = "bdguts.mni"; break;
					case 17: gf.filename = "bdbrktec.mni"; break;
					case 18: gf.filename = "bdclouds.mni"; break;
					case 19: gf.filename = "bdfutcty.mni"; break;
					case 20: gf.filename = "bdice2.mni"; break;
					case 21: gf.filename = "bdcliff.mni"; break;
					case 22: gf.filename = "bdspooky.mni"; break;
					case 23: gf.filename = "bdcrystl.mni"; break;
					case 24: gf.filename = "bdcircut.mni"; break;
					case 25: gf.filename = "bdcircpc.mni"; break;
					default: gf.filename = "bdblank.mni"; break;
				}
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

	// TESTED BY: fmt_map_cosmo_isinstance_c01/c02
	if (lenMap < 6 + CCA_LAYER_LEN_BG) return MapType::DefinitelyNo; // too short

	uint16_t mapWidth;
	psMap->seekg(2, stream::start);
	psMap >> u16le(mapWidth);

	// TESTED BY: fmt_map_cosmo_isinstance_c03
	if (mapWidth > CCA_MAX_WIDTH) return MapType::DefinitelyNo; // map too wide

	uint16_t numActorInts;
	psMap >> u16le(numActorInts);

	// TESTED BY: fmt_map_cosmo_isinstance_c04
	if (numActorInts > (CCA_MAX_ACTORS * 3)) return MapType::DefinitelyNo; // too many actors

	// TESTED BY: fmt_map_cosmo_isinstance_c05
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

	// Set the attributes
	Map::Attributes attributes;
	Map::Attribute attrBackdrop;
	attrBackdrop.type = Map::Attribute::Enum;
	attrBackdrop.name = "Backdrop";
	attrBackdrop.desc = "Index of backdrop to draw behind level.";
	attrBackdrop.enumValue = flags & 0x1F;
	attrBackdrop.enumValueNames.push_back("0 - Blank (bdblank.mni)");
	attrBackdrop.enumValueNames.push_back("1 - Pipe (bdpipe.mni)");
	attrBackdrop.enumValueNames.push_back("2 - Red Sky (bdredsky.mni)");
	attrBackdrop.enumValueNames.push_back("3 - Rock (bdrocktk.mni)");
	attrBackdrop.enumValueNames.push_back("4 - Jungle (bdjungle.mni)");
	attrBackdrop.enumValueNames.push_back("5 - Star (bdstar.mni)");
	attrBackdrop.enumValueNames.push_back("6 - Weird (bdwierd.mni)");
	attrBackdrop.enumValueNames.push_back("7 - Cave (bdcave.mni)");
	attrBackdrop.enumValueNames.push_back("8 - Ice (bdice.mni)");
	attrBackdrop.enumValueNames.push_back("9 - Shrum (bdshrum.mni)");
	attrBackdrop.enumValueNames.push_back("10 - Tech (bdtechms.mni)");
	attrBackdrop.enumValueNames.push_back("11 - New sky (bdnewsky.mni)");
	attrBackdrop.enumValueNames.push_back("12 - Star 2 (bdstar2.mni)");
	attrBackdrop.enumValueNames.push_back("13 - Star 3 (bdstar3.mni)");
	attrBackdrop.enumValueNames.push_back("14 - Forest (bdforest.mni)");
	attrBackdrop.enumValueNames.push_back("15 - Mountain (bdmountn.mni)");
	attrBackdrop.enumValueNames.push_back("16 - Guts (bdguts.mni)");
	attrBackdrop.enumValueNames.push_back("17 - Broken Tech (bdbrktec.mni)");
	attrBackdrop.enumValueNames.push_back("18 - Clouds (bdclouds.mni)");
	attrBackdrop.enumValueNames.push_back("19 - Future city (bdfutcty.mni)");
	attrBackdrop.enumValueNames.push_back("20 - Ice 2 (bdice2.mni)");
	attrBackdrop.enumValueNames.push_back("21 - Cliff (bdcliff.mni)");
	attrBackdrop.enumValueNames.push_back("22 - Spooky (bdspooky.mni)");
	attrBackdrop.enumValueNames.push_back("23 - Crystal (bdcrystl.mni)");
	attrBackdrop.enumValueNames.push_back("24 - Circuit (bdcircut.mni)");
	attrBackdrop.enumValueNames.push_back("25 - Circuit PC (bdcircpc.mni)");
	assert(attributes.size() == ATTR_BACKDROP); // make sure compile-time index is correct
	attributes.push_back(attrBackdrop);

	Map::Attribute attrRain;
	attrRain.type = Map::Attribute::Enum;
	attrRain.name = "Rain";
	attrRain.desc = "Is it raining in this level?";
	attrRain.enumValue = (flags >> 5) & 1;
	attrRain.enumValueNames.push_back("No");
	attrRain.enumValueNames.push_back("Yes");
	assert(attributes.size() == ATTR_RAIN); // make sure compile-time index is correct
	attributes.push_back(attrRain);

	Map::Attribute attrScrollX;
	attrScrollX.type = Map::Attribute::Enum;
	attrScrollX.name = "Scroll X";
	attrScrollX.desc = "Should the backdrop scroll horizontally?";
	attrScrollX.enumValue = (flags >> 6) & 1;
	attrScrollX.enumValueNames.push_back("No");
	attrScrollX.enumValueNames.push_back("Yes");
	assert(attributes.size() == ATTR_SCROLL_X); // make sure compile-time index is correct
	attributes.push_back(attrScrollX);

	Map::Attribute attrScrollY;
	attrScrollY.type = Map::Attribute::Enum;
	attrScrollY.name = "Scroll Y";
	attrScrollY.desc = "Should the backdrop scroll vertically?";
	attrScrollY.enumValue = (flags >> 7) & 1;
	attrScrollY.enumValueNames.push_back("No");
	attrScrollY.enumValueNames.push_back("Yes");
	assert(attributes.size() == ATTR_SCROLL_Y); // make sure compile-time index is correct
	attributes.push_back(attrScrollY);

	Map::Attribute attrPalAnim;
	attrPalAnim.type = Map::Attribute::Enum;
	attrPalAnim.name = "Palette animation";
	attrPalAnim.desc = "Type of colour animation to use in this level.  Only "
		"dark magenta (EGA colour 5) is animated.";
	attrPalAnim.enumValue = (flags >> 8) & 7;
	attrPalAnim.enumValueNames.push_back("0 - No animation");
	attrPalAnim.enumValueNames.push_back("1 - Lightning");
	attrPalAnim.enumValueNames.push_back("2 - Cycle: red -> yellow -> white");
	attrPalAnim.enumValueNames.push_back("3 - Cycle: red -> green -> blue");
	attrPalAnim.enumValueNames.push_back("4 - Cycle: black -> grey -> white");
	attrPalAnim.enumValueNames.push_back("5 - Flashing: red -> magenta -> white");
	attrPalAnim.enumValueNames.push_back("6 - Dark magenta -> black, bomb trigger");
	attrPalAnim.enumValueNames.push_back("7 - Unknown/unused");
	assert(attributes.size() == ATTR_PAL_ANIM); // make sure compile-time index is correct
	attributes.push_back(attrPalAnim);

	Map::Attribute attrMusic;
	attrMusic.type = Map::Attribute::Enum;
	attrMusic.name = "Music";
	attrMusic.desc = "Index of the song to play as background music in the level.";
	attrMusic.enumValue = flags >> 11;
	attrMusic.enumValueNames.push_back("0 - Caves (mcaves.mni)");
	attrMusic.enumValueNames.push_back("1 - Scarry (mscarry.mni)");
	attrMusic.enumValueNames.push_back("2 - Boss (mboss.mni)");
	attrMusic.enumValueNames.push_back("3 - Run Away (mrunaway.mni)");
	attrMusic.enumValueNames.push_back("4 - Circus (mcircus.mni)");
	attrMusic.enumValueNames.push_back("5 - Tech World (mtekwrd.mni)");
	attrMusic.enumValueNames.push_back("6 - Easy Level (measylev.mni)");
	attrMusic.enumValueNames.push_back("7 - Rock It (mrockit.mni)");
	attrMusic.enumValueNames.push_back("8 - Happy (mhappy.mni)");
	attrMusic.enumValueNames.push_back("9 - Devo (mdevo.mni)");
	attrMusic.enumValueNames.push_back("10 - Dadoda (mdadoda.mni)");
	attrMusic.enumValueNames.push_back("11 - Bells (mbells.mni)");
	attrMusic.enumValueNames.push_back("12 - Drums (mdrums.mni)");
	attrMusic.enumValueNames.push_back("13 - Banjo (mbanjo.mni)");
	attrMusic.enumValueNames.push_back("14 - Easy 2 (measy2.mni)");
	attrMusic.enumValueNames.push_back("15 - Tech 2 (mteck2.mni)");
	attrMusic.enumValueNames.push_back("16 - Tech 3 (mteck3.mni)");
	attrMusic.enumValueNames.push_back("17 - Tech 4 (mteck4.mni)");
	attrMusic.enumValueNames.push_back("18 - ZZ Top (mzztop.mni)");
	assert(attributes.size() == ATTR_MUSIC); // make sure compile-time index is correct
	attributes.push_back(attrMusic);

	// Read in the actor layer
	unsigned int numActors = numActorInts / 3;
	if (lenMap < numActors * 6) throw stream::error("Map file has been truncated!");
	Map2D::Layer::ItemPtrVectorPtr actors(new Map2D::Layer::ItemPtrVector());
	actors->reserve(numActors);
	for (unsigned int i = 0; i < numActors; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		input
			>> u16le(t->code)
			>> u16le(t->x)
			>> u16le(t->y)
		;
		switch (t->code) {
			case 295: // falling star
				t->type = Map2D::Layer::Item::Movement;
				t->movementFlags = Map2D::Layer::Item::DistanceLimit;
				t->movementDistLeft = 0;
				t->movementDistRight = 0;
				t->movementDistUp = 0;
				t->movementDistDown = Map2D::Layer::Item::DistIndeterminate;
				t->code = 31 + 1; // normal star image
				break;
		}
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

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i < CCA_NUM_SOLID_TILES; i++) {
		// Background tiles first
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i << 3;
		validBGItems->push_back(t);
	}
	for (unsigned int i = 0; i < CCA_NUM_MASKED_TILES; i++) {
		// Then foreground tiles
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = (CCA_NUM_SOLID_TILES + i * 5) << 3;
		validBGItems->push_back(t);
	}

	// Create the map structures
	Map2D::LayerPtr bgLayer(new CosmoBackgroundLayer(tiles, validBGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(actorLayer);

	Map2DPtr map(new Map2D_Cosmo(attributes, mapWidth, layers));

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

	if (map->attributes.size() != 6) {
		throw stream::error("Cannot write map as there is an incorrect number "
			"of attributes set.");
	}
	Map::Attribute *attr;

	// Backdrop attribute
	attr = &map->attributes[ATTR_BACKDROP];
	if (attr->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (backdrop != enum)");
	}
	flags |= attr->enumValue;

	// Rain attribute
	attr = &map->attributes[ATTR_RAIN];
	if (attr->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (rain != enum)");
	}
	flags |= attr->enumValue << 5;

	// Scroll-X attribute
	attr = &map->attributes[ATTR_SCROLL_X];
	if (attr->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (scrollX != enum)");
	}
	flags |= attr->enumValue << 6;

	// Scroll-Y attribute
	attr = &map->attributes[ATTR_SCROLL_Y];
	if (attr->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (scrollY != enum)");
	}
	flags |= attr->enumValue << 7;

	// Palette animation attribute
	attr = &map->attributes[ATTR_PAL_ANIM];
	if (attr->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (palanim != enum)");
	}
	flags |= attr->enumValue << 8;

	// Music attribute
	attr = &map->attributes[ATTR_MUSIC];
	if (attr->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (music != enum)");
	}
	flags |= attr->enumValue << 11;

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
		uint16_t finalCode = (*i)->code;

		// Map any falling actors to the correct code
		if (
			((*i)->type & Map2D::Layer::Item::Movement) &&
			((*i)->movementFlags & Map2D::Layer::Item::DistanceLimit) &&
			((*i)->movementDistDown == Map2D::Layer::Item::DistIndeterminate)
		) {
			switch ((*i)->code) {
				case 31 + 1: finalCode = 295; break; // falling star
			}
		}
		output
			<< u16le(finalCode)
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
