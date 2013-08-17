/**
 * @file   fmt-map-wordresc.cpp
 * @brief  MapType and Map2D implementation for Word Rescue levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Word_Rescue
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
#include <camoto/util.hpp>
#include "fmt-map-wordresc.hpp"

/// Width of tiles in background layer
#define WR_BGTILE_WIDTH           16
/// Height of tiles in background layer
#define WR_BGTILE_HEIGHT          16

/// Width of tiles in attribute layer
#define WR_ATTILE_WIDTH            8
/// Height of tiles in attribute layer
#define WR_ATTILE_HEIGHT           8

/// Map code to write for locations with no tile set.
#define WR_DEFAULT_BGTILE       0xFF

/// Map code to write for locations with no tile set.
#define WR_DEFAULT_ATTILE       0x20

/// This is the largest valid tile code in the background layer.
#define WR_MAX_VALID_TILECODE    240

/// Height of the door image, in pixels (to align it with the floor)
#define DOOR_HEIGHT               40

// Internal codes for various items
#define WR_CODE_GRUZZLE  1
#define WR_CODE_SLIME    2
#define WR_CODE_BOOK     3
#define WR_CODE_DRIP     4
#define WR_CODE_ANIM     5
#define WR_CODE_FG       6
#define WR_CODE_LETTER   7
#define WR_CODE_LETTER1  7 // same as WR_CODE_LETTER
#define WR_CODE_LETTER2  8
#define WR_CODE_LETTER3  9
#define WR_CODE_LETTER4  10
#define WR_CODE_LETTER5  11
#define WR_CODE_LETTER6  12
#define WR_CODE_LETTER7  13

#define WR_CODE_ENTRANCE 0x1001
#define WR_CODE_EXIT     0x1002

/// Fixed number of letters in each map (to spell a word)
#define WR_NUM_LETTERS   7

// Values used when writing items (also in isinstance)
#define INDEX_GRUZZLE 0
#define INDEX_DRIP    1
#define INDEX_SLIME   2
#define INDEX_BOOK    3
#define INDEX_LETTER  4
#define INDEX_ANIM    5
#define INDEX_FG      6
#define INDEX_SIZE    7

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;


class WordRescueBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		WordRescueBackgroundLayer(ItemPtrVectorPtr& items,
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

		virtual gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset) const
		{
			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (item->code >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[item->code]);
		}
};

class WordRescueObjectLayer: virtual public GenericMap2D::Layer
{
	public:
		WordRescueObjectLayer(const std::string& name, unsigned int caps,
			unsigned int w, unsigned int h, ItemPtrVectorPtr& items,
			ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					name,
					caps,
					0, 0,
					w, h,
					items, validItems
				)
		{
		}

		virtual gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset) const
		{
			ImagePurpose purpose;
			unsigned int index;
			switch (item->code) {
				case WR_CODE_GRUZZLE:  purpose = SpriteTileset1;     index = 15; break;
				case WR_CODE_SLIME:    purpose = BackgroundTileset1; index = 238; break;
				case WR_CODE_BOOK:     purpose = BackgroundTileset1; index = 239; break;
				case WR_CODE_DRIP:     purpose = BackgroundTileset1; index = 238; break;
				case WR_CODE_LETTER1:
				case WR_CODE_LETTER2:
				case WR_CODE_LETTER3:
				case WR_CODE_LETTER4:
				case WR_CODE_LETTER5:
				case WR_CODE_LETTER6:
				case WR_CODE_LETTER7:
					purpose = ForegroundTileset1;
					index = item->code - WR_CODE_LETTER;
					break;
				case WR_CODE_ENTRANCE: purpose = SpriteTileset1; index = 1; break;
				case WR_CODE_EXIT:     purpose = SpriteTileset1; index = 3; break;

				case WR_CODE_ANIM: // fall through (no image)
				case WR_CODE_FG:
				default: return ImagePtr();
			}

			TilesetCollection::const_iterator t = tileset->find(purpose);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[index]);
		}

		virtual bool tilePermittedAt(const Map2D::Layer::ItemPtr& item,
			unsigned int x, unsigned int y, unsigned int *maxCount)
		{
			if ((item->code == WR_CODE_ENTRANCE) || (item->code == WR_CODE_EXIT)) {
				*maxCount = 1; // only one level entrance/exit permitted
			} else {
				*maxCount = 0; // unlimited
			}
			return true; // anything can be placed anywhere
		}
};

class WordRescueAttributeLayer: virtual public GenericMap2D::Layer
{
	public:
		WordRescueAttributeLayer(ItemPtrVectorPtr& items,
			ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Attributes",
					Map2D::Layer::HasOwnTileSize,
					0, 0,
					WR_ATTILE_WIDTH, WR_ATTILE_HEIGHT,
					items, validItems
				)
		{
		}

		virtual gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset) const
		{
			ImagePurpose purpose;
			unsigned int index;
			switch (item->code) {
				case 0x0000: purpose = SpriteTileset1; index = 0; break; // first question mark box
				case 0x0001: purpose = SpriteTileset1; index = 0; break;
				case 0x0002: purpose = SpriteTileset1; index = 0; break;
				case 0x0003: purpose = SpriteTileset1; index = 0; break;
				case 0x0004: purpose = SpriteTileset1; index = 0; break;
				case 0x0005: purpose = SpriteTileset1; index = 0; break;
				case 0x0006: purpose = SpriteTileset1; index = 0; break; // last question mark box
				case 0x0073: return ImagePtr(); // solid
				case 0x0074: return ImagePtr(); // jump up through/climb
				case 0x00FD: return ImagePtr(); // what is this? end of layer flag?
				default: return ImagePtr();
			}
			TilesetCollection::const_iterator t = tileset->find(purpose);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[index]);
		}

		virtual bool tilePermittedAt(const Map2D::Layer::ItemPtr& item,
			unsigned int x, unsigned int y, unsigned int *maxCount)
		{
			if (x == 0) return false; // can't place tiles in this column
			return true; // otherwise unrestricted
		}
};

Map::GraphicsFilenamesPtr wr_getGraphicsFilenames(const Map *map)
{
	Map::AttributePtrVectorPtr attributes = map->getAttributes();
	assert(attributes); // this map format always has attributes
	assert(attributes->size() == 3);

	Map::GraphicsFilenamesPtr files(new Map::GraphicsFilenames);
	Map::GraphicsFilename gf;
	gf.type = "tls-wordresc";
	gf.filename = createString("back" << (int)(attributes->at(1)->enumValue + 1)
		<< ".wr");
	if (!gf.filename.empty()) (*files)[BackgroundTileset1] = gf;

	unsigned int dropNum = attributes->at(2)->enumValue;
	if (dropNum > 0) {
		gf.filename = createString("drop" << dropNum << ".wr");
		if (!gf.filename.empty()) (*files)[BackgroundImage] = gf;
	}

	return files;
}


/// Write the given data to the stream, RLE encoded
int rleWrite(stream::output_sptr output, uint8_t *data, int len)
{
	int lenWritten = 0;

	// RLE encode the data
	uint8_t lastCount = 0;
	uint8_t lastCode = data[0];
	for (int i = 0; i < len; i++) {
		if (data[i] == lastCode) {
			if (lastCount == 0xFF) {
				output
					<< u8(lastCount)
					<< u8(lastCode)
				;
				lenWritten += 2;
				lastCount = 1;
			} else {
				lastCount++;
			}
		} else {
			output
				<< u8(lastCount)
				<< u8(lastCode)
			;
			lenWritten += 2;
			lastCode = data[i];
			lastCount = 1;
		}
	}
	// Write out the last tile
	if (lastCount > 0) {
		output
			<< u8(lastCount)
			<< u8(lastCode)
		;
		lenWritten += 2;
	}

	return lenWritten;
}



std::string WordRescueMapType::getMapCode() const
{
	return "map-wordresc";
}

std::string WordRescueMapType::getFriendlyName() const
{
	return "Word Rescue level";
}

std::vector<std::string> WordRescueMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("s0");
	vcExtensions.push_back("s1");
	vcExtensions.push_back("s2");
	vcExtensions.push_back("s3");
	vcExtensions.push_back("s4");
	vcExtensions.push_back("s5");
	vcExtensions.push_back("s6");
	vcExtensions.push_back("s7");
	vcExtensions.push_back("s8");
	vcExtensions.push_back("s9");
	vcExtensions.push_back("s10");
	vcExtensions.push_back("s11");
	vcExtensions.push_back("s12");
	vcExtensions.push_back("s13");
	vcExtensions.push_back("s14");
	vcExtensions.push_back("s15");
	vcExtensions.push_back("s16");
	vcExtensions.push_back("s17");
	vcExtensions.push_back("s18");
	vcExtensions.push_back("s19");
	return vcExtensions;
}

std::vector<std::string> WordRescueMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Word Rescue");
	return vcGames;
}

MapType::Certainty WordRescueMapType::isInstance(stream::input_sptr psMap) const
{
	stream::pos lenMap = psMap->size();

#define WR_MIN_HEADER_SIZE (2*15 + 4*7) // includes INDEX_LETTER

	// Make sure file is large enough for the header
	// TESTED BY: fmt_map_wordresc_isinstance_c01
	if (lenMap < WR_MIN_HEADER_SIZE) return MapType::DefinitelyNo;

	uint16_t mapWidth, mapHeight;
	psMap->seekg(0, stream::start);
	psMap
		>> u16le(mapWidth)
		>> u16le(mapHeight)
	;
	psMap->seekg(2*7, stream::cur);

	// Check the items are each within range
	unsigned int minSize = WR_MIN_HEADER_SIZE;
	for (unsigned int i = 0; i < INDEX_SIZE; i++) {
		stream::len lenBlock;
		if (i == INDEX_LETTER) {
			lenBlock = WR_NUM_LETTERS * 4;
		} else {
			uint16_t count;
			psMap >> u16le(count);
			lenBlock = count * 4;
			if (i == INDEX_DRIP) {
				// Extra byte for each of these
				lenBlock += count * 2;
			}
		}

		minSize += lenBlock;
		// Don't need to count the u16le in minSize, it's in WR_MIN_HEADER_SIZE

		// Make sure the item count is within range
		// TESTED BY: fmt_map_wordresc_isinstance_c02
		if (lenMap < minSize) return MapType::DefinitelyNo;
		psMap->seekg(lenBlock, stream::cur);
	}

	// Read in the layer and make sure all the tile codes are within range
	for (int i = 0; i < mapWidth * mapHeight; ) {
		uint8_t num, code;
		minSize += 2;
		// Make sure the background layer isn't cut off
		// TESTED BY: fmt_map_wordresc_isinstance_c03
		if (lenMap < minSize) return MapType::DefinitelyNo;

		psMap >> u8(num) >> u8(code);
		i += num;

		// Ignore the default tile (otherwise it would be out of range)
		if (code == WR_DEFAULT_BGTILE) continue;

		// Make sure the tile values are within range
		// TESTED BY: fmt_map_wordresc_isinstance_c04
		if (code > WR_MAX_VALID_TILECODE) return MapType::DefinitelyNo;
	}

	// TESTED BY: fmt_map_wordresc_isinstance_c00
	return MapType::DefinitelyYes;
}

MapPtr WordRescueMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr WordRescueMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	input->seekg(0, stream::start);

	uint16_t mapWidth, mapHeight;
	uint16_t bgColour; // EGA 0-15
	uint16_t tileset; // 3 == suburban, 2 == medieval (backX.wr)
	uint16_t backdrop; // dropX.wr, 0 == none
	uint16_t startX, startY, endX, endY;
	input
		>> u16le(mapWidth)
		>> u16le(mapHeight)
		>> u16le(bgColour)
		>> u16le(tileset)
		>> u16le(backdrop)
		>> u16le(startX)
		>> u16le(startY)
		>> u16le(endX)
		>> u16le(endY)
	;

	Map::AttributePtrVectorPtr attributes(new Map::AttributePtrVector());
	Map::AttributePtr attrBGColour(new Map::Attribute);
	attrBGColour->type = Map::Attribute::Enum;
	attrBGColour->name = "Background colour";
	attrBGColour->desc = "Colour to draw where there are no tiles.  Only used if "
		"backdrop is not set.";
	attrBGColour->enumValue = bgColour;
	attrBGColour->enumValueNames.push_back("EGA 0 - Black");
	attrBGColour->enumValueNames.push_back("EGA 1 - Dark blue");
	attrBGColour->enumValueNames.push_back("EGA 2 - Dark green");
	attrBGColour->enumValueNames.push_back("EGA 3 - Dark cyan");
	attrBGColour->enumValueNames.push_back("EGA 4 - Dark red");
	attrBGColour->enumValueNames.push_back("EGA 5 - Dark magenta");
	attrBGColour->enumValueNames.push_back("EGA 6 - Brown");
	attrBGColour->enumValueNames.push_back("EGA 7 - Light grey");
	attrBGColour->enumValueNames.push_back("EGA 8 - Dark grey");
	attrBGColour->enumValueNames.push_back("EGA 9 - Light blue");
	attrBGColour->enumValueNames.push_back("EGA 10 - Light green");
	attrBGColour->enumValueNames.push_back("EGA 11 - Light cyan");
	attrBGColour->enumValueNames.push_back("EGA 12 - Light red");
	attrBGColour->enumValueNames.push_back("EGA 13 - Light magenta");
	attrBGColour->enumValueNames.push_back("EGA 14 - Yellow");
	attrBGColour->enumValueNames.push_back("EGA 15 - White");
	attributes->push_back(attrBGColour);

	Map::AttributePtr attrTileset(new Map::Attribute);
	attrTileset->type = Map::Attribute::Enum;
	attrTileset->name = "Tileset";
	attrTileset->desc = "Tileset to use for this map.";
	if (tileset > 0) tileset--; // just in case it *is* ever zero
	attrTileset->enumValue = tileset;
	attrTileset->enumValueNames.push_back("Desert");
	attrTileset->enumValueNames.push_back("Castle");
	attrTileset->enumValueNames.push_back("Suburban");
	attrTileset->enumValueNames.push_back("Spooky (episode 3 only)");
	attrTileset->enumValueNames.push_back("Industrial");
	attrTileset->enumValueNames.push_back("Custom (back6.wr)");
	attrTileset->enumValueNames.push_back("Custom (back7.wr)");
	attrTileset->enumValueNames.push_back("Custom (back8.wr)");
	attributes->push_back(attrTileset);

	Map::AttributePtr attrBackdrop(new Map::Attribute);
	attrBackdrop->type = Map::Attribute::Enum;
	attrBackdrop->name = "Backdrop";
	attrBackdrop->desc = "Image to show behind map (overrides background colour.)";
	attrBackdrop->enumValue = backdrop;
	attrBackdrop->enumValueNames.push_back("None (use background colour)");
	attrBackdrop->enumValueNames.push_back("Custom (drop1.wr)");
	attrBackdrop->enumValueNames.push_back("Cave (episodes 2-3 only)");
	attrBackdrop->enumValueNames.push_back("Desert");
	attrBackdrop->enumValueNames.push_back("Mountain");
	attrBackdrop->enumValueNames.push_back("Custom (drop5.wr)");
	attrBackdrop->enumValueNames.push_back("Custom (drop6.wr)");
	attrBackdrop->enumValueNames.push_back("Custom (drop7.wr)");
	attributes->push_back(attrBackdrop);

	Map2D::Layer::ItemPtrVectorPtr items8(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr items16(new Map2D::Layer::ItemPtrVector());

	uint16_t gruzzleCount;
	input >> u16le(gruzzleCount);
	for (unsigned int i = 0; i < gruzzleCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_GRUZZLE;
		items8->push_back(t);
	}

	uint16_t dripCount;
	input >> u16le(dripCount);
	for (unsigned int i = 0; i < dripCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Movement;
		t->movementFlags = Map2D::Layer::Item::DistanceLimit;
		t->movementDistLeft = 0;
		t->movementDistRight = 0;
		t->movementDistUp = 0;
		t->movementDistDown = Map2D::Layer::Item::DistIndeterminate;
		input
			>> u16le(t->x)
			>> u16le(t->y)
			>> u16le(t->code) // discard drip freq (nowhere to save it yet)
		;
		t->code = WR_CODE_DRIP;
		items8->push_back(t);
	}

	uint16_t slimeCount;
	input >> u16le(slimeCount);
	for (unsigned int i = 0; i < slimeCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_SLIME;
		items16->push_back(t);
	}

	uint16_t bookCount;
	input >> u16le(bookCount);
	for (unsigned int i = 0; i < bookCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_BOOK;
		items16->push_back(t);
	}

	for (unsigned int i = 0; i < WR_NUM_LETTERS; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_LETTER + i;
		items16->push_back(t);
	}

	uint16_t animCount;
	input >> u16le(animCount);
	for (unsigned int i = 0; i < animCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_ANIM;
		items16->push_back(t);
	}

	uint16_t fgCount;
	input >> u16le(fgCount);
	for (unsigned int i = 0; i < fgCount; i++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		input
			>> u16le(t->x)
			>> u16le(t->y)
		;
		t->code = WR_CODE_FG;
		items16->push_back(t);
	}

	// Add the map entrance and exit as special items
	{
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Player;
		t->x = startX;
		t->y = startY;
		t->playerNumber = 0;
		t->code = WR_CODE_ENTRANCE;
		items8->push_back(t);
	}
	{
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = endX;
		t->y = endY;
		t->code = WR_CODE_EXIT;
		items8->push_back(t);
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validItem8Items(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr validItem16Items(new Map2D::Layer::ItemPtrVector());
#define ADD_TILE(n, c)	  \
	{ \
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item()); \
		t->type = Map2D::Layer::Item::Default; \
		t->x = 0; \
		t->y = 0; \
		t->code = c; \
		validItem ## n ## Items->push_back(t); \
	}
	ADD_TILE(8, WR_CODE_GRUZZLE);
	ADD_TILE(16, WR_CODE_SLIME);
	ADD_TILE(16, WR_CODE_BOOK);
	ADD_TILE(16, WR_CODE_LETTER1);
	ADD_TILE(16, WR_CODE_LETTER2);
	ADD_TILE(16, WR_CODE_LETTER3);
	ADD_TILE(16, WR_CODE_LETTER4);
	ADD_TILE(16, WR_CODE_LETTER5);
	ADD_TILE(16, WR_CODE_LETTER6);
	ADD_TILE(16, WR_CODE_LETTER7);
	ADD_TILE(16, WR_CODE_ANIM);
	ADD_TILE(16, WR_CODE_FG);
	ADD_TILE(8, WR_CODE_ENTRANCE);
	ADD_TILE(8, WR_CODE_EXIT);
#undef ADD_TILE
	{
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Movement;
		t->movementFlags = Map2D::Layer::Item::DistanceLimit;
		t->movementDistLeft = 0;
		t->movementDistRight = 0;
		t->movementDistUp = 0;
		t->movementDistDown = Map2D::Layer::Item::DistIndeterminate;
		t->x = 0;
		t->y = 0;
		t->code = WR_CODE_DRIP;
		validItem8Items->push_back(t);
	}

	Map2D::LayerPtr item8Layer(new WordRescueObjectLayer(
		"Fine items", Map2D::Layer::HasOwnTileSize, 8, 8, items8, validItem8Items));
	Map2D::LayerPtr item16Layer(new WordRescueObjectLayer(
		"Coarse items", Map2D::Layer::NoCaps, 0, 0, items16, validItem16Items));

	// Read the background layer
	Map2D::Layer::ItemPtrVectorPtr tiles(new Map2D::Layer::ItemPtrVector());
	tiles->reserve(mapWidth * mapHeight);
	for (int i = 0; i < mapWidth * mapHeight; ) {
		uint8_t num, code;
		input >> u8(num) >> u8(code);
		if (code == WR_DEFAULT_BGTILE) {
			i += num;
		} else {
			while (num-- > 0) {
				Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
				t->type = Map2D::Layer::Item::Default;
				t->x = i % mapWidth;
				t->y = i / mapWidth;
				t->code = code;
				tiles->push_back(t);
				i++;
			}
		}
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= WR_MAX_VALID_TILECODE; i++) {
		// The default tile actually has an image, so don't exclude it
		if (i == WR_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}

	Map2D::LayerPtr bgLayer(new WordRescueBackgroundLayer(tiles, validBGItems));

	// Read the attribute layer
	Map2D::Layer::ItemPtrVectorPtr atItems(new Map2D::Layer::ItemPtrVector());
	uint16_t atWidth = mapWidth * 2;
	uint16_t atHeight = mapHeight * 2;
	atItems->reserve(atWidth * atHeight);
	for (int i = 0; i < atWidth * atHeight; ) {
		uint8_t num, code;
		try {
			input >> u8(num) >> u8(code);
		} catch (const stream::incomplete_read& e) {
			// Some level files seem to be truncated (maybe for efficiency)
			break;
		}
		if (code == WR_DEFAULT_ATTILE) {
			i += num;
		} else {
			while (num-- > 0) {
				Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
				t->x = i % atWidth + 1;
				t->y = i / atWidth;
				t->code = code;
				switch (code) {
					case 0x73:
						t->type = Map2D::Layer::Item::Blocking;
						t->blockingFlags =
							Map2D::Layer::Item::BlockLeft
							| Map2D::Layer::Item::BlockRight
							| Map2D::Layer::Item::BlockTop
							| Map2D::Layer::Item::BlockBottom
						;
						break;
					case 0x74:
						t->type = Map2D::Layer::Item::Blocking;
						t->blockingFlags =
							Map2D::Layer::Item::BlockTop
							| Map2D::Layer::Item::JumpDown
						;
						break;
					default:
						t->type = Map2D::Layer::Item::Default;
						break;
				}
				atItems->push_back(t);
				i++;
			}
		}
	}

	// Populate the list of permitted tiles
	Map2D::Layer::ItemPtrVectorPtr validAtItems(new Map2D::Layer::ItemPtrVector());

#define ADD_TILE(ty, c, bf) \
	{ \
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item()); \
		t->type = ty; \
		t->x = 0; \
		t->y = 0; \
		t->code = c; \
		t->blockingFlags = bf; \
		validAtItems->push_back(t); \
	}

	ADD_TILE(Map2D::Layer::Item::Blocking, 0x73, Map2D::Layer::Item::BlockLeft
		| Map2D::Layer::Item::BlockRight
		| Map2D::Layer::Item::BlockTop
		| Map2D::Layer::Item::BlockBottom);

	ADD_TILE(Map2D::Layer::Item::Blocking, 0x74, Map2D::Layer::Item::BlockTop
		| Map2D::Layer::Item::JumpDown);

	for (int i = 0; i < 7; i++) {
		ADD_TILE(Map2D::Layer::Item::Default, 0x0000 + i, 0); // question mark box
	}

	ADD_TILE(Map2D::Layer::Item::Default, 0x00FD, 0); // unknown (see tile mapping code)
#undef ADD_TILE

	Map2D::LayerPtr atLayer(new WordRescueAttributeLayer(atItems, validAtItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(atLayer);
	layers.push_back(item8Layer);
	layers.push_back(item16Layer);

	Map2DPtr map(new GenericMap2D(
		attributes, wr_getGraphicsFilenames,
		Map2D::HasViewport,
		288, 152, // viewport
		mapWidth, mapHeight,
		WR_BGTILE_WIDTH, WR_BGTILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void WordRescueMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 4)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	Map::AttributePtrVectorPtr attributes = map->getAttributes();
	if (attributes->size() != 3) {
		throw stream::error("Cannot write map as there is an incorrect number "
			"of attributes set.");
	}

	Map::Attribute *attrBG = attributes->at(0).get();
	if (attrBG->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (bg != enum)");
	}
	uint16_t bgColour = attrBG->enumValue;

	Map::Attribute *attrTileset = attributes->at(1).get();
	if (attrTileset->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (tileset != enum)");
	}
	uint16_t tileset = attrTileset->enumValue + 1;

	Map::Attribute *attrBackdrop = attributes->at(2).get();
	if (attrBackdrop->type != Map::Attribute::Enum) {
		throw stream::error("Cannot write map as there is an attribute of the "
			"wrong type (backdrop != enum)");
	}
	uint16_t backdrop = attrBackdrop->enumValue;

	typedef std::pair<uint16_t, uint16_t> point;
	std::vector<point> itemLocations[INDEX_SIZE];

	// Prefill the letter vector with the fixed number of letters
	for (unsigned int i = 0; i < WR_NUM_LETTERS; i++) itemLocations[INDEX_LETTER].push_back(point(0, 0));

	uint16_t startX = 0;
	uint16_t startY = 0;
	uint16_t endX = 0;
	uint16_t endY = 0;

	Map2D::LayerPtr layer = map2d->getLayer(2);
	const Map2D::Layer::ItemPtrVectorPtr items8 = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = items8->begin(); i != items8->end(); i++
	) {
		switch ((*i)->code) {
			case WR_CODE_GRUZZLE: itemLocations[INDEX_GRUZZLE].push_back(point((*i)->x, (*i)->y)); break;
			case WR_CODE_DRIP:    itemLocations[INDEX_DRIP].push_back(point((*i)->x, (*i)->y)); break;
			case WR_CODE_ENTRANCE:
				startX = (*i)->x;
				startY = (*i)->y;
				break;
			case WR_CODE_EXIT:
				endX = (*i)->x;
				endY = (*i)->y;
				break;
		}
	}

	layer = map2d->getLayer(3);
	const Map2D::Layer::ItemPtrVectorPtr items16 = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator
		i = items16->begin(); i != items16->end(); i++
	) {
		switch ((*i)->code) {
			case WR_CODE_SLIME:   itemLocations[INDEX_SLIME].push_back(point((*i)->x, (*i)->y)); break;
			case WR_CODE_BOOK:    itemLocations[INDEX_BOOK].push_back(point((*i)->x, (*i)->y)); break;
			case WR_CODE_LETTER1: itemLocations[INDEX_LETTER][0] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER2: itemLocations[INDEX_LETTER][1] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER3: itemLocations[INDEX_LETTER][2] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER4: itemLocations[INDEX_LETTER][3] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER5: itemLocations[INDEX_LETTER][4] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER6: itemLocations[INDEX_LETTER][5] = point((*i)->x, (*i)->y); break;
			case WR_CODE_LETTER7: itemLocations[INDEX_LETTER][6] = point((*i)->x, (*i)->y); break;
			case WR_CODE_ANIM:    itemLocations[INDEX_ANIM].push_back(point((*i)->x, (*i)->y)); break;
			case WR_CODE_FG:      itemLocations[INDEX_FG].push_back(point((*i)->x, (*i)->y)); break;
		}
	}

	output
		<< u16le(mapWidth)
		<< u16le(mapHeight)
		<< u16le(bgColour)
		<< u16le(tileset)
		<< u16le(backdrop)
		<< u16le(startX)
		<< u16le(startY)
		<< u16le(endX)
		<< u16le(endY)
	;

	// Write out all the gruzzles, slime buckets and book positions
	for (unsigned int i = 0; i < INDEX_SIZE; i++) {

		// Write the number of items first, except for letters which are fixed at 7
		if (i != INDEX_LETTER) {
			uint16_t len = itemLocations[i].size();
			output << u16le(len);
		}

		// Write the X and Y coordinates for each item
		for (std::vector<point>::const_iterator j = itemLocations[i].begin();
			j != itemLocations[i].end(); j++
		) {
			output
				<< u16le(j->first)
				<< u16le(j->second)
			;
			if (i == INDEX_DRIP) {
				// Add an extra value for the drip frequency
				output << u16le(0x44); // continuous dripping
			}
		}
	}

	// Write the background layer
	unsigned long lenTiles = mapWidth * mapHeight;
	uint8_t *tiles = new uint8_t[lenTiles];
	boost::scoped_array<uint8_t> stiles(tiles);
	// Set the default background tile
	memset(tiles, WR_DEFAULT_BGTILE, lenTiles);
	layer = map2d->getLayer(0);
	const Map2D::Layer::ItemPtrVectorPtr bgitems = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = bgitems->begin();
		i != bgitems->end();
		i++
	) {
		if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
			throw stream::error(createString("Layer has tiles outside map "
				"boundary at (" << (*i)->x << "," << (*i)->y << ")"));
		}
		tiles[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
	}

	rleWrite(output, tiles, lenTiles);

	// Write the attribute layer
	unsigned long lenAttr = mapWidth * mapHeight * 4;
	uint8_t *attr = new uint8_t[lenAttr];
	boost::scoped_array<uint8_t> sattr(attr);
	// Set the default attribute tile
	memset(attr, WR_DEFAULT_ATTILE, lenAttr);
	layer = map2d->getLayer(1);
	const Map2D::Layer::ItemPtrVectorPtr atitems = layer->getAllItems();
	for (Map2D::Layer::ItemPtrVector::const_iterator i = atitems->begin();
		i != atitems->end();
		i++
	) {
		uint16_t code = (*i)->code;

		if (((*i)->type & Map2D::Layer::Item::Blocking) && (*i)->blockingFlags) {
			if ((*i)->blockingFlags & Map2D::Layer::Item::JumpDown) {
				code = 0x74;
			} else {
				// Probably all Block* flags set
				code = 0x73;
			}
		}

		unsigned int xpos = (*i)->x;
		if (xpos < 1) continue; // skip first column, just in case
		xpos--;
		if ((xpos > mapWidth * 2) || ((*i)->y > mapHeight * 2)) {
			throw stream::error(createString("Layer has tiles outside map "
				"boundary at (" << xpos << "," << (*i)->y << ")"));
		}
		attr[(*i)->y * mapWidth * 2 + xpos] = code;
	}

	rleWrite(output, attr, lenAttr);
	output->flush();
	return;
}

SuppFilenames WordRescueMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	return supps;
}


} // namespace gamemaps
} // namespace camoto
