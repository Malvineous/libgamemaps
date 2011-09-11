/**
 * @file   gamemap.cpp
 * @brief  Command-line interface to libgamemaps.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/program_options.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/bind.hpp>
#include <camoto/gamegraphics.hpp>
#include <camoto/gamemaps.hpp>
#include <camoto/util.hpp>
#include <iostream>
#include <fstream>
#include "png++/png.hpp"

namespace po = boost::program_options;
namespace gm = camoto::gamemaps;
namespace gg = camoto::gamegraphics;

#define PROGNAME "gamemap"

/// Return value: All is good
#define RET_OK                 0
/// Return value: Bad arguments (missing/invalid parameters)
#define RET_BADARGS            1
/// Return value: Major error (couldn't open map file, etc.)
#define RET_SHOWSTOPPER        2
/// Return value: More info needed (-t auto didn't work, specify a type)
#define RET_BE_MORE_SPECIFIC   3
/// Return value: One or more files failed, probably user error (file not found, etc.)
#define RET_NONCRITICAL_FAILURE 4
/// Return value: Some files failed, but not in a common way (cut off write, disk full, etc.)
#define RET_UNCOMMON_FAILURE   5

/// Place to cache tiles when rendering a map to a .png file
struct CachedTile {
	unsigned int code;
	gg::StdImageDataPtr data;
	gg::StdImageDataPtr mask;
	unsigned int width;
	unsigned int height;
};

/// Open a tileset.
/**
 * @param filename
 *   File to open.
 *
 * @param type
 *   File type if it can't be autodetected.
 *
 * @return Shared pointer to the tileset.
 *
 * @throw std::ios::failure on error
 */
gg::TilesetPtr openTileset(const std::string& filename, const std::string& type)
	throw (std::ios::failure)
{
	gg::ManagerPtr pManager(gg::getManager());

	boost::shared_ptr<std::fstream> psTileset(new std::fstream());
	psTileset->exceptions(std::ios::badbit | std::ios::failbit);
	try {
		psTileset->open(filename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
	} catch (std::ios::failure& e) {
		std::cerr << "Error opening " << filename << std::endl;
		throw std::ios::failure("Unable to open tileset");
	}

	gg::TilesetTypePtr pGfxType;
	if (type.empty()) {
		// Need to autodetect the file format.
		gg::TilesetTypePtr pTestType;
		int i = 0;
		while ((pTestType = pManager->getTilesetType(i++))) {
			gg::TilesetType::Certainty cert = pTestType->isInstance(psTileset);
			switch (cert) {
				case gg::TilesetType::DefinitelyNo:
					break;
				case gg::TilesetType::Unsure:
					// If we haven't found a match already, use this one
					if (!pGfxType) pGfxType = pTestType;
					break;
				case gg::TilesetType::PossiblyYes:
					// Take this one as it's better than an uncertain match
					pGfxType = pTestType;
					break;
				case gg::TilesetType::DefinitelyYes:
					pGfxType = pTestType;
					// Don't bother checking any other formats if we got a 100% match
					goto finishTesting;
			}
		}
finishTesting:
		if (!pGfxType) {
			std::cerr << "Unable to automatically determine the graphics file "
				"type.  Use the --graphicstype option to manually specify the file "
				"format." << std::endl;
			throw std::ios::failure("Unable to open tileset");
		}
	} else {
		gg::TilesetTypePtr pTestType(pManager->getTilesetTypeByCode(type));
		if (!pTestType) {
			std::cerr << "Unknown file type given to -y/--graphicstype: " << type
				<< std::endl;
			throw std::ios::failure("Unable to open tileset");
		}
		pGfxType = pTestType;
	}

	assert(pGfxType != NULL);

	// See if the format requires any supplemental files
	camoto::SuppFilenames suppList = pGfxType->getRequiredSupps(filename);
	camoto::SuppData suppData;
	if (suppList.size() > 0) {
		for (camoto::SuppFilenames::iterator i = suppList.begin(); i != suppList.end(); i++) {
			try {
				boost::shared_ptr<std::fstream> suppStream(new std::fstream());
				suppStream->exceptions(std::ios::badbit | std::ios::failbit);
				std::cout << "Opening supplemental file " << i->second << std::endl;
				suppStream->open(i->second.c_str(), std::ios::in | std::ios::out | std::ios::binary);
				camoto::SuppItem si;
				si.stream = suppStream;
				si.fnTruncate = boost::bind<void>(camoto::truncateFromString, i->second, _1);
				suppData[i->first] = si;
			} catch (std::ios::failure e) {
				std::cerr << "Error opening supplemental file " << i->second.c_str() << std::endl;
				throw std::ios::failure("Unable to open tileset");
			}
		}
	}

	// Open the graphics file
	camoto::FN_TRUNCATE fnTruncate =
		boost::bind<void>(camoto::truncateFromString, filename, _1);
	gg::TilesetPtr pTileset(pGfxType->open(psTileset, fnTruncate, suppData));
	assert(pTileset);

	return pTileset;
}

/// Export a map to .png file
/**
 * Convert the given map into a PNG file on disk, by rendering the map as it
 * would appear in the game.
 *
 * @param map
 *   Map file to export.
 *
 * @param gfxFile
 *   Filename of the tileset to use.
 *
 * @param destFile
 *   Filename of destination (including ".png")
 *
 * @throw std::ios::failure on error
 */
void map2dToPng(gm::Map2DPtr map, gg::TilesetPtr tileset,
	const std::string& destFile)
	throw (std::ios::failure)
{
	int mapCaps = map->getCaps();
	int outWidth, outHeight; // in pixels
	if (mapCaps & gm::Map2D::HasGlobalSize) {
		map->getMapSize(&outWidth, &outHeight);
	} else {
		outWidth = outHeight = 0;
		int layerCount = map->getLayerCount();
		for (int i = 0; i < layerCount; i++) {
			gm::Map2D::LayerPtr layer = map->getLayer(i);
			int layerCaps = layer->getCaps();
			if (layerCaps & gm::Map2D::Layer::HasOwnSize) {
				int layerX, layerY;
				layer->getLayerSize(&layerX, &layerY);
				// Convert to pixels if not already
				if (layerCaps & gm::Map2D::Layer::HasOwnTileSize) {
					int x, y;
					layer->getTileSize(&x, &y);
					layerX *= x;
					layerY *= y;
				} else if (mapCaps & gm::Map2D::HasGlobalTileSize) {
					int x, y;
					map->getTileSize(&x, &y);
					layerX *= x;
					layerY *= y;
				} // else no grids at all, already in pixels
				if (layerX > outWidth) outWidth = layerX;
				if (layerY > outHeight) outHeight = layerY;
			}
		}
	}

	png::image<png::index_pixel> png(outWidth, outHeight);

	bool useMask;
	if (tileset->getCaps() & gg::Tileset::HasPalette) {
		gg::PaletteTablePtr srcPal = tileset->getPalette();
		png::palette pal(srcPal->size());
		int j = 0;
		//pal[ 0] = png::color(0xFF, 0x00, 0xFF); // transparent
		for (gg::PaletteTable::iterator i = srcPal->begin();
			i != srcPal->end();
			i++, j++
		) {
			pal[j] = png::color(i->red, i->green, i->blue);
		}
		png.set_palette(pal);
		useMask = false; // not enough room in the palette for transparent entry
	} else {
		// standard EGA palette
		png::palette pal(17);
		pal[ 0] = png::color(0xFF, 0x00, 0xFF); // transparent
		pal[ 1] = png::color(0x00, 0x00, 0x00);
		pal[ 2] = png::color(0x00, 0x00, 0xAA);
		pal[ 3] = png::color(0x00, 0xAA, 0x00);
		pal[ 4] = png::color(0x00, 0xAA, 0xAA);
		pal[ 5] = png::color(0xAA, 0x00, 0x00);
		pal[ 6] = png::color(0xAA, 0x00, 0xAA);
		pal[ 7] = png::color(0xAA, 0x55, 0x00);
		pal[ 8] = png::color(0xAA, 0xAA, 0xAA);
		pal[ 9] = png::color(0x55, 0x55, 0x55);
		pal[10] = png::color(0x55, 0x55, 0xFF);
		pal[11] = png::color(0x55, 0xFF, 0x55);
		pal[12] = png::color(0x55, 0xFF, 0xFF);
		pal[13] = png::color(0xFF, 0x55, 0x55);
		pal[14] = png::color(0xFF, 0x55, 0xFF);
		pal[15] = png::color(0xFF, 0xFF, 0x55);
		pal[16] = png::color(0xFF, 0xFF, 0xFF);
		png.set_palette(pal);
		useMask = true;

		// Make first colour transparent
		png::tRNS transparency;
		transparency.push_back(0);
		png.set_tRNS(transparency);
	}

	int layerCount = map->getLayerCount();
	for (int layerIndex = 0; layerIndex < layerCount; layerIndex++) {
		gm::Map2D::LayerPtr layer = map->getLayer(layerIndex);

		// Figure out the layer size (in tiles) and the tile size
		int layerWidth, layerHeight;
		int tileWidth, tileHeight;
		if (!getLayerDims(map, layer, &layerWidth, &layerHeight, &tileWidth,
			&tileHeight)
		) {
			std::cout << "Warning: Layer " << layerIndex + 1 << " has no dimensions, and "
				"neither does the map!  Skipping layer." << std::endl;
			continue;
		}

		// Prepare tileset
		std::vector<CachedTile> cache;
		const gg::Tileset::VC_ENTRYPTR& allTiles = tileset->getItems();

		// Run through all items in the layer and render them one by one
		const gm::Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		gm::Map2D::Layer::ItemPtrVector::const_iterator t = items->begin();
		int numItems = items->size();
		if (t != items->end()) {
			CachedTile thisTile;
			for (int y = 0; y < layerHeight; y++) {
				for (int x = 0; x < layerWidth; x++) {
					for (int i = 0; i < numItems; i++) {
						if (t == items->end()) t = items->begin();
						if (((*t)->x == x) && ((*t)->y == y)) break;
						t++;
					}
					if (((*t)->x == x) && ((*t)->y == y)) {
						// Found tile at this location

						// TODO: Move this mapping code into a per-format class
						int tileCode = (*t)->code;
						if (tileCode >= allTiles.size()) tileCode = 0;

						// Find the cached tile
						bool found = false;
						for (std::vector<CachedTile>::iterator ct = cache.begin();
							ct != cache.end(); ct++
						) {
							if (ct->code == tileCode) {
								thisTile = *ct;
								found = true;
								break;
							}
						}
						if (!found) {
							// Tile hasn't been cached yet, load it from the tileset
							gg::ImagePtr img = tileset->openImage(allTiles[tileCode]);
							thisTile.data = img->toStandard();
							thisTile.mask = img->toStandardMask();
							img->getDimensions(&thisTile.width, &thisTile.height);
							thisTile.code = tileCode;
							cache.push_back(thisTile);
						}

						// Draw tile onto png
						int offX = x * tileWidth;
						int offY = y * tileHeight;
						for (int tY = 0; tY < thisTile.height; tY++) {
							int pngY = offY+tY;
							if (pngY >= outHeight) break; // don't write past image edge
							for (int tX = 0; tX < thisTile.width; tX++) {
								int pngX = offX+tX;
								if (pngX >= outWidth) break; // don't write past image edge
								//png[offY + tY][offX + tX] = png::index_pixel(((*t)->code % 16) + 1);
								// Only write opaque pixels
								if (((thisTile.mask[tY*thisTile.width+tX] & 0x01) == 0) ||
									((!useMask) && (layerIndex == 0))
								) {
									png[pngY][pngX] =
										// +1 to the colour to skip over transparent (#0)
										png::index_pixel(thisTile.data[tY*thisTile.width+tX] + (useMask ? 1 : 0));
								} else {
									if (layerIndex == 0) {
										assert(useMask); // just to be sure my logic is right!
										png[pngY][pngX] = png::index_pixel(0);
									} // else let higher layers see through to lower ones
								}
							}
						}

					} // else no tile at all at this position!
				}
			}
		} // else layer is empty
	}

	png.write(destFile);
	return;
}

int main(int iArgC, char *cArgV[])
{
	// Set a better exception handler
	std::set_terminate( __gnu_cxx::__verbose_terminate_handler );

	// Disable stdin/printf/etc. sync for a speed boost
	std::ios_base::sync_with_stdio(false);

	// Declare the supported options.
	po::options_description poActions("Actions");
	poActions.add_options()
		("info,i",
			"display information about the map")

		("print,p", po::value<int>(),
			"print the given layer in ASCII")

		("render,r", po::value<std::string>(),
			"render the map to the given .png file")
	;

	po::options_description poOptions("Options");
	poOptions.add_options()
		("type,t", po::value<std::string>(),
			"specify the map type (default is autodetect)")
		("graphics,g", po::value<std::string>(),
			"filename storing game graphics (required with --render)")
		("graphicstype,y", po::value<std::string>(),
			"specify format of file passed with --graphics")
		("script,s",
			"format output suitable for script parsing")
		("force,f",
			"force open even if the map is not in the given format")
		("list,l",
			"list supported file types")
	;

	po::options_description poHidden("Hidden parameters");
	poHidden.add_options()
		("map", "map file to manipulate")
		("help", "produce help message")
	;

	po::options_description poVisible("");
	poVisible.add(poActions).add(poOptions);

	po::options_description poComplete("Parameters");
	poComplete.add(poActions).add(poOptions).add(poHidden);
	po::variables_map mpArgs;

	std::string strFilename, strType;
	std::string strGraphics, strGraphicsType;

	// Get the format handler for this file format
	gm::ManagerPtr pManager(gm::getManager());

	bool bScript = false; // show output suitable for script parsing?
	bool bForceOpen = false; // open anyway even if map not in given format?
	int iRet = RET_OK;
	try {
		po::parsed_options pa = po::parse_command_line(iArgC, cArgV, poComplete);

		// Parse the global command line options
		for (std::vector<po::option>::iterator i = pa.options.begin(); i != pa.options.end(); i++) {
			if (i->string_key.empty()) {
				// If we've already got an map filename, complain that a second one
				// was given (probably a typo.)
				if (!strFilename.empty()) {
					std::cerr << "Error: unexpected extra parameter (multiple map "
						"filenames given?!)" << std::endl;
					return 1;
				}
				assert(i->value.size() > 0);  // can't have no values with no name!
				strFilename = i->value[0];
			} else if (i->string_key.compare("help") == 0) {
				std::cout <<
					"Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>\n"
					"This program comes with ABSOLUTELY NO WARRANTY.  This is free software,\n"
					"and you are welcome to change and redistribute it under certain conditions;\n"
					"see <http://www.gnu.org/licenses/> for details.\n"
					"\n"
					"Utility to manipulate map files used by games to store data files.\n"
					"Build date " __DATE__ " " __TIME__ << "\n"
					"\n"
					"Usage: gamemap <map> <action> [action...]\n" << poVisible << "\n"
					<< std::endl;
				return RET_OK;
			} else if (
				(i->string_key.compare("t") == 0) ||
				(i->string_key.compare("type") == 0)
			) {
				strType = i->value[0];
			} else if (
				(i->string_key.compare("g") == 0) ||
				(i->string_key.compare("graphics") == 0)
			) {
				strGraphics = i->value[0];
			} else if (
				(i->string_key.compare("y") == 0) ||
				(i->string_key.compare("graphicstype") == 0)
			) {
				strGraphicsType = i->value[0];
			} else if (
				(i->string_key.compare("s") == 0) ||
				(i->string_key.compare("script") == 0)
			) {
				bScript = true;
			} else if (
				(i->string_key.compare("f") == 0) ||
				(i->string_key.compare("force") == 0)
			) {
				bForceOpen = true;
			} else if (
				(i->string_key.compare("l") == 0) ||
				(i->string_key.compare("list") == 0)
			) {
				std::cout << "Map types: (--type)\n";
				int i = 0;
				{
					gm::MapTypePtr nextType;
					while ((nextType = pManager->getMapType(i++))) {
						std::string code = nextType->getMapCode();
						std::cout << "  " << code;
						int len = code.length();
						if (len < 20) std::cout << std::string(20-code.length(), ' ');
						std::cout << ' ' << nextType->getFriendlyName();
						std::vector<std::string> ext = nextType->getFileExtensions();
						if (ext.size()) {
							std::vector<std::string>::iterator i = ext.begin();
							std::cout << " (*." << *i;
							for (i++; i != ext.end(); i++) {
								std::cout << "; *." << *i;
							}
							std::cout << ")";
						}
						std::cout << '\n';
					}
				}

				std::cout << "\nMap tilesets: (--graphicstype)\n";
				i = 0;
				gg::ManagerPtr pManager(gg::getManager());
				{
					gg::TilesetTypePtr nextType;
					while ((nextType = pManager->getTilesetType(i++))) {
						std::string code = nextType->getCode();
						std::cout << "  " << code;
						int len = code.length();
						if (len < 20) std::cout << std::string(20-code.length(), ' ');
						std::cout << ' ' << nextType->getFriendlyName();
						std::vector<std::string> ext = nextType->getFileExtensions();
						if (ext.size()) {
							std::vector<std::string>::iterator i = ext.begin();
							std::cout << " (*." << *i;
							for (i++; i != ext.end(); i++) {
								std::cout << "; *." << *i;
							}
							std::cout << ")";
						}
						std::cout << '\n';
					}
				}
				return RET_OK;
			}
		}

		if (strFilename.empty()) {
			std::cerr << "Error: no game map filename given" << std::endl;
			return RET_BADARGS;
		}
		std::cout << "Opening " << strFilename << " as type "
			<< (strType.empty() ? "<autodetect>" : strType) << std::endl;

		boost::shared_ptr<std::fstream> psMap(new std::fstream());
		psMap->exceptions(std::ios::badbit | std::ios::failbit);
		try {
			psMap->open(strFilename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
		} catch (std::ios::failure& e) {
			std::cerr << "Error opening " << strFilename << std::endl;
			#ifdef DEBUG
				std::cerr << "e.what(): " << e.what() << std::endl;
			#endif
			return RET_SHOWSTOPPER;
		}

		gm::MapTypePtr pMapType;
		if (strType.empty()) {
			// Need to autodetect the file format.
			gm::MapTypePtr pTestType;
			int i = 0;
			while ((pTestType = pManager->getMapType(i++))) {
				gm::MapType::Certainty cert = pTestType->isInstance(psMap);
				switch (cert) {
					case gm::MapType::DefinitelyNo:
						// Don't print anything (TODO: Maybe unless verbose?)
						break;
					case gm::MapType::Unsure:
						std::cout << "File could be a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getMapCode() << "]" << std::endl;
						// If we haven't found a match already, use this one
						if (!pMapType) pMapType = pTestType;
						break;
					case gm::MapType::PossiblyYes:
						std::cout << "File is likely to be a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getMapCode() << "]" << std::endl;
						// Take this one as it's better than an uncertain match
						pMapType = pTestType;
						break;
					case gm::MapType::DefinitelyYes:
						std::cout << "File is definitely a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getMapCode() << "]" << std::endl;
						pMapType = pTestType;
						// Don't bother checking any other formats if we got a 100% match
						goto finishTesting;
				}
				if (cert != gm::MapType::DefinitelyNo) {
					// We got a possible match, see if it requires any suppdata
					camoto::SuppFilenames suppList = pTestType->getRequiredSupps(strFilename);
					if (suppList.size() > 0) {
						// It has suppdata, see if it's present
						std::cout << "  * This format requires supplemental files..." << std::endl;
						bool bSuppOK = true;
						for (camoto::SuppFilenames::iterator i = suppList.begin(); i != suppList.end(); i++) {
							try {
								boost::shared_ptr<std::fstream> suppStream(new std::fstream());
								suppStream->exceptions(std::ios::badbit | std::ios::failbit);
								suppStream->open(i->second.c_str(), std::ios::in | std::ios::binary);
							} catch (std::ios::failure e) {
								bSuppOK = false;
								std::cout << "  * Could not find/open " << i->second
									<< ", map is probably not "
									<< pTestType->getMapCode() << std::endl;
								break;
							}
						}
						if (bSuppOK) {
							// All supp files opened ok
							std::cout << "  * All supp files present, map is likely "
								<< pTestType->getMapCode() << std::endl;
							// Set this as the most likely format
							pMapType = pTestType;
						}
					}
				}
			}
finishTesting:
			if (!pMapType) {
				std::cerr << "Unable to automatically determine the file type.  Use "
					"the --type option to manually specify the file format." << std::endl;
				return RET_BE_MORE_SPECIFIC;
			}
		} else {
			gm::MapTypePtr pTestType(pManager->getMapTypeByCode(strType));
			if (!pTestType) {
				std::cerr << "Unknown file type given to -t/--type: " << strType
					<< std::endl;
				return RET_BADARGS;
			}
			pMapType = pTestType;
		}

		assert(pMapType != NULL);

		// Check to see if the file is actually in this format
		if (!pMapType->isInstance(psMap)) {
			if (bForceOpen) {
				std::cerr << "Warning: " << strFilename << " is not a "
					<< pMapType->getFriendlyName() << ", open forced." << std::endl;
			} else {
				std::cerr << "Invalid format: " << strFilename << " is not a "
					<< pMapType->getFriendlyName() << "\n"
					<< "Use the -f option to try anyway." << std::endl;
				return RET_BE_MORE_SPECIFIC;
			}
		}

		// See if the format requires any supplemental files
		camoto::SuppFilenames suppList = pMapType->getRequiredSupps(strFilename);
		camoto::SuppData suppData;
		if (suppList.size() > 0) {
			for (camoto::SuppFilenames::iterator i = suppList.begin(); i != suppList.end(); i++) {
				try {
					boost::shared_ptr<std::fstream> suppStream(new std::fstream());
					suppStream->exceptions(std::ios::badbit | std::ios::failbit);
					std::cout << "Opening supplemental file " << i->second << std::endl;
					suppStream->open(i->second.c_str(), std::ios::in | std::ios::out | std::ios::binary);
					camoto::SuppItem si;
					si.stream = suppStream;
					si.fnTruncate = boost::bind<void>(camoto::truncateFromString, i->second, _1);
					suppData[i->first] = si;
				} catch (std::ios::failure e) {
					std::cerr << "Error opening supplemental file " << i->second.c_str() << std::endl;
					#ifdef DEBUG
						std::cerr << "e.what(): " << e.what() << std::endl;
					#endif
					return RET_SHOWSTOPPER;
				}
			}
		}

		// Open the map file
		//FN_TRUNCATE fnTruncate = boost::bind<void>(truncate, strFilename.c_str(), _1);
		gm::MapPtr pMap(pMapType->open(psMap, suppData));
		assert(pMap);

		// File type of inserted files defaults to empty, which means 'generic file'
		std::string strLastFiletype;

		// Last attribute value set with -b
		int iLastAttr;

		// Run through the actions on the command line
		for (std::vector<po::option>::iterator i = pa.options.begin(); i != pa.options.end(); i++) {
			if (i->string_key.compare("info") == 0) {
				std::cout << (bScript ? "attribute_count=" : "Number of attributes: ");
				gm::Map::AttributePtrVectorPtr attributes = pMap->getAttributes();
				std::cout << attributes->size() << "\n";
				int attrNum = 0;
				for (gm::Map::AttributePtrVector::const_iterator i = attributes->begin();
					i != attributes->end(); i++
				) {
					if (bScript) std::cout << "attribute" << attrNum << "_name=";
					else std::cout << "Attribute " << attrNum+1 << ": ";
					std::cout << (*i)->name << "\n";

					if (bScript) std::cout << "attribute" << attrNum << "_desc=";
					else std::cout << "  Description: ";
					std::cout << (*i)->desc << "\n";

					if (bScript) std::cout << "attribute" << attrNum << "_type=";
					else std::cout << "  Type: ";
					switch ((*i)->type) {

						case gm::Map::Attribute::Integer: {
							std::cout << (bScript ? "int" : "Integer value") << "\n";
							gm::Map::IntAttribute *a =
								dynamic_cast<gm::Map::IntAttribute *>(i->get());
							assert(a);

							if (bScript) std::cout << "attribute" << attrNum << "_value=";
							else std::cout << "  Current value: ";
							std::cout << a->value << "\n";

							if (bScript) {
								std::cout << "attribute" << attrNum << "_min=" << a->minValue
									<< "\nattribute" << attrNum << "_max=" << a->maxValue;
							} else {
								std::cout << "  Range: ";
								if ((a->minValue == 0) && (a->maxValue == 0)) {
									std::cout << "[unlimited]";
								} else {
									std::cout << a->minValue << " to " << a->maxValue;
								}
							}
							std::cout << "\n";
							break;
						}

						case gm::Map::Attribute::Enum: {
							std::cout << (bScript ? "enum" : "Item from list") << "\n";
							gm::Map::EnumAttribute *a =
								dynamic_cast<gm::Map::EnumAttribute *>(i->get());
							assert(a);

							if (bScript) std::cout << "attribute" << attrNum << "_value=";
							else std::cout << "  Current value: ";
							if (a->value > a->values.size()) {
								std::cout << (bScript ? "error" : "[out of range]");
							} else {
								if (bScript) std::cout << a->value;
								else std::cout << "[" << a->value << "] " << a->values[a->value];
							}
							std::cout << "\n";

							if (bScript) std::cout << "attribute" << attrNum
								<< "_choice_count=" << a->values.size() << "\n";

							int option = 0;
							for (std::vector<std::string>::const_iterator i = a->values.begin();
								i != a->values.end(); i++
							) {
								if (bScript) {
									std::cout << "attribute" << attrNum << "_choice" << option
										<< "=";
								} else {
									std::cout << "  Allowed value " << option << ": ";
								}
								std::cout << *i << "\n";
								option++;
							}
							break;
						}
						default:
							std::cout << (bScript ? "unknown" : "Unknown type (fix this!)");
							break;
					}
					attrNum++;
				}
				std::cout << (bScript ? "map_type=" : "Map type: ");
				gm::Map2DPtr map2d = boost::dynamic_pointer_cast<gm::Map2D>(pMap);
				if (map2d) {
					std::cout << (bScript ? "2d" : "2D grid-based") << "\n";
#define CAP(o, c, v)        " " __STRING(c) << ((v & o::c) ? '+' : '-')
#define MAP2D_CAP(c)        CAP(gm::Map2D,        c, mapCaps)
#define MAP2D_LAYER_CAP(c)  CAP(gm::Map2D::Layer, c, layerCaps)

					int mapCaps = map2d->getCaps();
					if (bScript) {
						std::cout << "map_caps=" << mapCaps << "\n";
					} else {
						std::cout << "Map capabilities:"
							<< MAP2D_CAP(HasGlobalSize)
							<< MAP2D_CAP(CanResize)
							<< MAP2D_CAP(HasGlobalTileSize)
							<< MAP2D_CAP(ChangeTileSize)
							<< MAP2D_CAP(HasViewport)
							<< "\n"
						;
					}
					if (mapCaps & gm::Map2D::HasGlobalTileSize) {
						int x, y;
						map2d->getTileSize(&x, &y);
						std::cout << (bScript ? "tile_width=" : "Tile size: ") << x
							<< (bScript ? "tile_height=" : "x") << y << "\n";
					}
					if (mapCaps & gm::Map2D::HasGlobalSize) {
						int x, y;
						map2d->getMapSize(&x, &y);
						std::cout
							<< (bScript ? "map_width=" : "Map size: ") << x
							<< (bScript ? "\nmap_height=" : "x") << y
							<< (bScript ? "\nmap_units=" : " ")
							<< ((mapCaps & gm::Map2D::HasGlobalTileSize) ? "tiles" : "pixels")
							<< "\n";
					}
					if (mapCaps & gm::Map2D::HasViewport) {
						int x, y;
						map2d->getViewport(&x, &y);
						std::cout << (bScript ? "viewport_width=" : "Viewport size: ") << x
							<< (bScript ? "viewport_height=" : "x") << y << "\n";
					}

					int layerCount = map2d->getLayerCount();
					std::cout << (bScript ? "layercount=" : "Layer count: ")
						<< layerCount << "\n";
					for (int i = 0; i < layerCount; i++) {
						gm::Map2D::LayerPtr layer = map2d->getLayer(i);
						std::string prefix;
						if (bScript) {
							std::stringstream ss;
							ss << "layer" << i << '_';
							prefix = ss.str();
							std::cout << prefix << "name=" << layer->getTitle() << "\n";
						} else {
							prefix = "  ";
							std::cout << "Layer " << i + 1 << ": \"" << layer->getTitle()
								<< "\"\n";
						}
						int layerCaps = layer->getCaps();
						if (bScript) std::cout << prefix << "caps=" << layerCaps << "\n";
						else std::cout << prefix << "Capabilities:"
							<< MAP2D_LAYER_CAP(HasOwnSize)
							<< MAP2D_LAYER_CAP(CanResize)
							<< MAP2D_LAYER_CAP(HasOwnTileSize)
							<< MAP2D_LAYER_CAP(ChangeTileSize)
							<< "\n"
						;
						if (layerCaps & gm::Map2D::Layer::HasOwnTileSize) {
							int x, y;
							layer->getTileSize(&x, &y);
							std::cout << prefix << (bScript ? "tile_width=" : "Tile size: ") << x;
							if (bScript) std::cout << "\n" << prefix << "tile_height=";
							else std::cout << "x";
							std::cout << y << "\n";
						}
						if (layerCaps & gm::Map2D::Layer::HasOwnSize) {
							int x, y;
							layer->getLayerSize(&x, &y);
							std::cout << prefix << (bScript ? "width=" : "Layer size: ") << x;
							if (bScript) std::cout << "\n" << prefix << "height=";
							else std::cout << "x";
							std::cout << y << "\n";
						} else {
							// Layer doesn't have own size, use map
							if (mapCaps & gm::Map2D::HasGlobalSize) {
								int x, y;
								map2d->getMapSize(&x, &y);
								if (layerCaps & gm::Map2D::Layer::HasOwnTileSize) {
									// The layer is the same size as the map, but it has a
									// different tile size.

									if (mapCaps & gm::Map2D::HasGlobalTileSize) {
										// The map also has a tile size, so multiply it out to get
										// dimensions in pixels (which they are if the map doesn't
										// have a global tile size.)
										int mtx, mty;
										map2d->getTileSize(&mtx, &mty);
										x *= mtx;
										y *= mty;
									}

									// Convert the global map size (in pixels) to this layer's
									// size (in tiles)
									int tx, ty;
									layer->getTileSize(&tx, &ty);
									std::cout << prefix << (bScript ? "width=" : "Layer size: ") << x / tx;
									if (bScript) std::cout << "\n" << prefix << "height=";
									else std::cout << "x";
									std::cout << y / ty << "\n";
								} else {
									if (bScript) std::cout << prefix << "width=map_width\n"
										<< prefix << "height=map_height\n";
									else std::cout << prefix << "Layer size: Same as map\n";
								}
							} else {
								// Both map and layer size have no dimensions!
								if (bScript) std::cout << prefix << "width=-1\n"
									<< prefix << "height=-1\n";
								else std::cout << prefix << "Layer size: Empty\n";
							}
						}
					}

				} else {
					std::cout << (bScript ? "unknown" : "Unknown!  Fix this!") << "\n";
				}

			} else if (i->string_key.compare("print") == 0) {
				gm::Map2DPtr map2d = boost::dynamic_pointer_cast<gm::Map2D>(pMap);
				if (map2d) {
					int targetLayer = strtoul(i->value[0].c_str(), NULL, 10);
					if (targetLayer == 0) {
						std::cerr << "Invalid layer index passed to --print.  Use --info "
							"to list layers in this map." << std::endl;
						iRet = RET_BADARGS;
						continue;
					}
					int layerCount = map2d->getLayerCount();
					if (targetLayer > layerCount) {
						std::cerr << "Invalid layer index passed to --print.  Use --info "
							"to list layers in this map." << std::endl;
						iRet = RET_BADARGS;
						continue;
					}

					gm::Map2D::LayerPtr layer = map2d->getLayer(targetLayer - 1);

					// Figure out the layer size
					int layerWidth, layerHeight;
					int tileWidth, tileHeight;
					if (!getLayerDims(map2d, layer, &layerWidth, &layerHeight, &tileWidth,
							&tileHeight)
					) {
						std::cout << "ERROR: Layer has no dimensions, and neither does "
							"the map!" << std::endl;
						iRet = RET_SHOWSTOPPER;
						continue;
					}
					std::cout << layerWidth << "," << layerHeight << std::endl;

					const gm::Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
					gm::Map2D::Layer::ItemPtrVector::const_iterator t = items->begin();
					int numItems = items->size();
					if (t != items->end()) {
						for (int y = 0; y < layerHeight; y++) {
							for (int x = 0; x < layerWidth; x++) {
								for (int i = 0; i < numItems; i++) {
									if (((*t)->x == x) && ((*t)->y == y)) break;
									t++;
									if (t == items->end()) t = items->begin();
								}
								if (((*t)->x != x) || ((*t)->y != y)) {
									// Grid position with no tile!
									std::cout << ' ';
								} else {
									std::cout << (char)(' ' + ((*t)->code % (126-32)));
								}
							}
							std::cout << "\n";
						}
					} else {
						std::cout << "Layer is empty!" << std::endl;
					}

				} else {
					std::cerr << "Support for printing this map type has not yet "
						"been implemented!" << std::endl;
				}

			} else if (i->string_key.compare("render") == 0) {
				if (strGraphics.empty()) {
					std::cerr << "You must use --graphics to specify a tileset."
						<< std::endl;
					iRet = RET_BADARGS;
					continue;
				}
				// Don't need to check i->value[0], program_options does that for us

				gm::Map2DPtr map2d = boost::dynamic_pointer_cast<gm::Map2D>(pMap);
				if (map2d) {
					gg::TilesetPtr tileset = openTileset(strGraphics, strGraphicsType);
					map2dToPng(map2d, tileset, i->value[0]);
				}

			// Ignore --type/-t
			} else if (i->string_key.compare("type") == 0) {
			} else if (i->string_key.compare("t") == 0) {
			// Ignore --script/-s
			} else if (i->string_key.compare("script") == 0) {
			} else if (i->string_key.compare("s") == 0) {
			// Ignore --force/-f
			} else if (i->string_key.compare("force") == 0) {
			} else if (i->string_key.compare("f") == 0) {

			}
		} // for (all command line elements)
		//pMap->flush();
	} catch (po::error& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return RET_BADARGS;
	} catch (std::ios::failure& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return RET_SHOWSTOPPER;
	}

	return iRet;
}
