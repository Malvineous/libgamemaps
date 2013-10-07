/**
 * @file   fmt-map-bash.cpp
 * @brief  MapType and Map2D implementation for Monster Bash levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Monster_Bash
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

#include <set>
#include <boost/scoped_array.hpp>
#include <camoto/iostream_helpers.hpp>
#include "map2d-generic.hpp"
#include "fmt-map-bash.hpp"

/// Width of map tiles
#define MB_TILE_WIDTH           16

/// Height of map tiles
#define MB_TILE_HEIGHT          16

/// Width of map view during gameplay, in pixels
#define MB_VIEWPORT_WIDTH      320

/// Height of map view during gameplay, in pixels
#define MB_VIEWPORT_HEIGHT     200

/// Map code to write for locations with no tile set.
#define MB_DEFAULT_BGTILE     0x00

/// Map code to write for locations with no tile set.
#define MB_DEFAULT_FGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define MB_MAX_VALID_BG_TILECODE 0x1FF

/// This is the largest valid tile code in the background layer.
#define MB_MAX_VALID_FG_TILECODE 0xFF

/// Number of fields in the .mif file
#define MB_NUM_ATTRIBUTES        7

/// Index of attribute for .snd file, which never gets its extension removed.
#define MB_ATTR_KEEP_EXT         5

namespace camoto {
namespace gamemaps {

/// List of sprites, used to convert between names and internal map codes.
static const char *spriteFilenames[] = {
	"apogee_logo",
	"arrows",
	"axe",
	"badguy",
	"blank",
	"block",
	"blocky",
	"bomb",
	"bone1",
	"bone2",
	"bone3",
	"border",
	"border2",
	"bounce",
	"box_slide",
	"break_screen",
	"bshelf",
	"cat",
	"chunk",
	"cloud",
	"crack",
	"cracked_block",
	"crawlleft",
	"crawlright",
	"cursor_arm",
	"cursor_xhair",
	"cyc_horn_l",
	"cyc_horn_r",
	"cyclops_l",
	"cyclops_r",
	"demo",
	"devil_l",
	"devil_r",
	"dirt_l",
	"dirt_r",
	"disolve1",
	"disolve2",
	"dog",
	"drag_b",
	"drag_eye",
	"drag_fire",
	"drag_l",
	"end_gargoyle",
	"end_gargoyle2",
	"end_hellfire",
	"end_main",
	"explode",
	"faces",
	"fire_bit",
	"flag",
	"float100",
	"glass",
	"grenade",
	"guage",
	"guy_jump",
	"hand_l",
	"hand_r",
	"hat",
	"heart",
	"horse",
	"i_main_shoot",
	"i_rock",
	"i_splat",
	"i_zombie_bot",
	"i_zombie_hand",
	"i_zombie_top",
	"iman_l",
	"iman_r",
	"intro1",
	"intro2",
	"intro3",
	"jaw_l",
	"jaw_r",
	"knife",
	"knifee",
	"knifee_ud",
	"leaf",
	"lightn",
	"main_broom",
	"main_die",
	"main_exit",
	"main_hat_l",
	"main_hat_r",
	"main_l",
	"main_meter",
	"main_r",
	"main_stars",
	"main_workout",
	"main_workout2",
	"main_workout3",
	"main_workout4",
	"mouse",
	"msm",
	"nemesis",
	"numbers",
	"pellet_h",
	"pfork_l",
	"pfork_r",
	"plank",
	"plank_r",
	"platfall",
	"platform",
	"platform_v",
	"plug",
	"preview",
	"rock",
	"saw",
	"score",
	"score1up",
	"skelet_l",
	"skelet_r",
	"skullw",
	"smoke1",
	"smoke_cloud",
	"snake",
	"spear_plat",
	"splat",
	"swamp2_l",
	"swamp2_r",
	"swamp_l",
	"swamp_r",
	"swamp_scum2_l",
	"swamp_scum2_r",
	"swamp_scum_l",
	"swamp_scum_r",
	"teeth_l",
	"teeth_r",
	"tman_bl",
	"tman_br",
	"tman_ld",
	"tman_lu",
	"tman_rd",
	"tman_ru",
	"tman_tl",
	"tman_tr",
	"vegies",
	"visa",
	"vulture",
	"vulture_l",
	"vulture_r",
	"white",
	"witch_eye",
	"witch_sill",
	"zb_l",
	"zb_r",
	"zbh_l",
	"zbh_r",
	"zhead",
	"zhead_r",
};

static const char *validTypes[] = {
	"tbg",
	"tfg",
	"tbn",
	"sgl",
	"pal",
	"snd",
	"",
};

using namespace camoto::gamegraphics;

class BashForegroundLayer: virtual public GenericMap2D::Layer
{
	public:
		BashForegroundLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Foreground",
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
			ImagePurpose purpose = ((item->code >> 7) & 1) ?
				ForegroundTileset1 : ForegroundTileset2;
			TilesetCollection::const_iterator t = tileset->find(purpose);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			unsigned int index = item->code & 0x7F;
			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[index]);
		}
};

class BashBackgroundLayer: virtual public GenericMap2D::Layer
{
	public:
		BashBackgroundLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
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

			unsigned int index = item->code & 0x1FF;
			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[index]);
		}
};

class BashSpriteLayer: virtual public GenericMap2D::Layer
{
	public:
		BashSpriteLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Sprites",
					Map2D::Layer::HasOwnTileSize | Map2D::Layer::UseImageDims,
					0, 0,
					1, 1,
					items, validItems
				)
		{
		}

		virtual gamegraphics::ImagePtr imageFromCode(
			const Map2D::Layer::ItemPtr& item,
			const TilesetCollectionPtr& tileset) const
		{
			TilesetCollection::const_iterator t = tileset->find(SpriteTileset1);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			if (item->code < 1000000) return ImagePtr(); // unknown sprite filename
			if (item->code - 1000000 > sizeof(spriteFilenames) / sizeof(const char *)) return ImagePtr(); // out of range somehow

			const char *img = spriteFilenames[item->code - 1000000];
			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			for (Tileset::VC_ENTRYPTR::const_iterator
				i = images.begin(); i != images.end(); i++
			) {
				if ((*i)->getName().compare(img) == 0) {
					TilesetPtr ts = t->second->openTileset(*i);
					const Tileset::VC_ENTRYPTR& tiles = ts->getItems();
					if (tiles.size() < 1) return ImagePtr(); // no images - TODO: generic 'unknown' image
					return ts->openImage(tiles[0]);
				}
			}
			std::cerr << "ERROR: Could not find image for Monster Bash sprite \""
				<< img << "\"" << std::endl;
			return ImagePtr();
		}
};

class BashAttributeLayer: virtual public GenericMap2D::Layer
{
	public:
		BashAttributeLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
			:	GenericMap2D::Layer(
					"Attributes",
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
			return ImagePtr(); // no images
		}
};

Map::GraphicsFilenamesPtr bash_getGraphicsFilenames(const Map *map)
{
	Map::AttributePtrVectorPtr attributes = map->getAttributes();
	assert(attributes); // this map format always has attributes
	assert(attributes->size() == MB_NUM_ATTRIBUTES);

	Map::GraphicsFilenamesPtr files(new Map::GraphicsFilenames);
	Map::GraphicsFilename gf;
	gf.type = "tls-bash-bg";
	gf.filename = attributes->at(0)->filenameValue;
	if (!gf.filename.empty()) (*files)[BackgroundTileset1] = gf; // bg tiles

	gf.type = "tls-bash-fg";
	gf.filename = attributes->at(1)->filenameValue;
	if (!gf.filename.empty()) (*files)[ForegroundTileset1] = gf; // fg tiles

	gf.filename = attributes->at(2)->filenameValue;
	if (!gf.filename.empty()) (*files)[ForegroundTileset2] = gf; // bon tiles

	return files;
}


std::string BashMapType::getMapCode() const
{
	return "map-bash";
}

std::string BashMapType::getFriendlyName() const
{
	return "Monster Bash level";
}

std::vector<std::string> BashMapType::getFileExtensions() const
{
	std::vector<std::string> vcExtensions;
	vcExtensions.push_back("mif");
	return vcExtensions;
}

std::vector<std::string> BashMapType::getGameList() const
{
	std::vector<std::string> vcGames;
	vcGames.push_back("Monster Bash");
	return vcGames;
}

MapType::Certainty BashMapType::isInstance(stream::input_sptr psMap) const
{
	bool maybe = false;
	stream::len len = psMap->size();

	// Make sure the file is large enough...
	// TESTED BY: fmt_map_bash_isinstance_c01
	if (len < 187) return MapType::DefinitelyNo;

	// ...but not too large.
	// TESTED BY: fmt_map_bash_isinstance_c02
	if (len > 217) return MapType::DefinitelyNo;

	uint8_t data[218]; // 7*31+1
	memset(data, 0, sizeof(data));
	uint8_t *d = data;
	psMap->seekg(0, stream::start);
	psMap->read(d, len);
	for (int n = 0; n < 7; n++) {
		bool null = false;
		for (int i = 0; i < 31; i++) {
			if (*d == 0) {
				null = true; // encountered the first null
			} else if (null) {
				// If there are chars after the null, it may not be the right format
				// TESTED BY: fmt_map_bash_isinstance_c03
				maybe = true;
			} else if ((*d < 32) || (*d > 127)) {
				// Make sure the filenames contain valid chars only
				// TESTED BY: fmt_map_bash_isinstance_c04
				return MapType::DefinitelyNo; // bad chars
			}
			d++;
		}
		// Make sure each entry has a terminating null
		// TESTED BY: fmt_map_bash_isinstance_c05
		if (!null) return MapType::DefinitelyNo;
	}
	if (maybe) return MapType::PossiblyYes;
	return MapType::DefinitelyYes;
}

MapPtr BashMapType::create(SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

MapPtr BashMapType::open(stream::input_sptr input, SuppData& suppData) const
{
	stream::input_sptr bg = suppData[SuppItem::Layer1];
	stream::input_sptr fg = suppData[SuppItem::Layer2];
	stream::input_sptr spr = suppData[SuppItem::Layer3];
	assert(bg);
	assert(fg);
	assert(spr);

	// Read the map info file
	static const char *attrNames[] = {
		"Background tileset",
		"Foreground tileset",
		"Bonus tileset",
		"Sprite list",
		"Palette",
		"Sound effects",
		"Unknown",
	};
	static const char *attrDesc[] = {
		"Filename of the tileset to use for drawing the map background layer",
		"Filename of the first tileset to use for drawing the map foreground layer",
		"Filename of the second tileset to use for drawing the map foreground layer",
		"Filename of sprite list - where the list of sprites used in this level is "
			"stored.  Don't change this unless you have just renamed the file in the "
			"main .DAT.",
		"EGA palette to use",
		"Filename to load PC speaker sounds from",
		"Unknown",
	};
	input->seekg(0, stream::start);
	Map::AttributePtrVectorPtr attributes(new Map::AttributePtrVector());
	for (unsigned int i = 0; i < MB_NUM_ATTRIBUTES; i++) {
		Map::AttributePtr attr(new Map::Attribute);
		attr->type = Map::Attribute::Filename;
		attr->name = attrNames[i];
		attr->desc = attrDesc[i];
		input >> nullPadded(attr->filenameValue, 31);
		if (attr->filenameValue.compare("UNNAMED") == 0) {
			attr->filenameValue.clear();
		} else {
			// Add the fake extension
			if (
				(!attr->filenameValue.empty())
				&& (i != MB_ATTR_KEEP_EXT) // need to keep .snd extension
			) {
				attr->filenameValue += ".";
				attr->filenameValue += validTypes[i];
			}
		}
		attr->filenameValidExtension = validTypes[i];
		attributes->push_back(attr);
	}

	// Read the background layer
	stream::pos lenBG = bg->size();
	bg->seekg(0, stream::start);

	// Read the background layer
	uint16_t unknown, mapWidth, mapPixelWidth, mapPixelHeight;
	bg
		>> u16le(unknown)
		>> u16le(mapWidth)
		>> u16le(mapPixelWidth)
		>> u16le(mapPixelHeight)
	;
	lenBG -= 8;

	if (lenBG < 2) throw stream::error("Background layer file too short");

	mapWidth >>= 1; // convert from # of bytes to # of ints (tiles)
	unsigned int mapHeight = mapPixelHeight / MB_TILE_HEIGHT;

	Map2D::Layer::ItemPtrVectorPtr bgtiles(new Map2D::Layer::ItemPtrVector());
	Map2D::Layer::ItemPtrVectorPtr bgattributes(new Map2D::Layer::ItemPtrVector());
	bgtiles->reserve(mapWidth * mapHeight);
	bgattributes->reserve(mapWidth * mapHeight);
	for (unsigned int y = 0; y < mapHeight; y++) {
		for (unsigned int x = 0; x < mapWidth; x++) {
			uint16_t code;
			bg >> u16le(code);
			lenBG -= 2;

			if ((code & 0x1FF) != MB_DEFAULT_BGTILE) {
				Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
				t->type = Map2D::Layer::Item::Default;
				t->x = x;
				t->y = y;
				t->code = code & 0x1FF;
				bgtiles->push_back(t);
			}

			Map2D::Layer::ItemPtr ta(new Map2D::Layer::Item());
			ta->type = Map2D::Layer::Item::Blocking;
			ta->x = x;
			ta->y = y;
			ta->code = (code >> 9) & 0x2F; // deselect point item flag
			ta->blockingFlags = 0;
			if (code & (1<<9)) ta->blockingFlags |= Map2D::Layer::Item::BlockLeft;
			if (code & (2<<9)) ta->blockingFlags |= Map2D::Layer::Item::BlockRight;
			if (code & (4<<9)) ta->blockingFlags |= Map2D::Layer::Item::BlockTop;
			if (code & (8<<9)) ta->blockingFlags |= Map2D::Layer::Item::BlockBottom;
			if (code & (32<<9)) ta->blockingFlags |= Map2D::Layer::Item::Slant45;
			bgattributes->push_back(ta);

			if (lenBG < 2) break;
		}
		if (lenBG < 2) break;
	}

	Map2D::Layer::ItemPtrVectorPtr validBGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= MB_MAX_VALID_BG_TILECODE; i++) {
		if (i == MB_DEFAULT_BGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validBGItems->push_back(t);
	}
	Map2D::LayerPtr bgLayer(new BashBackgroundLayer(bgtiles, validBGItems));

	Map2D::Layer::ItemPtrVectorPtr validAttrItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i < 16; i++) {
		Map2D::Layer::ItemPtr ta(new Map2D::Layer::Item());
		ta->type = Map2D::Layer::Item::Blocking;
		ta->x = 0;
		ta->y = 0;
		ta->code = i;
		ta->blockingFlags = 0;
		if (i & 1) ta->blockingFlags |= Map2D::Layer::Item::BlockLeft;
		if (i & 2) ta->blockingFlags |= Map2D::Layer::Item::BlockRight;
		if (i & 4) ta->blockingFlags |= Map2D::Layer::Item::BlockTop;
		if (i & 8) ta->blockingFlags |= Map2D::Layer::Item::BlockBottom;
		validAttrItems->push_back(ta);
	}
	{
		Map2D::Layer::ItemPtr ta(new Map2D::Layer::Item());
		ta->type = Map2D::Layer::Item::Blocking;
		ta->x = 0;
		ta->y = 0;
		ta->code = 32;
		ta->blockingFlags = 0;
		ta->blockingFlags |= Map2D::Layer::Item::Slant45;
		validAttrItems->push_back(ta);
	}
	Map2D::LayerPtr attrLayer(new BashAttributeLayer(bgattributes, validAttrItems));

	// Read the foreground layer
	stream::pos lenFG = fg->size();
	fg->seekg(2, stream::start); // skip width field
	lenFG -= 2;

	Map2D::Layer::ItemPtrVectorPtr fgtiles(new Map2D::Layer::ItemPtrVector());
	fgtiles->reserve(mapWidth * mapHeight);
	for (unsigned int y = 0; y < mapHeight; y++) {
		for (unsigned int x = 0; x < mapWidth; x++) {
			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = x;
			t->y = y;
			uint8_t code;
			fg >> u8(code);
			lenFG--;
			t->code = code;
			if (code != MB_DEFAULT_FGTILE) fgtiles->push_back(t);
			if (lenFG < 1) break;
		}
		if (lenFG < 1) break;
	}

	Map2D::Layer::ItemPtrVectorPtr validFGItems(new Map2D::Layer::ItemPtrVector());
	for (unsigned int i = 0; i <= MB_MAX_VALID_FG_TILECODE; i++) {
		// The default tile actually has an image, so don't exclude it
		if (i == MB_DEFAULT_FGTILE) continue;

		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = i;
		validFGItems->push_back(t);
	}
	Map2D::LayerPtr fgLayer(new BashForegroundLayer(fgtiles, validFGItems));

	// Read the sprite layer
	stream::pos lenSpr = spr->size();
	spr->seekg(2, stream::start); // skip unknown field
	lenSpr -= 2;

	Map2D::Layer::ItemPtrVectorPtr sprtiles(new Map2D::Layer::ItemPtrVector());
	while (lenSpr > 4) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		uint32_t lenEntry;
		uint32_t unknown1, unknown2;
		uint16_t unknown3;
		spr
			>> u32le(lenEntry)
			>> u32le(unknown1)
			>> u32le(unknown2)
			>> u16le(unknown3)
			>> u32le(t->x)
			>> u32le(t->y)
		;
		if (lenEntry > lenSpr) break; // corrupted file
		spr->seekg(22, stream::cur); // skip padding
		std::string filename;
		spr >> nullPadded(filename, lenEntry - (4+4+4+2+4+4+22));
		t->code = 0;
		for (unsigned int s = 0; s < sizeof(spriteFilenames) / sizeof(const char *); s++) {
			if (filename.compare(spriteFilenames[s]) == 0) {
				t->code = 1000000 + s;
			}
		}
		if (t->code == 0) {
			std::cout << "ERROR: Encounted Monster Bash sprite with unexpected name \""
				<< filename << "\" - unable to add to map.\n";
		}
		sprtiles->push_back(t);
		lenSpr -= lenEntry;
	}

	Map2D::Layer::ItemPtrVectorPtr validSprites(new Map2D::Layer::ItemPtrVector());
	for (unsigned int s = 0; s < sizeof(spriteFilenames) / sizeof(const char *); s++) {
		Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
		t->type = Map2D::Layer::Item::Default;
		t->x = 0;
		t->y = 0;
		t->code = 1000000 + s;
		validSprites->push_back(t);
	}
	Map2D::LayerPtr sprLayer(new BashSpriteLayer(sprtiles, validSprites));


	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);
	layers.push_back(sprLayer);
	layers.push_back(attrLayer);

	Map2DPtr map(new GenericMap2D(
		attributes, bash_getGraphicsFilenames,
		Map2D::HasViewport,
		MB_VIEWPORT_WIDTH, MB_VIEWPORT_HEIGHT,
		mapWidth, mapHeight,
		MB_TILE_WIDTH, MB_TILE_HEIGHT,
		layers, Map2D::PathPtrVectorPtr()
	));

	return map;
}

void BashMapType::write(MapPtr map, stream::expanding_output_sptr output,
	ExpandingSuppData& suppData) const
{
	Map2DPtr map2d = boost::dynamic_pointer_cast<Map2D>(map);
	if (!map2d) throw stream::error("Cannot write this type of map as this format.");
	if (map2d->getLayerCount() != 4)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	stream::output_sptr bg = suppData[SuppItem::Layer1];
	stream::output_sptr fg = suppData[SuppItem::Layer2];
	stream::output_sptr spr = suppData[SuppItem::Layer3];
	stream::output_sptr sgl = suppData[SuppItem::Extra1];
	assert(bg);
	assert(fg);
	assert(spr);
	assert(sgl);

	// Write map info file
	{
		Map::AttributePtrVectorPtr attributes = map->getAttributes();
		if (attributes->size() != MB_NUM_ATTRIBUTES) {
			throw stream::error("Cannot write map as there is an incorrect number "
				"of attributes set.");
		}
		std::string val;
		for (unsigned int i = 0; i < MB_NUM_ATTRIBUTES; i++) {
			Map::Attribute *attr = attributes->at(i).get();
			if (attr->filenameValue.empty()) {
				val = "UNNAMED";
			} else {
				std::string::size_type dot = attr->filenameValue.find_last_of('.');
				if (
					(i != MB_ATTR_KEEP_EXT) // need to keep .snd extension
					&& (dot != std::string::npos)
					&& (attr->filenameValue.substr(dot + 1).compare(validTypes[i]) == 0)
				) {
					// Extension matches, remove it
					val = attr->filenameValue.substr(0, dot);
				} else {
					val = attr->filenameValue; // don't chop off extension
				}
			}
			output << nullPadded(val, 31);
		}
	}

	// Write the background layer
	{
		const unsigned int lenBG = mapWidth * mapHeight;

		boost::scoped_array<uint16_t> bgdata(new uint16_t[lenBG]);
		memset(bgdata.get(), 0, lenBG * sizeof(uint16_t)); // default background tile

		Map2D::LayerPtr layer;
		{
			layer = map2d->getLayer(0);
			const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
			for (Map2D::Layer::ItemPtrVector::const_iterator
				i = items->begin(); i != items->end(); i++
			) {
				if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
					throw stream::error("Background layer has tiles outside map boundary!");
				}
				bgdata[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
			}
		}

		{
			// Merge in attribute layer
			layer = map2d->getLayer(3);
			const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
			for (Map2D::Layer::ItemPtrVector::const_iterator
				i = items->begin(); i != items->end(); i++
			) {
				const Map2D::Layer::ItemPtr& ta = (*i);
				if ((ta->x > mapWidth) || (ta->y > mapHeight)) {
					throw stream::error("Attribute layer has tiles outside map boundary!");
				}
				if (ta->type & Map2D::Layer::Item::Blocking) {
					uint16_t code = 0;
					if (ta->blockingFlags & Map2D::Layer::Item::BlockLeft) code |= (1<<9);
					if (ta->blockingFlags & Map2D::Layer::Item::BlockRight) code |= (2<<9);
					if (ta->blockingFlags & Map2D::Layer::Item::BlockTop) code |= (4<<9);
					if (ta->blockingFlags & Map2D::Layer::Item::BlockBottom) code |= (8<<9);
					if (ta->blockingFlags & Map2D::Layer::Item::Slant45) code |= (32<<9);
					if (code & (64<<9)) std::cerr << "FIXME: Got BG tile with unknown 0x8000 flag set!" << std::endl;
					bgdata[(*i)->y * mapWidth + (*i)->x] |= code;
				}
			}
		}

		{
			// Merge in foreground layer
			layer = map2d->getLayer(1);
			const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
			for (Map2D::Layer::ItemPtrVector::const_iterator
				i = items->begin(); i != items->end(); i++
			) {
				const Map2D::Layer::ItemPtr& ta = (*i);
				if ((ta->x > mapWidth) || (ta->y > mapHeight)) continue;
				if (
					((ta->code >= 0x07) && (ta->code <= 0x0A)) || // crumbling walkway
					(ta->code == 0x7B) || // tri-shot
					(ta->code == 0x7D) || // skull
					((ta->code >= 0xF9) && (ta->code <= 0xFF)) // points and powerups
				) {
					// This is a point item, mark it as such
					bgdata[(*i)->y * mapWidth + (*i)->x] |= 16<<9;
				}
			}
		}

		uint16_t mapStripe = mapHeight * (MB_TILE_WIDTH * MB_TILE_HEIGHT) + mapWidth;
		uint16_t mapWidthBytes = mapWidth * 2; // 2 == sizeof(uint16_t)
		uint16_t mapPixelWidth = mapWidth * MB_TILE_WIDTH;
		uint16_t mapPixelHeight = mapHeight * MB_TILE_HEIGHT;
		bg->seekp(0, stream::start);
		bg
			<< u16le(mapStripe)
			<< u16le(mapWidthBytes)
			<< u16le(mapPixelWidth)
			<< u16le(mapPixelHeight)
		;
		uint16_t *pbg = bgdata.get();
		for (unsigned int i = 0; i < lenBG; i++) {
			bg << u16le(*pbg++);
		}
	}

	// Write the foreground layer
	{
		const unsigned int lenFG = mapWidth * mapHeight;

		boost::scoped_array<uint8_t> fgdata(new uint8_t[lenFG]);
		memset(fgdata.get(), 0, lenFG); // default background tile
		Map2D::LayerPtr layer = map2d->getLayer(1);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = items->begin(); i != items->end(); i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Foreground layer has tiles outside map boundary!");
			}
			fgdata[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
		}

		uint16_t mapWidthBytes = mapWidth;
		fg->seekp(0, stream::start);
		fg
			<< u16le(mapWidthBytes)
		;
		uint8_t *pfg = fgdata.get();
		for (unsigned int i = 0; i < lenFG; i++) {
			fg << u8(*pfg++);
		}
	}

	std::set<std::string> usedSprites;
	// Write the sprite layer
	{
		// These sprites must always be present in a level
		usedSprites.insert("arrows");
usedSprites.insert("axe");
		usedSprites.insert("blank");
usedSprites.insert("bomb");
usedSprites.insert("bone1");
usedSprites.insert("bone2");
usedSprites.insert("bone3");
		usedSprites.insert("border");
		usedSprites.insert("border2");
		usedSprites.insert("cat");
		usedSprites.insert("chunk");
		usedSprites.insert("dog");
		usedSprites.insert("flag");
usedSprites.insert("float100");
		usedSprites.insert("guage");
		usedSprites.insert("heart");
		usedSprites.insert("leaf");
usedSprites.insert("plank");
usedSprites.insert("plank_r");
usedSprites.insert("rock");
		usedSprites.insert("score");
		usedSprites.insert("skullw"); // only needed if skulls present
		usedSprites.insert("splat");
		usedSprites.insert("white");

		Map2D::LayerPtr layer = map2d->getLayer(2);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		spr->seekp(0, stream::start);
		spr
			<< u16le(0xFFFE) /// @todo calculate correct value
		;
		for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			i != items->end();
			i++
		) {
			if ((*i)->code < 1000000) continue;
			unsigned int code = (*i)->code - 1000000;
			if (code >= sizeof(spriteFilenames) / sizeof(const char *)) {
				std::cerr << "ERROR: Tried to write out-of-range sprite to Monster Bash map\n";
				continue;
			}
			std::string filename(spriteFilenames[code]);
			int lenFilename = filename.length() + 2; // need two terminating nulls
			uint32_t lenEntry = 4+4+4+2+4+4+22+lenFilename;
			spr
				<< u32le(lenEntry)
				<< u32le(0)
				<< u32le(0)
				<< u16le(0)
				<< u32le((*i)->x)
				<< u32le((*i)->y)
				<< nullPadded("", 22)
				<< nullPadded(filename, lenFilename);
			;
			usedSprites.insert(filename);
			if (
				(filename.compare("main_l") == 0) ||
				(filename.compare("main_r") == 0)
			) {
				usedSprites.insert("main_l");
				usedSprites.insert("main_r");
				usedSprites.insert("break_screen");
				usedSprites.insert("crack");
				usedSprites.insert("crawlleft");
				usedSprites.insert("crawlright");
				usedSprites.insert("dirt_l");
				usedSprites.insert("dirt_r");
				usedSprites.insert("main_die");
				usedSprites.insert("main_exit");
				usedSprites.insert("main_hat_l");
				usedSprites.insert("main_hat_r");
				usedSprites.insert("main_l");
				usedSprites.insert("main_meter");
				usedSprites.insert("main_r");
				usedSprites.insert("main_stars");
			} else if (
				(filename.compare("cyclops_l") == 0) ||
				(filename.compare("cyclops_r") == 0)
			) {
				usedSprites.insert("cyclops_l");
				usedSprites.insert("cyclops_r");
				usedSprites.insert("cyc_horn_l");
				usedSprites.insert("cyc_horn_r");
			} else if (
				(filename.compare("devil_l") == 0) ||
				(filename.compare("devil_r") == 0)
			) {
				usedSprites.insert("devil_l");
				usedSprites.insert("devil_r");
				usedSprites.insert("pfork_l");
				usedSprites.insert("pfork_r");
			} else if (
				(filename.compare("hand_l") == 0) ||
				(filename.compare("hand_r") == 0)
			) {
				usedSprites.insert("hand_l");
				usedSprites.insert("hand_r");
			} else if (
				(filename.compare("iman_l") == 0) ||
				(filename.compare("iman_r") == 0)
			) {
				usedSprites.insert("iman_l");
				usedSprites.insert("iman_r");
				usedSprites.insert("hat");
			} else if (
				(filename.compare("knifee") == 0) ||
				(filename.compare("knifee_ud") == 0)
			) {
				usedSprites.insert("knife");
			} else if (
				(filename.compare("nemesis") == 0)
			) {
				usedSprites.insert("main_broom");
			} else if (
				(filename.compare("skelet_l") == 0) ||
				(filename.compare("skelet_r") == 0)
			) {
				usedSprites.insert("skelet_l");
				usedSprites.insert("sketet_r");
				usedSprites.insert("jaw_l");
				usedSprites.insert("jaw_r");
			} else if (
				(filename.compare("teeth_l") == 0) ||
				(filename.compare("teeth_r") == 0)
			) {
				usedSprites.insert("teeth_l");
				usedSprites.insert("teeth_r");
			} else if (
				(filename.compare("tman_bl") == 0) ||
				(filename.compare("tman_br") == 0)
			) {
				usedSprites.insert("tman_bl");
				usedSprites.insert("tman_br");
				usedSprites.insert("pellet_h");
			} else if (
				(filename.compare("tman_ld") == 0) ||
				(filename.compare("tman_lu") == 0)
			) {
				usedSprites.insert("tman_ld");
				usedSprites.insert("tman_lu");
				usedSprites.insert("pellet_h");
			} else if (
				(filename.compare("tman_rd") == 0) ||
				(filename.compare("tman_ru") == 0)
			) {
				usedSprites.insert("tman_rd");
				usedSprites.insert("tman_ru");
				usedSprites.insert("pellet_h");
			} else if (
				(filename.compare("tman_tl") == 0) ||
				(filename.compare("tman_tr") == 0)
			) {
				usedSprites.insert("tman_tl");
				usedSprites.insert("tman_tr");
				usedSprites.insert("pellet_h");
			} else if (
				(filename.compare("vulture") == 0) ||
				(filename.compare("vulture_l") == 0) ||
				(filename.compare("vulture_r") == 0)
			) {
				usedSprites.insert("vulture");
				usedSprites.insert("vulture_l");
				usedSprites.insert("vulture_r");
			} else if (
				(filename.compare("zb_l") == 0) ||
				(filename.compare("zb_r") == 0)
			) {
				usedSprites.insert("zb_l");
				usedSprites.insert("zb_r");
				usedSprites.insert("zbh_l"); // optional unless spawning zombie (then the others are needed too)
				usedSprites.insert("zbh_r"); // ditto
				usedSprites.insert("zhead");
				usedSprites.insert("zhead_r");
			}
		}
	}

	// Write out a list of all required sprites
	sgl->seekp(0, stream::start);
	for (std::set<std::string>::const_iterator
		i = usedSprites.begin(); i != usedSprites.end(); i++
	) {
		sgl
			<< nullPadded(*i, 31)
		;
	}

	// No need to truncate as we're writing into empty expanding streams.

	bg->flush();
	fg->flush();
	spr->flush();
	sgl->flush();
	output->flush();
	return;
}

SuppFilenames BashMapType::getRequiredSupps(stream::input_sptr input,
	const std::string& filename) const
{
	SuppFilenames supps;
	std::string baseName = filename.substr(0, filename.length() - 3);
	supps[SuppItem::Layer1] = baseName + "mbg";
	supps[SuppItem::Layer2] = baseName + "mfg";
	supps[SuppItem::Layer3] = baseName + "msp";

	input->seekg(31*3, stream::start);
	std::string sgl;
	input >> nullPadded(sgl, 31);
	supps[SuppItem::Extra1] = sgl + ".sgl";
	return supps;
}

} // namespace gamemaps
} // namespace camoto
