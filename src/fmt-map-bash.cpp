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
#define MB_DEFAULT_FGTILE     0x00

/// This is the largest valid tile code in the background layer.
#define MB_MAX_VALID_TILECODE 0x6C

/// Number of fields in the .mif file
#define MB_NUM_ATTRIBUTES        7

/// Index of attribute for .snd file, which never gets its extension removed.
#define MB_ATTR_KEEP_EXT         5

namespace camoto {
namespace gamemaps {

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
			const TilesetCollectionPtr& tileset)
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
			const TilesetCollectionPtr& tileset)
		{
			TilesetCollection::const_iterator t = tileset->find(BackgroundTileset1);
			if (t == tileset->end()) return ImagePtr(); // no tileset?!

			unsigned int index = item->code & 0x1FF;
			const Tileset::VC_ENTRYPTR& images = t->second->getItems();
			if (index >= images.size()) return ImagePtr(); // out of range
			return t->second->openImage(images[index]);
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
			const TilesetCollectionPtr& tileset)
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
	assert(bg);
	assert(fg);

	// Read the map info file
	static const char *attrNames[] = {
		"Background tileset",
		"Foreground tileset",
		"Bonus tileset",
		"Sprite information",
		"Palette",
		"Sound effects",
		"Unknown",
	};
	static const char *attrDesc[] = {
		"Filename of the tileset to use for drawing the map background layer",
		"Filename of the first tileset to use for drawing the map foreground layer",
		"Filename of the second tileset to use for drawing the map foreground layer",
		"Something to do with sprite data?",
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

			Map2D::Layer::ItemPtr t(new Map2D::Layer::Item());
			t->type = Map2D::Layer::Item::Default;
			t->x = x;
			t->y = y;
			t->code = code & 0x1FF;
			bgtiles->push_back(t);

			Map2D::Layer::ItemPtr ta(new Map2D::Layer::Item());
			ta->type = Map2D::Layer::Item::Blocking;
			ta->x = x;
			ta->y = y;
			ta->code = code >> 9;
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
	Map2D::LayerPtr bgLayer(new BashBackgroundLayer(bgtiles, validBGItems));

	Map2D::Layer::ItemPtrVectorPtr validAttrItems(new Map2D::Layer::ItemPtrVector());
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
	Map2D::LayerPtr fgLayer(new BashForegroundLayer(fgtiles, validFGItems));

	Map2D::LayerPtrVector layers;
	layers.push_back(bgLayer);
	layers.push_back(fgLayer);
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
	if (map2d->getLayerCount() != 3)
		throw stream::error("Incorrect layer count for this format.");

	unsigned int mapWidth, mapHeight;
	map2d->getMapSize(&mapWidth, &mapHeight);

	stream::output_sptr bg = suppData[SuppItem::Layer1];
	stream::output_sptr fg = suppData[SuppItem::Layer2];
	assert(bg);
	assert(fg);

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
		memset(bgdata.get(), 0, lenBG); // default background tile

		Map2D::LayerPtr layer = map2d->getLayer(0);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			i != items->end();
			i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			bgdata[(*i)->y * mapWidth + (*i)->x] = (*i)->code;
		}

		// Merge in attribute layer
		layer = map2d->getLayer(2);
		const Map2D::Layer::ItemPtrVectorPtr atitems = layer->getAllItems();
		for (Map2D::Layer::ItemPtrVector::const_iterator i = atitems->begin();
			i != atitems->end();
			i++
		) {
			const Map2D::Layer::ItemPtr& ta = (*i);
			if ((ta->x > mapWidth) || (ta->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
			}
			if (ta->type & Map2D::Layer::Item::Blocking) {
				uint16_t code = 0;
				if (ta->blockingFlags & Map2D::Layer::Item::BlockLeft) code |= (1<<9);
				if (ta->blockingFlags & Map2D::Layer::Item::BlockRight) code |= (2<<9);
				if (ta->blockingFlags & Map2D::Layer::Item::BlockTop) code |= (4<<9);
				if (ta->blockingFlags & Map2D::Layer::Item::BlockBottom) code |= (8<<9);
				if (ta->blockingFlags & Map2D::Layer::Item::Slant45) code |= (32<<9);
				bgdata[(*i)->y * mapWidth + (*i)->x] |= code;
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
		for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			i != items->end();
			i++
		) {
			if (((*i)->x > mapWidth) || ((*i)->y > mapHeight)) {
				throw stream::error("Layer has tiles outside map boundary!");
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

	bg->flush();
	fg->flush();
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
	return supps;
}

} // namespace gamemaps
} // namespace camoto
