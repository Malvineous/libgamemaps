/**
 * @file  camoto/gamemaps/map.hpp
 * @brief Declaration of top-level Map class, for accessing files
 *        storing game map data.
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

#ifndef _CAMOTO_GAMEMAPS_MAP_HPP_
#define _CAMOTO_GAMEMAPS_MAP_HPP_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <camoto/error.hpp>
#include <camoto/attribute.hpp>
#include <camoto/gamegraphics/tileset.hpp>

namespace camoto {
namespace gamemaps {

/// Generic "invalid map format" exception.
class EInvalidFormat: virtual public error {
};

/// What an image or tileset is used for.
enum class ImagePurpose {
	GenericTileset1 = 0,
	GenericTileset2,
	GenericTileset3,
	GenericTileset4,
	GenericTileset5,
	GenericTileset6,
	GenericTileset7,
	GenericTileset8,
	GenericTileset9,
	BackgroundTileset1,
	BackgroundTileset2,
	BackgroundTileset3,
	BackgroundTileset4,
	BackgroundTileset5,
	BackgroundTileset6,
	BackgroundTileset7,
	BackgroundTileset8,
	BackgroundTileset9,
	ForegroundTileset1,
	ForegroundTileset2,
	ForegroundTileset3,
	ForegroundTileset4,
	ForegroundTileset5,
	ForegroundTileset6,
	ForegroundTileset7,
	ForegroundTileset8,
	ForegroundTileset9,
	SpriteTileset1,
	SpriteTileset2,
	SpriteTileset3,
	SpriteTileset4,
	SpriteTileset5,
	SpriteTileset6,
	SpriteTileset7,
	SpriteTileset8,
	SpriteTileset9,
	FontTileset1,
	FontTileset2,
	FontTileset3,
	FontTileset4,
	FontTileset5,
	FontTileset6,
	FontTileset7,
	FontTileset8,
	FontTileset9,
	BackgroundImage, ///< First image - for ImagePurpose_IsImage()
	ImagePurposeCount // must always be last
};

/// Is this ImagePurpose for an Image?
constexpr bool ImagePurpose_IsImage(ImagePurpose p)
{
	return p >= ImagePurpose::BackgroundImage;
}

/// Is this ImagePurpose for a tileset?
constexpr bool ImagePurpose_IsTileset(ImagePurpose p)
{
	return p < ImagePurpose::BackgroundImage;
}

/// Convert an ImagePurpose to a string
// implemented in map-core.cpp
const char* CAMOTO_GAMEMAPS_API toString(ImagePurpose p);

/// List of Tileset shared pointers.
typedef std::map<ImagePurpose, std::shared_ptr<gamegraphics::Tileset>> TilesetCollection;

/// Primary interface to a map file.
/**
 * This class represents a map file.  Its functions are used to edit the map.
 *
 * @note Multithreading: Only call one function in this class at a time.
 */
class Map: public HasAttributes
{
	public:
		/// Information about a graphics file used to render this map.
		struct GraphicsFilename {
			std::string filename;  ///< Actual filename
			std::string type;      ///< Type code (e.g. "tls-blah")
		};

		inline virtual ~Map() {};

		/// Get a list of tileset/background image filenames needed for rendering.
		virtual std::map<ImagePurpose, GraphicsFilename> graphicsFilenames()
			const = 0;

		/// Save any modifications to the map back to the original files.
		virtual void flush() = 0;
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_HPP_
