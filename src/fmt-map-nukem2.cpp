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

#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp> // make_unique
#include "map-core.hpp"
#include "map2d-core.hpp"
#include "fmt-map-nukem2.hpp"

/// Width of each tile in pixels
#define DN2_TILE_WIDTH 8

/// Height of each tile in pixels
#define DN2_TILE_HEIGHT 8

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

class Layer_Nukem2_Actors: public Map2DCore::LayerCore
{
	public:
		Layer_Nukem2_Actors(stream::input& content, stream::len* lenMap)
		{
			unsigned int numActorInts;
			content
				>> u16le(numActorInts)
			;
			unsigned int numActors = numActorInts / 3;
			if (*lenMap < numActors * 6) throw stream::error("Map file has been truncated!");

			this->v_allItems.reserve(numActors);
			for (unsigned int i = 0; i < numActors; i++) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();
				t.type = Item::Type::Default;
				content
					>> u16le(t.code)
					>> u16le(t.pos.x)
					>> u16le(t.pos.y)
				;
			}
			*lenMap -= 6 * numActors;
		}

		virtual ~Layer_Nukem2_Actors()
		{
		}

		void flush(stream::output& content, const Point& mapSize)
		{
			uint16_t numActorInts = this->v_allItems.size() * 3;
			content << u16le(numActorInts);
			for (auto& i : this->v_allItems) {
				assert((i.pos.x < mapSize.x) && (i.pos.y < mapSize.y));
				content
					<< u16le(i.code)
					<< u16le(i.pos.x)
					<< u16le(i.pos.y)
				;
			}
			return;
		}

		virtual std::string title() const
		{
			return "Actors";
		}

		virtual Caps caps() const
		{
			return Caps::UseImageDims;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			auto t = tileset.find(ImagePurpose::SpriteTileset1);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			unsigned int index = item.code - 31;
			unsigned int num = images.size();
			if (index >= num) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}
			while (!(images[index]->fAttr & gamearchive::Archive::File::Attribute::Folder)) {
				// Some images are duplicated, but libgamegraphics reports these as
				// empty tilesets.  So if we encounter an empty one, find the next
				// available actor.
				index++;
				if (index >= num) { // out of range
					ret.type = ImageFromCodeInfo::ImageType::Unknown;
					return ret;
				}
			}

			auto tsActor = t->second->openTileset(images[index]);
			auto& actorFrames = tsActor->files();
			if (actorFrames.size() <= 0) { // no images
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = tsActor->openImage(actorFrames[0]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
#warning TODO
			for (int i = 0; i < 10; i++) {
				validItems.emplace_back();
				Item& item = validItems.back();
				item.type = Item::Type::Default;
				item.pos = {0, 0};
				item.code = i + 31;
				validItems.push_back(item);
			}
			return validItems;
		}
};

class Layer_Nukem2_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Nukem2_Background(std::vector<Item>& items)
		{
			this->v_allItems = items;
		}

		virtual ~Layer_Nukem2_Background()
		{
		}

		void flush(stream::output& content, const Point& mapSize)
		{
			return;
		}

		virtual std::string title() const
		{
			return "Background";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			unsigned int czoneIndex = 1;

			auto t = tileset.find(ImagePurpose::BackgroundTileset1);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& czones = t->second->files();
			if (czoneIndex >= czones.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}
			auto czoneTiles = t->second->openTileset(czones[czoneIndex]);

			auto& images = czoneTiles->files();
			if (item.code >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = czoneTiles->openImage(images[item.code]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i < DN2_NUM_SOLID_TILES; i++) {
				validItems.emplace_back();
				auto& t = validItems.back();

				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				validItems.push_back(t);
			}
			return validItems;
		}
};

class Layer_Nukem2_Foreground: public Map2DCore::LayerCore
{
	public:
		Layer_Nukem2_Foreground(std::vector<Item>& items)
		{
			this->v_allItems = items;
		}

		virtual ~Layer_Nukem2_Foreground()
		{
		}

		void flush(stream::output& content, const Point& mapSize)
		{
			return;
		}

		virtual std::string title() const
		{
			return "Foreground";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			unsigned int czoneIndex = 2;

			auto t = tileset.find(ImagePurpose::BackgroundTileset1);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& czones = t->second->files();
			if (czoneIndex >= czones.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}
			auto czoneTiles = t->second->openTileset(czones[czoneIndex]);

			auto& images = czoneTiles->files();
			if (item.code >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = czoneTiles->openImage(images[item.code]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;
			for (unsigned int i = 0; i < DN2_NUM_MASKED_TILES; i++) {
				validItems.emplace_back();
				auto& t = validItems.back();

				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				validItems.push_back(t);
			}
			return validItems;
		}
};

class Map_Nukem2: public MapCore, public Map2DCore
{
	public:
		Map_Nukem2(std::unique_ptr<stream::inout> content)
			:	content(std::move(content))
		{
			stream::pos lenMap = this->content->size();
			this->content->seekg(0, stream::start);

			uint16_t bgOffset, unk;
			std::string zoneFile, backFile, musFile;
			*this->content
				>> u16le(bgOffset)
			;

			// Set the attributes
			{
				assert(this->v_attributes.size() == ATTR_CZONE); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Filename;
				a.name = "CZone tileset";
				a.desc = "Filename of the tileset to use for drawing the foreground and background layers.";
				*this->content >> nullPadded(a.filenameValue, 13);
				// Trim off the padding spaces
				a.filenameValue = a.filenameValue.substr(0, a.filenameValue.find_last_not_of(' ') + 1);
				a.filenameSpec.push_back("*.mni");
			};
			{
				assert(this->v_attributes.size() == ATTR_BACKDROP); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Filename;
				a.name = "Backdrop";
				a.desc = "Filename of the backdrop to draw behind the map.";
				*this->content >> nullPadded(a.filenameValue, 13);
				// Trim off the padding spaces
				a.filenameValue = a.filenameValue.substr(0, a.filenameValue.find_last_not_of(' ') + 1);
				a.filenameSpec.push_back("*.mni");
			};
			{
				assert(this->v_attributes.size() == ATTR_MUSIC); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Filename;
				a.name = "Song";
				a.desc = "File to play as background music.";
				*this->content >> nullPadded(a.filenameValue, 13);
				// Trim off the padding spaces
				a.filenameValue = a.filenameValue.substr(0, a.filenameValue.find_last_not_of(' ') + 1);
				a.filenameSpec.push_back("*.imf");
			};

			uint8_t flags, altBack;
			*this->content
				>> u8(flags)
				>> u8(altBack)
				>> u16le(unk)
			;
			lenMap -= 2+13+13+13+1+1+2+2;

			{
				assert(this->v_attributes.size() == ATTR_USEALTBD); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Enum;
				a.name = "Alt backdrop?";
				a.desc = "When should the alternate backdrop file be used?";
				a.enumValue = (flags >> 6) & 3;
				a.enumValueNames = {
					"Never",
					"After destroying force field",
					"After teleporting",
					"Both? (this value has an unknown/untested effect)",
				};
			};
			{
				assert(this->v_attributes.size() == ATTR_QUAKE); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Enum;
				a.name = "Earthquake";
				a.desc = "Should the level shake like there is an earthquake?";
				a.enumValue = (flags >> 5) & 1;
				a.enumValueNames = {
					"No",
					"Yes",
				};
			};
			{
				assert(this->v_attributes.size() == ATTR_SCROLLBD); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Enum;
				a.name = "Backdrop movement";
				a.desc = "Should the backdrop move when the player is stationary?";
				a.enumValue = (flags >> 3) & 3;
				a.enumValueNames = {
					"No",
					"Scroll left",
					"Scroll up",
					"3 (this value has an unknown/untested effect)",
				};
			};
			{
				assert(this->v_attributes.size() == ATTR_PARALLAX); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Enum;
				a.name = "Parallax";
				a.desc = "How should the backdrop scroll when the player moves?";
				a.enumValue = (flags >> 0) & 3;
				a.enumValueNames = {
					"Fixed - no movement",
					"Horizontal and vertical movement",
					"Horizontal movement only",
					"3 (this value has an unknown/untested effect)",
				};
			};
			{
				assert(this->v_attributes.size() == ATTR_ALTBD); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Integer;
				a.name = "Alt backdrop pic";
				a.desc = "Number of alternate backdrop file (DROPx.MNI), 0 if unused";
				a.integerValue = altBack;
				a.integerMinValue = 0;
				a.integerMaxValue = 24;
			};

			// Read in the actor layer
			auto layerAC = std::make_shared<Layer_Nukem2_Actors>(
				*this->content, &lenMap
			);

			// Read the main layer
			this->content->seekg(bgOffset, stream::start);
			*this->content
				>> u16le(this->mapWidth)
			;

			unsigned int tileValues[DN2_NUM_TILES_BG];
			memset(tileValues, 0, sizeof(tileValues));

			unsigned int *v = tileValues;
			for (unsigned int i = 0; (i < DN2_NUM_TILES_BG) && (lenMap >= 2); i++) {
				*this->content >> u16le(*v++);
				lenMap -= 2;
			}

			uint16_t lenExtra;
			*this->content >> u16le(lenExtra);
			unsigned int extraValues[DN2_NUM_TILES_BG];
			memset(extraValues, 0, sizeof(extraValues));
			unsigned int *ev = extraValues;
			unsigned int *ev_end = extraValues + DN2_NUM_TILES_BG;
			for (unsigned int i = 0; i < lenExtra; i++) {
				uint8_t code;
				*this->content >> u8(code);
				lenMap--;
				if (code & 0x80) {
					// Multiple bytes concatenated together
					// code == 0xFF for one byte, 0xFE for two bytes, etc.
					unsigned int len = 0x100 - code;
					while (len--) {
						*this->content >> u8(code);
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
					*this->content >> u8(code);
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
					this->content->seekg(lenExtra - i - 1, stream::cur);
					break;
				}
			}

			std::vector<Layer::Item> bgItems, fgItems;

			v = tileValues;
			ev = extraValues;
			for (unsigned int i = 0; i < DN2_NUM_TILES_BG; i++) {
				if (*v & 0x8000) {
					// This cell has a foreground and background tile
					{
						int code = *v & 0x3FF;
						if (code != DN2_DEFAULT_BGTILE) {
							bgItems.emplace_back();
							auto& t = bgItems.back();
							t.type = Layer::Item::Type::Default;
							t.pos.x = i % mapWidth;
							t.pos.y = i / mapWidth;
							t.code = code;
						}
					}

					{
						fgItems.emplace_back();
						auto& t = fgItems.back();
						t.type = Layer::Item::Type::Default;
						t.pos.x = i % mapWidth;
						t.pos.y = i / mapWidth;
						t.code = ((*v >> 10) & 0x1F) | *ev;
					}

				} else if (*v < DN2_NUM_SOLID_TILES * DN2_TILE_WIDTH) {
					// Background only tile

					int code = *v >> 3;
					if (code != DN2_DEFAULT_BGTILE) {
						bgItems.emplace_back();
						auto& t = bgItems.back();
						t.type = Layer::Item::Type::Default;
						t.pos.x = i % mapWidth;
						t.pos.y = i / mapWidth;
						t.code = code;
					}
				} else {
					// Foreground only tile

					fgItems.emplace_back();
					auto& t = fgItems.back();
					t.type = Layer::Item::Type::Default;
					t.pos.x = i % mapWidth;
					t.pos.y = i / mapWidth;
					t.code = ((*v >> 3) - DN2_NUM_SOLID_TILES) / 5;
				}
				v++;
				ev++;
			}

			auto layerBG = std::make_shared<Layer_Nukem2_Background>(bgItems);
			auto layerFG = std::make_shared<Layer_Nukem2_Foreground>(fgItems);

			// Trailing filenames
			{
				assert(this->v_attributes.size() == ATTR_ZONEATTR); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Filename;
				a.name = "Zone attribute";
				a.desc = "Filename of the zone tile attributes.";
				*this->content >> nullPadded(a.filenameValue, 13);
				// Trim off the padding spaces
				a.filenameValue = a.filenameValue.substr(0, a.filenameValue.find_last_not_of(' ') + 1);
				a.filenameSpec.push_back("*.mni");
			}
			{
				assert(this->v_attributes.size() == ATTR_ZONETSET); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Filename;
				a.name = "Zone tileset";
				a.desc = "Filename of the zone solid tileset.";
				*this->content >> nullPadded(a.filenameValue, 13);
				// Trim off the padding spaces
				a.filenameValue = a.filenameValue.substr(0, a.filenameValue.find_last_not_of(' ') + 1);
				a.filenameSpec.push_back("*.mni");
			}
			{
				assert(this->v_attributes.size() == ATTR_ZONEMSET); // make sure compile-time index is correct
				this->v_attributes.emplace_back();
				auto& a = this->v_attributes.back();
				a.type = Attribute::Type::Filename;
				a.name = "Zone masked tileset";
				a.desc = "Filename of the zone masked tileset.";
				*this->content >> nullPadded(a.filenameValue, 13);
				// Trim off the padding spaces
				a.filenameValue = a.filenameValue.substr(0, a.filenameValue.find_last_not_of(' ') + 1);
				a.filenameSpec.push_back("*.mni");
			}

			this->v_layers.push_back(layerBG);
			this->v_layers.push_back(layerFG);
			this->v_layers.push_back(layerAC);
		}

		virtual ~Map_Nukem2()
		{
		}

		virtual void flush()
		{
			auto layerBG = dynamic_cast<Layer_Nukem2_Background*>(this->v_layers[0].get());
			auto layerFG = dynamic_cast<Layer_Nukem2_Foreground*>(this->v_layers[1].get());
			auto layerAC = dynamic_cast<Layer_Nukem2_Actors*>(this->v_layers[2].get());

			// Figure out where the main data will start
			auto& actors = layerAC->items();
			stream::pos offBG = 2+13+13+13+1+1+2+2+6*actors.size();

			this->content->seekp(0, stream::start);
			*this->content
				<< u16le(offBG)
			;

			// CZone
			auto& attr0 = this->v_attributes[ATTR_CZONE];
			std::string val = attr0.filenameValue;
			int padamt = 12 - val.length();
			val += std::string(padamt, ' '); // pad with spaces
			*this->content << nullPadded(val, 13);

			// Backdrop
			auto& attr1 = this->v_attributes[ATTR_BACKDROP];
			val = attr1.filenameValue;
			padamt = 12 - val.length();
			val += std::string(padamt, ' '); // pad with spaces
			*this->content << nullPadded(val, 13);

			// Song
			auto& attr2 = this->v_attributes[ATTR_MUSIC];
			val = attr2.filenameValue;
			padamt = 12 - val.length();
			val += std::string(padamt, ' '); // pad with spaces
			*this->content << nullPadded(val, 13);

			uint8_t flags = 0;

			auto& attr3 = this->v_attributes[ATTR_USEALTBD];
			flags |= attr3.enumValue << 6;

			auto& attr4 = this->v_attributes[ATTR_QUAKE];
			flags |= attr4.enumValue << 5;

			auto& attr5 = this->v_attributes[ATTR_SCROLLBD];
			flags |= attr5.enumValue << 3;

			auto& attr6 = this->v_attributes[ATTR_PARALLAX];
			flags |= attr6.enumValue << 0;

			*this->content << u8(flags);

			auto& attr7 = this->v_attributes[ATTR_ALTBD];
			*this->content << u8(attr7.integerValue);

			*this->content << u16le(0);

			auto mapDims = this->mapSize();

			// Write the actor layer
			layerAC->flush(*this->content, mapDims);

			// Write the background layer
			std::vector<uint16_t> bg(DN2_NUM_TILES_BG, DN2_DEFAULT_BGTILE);

			// Set the default foreground tile
			std::vector<uint16_t> fg(DN2_NUM_TILES_BG, (uint16_t)-1);

			// Set the default extra bits
			std::vector<uint16_t> extra(DN2_NUM_TILES_BG, 0x00);

			for (auto& i : layerBG->items()) {
				assert((i.pos.x < mapDims.x) && (i.pos.y < mapDims.y));
				bg[i.pos.y * mapDims.x + i.pos.x] = i.code;
			}

			for (auto& i : layerFG->items()) {
				assert((i.pos.x < mapDims.x) && (i.pos.y < mapDims.y));
				fg[i.pos.y * mapDims.x + i.pos.x] = i.code;
			}

			*this->content << u16le(mapDims.x);

			assert(mapDims.x * mapDims.y < DN2_NUM_TILES_BG);
			for (unsigned int i = 0; i < DN2_NUM_TILES_BG; i++) {
				if (fg[i] == (uint16_t)-1) {
					// BG tile only
					*this->content << u16le(bg[i] * 8);
				} else if (bg[i] == 0x00) {
					// FG tile only
					*this->content << u16le((fg[i] * 5 + DN2_NUM_SOLID_TILES) * 8);
				} else {
					// BG and FG tile
					uint16_t code = 0x8000 | bg[i] | ((fg[i] & 0x1F) << 10);
					if (fg[i] & 0x60) {
						// Need to save these extra bits
						extra[i] = fg[i] & 0x60;
					}
					*this->content << u16le(code);
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

			std::vector<uint8_t> rleExtra;
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

			*this->content << u16le(rleExtra.size());
			this->content->write(rleExtra.data(), rleExtra.size());

			// Zone attribute filename (null-padded, not space-padded)
			auto& attr8 = this->v_attributes[ATTR_ZONEATTR];
			*this->content << nullPadded(attr8.filenameValue, 13);

			// Zone solid tileset filename (null-padded, not space-padded)
			auto& attr9 = this->v_attributes[ATTR_ZONETSET];
			*this->content << nullPadded(attr9.filenameValue, 13);

			// Zone masked tileset filename (null-padded, not space-padded)
			auto& attr10 = this->v_attributes[ATTR_ZONEMSET];
			*this->content << nullPadded(attr10.filenameValue, 13);

			this->content->flush();
			return;
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {
				// Background tiles
				std::make_pair(
					ImagePurpose::BackgroundTileset1,
					GraphicsFilename{
						this->v_attributes[ATTR_CZONE].filenameValue,
						"tls-nukem2-czone"
					}
				),
				// Backdrop
				std::make_pair(
					ImagePurpose::BackgroundImage,
					GraphicsFilename{
						this->v_attributes[ATTR_BACKDROP].filenameValue,
						"img-nukem2-backdrop"
					}
				),
			};
		}

		virtual Caps caps() const
		{
			return
				Map2D::Caps::HasViewport
				| Map2D::Caps::HasMapSize
				| Map2D::Caps::SetMapSize
				| Map2D::Caps::HasTileSize
			;
		}

		virtual Point viewport() const
		{
			return {256, 160};
		}

		virtual Point mapSize() const
		{
			return {this->mapWidth, 32750 / this->mapWidth};
		}

		virtual void mapSize(const Point& newSize)
		{
			if (newSize.x * newSize.y > 32750) {
				throw camoto::error("Map dimensions too large.  "
					"Width multiplied by height must be less than 32751.");
			}
			this->mapWidth = newSize.x;
			return;
		}

		virtual Point tileSize() const
		{
			return {DN2_TILE_WIDTH, DN2_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundUseBGImage(tileset);
		}

	private:
		std::unique_ptr<stream::inout> content;
		unsigned int mapWidth;
};


std::string MapType_Nukem2::code() const
{
	return "map2d-nukem2";
}

std::string MapType_Nukem2::friendlyName() const
{
	return "Duke Nukem II level";
}

std::vector<std::string> MapType_Nukem2::fileExtensions() const
{
	return {"mni"};
}

std::vector<std::string> MapType_Nukem2::games() const
{
	return {
		"Duke Nukem II",
	};
}

MapType::Certainty MapType_Nukem2::isInstance(stream::input& content) const
{
	stream::len lenMap = content.size();

	// TESTED BY: fmt_map_nukem2_isinstance_c01
	if (lenMap < 2+13+13+13+1+1+2+2 + 2+DN2_LAYER_LEN_BG) return MapType::DefinitelyNo; // too short

	content.seekg(0, stream::start);
	uint16_t bgOffset;
	std::string zoneFile, backFile, musFile;
	content
		>> u16le(bgOffset)
	;

	// TESTED BY: fmt_map_nukem2_isinstance_c02
	if (bgOffset > lenMap - (2+DN2_LAYER_LEN_BG)) return MapType::DefinitelyNo; // offset wrong

	content.seekg(13 * 3 + 4, stream::cur);

	uint16_t numActorInts;
	content >> u16le(numActorInts);

	// TESTED BY: fmt_map_nukem2_isinstance_c03
	if (2+13*3+6 + numActorInts * 2 + 2+DN2_LAYER_LEN_BG > lenMap) return MapType::DefinitelyNo; // too many actors

	content.seekg(bgOffset + 2+DN2_LAYER_LEN_BG, stream::start);
	uint16_t lenExtra;
	content >> u16le(lenExtra);
	// TESTED BY: fmt_map_nukem2_isinstance_c04
	if (bgOffset + 2+DN2_LAYER_LEN_BG + lenExtra+2 > lenMap) return MapType::DefinitelyNo; // extra data too long

	// TESTED BY: fmt_map_nukem2_isinstance_c00
	if (bgOffset + 2+DN2_LAYER_LEN_BG + lenExtra+2 + 13*3 == lenMap) return MapType::DefinitelyYes;

	// TESTED BY: fmt_map_nukem2_isinstance_c05
	return MapType::PossiblyYes;
}

std::unique_ptr<Map> MapType_Nukem2::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Nukem2::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_Nukem2>(std::move(content));
}

SuppFilenames MapType_Nukem2::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

} // namespace gamemaps
} // namespace camoto
