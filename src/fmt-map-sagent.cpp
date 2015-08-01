/**
 * @file  fmt-map-sagent.cpp
 * @brief MapType and Map2D implementation for Secret Agent levels.
 *
 * This file format is fully documented on the ModdingWiki:
 *   http://www.shikadi.net/moddingwiki/Secret_Agent
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
#include "fmt-map-sagent.hpp"

#define SAM_MAP_WIDTH            40 // not including CRLF
#define SAM_MAP_WIDTH_BYTES      42 // including CRLF

#define SAM_TILE_WIDTH           16
#define SAM_TILE_HEIGHT          16

/// Size of each map file, in bytes.
#define SAM_MAP_FILESIZE       2016

/// Maximum number of rows in a level with no foreground data.
#define SAM_MAX_ROWS (SAM_MAP_FILESIZE / SAM_MAP_WIDTH_BYTES - 2) // 2 == two header rows

/// This is the largest valid tile code in the background layer.
#define SAM_MAX_VALID_TILECODE   0xfb

/// This is the largest valid tile code in the world map.
#define SAM_MAX_VALID_TILECODE_WORLD   0x7a

/// Tile code that means "no tile here"
#define SAMT_EMPTY 0x20

/// Create a tile number from a tileset number and an index into the tileset.
#define MAKE_TILE(tileset, tile) (((tileset) << 8) | (tile))
#define ST(tileset, tile) (((tileset) << 8) | (tile))
#define __________ (-1)

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

typedef struct {
	uint8_t code;
	int tiles[4 * 3];
} TILE_MAP;

#include "fmt-map-sagent-mapping.hpp"

class Layer_SAgent_Common: public Map2DCore::LayerCore
{
	public:
		Layer_SAgent_Common()
		{
		}

		virtual ~Layer_SAgent_Common()
		{
		}

		virtual ImageFromCodeInfo imageFromCode(const Item& item,
			const TilesetCollection& tileset) const
		{
			ImageFromCodeInfo ret;

			unsigned int ti, i;
			ti = item.code >> 8;
			i = item.code & 0xFF;

			auto t = tileset.find(ImagePurpose::BackgroundTileset1);
			if (t == tileset.end()) { // no tileset?!
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto& subtilesets = t->second->files();
			if (ti >= subtilesets.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			auto subtileset = t->second->openTileset(subtilesets[ti]);
			auto& images = subtileset->files();
			if (i >= images.size()) { // out of range
				ret.type = ImageFromCodeInfo::ImageType::Unknown;
				return ret;
			}

			ret.img = subtileset->openImage(images[i]);
			ret.type = ImageFromCodeInfo::ImageType::Supplied;
			return ret;
		}
};

class Layer_SAgent_Background: public Layer_SAgent_Common
{
	public:
		Layer_SAgent_Background(unsigned int tileBG, const TILE_MAP *tm)
			:	tileBG(tileBG),
				tm(tm)
		{
		}

		virtual ~Layer_SAgent_Background()
		{
		}

		virtual std::string title() const
		{
			return "Background";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual std::vector<Item> availableItems() const
		{
			std::vector<Item> validItems;

			for (unsigned int i = 1; i < 4; i++) {
				// Add the background tiles
				validItems.emplace_back();
				auto& t = validItems.back();
				t.type = Map2D::Layer::Item::Type::Default;
				t.pos = {0, 0};
				t.code = this->tileBG + i;
			}
			for (const TILE_MAP *next = this->tm; next->code > 0; next++) {
				for (unsigned int i = 0; i < 4 * 3; i++) {
					if (next->tiles[i] >= 0) {
						validItems.emplace_back();
						auto& t = validItems.back();
						t.type = Map2D::Layer::Item::Type::Default;
						t.pos = {0, 0};
						t.code = next->tiles[i];
					}
				}
			}
			return validItems;
		}

	private:
		unsigned int tileBG; ///< Index of current background tile
		const TILE_MAP *tm;
};

class Layer_SAgent_Foreground: public Layer_SAgent_Common
{
	public:
		Layer_SAgent_Foreground(const TILE_MAP *tm)
			:	tm(tm)
		{
		}

		virtual ~Layer_SAgent_Foreground()
		{
		}

		virtual std::string title() const
		{
			return "Foreground";
		}

		virtual Caps caps() const
		{
			return Caps::Default;
		}

		virtual std::vector<Item> availableItems() const
		{
#warning TODO: These tiles are not all valid here, only include ones that work (or perhaps only include ones where the background layer is not null in that cell)
			std::vector<Item> validItems;

			for (const TILE_MAP *next = this->tm; next->code > 0; next++) {
				for (unsigned int i = 0; i < 4 * 3; i++) {
					if (next->tiles[i] >= 0) {
						validItems.emplace_back();
						auto& t = validItems.back();
						t.type = Map2D::Layer::Item::Type::Default;
						t.pos = {0, 0};
						t.code = next->tiles[i];
					}
				}
			}
			return validItems;
		}

		virtual bool tilePermittedAt(const Map2D::Layer::Item& item,
			const Point& pos, unsigned int *maxCount) const
		{
			if (pos.x == 0) return false; // can't place tiles in this column
			return true; // otherwise unrestricted
		}

	private:
		const TILE_MAP *tm;
};

class Map_SAgent: public MapCore, public Map2DCore
{
	public:
		Map_SAgent(std::unique_ptr<stream::inout> content, bool isWorldMap)
			:	content(std::move(content)),
				isWorldMap(isWorldMap)
		{
			stream::pos lenMap = this->content->size();
			this->mapHeight = lenMap / (SAM_MAP_WIDTH + 2);

			// Read the background layer
			this->content->seekg(0, stream::start);
			std::vector<uint8_t> bgdata(lenMap, SAMT_EMPTY);
			this->content->read(bgdata.data(), lenMap);

			// Read the background code
			unsigned int bgcode = strtod((char *)bgdata.data(), NULL);
			auto bg = bgdata.data();
			bg += SAM_MAP_WIDTH + 2; // skip full first line (it only has the BG code)
			bg += SAM_MAP_WIDTH + 2; // skip full second line (not displayed in the game)

			unsigned int bgtile;
			{
				this->attr.emplace_back();
				Attribute& attr = this->attr.back();
				attr.type = Attribute::Type::Enum;
				attr.name = "Background tile";
				attr.desc = "Default tile to use as level background";
#warning TODO: Make this an image list
				switch (bgcode) {
					case 667: bgtile = MAKE_TILE( 6, 16); attr.enumValue = 0; break;
					case 695: bgtile = MAKE_TILE( 6, 44); attr.enumValue = 1; break;
					case 767: bgtile = MAKE_TILE( 8, 16); attr.enumValue = 2; break;
					case 771: bgtile = MAKE_TILE( 8, 20); attr.enumValue = 3; break;
					case 325: bgtile = MAKE_TILE( 9, 24); attr.enumValue = 4; break;
					case 329: bgtile = MAKE_TILE( 9, 28); attr.enumValue = 5; break;
					case 333: bgtile = MAKE_TILE( 9, 32); attr.enumValue = 6; break;
					case 337: bgtile = MAKE_TILE( 9, 36); attr.enumValue = 7; break;
					case 341: bgtile = MAKE_TILE( 9, 40); attr.enumValue = 8; break;
					case 209: bgtile = MAKE_TILE(11,  8); attr.enumValue = 9; break;
					case 213: bgtile = MAKE_TILE(11, 12); attr.enumValue = 10; break;
					case 217: bgtile = MAKE_TILE(11, 16); attr.enumValue = 11; break;
					case 233: bgtile = MAKE_TILE(11, 32); attr.enumValue = 12; break;
					case 237: bgtile = MAKE_TILE(11, 36); attr.enumValue = 13; break;
					case 241: bgtile = MAKE_TILE(11, 40); attr.enumValue = 14; break;
					case 245: bgtile = MAKE_TILE(11, 44); attr.enumValue = 15; break;
					case 501: bgtile = MAKE_TILE( 1,  0); attr.enumValue = 16; break;
					default: bgtile = MAKE_TILE( 6, 16); attr.enumValue = 0; break;
				}
				attr.enumValueNames.push_back("Blue sky/grey tiles");
				attr.enumValueNames.push_back("Grey tiles");
				attr.enumValueNames.push_back("Grey stone");
				attr.enumValueNames.push_back("Blue brick");
				attr.enumValueNames.push_back("Blue diamonds");
				attr.enumValueNames.push_back("Red stone");
				attr.enumValueNames.push_back("Night sky/grey tiles");
				attr.enumValueNames.push_back("Grey/spare 1");
				attr.enumValueNames.push_back("Grey/spare 2");
				attr.enumValueNames.push_back("Red/grey tiles");
				attr.enumValueNames.push_back("Grey pattern");
				attr.enumValueNames.push_back("Blue");
				attr.enumValueNames.push_back("Diagonal red brick");
				attr.enumValueNames.push_back("Grey with white line");
				attr.enumValueNames.push_back("Blue dirt");
				attr.enumValueNames.push_back("Overlapping red squares");
				attr.enumValueNames.push_back("Grass");
			}

			const TILE_MAP *tm = this->isWorldMap ? worldMap : tileMap;

			auto layerBG = std::make_shared<Layer_SAgent_Background>(bgtile, tm);
			auto layerFG = std::make_shared<Layer_SAgent_Foreground>(tm);

			this->v_layers.push_back(layerBG);
			this->v_layers.push_back(layerFG);

			auto& tilesBG = layerBG->items();
			auto& tilesFG = layerFG->items();
			auto tiles = &tilesBG;

			tilesBG.reserve(SAM_MAP_WIDTH * this->mapHeight);

			for (unsigned int y = 0; y < this->mapHeight; y++) {
				// If the row starts with a '*' then the rest of the row goes to the FG layer
				if (*bg == 0x2A) {
					y--; this->mapHeight--;
					tiles = &tilesFG;
				} else if ((bg[SAM_MAP_WIDTH] != 0x0D) || (bg[SAM_MAP_WIDTH+1] != 0x0A)) {
					// First blank line signals the end of the map
					this->mapHeight = y;
					if (this->mapHeight == 0) throw stream::error("Map height is zero");
					break;
				} else {
					tiles = &tilesBG;
				}
				for (unsigned int x = 0; x < SAM_MAP_WIDTH; x++, bg++) {
					int code = -1;
					switch (*bg) {
						case SAMT_EMPTY: continue; // empty space
						case 0x35: code = bgtile + 1; break; // Light shadow left
						case 0x36: code = bgtile + 2; break; // Light shadow mid
						case 0x37: code = bgtile + 3; break; // Light shadow right

							// '*' fg layer marker, valid when x>0
						case 0x2A: if (x == 0) continue; // else fall through
						default:
							// Look in the tile map
							for (unsigned int i = 0; i < sizeof(tileMap) / sizeof(TILE_MAP); i++) {
								const TILE_MAP& m = tm[i];
								if (*bg == m.code) {
									for (unsigned int dy = 0; dy < 3; dy++) {
										for (unsigned int dx = 0; dx < 4; dx++) {
											int code = m.tiles[dy * 4 + dx];
											if (code >= 0) {
												tiles->emplace_back();
												auto& t = tiles->back();
												t.type = Layer::Item::Type::Default;
												t.pos = {x - (3 - dx), y - (2 - dy)};
												t.code = code;
											}
										}
									}
									break;
								}
							}
							break;
					}
					if (code >= 0) {
						// There's a tile from the first list (not from the tileMap) so add that
						tiles->emplace_back();
						auto& t = tiles->back();
						t.type = Layer::Item::Type::Default;
						t.pos = {x, y};
						t.code = code;
					}
				}
				bg += 2; // skip CRLF
			}
		}

		virtual ~Map_SAgent()
		{
		}

		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames() const
		{
			return {
				std::make_pair(
					ImagePurpose::BackgroundTileset1,
					GraphicsFilename{"cc1.gfx", "tls-ccaves-main"}
				),
			};
		}

		virtual void flush()
		{
			assert(this->layers().size() == 2);

			auto mapSize = this->mapSize();

			if (mapSize.x != SAM_MAP_WIDTH)
				throw stream::error("This map format can only be "
					TOSTRING(SAM_MAP_WIDTH) " cells wide.");

			if (mapSize.y > SAM_MAX_ROWS)
				throw stream::error("This map format must be less than "
					"46"/*TOSTRING(SAM_MAX_ROWS)*/ " cells tall.");

			if (this->attr.size() != 1) {
				throw stream::error("Cannot write map as there is an incorrect number "
					"of attributes set.");
			}

			unsigned int lenMap = mapSize.x * mapSize.y;

			// Extract the tile codes into a big array so it's easier to cross reference
			std::vector<int> bgsrc(lenMap, -1);
			std::vector<int> fgsrc(lenMap, -1);

			for (const auto& i : this->v_layers[0]->items()) {
				if ((i.pos.x >= mapSize.x) || (i.pos.y >= mapSize.y)) {
					throw stream::error("Background layer has tiles outside map boundary!");
				}
				auto pos = i.pos.y * mapSize.x + i.pos.x;
				bgsrc[pos] = i.code;
			}

			for (const auto& i : this->v_layers[1]->items()) {
				if ((i.pos.x >= mapSize.x) || (i.pos.y >= mapSize.y)) {
					throw stream::error("Foreground layer has tiles outside map boundary!");
				}
				auto pos = i.pos.y * mapSize.x + i.pos.x;
				fgsrc[pos] = i.code;
			}

			// Convert our codes into SAM ones
			std::vector<uint8_t> bgdst(SAM_MAP_WIDTH * SAM_MAX_ROWS, SAMT_EMPTY);
			std::vector<uint8_t> fgdst(SAM_MAP_WIDTH * SAM_MAX_ROWS, SAMT_EMPTY);
			unsigned int lineCount = 0; // one per BG line, one per FG line

			int *inbg = bgsrc.data();
			int *infg = fgsrc.data();
			uint8_t *outbg = bgdst.data();
			uint8_t *outfg = fgdst.data();

			auto tm = this->isWorldMap ? worldMap : tileMap;
			bool fgRowValid[SAM_MAX_ROWS];
			for (unsigned int y = 0; y < SAM_MAX_ROWS; y++) {
				if (y >= mapSize.y) break;

				fgRowValid[y] = false;
				for (unsigned int x = 0; x < SAM_MAP_WIDTH; x++,
					inbg++, outbg++, infg++, outfg++
				) {

					// Skip this cell if there is no tile in either layer
					if ((*inbg == -1) && (*infg == -1)) continue;

					bool foundBG = false;
					bool foundFG = false;

					// Check lights
					switch (*inbg) {
						// Normal background tile that made it in somehow
						case MAKE_TILE( 6, 16):
						case MAKE_TILE( 6, 44):
						case MAKE_TILE( 8, 16):
						case MAKE_TILE( 8, 20):
						case MAKE_TILE( 9, 24):
						case MAKE_TILE( 9, 28):
						case MAKE_TILE( 9, 32):
						case MAKE_TILE( 9, 36):
						case MAKE_TILE( 9, 40):
						case MAKE_TILE(11,  8):
						case MAKE_TILE(11, 12):
						case MAKE_TILE(11, 16):
						case MAKE_TILE(11, 32):
						case MAKE_TILE(11, 36):
						case MAKE_TILE(11, 40):
						case MAKE_TILE(11, 44):
							//case MAKE_TILE( 1,  0):
							*inbg = -1; // don't process it below
						foundBG = true;
						*outbg = 0x20;
						break;
						case MAKE_TILE( 6, 16 + 1):
						case MAKE_TILE( 6, 44 + 1):
						case MAKE_TILE( 8, 16 + 1):
						case MAKE_TILE( 8, 20 + 1):
						case MAKE_TILE( 9, 24 + 1):
						case MAKE_TILE( 9, 28 + 1):
						case MAKE_TILE( 9, 32 + 1):
						case MAKE_TILE( 9, 36 + 1):
						case MAKE_TILE( 9, 40 + 1):
						case MAKE_TILE(11,  8 + 1):
						case MAKE_TILE(11, 12 + 1):
						case MAKE_TILE(11, 16 + 1):
						case MAKE_TILE(11, 32 + 1):
						case MAKE_TILE(11, 36 + 1):
						case MAKE_TILE(11, 40 + 1):
						case MAKE_TILE(11, 44 + 1):
							//case MAKE_TILE( 1,  0 + 1):
							*inbg = -1; // don't process it below
						foundBG = true;
						*outbg = 0x35;
						break;
						case MAKE_TILE( 6, 16 + 2):
						case MAKE_TILE( 6, 44 + 2):
						case MAKE_TILE( 8, 16 + 2):
						case MAKE_TILE( 8, 20 + 2):
						case MAKE_TILE( 9, 24 + 2):
						case MAKE_TILE( 9, 28 + 2):
						case MAKE_TILE( 9, 32 + 2):
						case MAKE_TILE( 9, 36 + 2):
						case MAKE_TILE( 9, 40 + 2):
						case MAKE_TILE(11,  8 + 2):
						case MAKE_TILE(11, 12 + 2):
						case MAKE_TILE(11, 16 + 2):
						case MAKE_TILE(11, 32 + 2):
						case MAKE_TILE(11, 36 + 2):
						case MAKE_TILE(11, 40 + 2):
						case MAKE_TILE(11, 44 + 2):
							//case MAKE_TILE( 1,  0 + 2):
							*inbg = -1; // don't process it below
						foundBG = true;
						*outbg = 0x36;
						break;
						case MAKE_TILE( 6, 16 + 3):
						case MAKE_TILE( 6, 44 + 3):
						case MAKE_TILE( 8, 16 + 3):
						case MAKE_TILE( 8, 20 + 3):
						case MAKE_TILE( 9, 24 + 3):
						case MAKE_TILE( 9, 28 + 3):
						case MAKE_TILE( 9, 32 + 3):
						case MAKE_TILE( 9, 36 + 3):
						case MAKE_TILE( 9, 40 + 3):
						case MAKE_TILE(11,  8 + 3):
						case MAKE_TILE(11, 12 + 3):
						case MAKE_TILE(11, 16 + 3):
						case MAKE_TILE(11, 32 + 3):
						case MAKE_TILE(11, 36 + 3):
						case MAKE_TILE(11, 40 + 3):
						case MAKE_TILE(11, 44 + 3):
							//case MAKE_TILE( 1,  0 + 3):
							*inbg = -1; // don't process it below
						foundBG = true;
						*outbg = 0x37;
						break;
					}

					// Check other tiles
					for (const TILE_MAP *tnext = tm; tnext->code > 0; tnext++) {
						if ((!foundBG) && (*inbg == tnext->tiles[4 * 3 - 1])) {
							// TODO: Check surrounding area?
							*outbg = tnext->code;
							if (foundFG) break;
							foundBG = true;
						}
						if ((!foundFG) && (*infg == tnext->tiles[4 * 3 - 1])) {
							// TODO: Check surrounding area?
							*outfg = tnext->code;
							fgRowValid[y] = true; // remember to write out this row later
							if (foundBG) break;
							foundFG = true;
						}
					}
				}
				lineCount++; // background layer
				if (fgRowValid[y]) lineCount++; // foreground layer
			}

			if (lineCount > SAM_MAX_ROWS) {
				throw stream::error(createString("Too many rows with foreground tiles.  "
						"You need to remove all foreground tiles from "
						<< (lineCount - SAM_MAX_ROWS) << " row(s) in order to save the level."));
			}

			auto& attrBG = this->attr[0];
			if (attrBG.type != Attribute::Type::Enum) {
				throw stream::error("Cannot write map as there is an attribute of the "
					"wrong type (bg != enum)");
			}
			int bgcode;
			switch (attrBG.enumValue) {
				case 0: bgcode = 667; break;
				case 1: bgcode = 695; break;
				case 2: bgcode = 767; break;
				case 3: bgcode = 771; break;
				case 4: bgcode = 325; break;
				case 5: bgcode = 329; break;
				case 6: bgcode = 333; break;
				case 7: bgcode = 337; break;
				case 8: bgcode = 341; break;
				case 9: bgcode = 209; break;
				case 10: bgcode = 213; break;
				case 11: bgcode = 217; break;
				case 12: bgcode = 233; break;
				case 13: bgcode = 237; break;
				case 14: bgcode = 241; break;
				case 15: bgcode = 245; break;
				case 16: bgcode = 501; break;
				default: bgcode = 667; break;
			}

			// Write out the map
			this->content->truncate(SAM_MAP_FILESIZE);
			this->content->seekp(0, stream::start);
			outbg = bgdst.data();
			outfg = fgdst.data();

			std::string strBGcode = createString(bgcode);
			strBGcode += std::string(SAM_MAP_WIDTH - strBGcode.length(), ' ');
			strBGcode.append("\x0D\x0A");
			assert(strBGcode.length() == SAM_MAP_WIDTH + 2);
			this->content->write(strBGcode);

			*this->content
				<< u8(0x20) // unknown
				<< u8(0x20) // background overlay
				<< u8(0x20) // unknown
				<< u8(0x33) // tile 0x33 image?
				<< u8(0x35) // tile 0x35 image
				<< u8(0x36) // tile 0x36 image
				<< u8(0x37) // tile 0x37 image
			;
			*this->content << std::string(SAM_MAP_WIDTH - 7, ' ') << u8(0x0D) << u8(0x0A);
			unsigned int numLinesWritten = 2;
			for (unsigned int y = 0; y < SAM_MAX_ROWS; y++) {
				if (y >= mapSize.y) {
					// Past the end of the map, pad out with nulls
					*this->content << nullPadded("", SAM_MAP_FILESIZE - numLinesWritten * SAM_MAP_WIDTH_BYTES);
					break;
				}
				this->content->write(outbg, SAM_MAP_WIDTH);
				*this->content << u8(0x0D) << u8(0x0A);
				numLinesWritten++;
				if (fgRowValid[y]) {
					*outfg = 0x2A; // Override first char with '*'
					this->content->write(outfg, SAM_MAP_WIDTH);
					*this->content << u8(0x0D) << u8(0x0A);
					numLinesWritten++;
				}
				outbg += SAM_MAP_WIDTH;
				outfg += SAM_MAP_WIDTH;
			}
			assert(this->content->tellp() == SAM_MAP_FILESIZE);
			return;
		}

		virtual Caps caps() const
		{
			return
				Caps::HasViewport
				| Caps::HasMapSize
				| Caps::HasTileSize
			;
		}

		virtual Point viewport() const
		{
			return {320, 192};
		}

		virtual Point mapSize() const
		{
			return {SAM_MAP_WIDTH, this->mapHeight};
		}

		virtual Point tileSize() const
		{
			return {SAM_TILE_WIDTH, SAM_TILE_HEIGHT};
		}

		Background background(const TilesetCollection& tileset) const
		{
			return this->backgroundFromTilecode(tileset, SAMT_EMPTY);
		}

	private:
		std::unique_ptr<stream::inout> content;
		bool isWorldMap;
		unsigned int mapHeight;
};

std::string MapType_SAgent::code() const
{
	if (this->isWorldMap()) {
		return "map2d-sagent-world";
	}
	return "map2d-sagent";
}

std::string MapType_SAgent::friendlyName() const
{
	if (this->isWorldMap()) {
		return "Secret Agent world map";
	}
	return "Secret Agent level";
}

std::vector<std::string> MapType_SAgent::fileExtensions() const
{
	return {"sam"};
}

std::vector<std::string> MapType_SAgent::games() const
{
	return {"Secret Agent"};
}

MapType::Certainty MapType_SAgent::isInstance(stream::input& content) const
{
	stream::pos lenMap = content.size();

	// TESTED BY: fmt_map_sagent_isinstance_c01
	if (lenMap != SAM_MAP_FILESIZE) return MapType::DefinitelyNo; // too small

	bool worldMap = this->isWorldMap();

	// Skip first row
	content.seekg(SAM_MAP_WIDTH_BYTES, stream::start);

	uint8_t row[SAM_MAP_WIDTH_BYTES];
	unsigned int y;
	for (y = 0; (y < SAM_MAP_FILESIZE / SAM_MAP_WIDTH_BYTES - 1) && lenMap; y++) {
		// Ensure the row data is valid
		content.read(row, SAM_MAP_WIDTH_BYTES);
		for (unsigned int x = 0; x < SAM_MAP_WIDTH; x++) {
			// Invalid tile
			// TESTED BY: fmt_map_sagent_isinstance_c02
			if (row[x] > SAM_MAX_VALID_TILECODE) return MapType::DefinitelyNo;
			if ((worldMap) && (row[x] > SAM_MAX_VALID_TILECODE_WORLD))
				return MapType::DefinitelyNo;
		}

		lenMap -= SAM_MAP_WIDTH_BYTES;
		// No CRLF at end of each row
		// TESTED BY: fmt_map_sagent_isinstance_c03
		if (
			(
				(row[SAM_MAP_WIDTH_BYTES - 2] != 0x0D)
				|| (row[SAM_MAP_WIDTH_BYTES - 1] != 0x0A)
			) && (
				(row[SAM_MAP_WIDTH_BYTES - 2] != 0x00)
				|| (row[SAM_MAP_WIDTH_BYTES - 1] != 0x00)
			)
		) {
			return MapType::DefinitelyNo;
		}
	}

	// This is an assert because the logic above should never allow this to happen
	// (the for loop doesn't iterate enough times.)
	assert(y <= SAM_MAX_ROWS + 1);

	// TESTED BY: fmt_map_sagent_isinstance_c00
	return MapType::DefinitelyYes;
}

std::unique_ptr<Map> MapType_SAgent::create(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	// TODO: Implement
	throw stream::error("Not implemented yet!");
}

std::unique_ptr<Map> MapType_SAgent::open(
	std::unique_ptr<stream::inout> content, SuppData& suppData) const
{
	return std::make_unique<Map_SAgent>(std::move(content), this->isWorldMap());
}

SuppFilenames MapType_SAgent::getRequiredSupps(stream::input& content,
	const std::string& filename) const
{
	return {};
}

bool MapType_SAgent::isWorldMap() const
{
	return false;
}


bool MapType_SAgentWorld::isWorldMap() const
{
	return true;
}

} // namespace gamemaps
} // namespace camoto
