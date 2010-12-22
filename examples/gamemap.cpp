/**
 * @file   gamemap.cpp
 * @brief  Command-line interface to libgamemaps.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/gamemaps.hpp>
#include <iostream>
#include <fstream>

namespace po = boost::program_options;
namespace gm = camoto::gamemaps;

#define PROGNAME "gamemap"

/*** Return values ***/
// All is good
#define RET_OK                 0
// Bad arguments (missing/invalid parameters)
#define RET_BADARGS            1
// Major error (couldn't open map file, etc.)
#define RET_SHOWSTOPPER        2
// More info needed (-t auto didn't work, specify a type)
#define RET_BE_MORE_SPECIFIC   3
// One or more files failed, probably user error (file not found, etc.)
#define RET_NONCRITICAL_FAILURE 4
// Some files failed, but not in a common way (cut off write, disk full, etc.)
#define RET_UNCOMMON_FAILURE   5

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
	;

	po::options_description poOptions("Options");
	poOptions.add_options()
		("type,t", po::value<std::string>(),
			"specify the map type (default is autodetect)")
		("script,s",
			"format output suitable for script parsing")
		("force,f",
			"force open even if the map is not in the given format")
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

	std::string strFilename;
	std::string strType;

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
					"Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>\n"
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
				if (i->value.size() == 0) {
					std::cerr << PROGNAME ": --type (-t) requires a parameter."
						<< std::endl;
					return RET_BADARGS;
				}
				strType = i->value[0];
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

		// Get the format handler for this file format
		boost::shared_ptr<gm::Manager> pManager(gm::getManager());

		gm::MapTypePtr pMapType;
		if (strType.empty()) {
			// Need to autodetect the file format.
			gm::MapTypePtr pTestType;
			int i = 0;
			while ((pTestType = pManager->getMapType(i++))) {
				gm::E_CERTAINTY cert = pTestType->isInstance(psMap);
				switch (cert) {
					case gm::EC_DEFINITELY_NO:
						// Don't print anything (TODO: Maybe unless verbose?)
						break;
					case gm::EC_UNSURE:
						std::cout << "File could be a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getMapCode() << "]" << std::endl;
						// If we haven't found a match already, use this one
						if (!pMapType) pMapType = pTestType;
						break;
					case gm::EC_POSSIBLY_YES:
						std::cout << "File is likely to be a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getMapCode() << "]" << std::endl;
						// Take this one as it's better than an uncertain match
						pMapType = pTestType;
						break;
					case gm::EC_DEFINITELY_YES:
						std::cout << "File is definitely a " << pTestType->getFriendlyName()
							<< " [" << pTestType->getMapCode() << "]" << std::endl;
						pMapType = pTestType;
						// Don't bother checking any other formats if we got a 100% match
						goto finishTesting;
				}
				if (cert != gm::EC_DEFINITELY_NO) {
					// We got a possible match, see if it requires any suppdata
					gm::MP_SUPPLIST suppList = pTestType->getRequiredSupps(strFilename);
					if (suppList.size() > 0) {
						// It has suppdata, see if it's present
						std::cout << "  * This format requires supplemental files..." << std::endl;
						bool bSuppOK = true;
						for (gm::MP_SUPPLIST::iterator i = suppList.begin(); i != suppList.end(); i++) {
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
		gm::MP_SUPPLIST suppList = pMapType->getRequiredSupps(strFilename);
		gm::MP_SUPPDATA suppData;
		if (suppList.size() > 0) {
			for (gm::MP_SUPPLIST::iterator i = suppList.begin(); i != suppList.end(); i++) {
				try {
					boost::shared_ptr<std::fstream> suppStream(new std::fstream());
					suppStream->exceptions(std::ios::badbit | std::ios::failbit);
					std::cout << "Opening supplemental file " << i->second << std::endl;
					suppStream->open(i->second.c_str(), std::ios::in | std::ios::out | std::ios::binary);
					gm::SuppItem si;
					si.stream = suppStream;
					si.fnTruncate = boost::bind<void>(truncate, i->second.c_str(), _1);
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

					int layerCount = map2d->getLayerCount();
					std::cout << (bScript ? "layercount=" : "Layer count: ")
						<< layerCount << "\n";
					for (int i = 0; i < layerCount; i++) {
						std::string prefix;
						if (bScript) {
							std::stringstream ss;
							ss << "layer" << i << '_';
							prefix = ss.str();
						}
						else {
							prefix = "  ";
							std::cout << "Layer " << i + 1 << ":\n";
						}
						gm::Map2D::LayerPtr layer = map2d->getLayer(i);
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

					int mapCaps = map2d->getCaps();
					gm::Map2D::LayerPtr layer = map2d->getLayer(targetLayer - 1);
					int layerCaps = layer->getCaps();

					// Figure out the layer size
					int layerWidth, layerHeight;
					if (layerCaps & gm::Map2D::Layer::HasOwnSize) {
						layer->getLayerSize(&layerWidth, &layerHeight);
					} else if (mapCaps & gm::Map2D::HasGlobalSize) {
						map2d->getMapSize(&layerWidth, &layerHeight);
						// If this layer has a tile size divide it out by the map size
						if (layerCaps & gm::Map2D::Layer::HasOwnTileSize) {
							// But if the map itself has a different size, multiply it out
							// to a pixel width/height first.
							if (mapCaps & gm::Map2D::HasGlobalTileSize) {
								int mtx, mty;
								map2d->getTileSize(&mtx, &mty);
								layerWidth *= mtx;
								layerHeight *= mty;
							}
							// Convert the map's pixel size down into a tile size for this layer
							int tx, ty;
							layer->getTileSize(&tx, &ty);
							layerWidth /= tx;
							layerHeight /= ty;
						}
					}

					const gm::Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
					gm::Map2D::Layer::ItemPtrVector::const_iterator t = items->begin();
					int numItems = items->size();
					if (t != items->end()) {
						for (int y = 0; y < layerHeight; y++) {
							for (int x = 0; x < layerWidth; x++) {
								for (int i = 0; i < numItems; i++) {
									if (t == items->end()) t = items->begin();
									if (((*t)->x == x) && ((*t)->y == y)) break;
									t++;
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
	} catch (po::unknown_option& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return RET_BADARGS;
	} catch (po::invalid_command_line_syntax& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< ".  Use --help for help." << std::endl;
		return RET_BADARGS;
	}

	return iRet;
}
