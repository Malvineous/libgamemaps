/**
 * @file  gamemap.cpp
 * @brief Command-line interface to libgamemaps.
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

#include <iomanip>
#include <iostream>
#include <memory>
#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/program_options.hpp>
#include <camoto/gamegraphics.hpp>
#include <camoto/gamemaps.hpp>
#include <camoto/util.hpp>
#include <camoto/stream_file.hpp>
#include <png++/png.hpp>

namespace po = boost::program_options;
namespace gm = camoto::gamemaps;
namespace gg = camoto::gamegraphics;
namespace stream = camoto::stream;

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

// mingw32 doesn't have __STRING
#ifndef __STRING
#define __STRING(x) #x
#endif

/// Place to cache tiles when rendering a map to a .png file
struct CachedTile {
	unsigned int code;
	gg::Pixels data;
	gg::Pixels mask;
	gg::Point dims;
};

// Copied from libgamearchive/examples/gamearch.cpp
// Split a string in two at a delimiter, e.g. "one=two" becomes "one" and "two"
// and true is returned.  If there is no delimiter both output strings will be
// the same as the input string and false will be returned.
//
// If delim == '=' then:
// in        -> ret, out1, out2
// "one=two" -> true, "one", "two"
// "one=two=three" -> true, "one=two", "three"
// "four" -> false, "four", "four"
// If delim == '@' then:
// "one@two" -> true, "one", "two"
// "@1=myfile@@4"
// "test.txt@here.txt"
// "@2=test.txt"
// "e1m1.mid=mysong.mid:@4"
// "e1m1.mid=mysong.mid:e1m2.mid"
bool split(const std::string& in, char delim, std::string *out1, std::string *out2)
{
	std::string::size_type iEqualPos = in.find_last_of(delim);
	*out1 = in.substr(0, iEqualPos);
	// Does the destination have a different filename?
	bool bAltDest = iEqualPos != std::string::npos;
	*out2 = bAltDest ? in.substr(iEqualPos + 1) : *out1;
	return bAltDest;
}

/// Open an image.
/**
 * @param filename
 *   File to open.
 *
 * @param type
 *   File type if it can't be autodetected.
 *
 * @return Shared pointer to the image.
 *
 * @throw stream::error on error
 */
std::shared_ptr<gg::Image> openImage(const std::string& filename,
	const std::string& type)
{
	std::unique_ptr<stream::inout> psImage;
	try {
		psImage = std::make_unique<stream::file>(filename, false);
	} catch (const stream::open_error& e) {
		std::cerr << "Error opening " << filename << ": " << e.what()
			<< std::endl;
		throw stream::error("Unable to open image " + filename + ": "
			+ e.get_message());
	}

	gg::ImageManager::handler_t imageType;
	if (type.empty()) {
		// Need to autodetect the file format.
		for (auto& i : gg::ImageManager::formats()) {
			switch (i->isInstance(*psImage)) {
				case gg::ImageType::Certainty::DefinitelyNo:
					break;
				case gg::ImageType::Certainty::Unsure:
					// If we haven't found a match already, use this one
					if (!imageType) imageType = i;
					break;
				case gg::ImageType::Certainty::PossiblyYes:
					// Take this one as it's better than an uncertain match
					imageType = i;
					break;
				case gg::ImageType::Certainty::DefinitelyYes:
					imageType = i;
					// Don't bother checking any other formats if we got a 100% match
					goto finishTesting;
			}
		}
finishTesting:
		if (!imageType) {
			std::cerr << "Unable to automatically determine the graphics file "
				"type.  Use the --graphicstype option to manually specify the file "
				"format." << std::endl;
			throw stream::error("Unable to open image");
		}
	} else {
		imageType = gg::ImageManager::byCode(type);
		if (!imageType) {
			std::cerr << "Unknown file type given to -y/--graphicstype: " << type
				<< std::endl;
			throw stream::error("Unable to open image");
		}
	}
	assert(imageType);

	// See if the format requires any supplemental files
	camoto::SuppData suppData;
	for (auto i : imageType->getRequiredSupps(*psImage, filename)) {
		try {
			std::cerr << "Opening supplemental file " << i.second << std::endl;
			suppData[i.first] = std::make_unique<stream::file>(i.second, false);
		} catch (const stream::open_error& e) {
			std::cerr << "Error opening supplemental file " << i.second << ": "
				<< e.what() << std::endl;
			throw stream::error("Unable to open supplemental file " + i.second
				+ ": " +  e.get_message());
		}
	}

	// Open the graphics file
	std::cout << "Opening image " << filename << " as "
		<< imageType->code() << std::endl;

	return imageType->open(std::move(psImage), suppData);
}

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
 * @throw stream::error on error
 */
std::shared_ptr<gg::Tileset> openTileset(const std::string& filename,
	const std::string& type)
{
	if (type.substr(0, 4).compare("img-") == 0) {
		// This is an image, not a tileset, so create a tileset with one image in it
		return gg::make_Tileset_FromImageList(
			{
				{
					openImage(filename, type),
					gg::Tileset_FromImageList::Item::AttachmentType::Append,
					gg::Tileset_FromImageList::Item::SplitType::SingleTile,
					{0, 0}, // tile size
					{0, 0, 0, 0}, // image size
					{}
				}
			},
			1
		);
	}
	std::unique_ptr<stream::inout> psTileset;
	try {
		psTileset = std::make_unique<stream::file>(filename, false);
	} catch (const stream::open_error& e) {
		std::cerr << "Error opening " << filename << ": " << e.what()
			<< std::endl;
		throw stream::error("Unable to open tileset " + filename + ": "
			+ e.get_message());
	}

	gg::TilesetManager::handler_t tilesetType;
	if (type.empty()) {
		// Need to autodetect the file format.
		for (auto& i : gg::TilesetManager::formats()) {
			switch (i->isInstance(*psTileset)) {
				case gg::TilesetType::Certainty::DefinitelyNo:
					break;
				case gg::TilesetType::Certainty::Unsure:
					// If we haven't found a match already, use this one
					if (!tilesetType) tilesetType = i;
					break;
				case gg::TilesetType::Certainty::PossiblyYes:
					// Take this one as it's better than an uncertain match
					tilesetType = i;
					break;
				case gg::TilesetType::Certainty::DefinitelyYes:
					tilesetType = i;
					// Don't bother checking any other formats if we got a 100% match
					goto finishTesting;
			}
		}
finishTesting:
		if (!tilesetType) {
			std::cerr << "Unable to automatically determine the graphics file "
				"type.  Use the --graphicstype option to manually specify the file "
				"format." << std::endl;
			throw stream::error("Unable to open tileset");
		}
	} else {
		tilesetType = gg::TilesetManager::byCode(type);
		if (!tilesetType) {
			std::cerr << "Unknown file type given to -y/--graphicstype: " << type
				<< std::endl;
			throw stream::error("Unable to open tileset");
		}
	}
	assert(tilesetType);

	// See if the format requires any supplemental files
	camoto::SuppData suppData;
	for (auto i : tilesetType->getRequiredSupps(*psTileset, filename)) {
		try {
			std::cerr << "Opening supplemental file " << i.second << std::endl;
			suppData[i.first] = std::make_unique<stream::file>(i.second, false);
		} catch (const stream::open_error& e) {
			std::cerr << "Error opening supplemental file " << i.second << ": "
				<< e.what() << std::endl;
			throw stream::error("Unable to open supplemental file " + i.second
				+ ": " +  e.get_message());
		}
	}

	// Open the graphics file
	std::cout << "Opening tileset " << filename << " as "
		<< tilesetType->code() << std::endl;

	return tilesetType->open(std::move(psTileset), suppData);
}

/// Export a map to .png file
/**
 * Convert the given map into a PNG file on disk, by rendering the map as it
 * would appear in the game.
 *
 * @param map
 *   Map file to export.
 *
 * @param allTilesets
 *   Collection of tilesets to use when rendering the map.
 *
 * @param destFile
 *   Filename of destination (including ".png")
 *
 * @throw stream::error on error
 */
void map2dToPng(const gm::Map2D& map, const gm::TilesetCollection& allTilesets,
	const std::string& destFile)
{
	gg::Point outSize = map.mapSize(); // in pixels
	gg::Point globalTileSize = map.tileSize();
	outSize.x *= globalTileSize.x;
	outSize.y *= globalTileSize.y;

	png::image<png::index_pixel> png(outSize.x, outSize.y);

	bool useMask;
	std::shared_ptr<gg::Palette> srcPal;
	for (auto& i : allTilesets) {
		if (i.second->caps() & gg::Tileset::Caps::HasPalette) {
			srcPal = std::make_shared<gg::Palette>(*i.second->palette());
			break;
		}
	}
	if (!srcPal) {
		srcPal = gg::createPalette_DefaultVGA();
		// Force last colour to be transparent
		srcPal->at(255).red = 255;
		srcPal->at(255).green = 0;
		srcPal->at(255).blue = 192;
		srcPal->at(255).alpha = 0;
	}

	png::palette pal(srcPal->size());
	png::tRNS transparency(srcPal->size());
	int j = 0;
	png::index_pixel xp(0);
	bool hasXP = false;
	for (auto& i : *srcPal) {
		pal[j] = png::color(i.red, i.green, i.blue);
		transparency[j] = i.alpha;
		if (i.alpha == 0) {
			xp = j;
			hasXP = true;
		}
		j++;
	}
	if ((srcPal->size() < 255) && (!hasXP)) {
		// Palette has no transparent entry but has room for one, so insert entry at
		// start of palette (this means tRNS block can be only one byte long)
		pal.insert(pal.begin(), png::color(255, 0, 192));
		transparency.insert(transparency.begin(), 0);
		useMask = true; // increment all palette indices from now on
	}
	png.set_palette(pal);
	if (transparency.size() > 0) png.set_tRNS(transparency);

	// Get the map background
	auto bg = map.background(allTilesets);
	switch (bg.att) {
		case gm::Map2D::Background::Attachment::NoBackground: {
			for (unsigned int y = 0; y < outSize.y; y++) {
				for (unsigned int x = 0; x < outSize.x; x++) {
					png[y][x] = xp;
				}
			}
			break;
		}
		case gm::Map2D::Background::Attachment::SingleColour: {
			// Find the background colour in the palette
			png::index_pixel clr(0);
			unsigned int palIndex = 0;
			for (auto& i : *srcPal) {
				if (
					(bg.clr.red == i.red)
					&& (bg.clr.green == i.green)
					&& (bg.clr.blue == i.blue)
					&& (bg.clr.alpha == i.alpha)
				) {
					clr = palIndex;
					break;
				}
				palIndex++;
			}
			for (unsigned int y = 0; y < outSize.y; y++) {
				for (unsigned int x = 0; x < outSize.x; x++) {
					png[y][x] = clr;
				}
			}
			break;
		}
		// TODO - case gm::Map2D::Background::Attachment::SingleImageCentred:
		case gm::Map2D::Background::Attachment::SingleImageTiled: {
			auto tilePixels = bg.img->convert();
			auto tileMask = bg.img->convert_mask();
			auto tileSize = bg.img->dimensions();
			for (unsigned int y = 0; y < outSize.y; y++) {
				for (unsigned int x = 0; x < outSize.x; x++) {
					auto pos = (y % tileSize.y) * tileSize.x + (x % tileSize.x);
					png[y][x] =
						(tileMask[pos] & (int)gg::Image::Mask::Transparent) ? (
							xp
						) : (
							png::index_pixel(
								// +1 to the colour to skip over transparent (#0)
								tilePixels[pos] + (useMask ? 1 : 0)
							)
						);
				}
			}
			break;
		}
	}

	for (auto& layer : map.layers()) {
		// Figure out the layer size (in tiles) and the tile size
		gg::Point layerSize, tileSize;
		getLayerDims(map, *layer, &layerSize, &tileSize);

		// Prepare tileset
		std::vector<CachedTile> cache;

		// Run through all items in the layer and render them one by one
		for (auto& t : layer->items()) {
			CachedTile thisTile;
			unsigned int tileCode = t.code;

			// Find the cached tile
			bool found = false;
			for (auto& ct : cache) {
				if (ct.code == tileCode) {
					thisTile = ct;
					found = true;
					break;
				}
			}
			if (!found) {
				// Tile hasn't been cached yet, load it from the tileset
				gm::Map2D::Layer::ImageFromCodeInfo imgType;
				try {
					imgType = layer->imageFromCode(t, allTilesets);
				} catch (const std::exception& e) {
					std::cerr << "Error loading image: " << e.what() << std::endl;
					imgType.type = gm::Map2D::Layer::ImageFromCodeInfo::ImageType::Unknown;
				}
				switch (imgType.type) {
					case gm::Map2D::Layer::ImageFromCodeInfo::ImageType::Supplied:
						assert(imgType.img);
						thisTile.data = imgType.img->convert();
						thisTile.mask = imgType.img->convert_mask();
						thisTile.dims = imgType.img->dimensions();
						thisTile.code = tileCode;
						break;
					case gm::Map2D::Layer::ImageFromCodeInfo::ImageType::Blank:
						thisTile.dims = {0, 0};
						break;
					case gm::Map2D::Layer::ImageFromCodeInfo::ImageType::NumImageTypes: // Avoid compiler warning about unhandled enum
						assert(false);
						// fall through
					case gm::Map2D::Layer::ImageFromCodeInfo::ImageType::Unknown:
					case gm::Map2D::Layer::ImageFromCodeInfo::ImageType::HexDigit:
					case gm::Map2D::Layer::ImageFromCodeInfo::ImageType::Interactive:
						// Display nothing, but could be changed to a question mark
						thisTile.dims = {0, 0};
						break;
				}
				cache.push_back(thisTile);
			}

			if ((thisTile.dims.x == 0) || (thisTile.dims.y == 0)) continue; // no image

			// Draw tile onto png
			unsigned int offX = t.pos.x * tileSize.x;
			unsigned int offY = t.pos.y * tileSize.y;
			for (unsigned int tY = 0; tY < thisTile.dims.y; tY++) {
				unsigned int pngY = offY+tY;
				if (pngY >= outSize.y) break; // don't write past image edge
				for (unsigned int tX = 0; tX < thisTile.dims.x; tX++) {
					unsigned int pngX = offX+tX;
					if (pngX >= outSize.x) break; // don't write past image edge
					// Only write opaque pixels
					if ((thisTile.mask[tY * thisTile.dims.x + tX] & (int)gg::Image::Mask::Transparent) == 0) {
						auto pos = tY * thisTile.dims.x + tX;
						png[pngY][pngX] =
							(thisTile.mask[pos] & (int)gg::Image::Mask::Transparent) ? (
								xp
							) : (
								png::index_pixel(
									// +1 to the colour to skip over transparent (#0)
									thisTile.data[pos] + (useMask ? 1 : 0)
								)
							);
					} // else let higher layers see through to lower ones
				}
			}
		} // else layer is empty
	}

	png.write(destFile);
	return;
}

int main(int iArgC, char *cArgV[])
{
#ifdef __GLIBCXX__
	// Set a better exception handler
	std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
#endif

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
		("script,s",
			"format output suitable for script parsing")
		("force,f",
			"force open even if the map is not in the given format")
		("list-types",
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
	std::map<gm::ImagePurpose, gm::Map::GraphicsFilename> manualGfx;

	bool bScript = false; // show output suitable for script parsing?
	bool bForceOpen = false; // open anyway even if map not in given format?
	int iRet = RET_OK;
	try {
		po::parsed_options pa = po::parse_command_line(iArgC, cArgV, poComplete);

		// Parse the global command line options
		for (auto& i : pa.options) {
			if (i.string_key.empty()) {
				// If we've already got an map filename, complain that a second one
				// was given (probably a typo.)
				if (!strFilename.empty()) {
					std::cerr << "Error: unexpected extra parameter (multiple map "
						"filenames given?!)" << std::endl;
					return 1;
				}
				assert(i.value.size() > 0);  // can't have no values with no name!
				strFilename = i.value[0];
			} else if (i.string_key.compare("help") == 0) {
				std::cout <<
					"Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>\n"
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
				(i.string_key.compare("t") == 0) ||
				(i.string_key.compare("type") == 0)
			) {
				strType = i.value[0];
			} else if (
				(i.string_key.compare("g") == 0) ||
				(i.string_key.compare("graphics") == 0)
			) {
				std::string purpose, temp;
				gm::Map::GraphicsFilename gf;
				bool a = split(i.value[0], '=', &purpose, &temp);
				bool b = split(temp, ':', &gf.type, &gf.filename);
				if (!a || !b) {
					std::cerr << "Malformed -g/--graphics parameter.  Must be of the "
						"form purpose=type:filename.\n"
						"Use --help or --list-types for details." << std::endl;
					return RET_BADARGS;
				}
				bool found = false;
				for (unsigned int i = 0; i < (unsigned int)gm::ImagePurpose::ImagePurposeCount; i++) {
					gm::ImagePurpose p = (gm::ImagePurpose)i;
					if (purpose.compare(toString(p)) == 0) {
						manualGfx[p] = gf;
						found = true;
					}
				}
				if (!found) {
					std::cerr << "No match for tileset purpose: " << purpose << "\n"
						<< "Use --list-types for details." << std::endl;
					return RET_BADARGS;
				}
			} else if (
				(i.string_key.compare("s") == 0) ||
				(i.string_key.compare("script") == 0)
			) {
				bScript = true;
			} else if (
				(i.string_key.compare("f") == 0) ||
				(i.string_key.compare("force") == 0)
			) {
				bForceOpen = true;
			} else if (
				(i.string_key.compare("list-types") == 0)
			) {
				std::cout << "Tileset purposes: (--graphics purpose=type:file)\n";
				for (unsigned int i = 0; i < (unsigned int)gm::ImagePurpose::ImagePurposeCount; i++) {
					gm::ImagePurpose p = (gm::ImagePurpose)i;
					std::cout << "  " << toString(p) << "\n";
				}

				std::cout << "\nTileset types: (--graphics purpose=type:file)\n";
				for (auto& tilesetType : gg::TilesetManager::formats()) {
					std::string code = tilesetType->code();
					std::cout << "  " << code;
					int len = code.length();
					if (len < 20) std::cout << std::string(20-code.length(), ' ');
					std::cout << ' ' << tilesetType->friendlyName();
					auto ext = tilesetType->fileExtensions();
					if (ext.size()) {
						auto i = ext.begin();
						std::cout << " (*." << *i;
						for (i++; i != ext.end(); i++) {
							std::cout << "; *." << *i;
						}
						std::cout << ")";
					}
					std::cout << '\n';
				}

				std::cout << "\nMap types: (--type)\n";
				for (auto& mapType : gm::MapManager::formats()) {
					std::string code = mapType->code();
					std::cout << "  " << code;
					int len = code.length();
					if (len < 20) std::cout << std::string(20 - code.length(), ' ');
					std::cout << ' ' << mapType->friendlyName();
					auto ext = mapType->fileExtensions();
					if (ext.size()) {
						auto i = ext.begin();
						std::cout << " (*." << *i;
						for (i++; i != ext.end(); i++) {
							std::cout << "; *." << *i;
						}
						std::cout << ")";
					}
					std::cout << '\n';
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

		std::unique_ptr<stream::inout> content;
		try {
			content = std::make_unique<stream::file>(strFilename, false);
		} catch (const stream::open_error& e) {
			std::cerr << "Error opening " << strFilename << ": " << e.what()
				<< std::endl;
			return RET_SHOWSTOPPER;
		}

		gm::MapManager::handler_t mapType;
		if (strType.empty()) {
			// Need to autodetect the file format.
			for (auto& mapTestType : gm::MapManager::formats()) {
				gm::MapType::Certainty cert = mapTestType->isInstance(*content);
				switch (cert) {
					case gm::MapType::Certainty::DefinitelyNo:
						// Don't print anything (TODO: Maybe unless verbose?)
						break;
					case gm::MapType::Certainty::Unsure:
						std::cout << "File could be a " << mapTestType->friendlyName()
							<< " [" << mapTestType->code() << "]" << std::endl;
						// If we haven't found a match already, use this one
						if (!mapType) mapType = mapTestType;
						break;
					case gm::MapType::Certainty::PossiblyYes:
						std::cout << "File is likely to be a " << mapTestType->friendlyName()
							<< " [" << mapTestType->code() << "]" << std::endl;
						// Take this one as it's better than an uncertain match
						mapType = mapTestType;
						break;
					case gm::MapType::Certainty::DefinitelyYes:
						std::cout << "File is definitely a " << mapTestType->friendlyName()
							<< " [" << mapTestType->code() << "]" << std::endl;
						mapType = mapTestType;
						// Don't bother checking any other formats if we got a 100% match
						goto finishTesting;
				}
				if (cert != gm::MapType::Certainty::DefinitelyNo) {
					// We got a possible match, see if it requires any suppdata
					auto suppList = mapTestType->getRequiredSupps(*content, strFilename);
					if (suppList.size() > 0) {
						// It has suppdata, see if it's present
						std::cout << "  * This format requires supplemental files..." << std::endl;
						bool bSuppOK = true;
						for (auto& i : suppList) {
							try {
								auto suppStream = std::make_unique<stream::file>(i.second, false);
							} catch (const stream::open_error&) {
								bSuppOK = false;
								std::cout << "  * Could not find/open " << i.second
									<< ", map is probably not "
									<< mapTestType->code() << std::endl;
								break;
							}
						}
						if (bSuppOK) {
							// All supp files opened ok
							std::cout << "  * All supp files present, map is likely "
								<< mapTestType->code() << std::endl;
							// Set this as the most likely format
							mapType = mapTestType;
						}
					}
				}
			}
finishTesting:
			if (!mapType) {
				std::cerr << "Unable to automatically determine the file type.  Use "
					"the --type option to manually specify the file format." << std::endl;
				return RET_BE_MORE_SPECIFIC;
			}
		} else {
			mapType = gm::MapManager::byCode(strType);
			if (!mapType) {
				std::cerr << "Unknown file type given to -t/--type: " << strType
					<< std::endl;
				return RET_BADARGS;
			}
		}

		assert(mapType != NULL);

		// Check to see if the file is actually in this format
		if (!mapType->isInstance(*content)) {
			if (bForceOpen) {
				std::cerr << "Warning: " << strFilename << " is not a "
					<< mapType->friendlyName() << ", open forced." << std::endl;
			} else {
				std::cerr << "Invalid format: " << strFilename << " is not a "
					<< mapType->friendlyName() << "\n"
					<< "Use the -f option to try anyway." << std::endl;
				return RET_BE_MORE_SPECIFIC;
			}
		}

		// See if the format requires any supplemental files
		camoto::SuppData suppData;
		for (auto& i : mapType->getRequiredSupps(*content, strFilename)) {
			try {
				std::cerr << "Opening supplemental file " << i.second << std::endl;
				suppData[i.first] = std::make_unique<stream::file>(i.second, false);
			} catch (const stream::open_error& e) {
				std::cerr << "Error opening supplemental file " << i.second << ": "
					<< e.what() << std::endl;
				return RET_SHOWSTOPPER;
			}
		}

		// Open the map file
		std::shared_ptr<gm::Map> pMap = mapType->open(std::move(content), suppData);
		assert(pMap);

		// File type of inserted files defaults to empty, which means 'generic file'
		std::string strLastFiletype;

		// Run through the actions on the command line
		for (auto& i : pa.options) {
			if (i.string_key.compare("info") == 0) {
				auto attributes = pMap->attributes();
				std::cout << (bScript ? "attribute_count=" : "Number of attributes: ")
					<< attributes.size() << "\n";
				int attrNum = 0;
				for (auto& a : attributes) {

					if (bScript) std::cout << "attribute" << attrNum << "_name=";
					else std::cout << "Attribute " << attrNum+1 << ": ";
					std::cout << a.name << "\n";

					if (bScript) std::cout << "attribute" << attrNum << "_desc=";
					else std::cout << "  Description: ";
					std::cout << a.desc << "\n";

					if (bScript) std::cout << "attribute" << attrNum << "_type=";
					else std::cout << "  Type: ";
					switch (a.type) {

						case gm::Attribute::Type::Integer: {
							std::cout << (bScript ? "int" : "Integer value") << "\n";

							if (bScript) std::cout << "attribute" << attrNum << "_value=";
							else std::cout << "  Current value: ";
							std::cout << a.integerValue << "\n";

							if (bScript) {
								std::cout << "attribute" << attrNum << "_min=" << a.integerMinValue
									<< "\nattribute" << attrNum << "_max=" << a.integerMaxValue;
							} else {
								std::cout << "  Range: ";
								if ((a.integerMinValue == 0) && (a.integerMaxValue == 0)) {
									std::cout << "[unlimited]";
								} else {
									std::cout << a.integerMinValue << " to " << a.integerMaxValue;
								}
							}
							std::cout << "\n";
							break;
						}

						case gm::Attribute::Type::Enum: {
							std::cout << (bScript ? "enum" : "Item from list") << "\n";

							if (bScript) std::cout << "attribute" << attrNum << "_value=";
							else std::cout << "  Current value: ";
							if (a.enumValue > a.enumValueNames.size()) {
								std::cout << (bScript ? "error" : "[out of range]");
							} else {
								if (bScript) std::cout << a.enumValue;
								else std::cout << "[" << a.enumValue << "] "
									<< a.enumValueNames[a.enumValue];
							}
							std::cout << "\n";

							if (bScript) std::cout << "attribute" << attrNum
								<< "_choice_count=" << a.enumValueNames.size() << "\n";

							int option = 0;
							for (std::vector<std::string>::const_iterator
								j = a.enumValueNames.begin(); j != a.enumValueNames.end(); j++
							) {
								if (bScript) {
									std::cout << "attribute" << attrNum << "_choice" << option
										<< "=";
								} else {
									std::cout << "  Allowed value " << option << ": ";
								}
								std::cout << *j << "\n";
								option++;
							}
							break;
						}

						case gm::Attribute::Type::Filename: {
							std::cout << (bScript ? "filename" : "Filename") << "\n";

							if (bScript) std::cout << "attribute" << attrNum << "_value=";
							else std::cout << "  Current value: ";
							std::cout << a.filenameValue << "\n";

							if (bScript) std::cout << "attribute" << attrNum
								<< "_filespec=";
							else std::cout << "  Valid files: ";
							std::cout << "*";
							if (!a.filenameValidExtension.empty()) {
								std::cout << '.' << a.filenameValidExtension;
							}
							std::cout << "\n";
							break;
						}

						default:
							std::cout << (bScript ? "unknown" : "Unknown type (fix this!)");
							break;
					}
					attrNum++;
				}

				std::cout << (bScript ? "gfx_filename_count=" : "Number of graphics filenames: ")
					<< pMap->graphicsFilenames().size() << "\n";
				int fileNum = 0;
				for (auto& a : pMap->graphicsFilenames()) {
					if (bScript) {
						std::cout << "gfx_file" << fileNum << "_name=" << a.second.filename << "\n";
						std::cout << "gfx_file" << fileNum << "_type=" << a.second.type << "\n";
						std::cout << "gfx_file" << fileNum << "_purpose=" << (unsigned int)a.first << "\n";
					} else {
						std::cout << "Graphics file " << fileNum+1 << ": " << a.second.filename
							<< " [";
						switch (a.first) {
							case gm::ImagePurpose::GenericTileset1:    std::cout << "Generic tileset 1"; break;
							case gm::ImagePurpose::BackgroundImage:    std::cout << "Background image"; break;
							case gm::ImagePurpose::BackgroundTileset1: std::cout << "Background tileset 1"; break;
							case gm::ImagePurpose::BackgroundTileset2: std::cout << "Background tileset 2"; break;
							case gm::ImagePurpose::ForegroundTileset1: std::cout << "Foreground tileset 1"; break;
							case gm::ImagePurpose::ForegroundTileset2: std::cout << "Foreground tileset 2"; break;
							case gm::ImagePurpose::SpriteTileset1:     std::cout << "Sprite tileset 1"; break;
							case gm::ImagePurpose::FontTileset1:       std::cout << "Font tileset 1"; break;
							case gm::ImagePurpose::FontTileset2:       std::cout << "Font tileset 2"; break;
							default:
								std::cout << "Unknown purpose <fix this>";
								break;
						}
						std::cout << " of type " << a.second.type << "]\n";
					}
					fileNum++;
				}

				std::cout << (bScript ? "map_type=" : "Map type: ");
				auto map2d = std::dynamic_pointer_cast<gm::Map2D>(pMap);
				if (map2d) {
					std::cout << (bScript ? "2d" : "2D grid-based") << "\n";
#define CAP(o, c, v)        " " __STRING(c) << ((v & o::Caps::c) ? '+' : '-')
#define MAP2D_CAP(c)        CAP(gm::Map2D,        c, mapCaps)
#define MAP2D_LAYER_CAP(c)  CAP(gm::Map2D::Layer, c, layerCaps)

					auto mapCaps = map2d->caps();
					if (bScript) {
						std::cout << "map_caps=" << (unsigned int)mapCaps << "\n";
					} else {
						std::cout << "Map capabilities:"
							<< MAP2D_CAP(HasViewport)
							<< MAP2D_CAP(HasMapSize)
							<< MAP2D_CAP(SetMapSize)
							<< MAP2D_CAP(HasTileSize)
							<< MAP2D_CAP(SetTileSize)
							<< MAP2D_CAP(AddPaths)
							<< "\n"
						;
					}
					auto mapTileSize = map2d->tileSize();
					std::cout << (bScript ? "tile_width=" : "Tile size: ") << mapTileSize.x
						<< (bScript ? "\ntile_height=" : "x") << mapTileSize.y << "\n";

					auto mapSize = map2d->mapSize();
					std::cout
						<< (bScript ? "map_width=" : "Map size: ") << mapSize.x
						<< (bScript ? "\nmap_height=" : "x") << mapSize.y
						<< (bScript ? "" : " tiles")
						<< "\n";

					if (mapCaps & gm::Map2D::Caps::HasViewport) {
						auto vp = map2d->viewport();
						std::cout << (bScript ? "viewport_width=" : "Viewport size: ")
							<< vp.x
							<< (bScript ? "\nviewport_height=" : "x") << vp.y
							<< (bScript ? "" : " pixels") << "\n";
					}

					unsigned int layerCount = map2d->layers().size();
					std::cout << (bScript ? "layercount=" : "Layer count: ")
						<< layerCount << "\n";
					unsigned int layerIndex = 0;
					for (auto& layer : map2d->layers()) {
						std::string prefix;
						if (bScript) {
							std::stringstream ss;
							ss << "layer" << layerIndex << '_';
							prefix = ss.str();
							std::cout << prefix << "name=" << layer->title() << "\n";
						} else {
							prefix = "  ";
							std::cout << "Layer " << layerIndex + 1 << ": \"" << layer->title()
								<< "\"\n";
						}
						auto layerCaps = layer->caps();
						if (bScript) std::cout << prefix << "caps="
							<< (unsigned int)layerCaps << "\n";
						else std::cout << prefix << "Capabilities:"
							<< MAP2D_LAYER_CAP(HasOwnSize)
							<< MAP2D_LAYER_CAP(SetOwnSize)
							<< MAP2D_LAYER_CAP(HasOwnTileSize)
							<< MAP2D_LAYER_CAP(SetOwnTileSize)
							<< MAP2D_LAYER_CAP(HasPalette)
							<< MAP2D_LAYER_CAP(UseImageDims)
							<< "\n"
						;

						gg::Point layerTileSize;
						bool layerTileSame;
						if (layerCaps & gm::Map2D::Layer::Caps::HasOwnTileSize) {
							layerTileSize = layer->tileSize();
							layerTileSame = false;
						} else {
							layerTileSize = mapTileSize;
							layerTileSame = true;
						}
						std::cout << prefix << (bScript ? "tile_width=" : "Tile size: ")
							<< layerTileSize.x;
						if (bScript) std::cout << "\n" << prefix << "tile_height=";
						else std::cout << "x";
						std::cout << layerTileSize.y;
						if (layerTileSame && (!bScript)) {
							std::cout << " (same as map)";
						}
						std::cout << "\n";

						gg::Point layerSize;
						bool layerSame;
						if (layerCaps & gm::Map2D::Layer::Caps::HasOwnSize) {
							layerSize = layer->layerSize();
							layerSame = false;
						} else {
							// Convert from map tilesize to layer tilesize, leaving final
							// pixel dimensions unchanged
							layerSize.x = mapSize.x * mapTileSize.x / layerTileSize.x;
							layerSize.y = mapSize.y * mapTileSize.y / layerTileSize.y;
							layerSame = true;
						}
						std::cout << prefix << (bScript ? "width=" : "Layer size: ")
							<< layerSize.x;
						if (bScript) std::cout << "\n" << prefix << "height=";
						else std::cout << "x";
						std::cout << layerSize.y;
						if (layerSame && (!bScript)) {
							std::cout << " (same as map)";
						}
						std::cout << "\n";

						layerIndex++;
					}

				} else {
					std::cout << (bScript ? "unknown" : "Unknown!  Fix this!") << "\n";
				}

			} else if (i.string_key.compare("print") == 0) {
				auto map2d = std::dynamic_pointer_cast<gm::Map2D>(pMap);
				if (map2d) {
					unsigned int targetLayer = strtoul(i.value[0].c_str(), NULL, 10);
					if (targetLayer == 0) {
						std::cerr << "Invalid layer index passed to --print.  Use --info "
							"to list layers in this map." << std::endl;
						iRet = RET_BADARGS;
						continue;
					}
					if (targetLayer > map2d->layers().size()) {
						std::cerr << "Invalid layer index passed to --print.  Use --info "
							"to list layers in this map." << std::endl;
						iRet = RET_BADARGS;
						continue;
					}

					auto layer = map2d->layers().at(targetLayer - 1);
					// If this fails, the map format returned a null pointer for the layer
					assert(layer);

					// Figure out the layer size
					gg::Point layerSize, tileSize;
					getLayerDims(*map2d, *layer, &layerSize, &tileSize);

					auto items = layer->items();
					auto t = items.begin();
					unsigned int numItems = items.size();
					if (t != items.end()) {
						for (unsigned int y = 0; y < layerSize.y; y++) {
							for (unsigned int x = 0; x < layerSize.x; x++) {
								for (unsigned int i = 0; i < numItems; i++) {
									if ((t->pos.x == x) && (t->pos.y == y)) break;
									t++;
									if (t == items.end()) t = items.begin();
								}
								if ((t->pos.x != x) || (t->pos.y != y)) {
									// Grid position with no tile!
									std::cout << "     ";
								} else {
									std::cout << std::hex << std::setw(4)
										<< (unsigned int)t->code << ' ';
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

			} else if (i.string_key.compare("render") == 0) {
				// Don't need to check i.value[0], program_options does that for us

				auto map2d = std::dynamic_pointer_cast<gm::Map2D>(pMap);
				if (map2d) {
					gm::TilesetCollection allTilesets;

					for (auto& a : manualGfx) {
						if (!bScript) {
							std::cout << "Loading " << a.second.type << " from "
								<< a.second.filename << std::endl;
						}
						allTilesets[a.first] = openTileset(a.second.filename, a.second.type);
					}

					for (auto& a : pMap->graphicsFilenames()) {
						if (allTilesets.find(a.first) == allTilesets.end()) {
							// This tileset hasn't been specified on the command line, but the
							// map format handler has given us a filename, so open the file
							// suggested from the map.
							allTilesets[a.first] = openTileset(a.second.filename, a.second.type);
						} else {
							std::cout << toString(a.first) << " overridden on command-line\n";
						}
					}

					if (allTilesets.empty()) {
						std::cerr << "You must use --graphics to specify a tileset."
							<< std::endl;
						iRet = RET_BADARGS;
						continue;
					}
					map2dToPng(*map2d, allTilesets, i.value[0]);
				} else {
					std::cerr << PROGNAME ": Rendering this type of map is not yet "
						"implemented." << std::endl;
					return RET_SHOWSTOPPER;
				}

			// Ignore --type/-t
			} else if (i.string_key.compare("type") == 0) {
			} else if (i.string_key.compare("t") == 0) {
			// Ignore --script/-s
			} else if (i.string_key.compare("script") == 0) {
			} else if (i.string_key.compare("s") == 0) {
			// Ignore --force/-f
			} else if (i.string_key.compare("force") == 0) {
			} else if (i.string_key.compare("f") == 0) {

			}
		} // for (all command line elements)
		//pMap->flush();
	} catch (const po::error& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< "  Use --help for help." << std::endl;
		return RET_BADARGS;
	} catch (const stream::error& e) {
		std::cerr << PROGNAME ": " << e.what()
			<< "  Use --help for help." << std::endl;
		return RET_SHOWSTOPPER;
	}

	return iRet;
}
