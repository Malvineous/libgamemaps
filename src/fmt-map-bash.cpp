/**
 * @file  fmt-map-bash.cpp
 * @brief MapType and Map2D implementation for Monster Bash levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Monster_Bash
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

#include <algorithm>
#include <set>
#include <cerrno>
#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp> // make_unique
#include "map-core.hpp"
#include "map2d-core.hpp"
#include "fmt-map-bash.hpp"

/// Width of map tiles
#define MB_TILE_WIDTH           16

/// Height of map tiles
#define MB_TILE_HEIGHT          16

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

/// All sprite tilecodes are offset by this amount, so there is no confusion
/// about the "fake" nature of the code.
#define BASH_SPRITE_OFFSET 1000000

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

class Layer_Bash_Background: public Map2DCore::LayerCore
{
	public:
		Layer_Bash_Background(std::unique_ptr<stream::inout> content,
			unsigned long* mapWidth, unsigned long* mapHeight,
			std::vector<uint16_t>* bgdata)
			:	content(std::move(content))
		{
			// Read the background layer
			stream::pos lenBG = this->content->size();
			this->content->seekg(0, stream::start);

			// Read the background layer
			uint16_t unknown, mapPixelWidth, mapPixelHeight;
			*this->content
				>> u16le(unknown)
				>> u16le(this->mapWidth)
				>> u16le(mapPixelWidth)
				>> u16le(mapPixelHeight)
			;
			lenBG -= 8;

			if (lenBG < 2) throw stream::error("Background layer file too short");

			this->mapWidth >>= 1; // convert from # of bytes to # of ints (tiles)
			this->mapHeight = mapPixelHeight / MB_TILE_HEIGHT;

			std::vector<Item> bgattributes;
			std::vector<Item> bgpoints;
			auto lenLayer = this->mapWidth * this->mapHeight;
			this->v_allItems.reserve(lenLayer);
			bgdata->reserve(lenLayer);
			for (unsigned int y = 0; y < this->mapHeight; y++) {
				for (unsigned int x = 0; x < this->mapWidth; x++) {
					uint16_t code;
					*this->content >> u16le(code);
					lenBG -= 2;
					bgdata->push_back(code);

					if ((code & 0x1FF) != MB_DEFAULT_BGTILE) {
						this->v_allItems.emplace_back();
						auto& t = this->v_allItems.back();
						t.type = Map2D::Layer::Item::Type::Default;
						t.pos = {x, y};
						t.code = code & 0x1FF;
					}

					if (lenBG < 2) break;
				}
				if (lenBG < 2) break;
			}
			*mapWidth = this->mapWidth;
			*mapHeight = this->mapHeight;
		}

		// Populate an array with the tile codes
		void populate(std::vector<uint16_t>* tiles)
		{
			for (auto& i : this->items()) {
				if (
					(i.pos.x >= (signed long)this->mapWidth)
					|| (i.pos.y >= (signed long)this->mapHeight)
				) {
					throw stream::error("Background layer has tiles outside map boundary!");
				}
				(*tiles)[i.pos.y * this->mapWidth + i.pos.x] = i.code;
			}
			return;
		}

		// Write a tile code array to the underlying file
		void flush(const std::vector<uint16_t>& tiles)
		{
			this->content->truncate(2*4 + tiles.size()*2);
			this->content->seekp(0, stream::start);
			uint16_t mapStripe = this->mapHeight * (MB_TILE_WIDTH * MB_TILE_HEIGHT)
				+ this->mapWidth;
			uint16_t mapWidthBytes = this->mapWidth * 2; // 2 == sizeof(uint16_t)
			uint16_t mapPixelWidth = this->mapWidth * MB_TILE_WIDTH;
			uint16_t mapPixelHeight = this->mapHeight * MB_TILE_HEIGHT;
			*this->content
				<< u16le(mapStripe)
				<< u16le(mapWidthBytes)
				<< u16le(mapPixelWidth)
				<< u16le(mapPixelHeight)
			;
			for (auto& i : tiles) {
				*this->content << u16le(i);
			}
			this->content->flush();
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

			auto t = tileset.find(ImagePurpose::BackgroundTileset1);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& images = t->second->files();
			if (item.code >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[item.code]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> items;
			for (unsigned int i = 0; i <= MB_MAX_VALID_BG_TILECODE; i++) {
				// The default tile actually has an image, so don't exclude it
				if (i == MB_DEFAULT_BGTILE) continue;

				Map2D::Layer::Item t;
				t.type = Map2D::Layer::Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				items.push_back(t);
			}
			return items;
		}

	private:
		std::unique_ptr<stream::inout> content;
		unsigned long mapWidth;
		unsigned long mapHeight;
};

class Layer_Bash_Foreground: public Map2DCore::LayerCore
{
	public:
		Layer_Bash_Foreground(std::unique_ptr<stream::inout> content,
			unsigned long mapWidth, unsigned long mapHeight,
			std::vector<uint8_t>* fgdata)
			:	content(std::move(content)),
				mapWidth(mapWidth),
				mapHeight(mapHeight)
		{
			// Read the foreground layer
			unsigned long lenFG = this->content->size();
			this->content->seekg(2, stream::start); // skip width field
			lenFG -= 2;

			unsigned long lenLayer = this->mapWidth * this->mapHeight;
			this->v_allItems.reserve(lenLayer);
			fgdata->resize(lenLayer, MB_DEFAULT_FGTILE);
			auto fg = fgdata->data();
			this->content->read(fg, std::min(lenLayer, lenFG));
			for (unsigned int y = 0; y < this->mapHeight; y++) {
				for (unsigned int x = 0; x < this->mapWidth; x++) {
					uint8_t code = *fg++;
					if (code != MB_DEFAULT_FGTILE) {
						this->v_allItems.emplace_back();
						auto& t = this->v_allItems.back();
						t.type = Item::Type::Default;
						t.pos = {x, y};
						t.code = code;
					}
				}
			}
		}

		// Populate an array with the tile codes
		void populate(std::vector<uint8_t>* tiles,
			std::set<std::string>* usedSprites)
		{
			for (auto& i : this->items()) {
				if (
					(i.pos.x >= (signed long)this->mapWidth)
					|| (i.pos.y >= (signed long)this->mapHeight)
				) {
					throw stream::error("Foreground layer has tiles outside map boundary!");
				}
				(*tiles)[i.pos.y * this->mapWidth + i.pos.x] = i.code;
#warning TODO: If the foreground layer contains a skull or collapsing walkway, add the sprites to usedSprites (and remove from "*" in the XML)
			}
			return;
		}

		// Write a tile code array to the underlying file
		void flush(const std::vector<uint8_t>& tiles)
		{
			this->content->truncate(2 + tiles.size());
			this->content->seekp(0, stream::start);
			uint16_t mapWidthBytes = this->mapWidth;
			*this->content
				<< u16le(mapWidthBytes)
			;
			// Only byte-length fields, so can write as a block
			this->content->write(tiles.data(), tiles.size());
			this->content->flush();
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

			auto purpose = (item.code & 0x80) ? ImagePurpose::ForegroundTileset1 : ImagePurpose::ForegroundTileset2;
			auto t = tileset.find(purpose);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			unsigned int index = item.code & 0x7F;
			auto& images = t->second->files();
			if (item.code >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = t->second->openImage(images[index]);
			if (
				(purpose == ImagePurpose::ForegroundTileset1)
				&& (index > 0)
				&& (index < 16)
			) {
				// The first 16 bytes can be special if no image is set
				auto mask = ret.img->convert_mask();
				bool completelyInvisible = true;
				for (auto& p : mask) {
					if (!(p & (uint8_t)Image::Mask::Transparent)) {
						completelyInvisible = false;
						break;
					}
				}
				if (completelyInvisible) {
					// Return a number instead
					ret.type = ImageFromCodeInfo::ImageType::HexDigit;
					ret.digit = 0x10 | index;
					return ret;
				}
			}

			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> items;
			for (unsigned int i = 0; i <= MB_MAX_VALID_FG_TILECODE; i++) {
				if (i == MB_DEFAULT_FGTILE) continue;

				Map2D::Layer::Item t;
				t.type = Map2D::Layer::Item::Type::Default;
				t.pos = {0, 0};
				t.code = i;
				items.push_back(t);
			}
			return items;
		}

	private:
		std::unique_ptr<stream::inout> content;
		unsigned long mapWidth;
		unsigned long mapHeight;
};

class Layer_Bash_Sprite: public Map2DCore::LayerCore
{
	public:
		Layer_Bash_Sprite(std::unique_ptr<stream::inout> content,
			std::unique_ptr<stream::inout> contentSGL,
			std::unique_ptr<stream::input> contentSpriteDeps)
			:	content(std::move(content)),
				contentSGL(std::move(contentSGL))
		{
			// Read the list of sprite dependencies
			auto lenSpriteDeps = contentSpriteDeps->size();
			if (lenSpriteDeps > 1048576) {
				throw stream::error("List of sprite dependencies (in XML file) is too "
					"large.");
			}
			auto depText = contentSpriteDeps->read(lenSpriteDeps);
			std::string sprite, dep;
			int state = 0;
			// Append a space so the last element always gets processed
			depText.append(1, ' ');
			for (auto& c : depText) {
				switch (state) {
					case 0:
						// Ignore any leading whitespace
						if (isspace(c)) break;
						state = 1;
						// fall through
					case 1:
						// Read sprite name
						if (c != '=') {
							sprite.append(1, c);
							break;
						}
						// Found an equals sign
						state = 2;
						break;
					case 2:
						if (!isspace(c)) {
							dep.append(1, c);
							break;
						}
						// Reached end of dep
						this->spriteDeps.insert(std::make_pair(sprite, dep));
						if (std::find(this->spriteFilenames.begin(),
								this->spriteFilenames.end(), sprite) == this->spriteFilenames.end()
						) {
							this->spriteFilenames.push_back(sprite);
						}
						if (std::find(this->spriteFilenames.begin(),
								this->spriteFilenames.end(), dep) == this->spriteFilenames.end()
						) {
							this->spriteFilenames.push_back(dep);
						}
						sprite.clear();
						dep.clear();
						state = 0;
						break;
				}
			}
			// Make sure the last element got added, which it should because we
			// always end with a space.
			assert(sprite.empty());
			assert(dep.empty());

			// Read the sprite layer
			stream::pos lenSpr = this->content->size();
			this->content->seekg(2, stream::start); // skip unknown field
			lenSpr -= 2;

			while (lenSpr > 4) {
				this->v_allItems.emplace_back();
				auto& t = this->v_allItems.back();
				t.type = Item::Type::Default;
				uint32_t lenEntry;
				uint32_t unknown1, unknown2;
				uint16_t unknown3;
				*this->content
					>> u32le(lenEntry)
					>> u32le(unknown1)
					>> u32le(unknown2)
					>> u16le(unknown3)
					>> u32le(t.pos.x)
					>> u32le(t.pos.y)
				;
				if (lenEntry > lenSpr) break; // corrupted file
				this->content->seekg(22, stream::cur); // skip padding
				std::string filename;
				*this->content >> nullPadded(filename, lenEntry - (4+4+4+2+4+4+22));
				auto it = std::find(this->spriteFilenames.begin(),
					this->spriteFilenames.end(), filename);
				if (it != this->spriteFilenames.end()) {
					t.code = BASH_SPRITE_OFFSET
						+ std::distance(this->spriteFilenames.begin(), it);
				} else {
					std::cout << "ERROR: Encounted Monster Bash sprite with unexpected "
						"name \"" << filename << "\" - unable to add to map.\n";
					t.code = 0;
				}
				lenSpr -= lenEntry;
			}
		}

		void flush(std::set<std::string>& usedSprites)
		{
			// Write the sprite layer

			// These sprites must always be present in a level
			auto extraSprites = this->spriteDeps.equal_range("*");
			for (auto i = extraSprites.first; i != extraSprites.second; i++) {
				usedSprites.insert(i->second);
			}

			auto items = this->items();

			// Figure out how much data we have to write
			stream::len lenTotal = 2;
			for (auto& i : items) {
				if (i.code < BASH_SPRITE_OFFSET) continue;
				unsigned int code = i.code - BASH_SPRITE_OFFSET;
				if (code >= this->spriteFilenames.size()) {
					std::cerr << "ERROR: Tried to write out-of-range sprite to Monster Bash map\n";
					continue;
				}
				std::string filename = this->spriteFilenames[code];
				int lenFilename = filename.length() + 2; // need two terminating nulls
				stream::len lenEntry = 4+4+4+2+4+4+22+lenFilename;
				lenTotal += lenEntry;
			}
			this->content->truncate(lenTotal);
			this->content->seekp(0, stream::start);

			// Write the signature (purpose is actually unknown)
			*this->content << u16le(0xFFFE);

			// Write the data
			for (auto& i : items) {
				if (i.code < BASH_SPRITE_OFFSET) continue;
				std::string filename = this->spriteFilenames[i.code - BASH_SPRITE_OFFSET];
				int lenFilename = filename.length() + 2; // need two terminating nulls
				uint32_t lenEntry = 4+4+4+2+4+4+22+lenFilename;
				*this->content
					<< u32le(lenEntry)
					<< u32le(0)
					<< u32le(0)
					<< u16le(0)
					<< u32le(i.pos.x)
					<< u32le(i.pos.y)
					<< nullPadded("", 22)
					<< nullPadded(filename, lenFilename);
				;

				usedSprites.insert(filename);

				// Add any dependent sprites to the list
				auto extraSprites = this->spriteDeps.equal_range(filename);
				for (auto i = extraSprites.first; i != extraSprites.second; i++) {
					usedSprites.insert(i->second);
				}
			}
			this->content->flush();
			assert(this->content->tellp() == lenTotal);

			// Write out a list of all required sprites
			this->contentSGL->truncate(usedSprites.size() * 31);
			this->contentSGL->seekp(0, stream::start);
			for (auto& i : usedSprites) {
				*this->contentSGL << nullPadded(i, 31);
			}
			this->contentSGL->flush();
			assert(this->contentSGL->tellp() == usedSprites.size() * 31);
			return;
		}

		virtual std::string title() const
		{
			return "Sprites";
		}

		virtual Caps caps() const
		{
			return Caps::HasOwnTileSize | Caps::UseImageDims;
		}

		virtual Point tileSize() const
		{
			return {1, 1};
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

			if (item.code < BASH_SPRITE_OFFSET) {
				// Unknown sprite filename
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}
			if (item.code - BASH_SPRITE_OFFSET > this->spriteFilenames.size()) {
				// Out of range somehow
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto img = this->spriteFilenames[item.code - BASH_SPRITE_OFFSET];
			auto& images = t->second->files();
			for (auto& i : images) {
				if (i->strName.compare(img) == 0) {
					auto ts = t->second->openTileset(i);
					auto frames = ts->files();
					if (frames.size() < 1) {
						// No images
						ret.type = ImageFromCodeInfo::ImageType::Unknown;
						return ret;
					}
					ret.img = ts->openImage(frames[0]);
					ret.type = ImageFromCodeInfo::ImageType::Supplied;
					return ret;
				}
			}
			std::cerr << "ERROR: Could not find image for Monster Bash sprite \""
				<< img << "\"" << std::endl;

			ret.type = ImageFromCodeInfo::ImageType::Unknown;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> items;
			for (unsigned int i = 0; i < this->spriteFilenames.size(); i++) {
				items.emplace_back();
				auto& t = items.back();
				t.type = Item::Type::Default;
				t.pos = {0, 0};
				t.code = BASH_SPRITE_OFFSET + i;
			}
			return items;
		}

	private:
		std::unique_ptr<stream::inout> content; // sprite layer
		std::unique_ptr<stream::inout> contentSGL; // sprite filename list

		/// List of sprites and which additional sprites they require (can have
		/// multiple entries for each sprite)
		std::multimap<std::string, std::string> spriteDeps;

		/// Unique list of all known sprite names
		std::vector<std::string> spriteFilenames;
};

class Layer_Bash_Attribute: virtual public Map2DCore::LayerCore
{
	public:
		Layer_Bash_Attribute(std::unique_ptr<stream::input> contentPropBG,
			std::unique_ptr<stream::input> contentPropFG,
			std::unique_ptr<stream::input> contentPropBO,
			const std::vector<uint16_t>& bgdata,
			const std::vector<uint8_t>& fgdata,
			unsigned long mapWidth, unsigned long mapHeight)
			:	mapWidth(mapWidth),
				mapHeight(mapHeight)
		{
			this->parseValues(*contentPropBG, &this->propBG);
			this->parseValues(*contentPropFG, &this->propFG);
			this->parseValues(*contentPropBO, &this->propBO);

			// Process the attributes read in from the background layer, and create
			// items in this layer for them - but only if they don't match the tile
			// attributes parsed above.
			int x = 0, y = 0;
			assert(bgdata.size() == this->mapWidth * this->mapHeight);
			assert(fgdata.size() == this->mapWidth * this->mapHeight);
			for (auto& i : bgdata) {
				unsigned int attr = i >> 9;
				unsigned int bg = i & 0x1FF;
				if (bg < this->propBG.size()) {
					// We have a tile property for this tile
					auto propStd = this->propBG[bg];
					if (propStd != attr) {
						// This tile has a non-standard attribute, see which bits are
						// different
						if (attr & 0x0F) { // blocking flags
							this->v_allItems.emplace_back();
							auto& t = this->v_allItems.back();
							t.type = Item::Type::Blocking;
							t.pos = {x, y};
							t.code = attr & 0x0F;
							t.blockingFlags = Item::BlockingFlags::Default;
							if (attr & 1) t.blockingFlags |= Item::BlockingFlags::BlockLeft;
							if (attr & 2) t.blockingFlags |= Item::BlockingFlags::BlockRight;
							if (attr & 4) t.blockingFlags |= Item::BlockingFlags::BlockTop;
							if (attr & 8) t.blockingFlags |= Item::BlockingFlags::BlockBottom;
						}
						if (attr & 16) { // point item
							this->v_allItems.emplace_back();
							auto& t = this->v_allItems.back();
							t.type = Item::Type::Flags;
							t.pos = {x, y};
							t.code = 16;
							t.generalFlags = Item::GeneralFlags::Interactive;
						}
						if (attr & 32) { // slanted tile
							this->v_allItems.emplace_back();
							auto& t = this->v_allItems.back();
							t.type = Item::Type::Blocking;
							t.pos = {x, y};
							t.code = 32;
							t.blockingFlags = Item::BlockingFlags::Default;
							t.blockingFlags |= Item::BlockingFlags::Slant45;
						}
						if (attr & 64) { // ladder
							this->v_allItems.emplace_back();
							auto& t = this->v_allItems.back();
							t.type = Item::Type::Movement;
							t.pos = {x, y};
							t.code = 64;
							t.movementFlags = Item::MovementFlags::DistanceLimit;
							t.movementDistLeft = 0;
							t.movementDistRight = 0;
							t.movementDistUp = Item::DistIndeterminate;
							t.movementDistDown = Item::DistIndeterminate;
						}
					}
				} // else attribute is standard, ignore

				x++;
				if (x >= (signed long)this->mapWidth) {
					x = 0;
					y++;
				}
			}
		}

		void parseValues(stream::input& content, std::vector<uint8_t>* values)
		{
			auto len = content.size();
			if (len > 1048576) throw stream::error("Tile property data (<content/> in XML for tile properties) too large.");
			std::vector<char> data(len, 0);
			char *d = data.data();
			char *end = d;
			content.read(d, len);
			do {
				d = end;
				errno = 0;
				auto val = strtoul(d, &end, 16);
				if (errno) {
					throw stream::error("Error parsing tileinfo content - ensure this "
						"part of the XML file contains hex digits and whitespace only!");
				}
				values->push_back(val);
			} while (end != d);
			return;
		}

		// Run through the tile properties for the layers, and update
		// the flags in the BG layer as needed
		void populate(std::vector<uint16_t>* bgdata, std::vector<uint8_t>* fgdata)
		{
			// Retrieve the attribute tiles
			auto lenLayer = this->mapWidth * this->mapHeight;
			std::vector<uint8_t> atdata(lenLayer, 0);
			for (auto& i : this->items()) {
				if (
					(i.pos.x >= (signed long)this->mapWidth)
					|| (i.pos.y >= (signed long)this->mapHeight)
				) {
					throw stream::error("Attribute layer has tiles outside map boundary!");
				}
				// Multiple tiles can go in the same spot, so combine them
				atdata[i.pos.y * this->mapWidth + i.pos.x] |= i.code;
			}

			// Merge everything together
			const auto sizeBG = this->propBG.size();
			const auto sizeFG = this->propFG.size();
			const auto sizeBO = this->propBO.size();
			auto bg = bgdata->data();
			auto fg = fgdata->data();
			auto at = atdata.data();
			while (lenLayer-- > 0) {
				if (*at) {
					// There is an item in the attribute layer, so this trumps all the
					// standard codes.
					*bg |= *at << 9;
				} else {
					// bg has no flags set yet, so just here it's a raw tilecode
					if (*bg < sizeBG) {
						*bg |= propBG[*bg] << 9;
					}
					// bg now has flags set, so it's no longer a raw tilecode

					// Combine the value from the foreground layer
					auto& propFGBO = (*fg & 0x80) ? propFG : propBO;
					auto& sizeFGBO = (*fg & 0x80) ? sizeFG : sizeBO;
					if ((*fg & 0x7F) < sizeFGBO) {
						// Set background flags from foreground tile (not a typo, we want to
						// update *bg)
						*bg |= propFGBO[*fg & 0x7F] << 9;
					}
				}
				bg++;
				fg++;
				at++;
			}
			return;
		}

		virtual std::string title() const
		{
			return "Attributes";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;
			ret.type = ImageFromCodeInfo::ImageType::Blank;
			return ret;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> items;
			for (unsigned int i = 0; i < 16; i++) {
				items.emplace_back();
				auto& t = items.back();
				t.type = Item::Type::Blocking;
				t.pos = {0, 0};
				t.code = i;
				t.blockingFlags = Item::BlockingFlags::Default;
				if (i & 1) t.blockingFlags |= Item::BlockingFlags::BlockLeft;
				if (i & 2) t.blockingFlags |= Item::BlockingFlags::BlockRight;
				if (i & 4) t.blockingFlags |= Item::BlockingFlags::BlockTop;
				if (i & 8) t.blockingFlags |= Item::BlockingFlags::BlockBottom;
			}
			{
				// Interactive (point) item
				items.emplace_back();
				auto& t = items.back();
				t.type = Item::Type::Flags;
				t.pos = {0, 0};
				t.code = 16;
				t.generalFlags = Item::GeneralFlags::Interactive;
			}
			{
				items.emplace_back();
				auto& t = items.back();
				t.type = Item::Type::Blocking;
				t.pos = {0, 0};
				t.code = 32;
				t.blockingFlags = Item::BlockingFlags::Default;
				t.blockingFlags |= Item::BlockingFlags::Slant45;
			}
			{
				// Ladder
				items.emplace_back();
				auto& t = items.back();
				t.type = Item::Type::Movement;
				t.pos = {0, 0};
				t.code = 64;
				t.movementFlags = Item::MovementFlags::DistanceLimit;
				t.movementDistLeft = 0;
				t.movementDistRight = 0;
				t.movementDistUp = Item::DistIndeterminate;
				t.movementDistDown = Item::DistIndeterminate;
			}
			return items;
		}

	private:
		std::vector<uint8_t> propBG, propFG, propBO;
		unsigned long mapWidth;
		unsigned long mapHeight;
};

class Map_Bash: public MapCore, public Map2DCore
{
	public:
		Map_Bash(
			std::unique_ptr<stream::inout> contentInf,
			std::unique_ptr<stream::inout> contentBG,
			std::unique_ptr<stream::inout> contentFG,
			std::unique_ptr<stream::inout> contentSP,
			std::unique_ptr<stream::inout> contentSGL,
			std::unique_ptr<stream::input> contentPropBG,
			std::unique_ptr<stream::input> contentPropFG,
			std::unique_ptr<stream::input> contentPropBO,
			std::unique_ptr<stream::input> contentSpriteDeps
		)
			:	content(std::move(contentInf))
		{
			assert(this->content);

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
			this->content->seekg(0, stream::start);
			for (unsigned int i = 0; i < MB_NUM_ATTRIBUTES; i++) {
				this->attr.emplace_back();
				auto& attr = this->attr.back();
				attr.type = Attribute::Type::Filename;
				attr.name = attrNames[i];
				attr.desc = attrDesc[i];
				*this->content >> nullPadded(attr.filenameValue, 31);
				if (attr.filenameValue.compare("UNNAMED") == 0) {
					attr.filenameValue.clear();
				} else {
					// Add the fake extension
					if (
						(!attr.filenameValue.empty())
						&& (i != MB_ATTR_KEEP_EXT) // need to keep .snd extension
					) {
						attr.filenameValue += ".";
						attr.filenameValue += validTypes[i];
					}
				}
				attr.filenameValidExtension = validTypes[i];
			}

			std::vector<uint16_t> bgdata;
			std::vector<uint8_t> fgdata;

			// Read each layer
			auto layerBG = std::make_shared<Layer_Bash_Background>(
				std::move(contentBG),
				&this->mapWidth,
				&this->mapHeight,
				&bgdata
			);
			this->v_layers.push_back(layerBG);

			this->v_layers.push_back(
				std::make_shared<Layer_Bash_Foreground>(
					std::move(contentFG),
					this->mapWidth,
					this->mapHeight,
					&fgdata
				)
			);

			this->v_layers.push_back(
				std::make_shared<Layer_Bash_Attribute>(
					std::move(contentPropBG),
					std::move(contentPropFG),
					std::move(contentPropBO),
					bgdata,
					fgdata,
					this->mapWidth,
					this->mapHeight
				)
			);

			this->v_layers.push_back(
				std::make_shared<Layer_Bash_Sprite>(
					std::move(contentSP),
					std::move(contentSGL),
					std::move(contentSpriteDeps)
				)
			);
		}

		virtual ~Map_Bash()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {
				// Background tiles
				std::make_pair(
					ImagePurpose::BackgroundTileset1,
					GraphicsFilename{this->attr[0].filenameValue, "tls-bash-bg"}
				),
				// Foreground tiles
				std::make_pair(
					ImagePurpose::ForegroundTileset1,
					GraphicsFilename{this->attr[1].filenameValue, "tls-bash-fg"}
				),
				// Bonus tiles
				std::make_pair(
					ImagePurpose::ForegroundTileset2,
					GraphicsFilename{this->attr[2].filenameValue, "tls-bash-fg"}
				),
			};
		}

		virtual void flush()
		{
			// Write map info file
			assert(this->attr.size() == MB_NUM_ATTRIBUTES);
			this->content->seekp(0, stream::start);
			std::string val;
			unsigned int i = 0;
			for (auto& attr : this->attributes()) {
				if (attr.filenameValue.empty()) {
					val = "UNNAMED";
				} else {
					auto dot = attr.filenameValue.find_last_of('.');
					if (
						(i != MB_ATTR_KEEP_EXT) // need to keep .snd extension
						&& (dot != std::string::npos)
						&& (attr.filenameValue.substr(dot + 1).compare(validTypes[i]) == 0)
					) {
						// Extension matches, remove it
						val = attr.filenameValue.substr(0, dot);
					} else {
						val = attr.filenameValue; // don't chop off extension
					}
				}
				*this->content << nullPadded(val, 31);
				i++;
			}

			auto lenLayer = this->mapWidth * this->mapHeight;
			std::set<std::string> usedSprites;

			auto layerBG = dynamic_cast<Layer_Bash_Background*>(this->v_layers[0].get());
			auto layerFG = dynamic_cast<Layer_Bash_Foreground*>(this->v_layers[1].get());
			auto layerAT = dynamic_cast<Layer_Bash_Attribute*>(this->v_layers[2].get());
			auto layerSP = dynamic_cast<Layer_Bash_Sprite*>(this->v_layers[3].get());

			// Populate the background data.  We can't write it yet as it contains
			// flags which might be changed by tiles in the the foreground layer.
			std::vector<uint16_t> bgdata(lenLayer, MB_DEFAULT_BGTILE);
			layerBG->populate(&bgdata);

			// Populate the foreground data.  We can't write this yet either, as the
			// background data must be written first.
			std::vector<uint8_t> fgdata(lenLayer, MB_DEFAULT_FGTILE);
			layerFG->populate(&fgdata, &usedSprites);

			// Run through the tile properties for the foreground layer, and update
			// the flags in the BG layer as needed
			layerAT->populate(&bgdata, &fgdata);

			// Now write the data to the underlying files
			layerBG->flush(bgdata);
			layerFG->flush(fgdata);
			layerSP->flush(usedSprites);

			return;
		}

		virtual Caps caps() const
		{
			return
				Map2D::Caps::HasViewport
				| Map2D::Caps::HasMapSize
				| Map2D::Caps::HasTileSize
			;
		}

		virtual Point viewport() const
		{
			return {320, 200};
		}

		virtual Point mapSize() const
		{
			return {(signed long)this->mapWidth, (signed long)this->mapHeight};
		}

		virtual Point tileSize() const
		{
			return {MB_TILE_WIDTH, MB_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			Background bg;
			bg.att = Background::Attachment::SingleColour;
			bg.clr = gamegraphics::PaletteEntry{0, 0, 0, 255};
			return bg;
		}

	private:
		std::unique_ptr<stream::inout> content;
		unsigned long mapWidth;
		unsigned long mapHeight;
};


std::string MapType_Bash::code() const
{
	return "map2d-bash";
}

std::string MapType_Bash::friendlyName() const
{
	return "Monster Bash level";
}

std::vector<std::string> MapType_Bash::fileExtensions() const
{
	return {"mif"};
}

std::vector<std::string> MapType_Bash::games() const
{
	return {
		"Monster Bash",
		"Realms of Chaos (beta)",
		"Scubaventure",
	};
}

MapType::Certainty MapType_Bash::isInstance(stream::input& content) const
{
	bool maybe = false;
	stream::len len = content.size();

	// Make sure the file is large enough...
	// TESTED BY: fmt_map_bash_isinstance_c01
	if (len < 187) return MapType::DefinitelyNo;

	// ...but not too large.
	// TESTED BY: fmt_map_bash_isinstance_c02
	if (len > 217) return MapType::DefinitelyNo;

	uint8_t data[218]; // 7*31+1
	memset(data, 0, sizeof(data));
	uint8_t *d = data;
	content.seekg(0, stream::start);
	content.read(d, len);
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

std::unique_ptr<Map> MapType_Bash::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_Bash::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	auto suppBG = suppData.find(SuppItem::Layer1);
	auto suppFG = suppData.find(SuppItem::Layer2);
	auto suppSP = suppData.find(SuppItem::Layer3);
	auto suppSGL = suppData.find(SuppItem::Extra1);
	auto suppPropBG = suppData.find(SuppItem::Extra2);
	auto suppPropFG = suppData.find(SuppItem::Extra3);
	auto suppPropBO = suppData.find(SuppItem::Extra4);
	auto suppDepsSP = suppData.find(SuppItem::Extra5);
	if (suppBG == suppData.end()) throw stream::error("Missing content for Layer1 (background) supplementary item.");
	if (suppFG == suppData.end()) throw stream::error("Missing content for Layer2 (foreground) supplementary item.");
	if (suppSP == suppData.end()) throw stream::error("Missing content for Layer3 (sprite) supplementary item.");
	if (suppSGL == suppData.end()) throw stream::error("Missing content for Extra1 (sprite list) supplementary item.");
	if (suppPropBG == suppData.end()) throw stream::error("Missing content for Extra2 (background tile properties) supplementary item.");
	if (suppPropFG == suppData.end()) throw stream::error("Missing content for Extra3 (foreground tile properties) supplementary item.");
	if (suppPropBO == suppData.end()) throw stream::error("Missing content for Extra4 (bonus tile properties) supplementary item.");
	if (suppDepsSP == suppData.end()) throw stream::error("Missing content for Extra5 (full sprite list) supplementary item.");
	return std::make_unique<Map_Bash>(
		std::move(content),
		std::move(suppBG->second),
		std::move(suppFG->second),
		std::move(suppSP->second),
		std::move(suppSGL->second),
		std::move(suppPropBG->second),
		std::move(suppPropFG->second),
		std::move(suppPropBO->second),
		std::move(suppDepsSP->second)
	);
}

SuppFilenames MapType_Bash::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	SuppFilenames supps;
	std::string baseName = filename.substr(0, filename.length() - 3);
	supps[SuppItem::Layer1] = baseName + "mbg";
	supps[SuppItem::Layer2] = baseName + "mfg";
	supps[SuppItem::Layer3] = baseName + "msp";

	content.seekg(31*3, stream::start);
	std::string sgl;
	content >> nullPadded(sgl, 31);
	supps[SuppItem::Extra1] = sgl + ".sgl";

	// These filenames aren't part of the game, but are extra data we need
	// to make editing the maps managable.
	supps[SuppItem::Extra2] = baseName + "xbg";
	supps[SuppItem::Extra3] = baseName + "xfg";
	supps[SuppItem::Extra4] = baseName + "xbn";
	supps[SuppItem::Extra5] = baseName + "xsp";
	return supps;
}

} // namespace gamemaps
} // namespace camoto
