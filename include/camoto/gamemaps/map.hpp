/**
 * @file   map.hpp
 * @brief  Declaration of top-level Map class, for accessing files
 *         storing game map data.
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
#include <camoto/metadata.hpp>
#include <camoto/gamegraphics/tileset.hpp>

namespace camoto {
namespace gamemaps {

/// Generic "invalid map format" exception.
class EInvalidFormat: virtual public error {
};

/// What an image or tileset is used for.
enum ImagePurpose {
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
	_FIRST_IMAGE, ///< Do not use (for IMAGEPURPOSE_IS_* macro)
	BackgroundImage,
	TilesetPurposeCount // must always be last
};

/// Is this ImagePurpose for an Image?
inline bool ImagePurpose_IsImage(const ImagePurpose& p)
{
	return p > _FIRST_IMAGE;
}

/// Is this ImagePurpose for a tileset?
inline bool ImagePurpose_IsTileset(const ImagePurpose& p)
{
	return p < _FIRST_IMAGE;
}

/// List of Tileset shared pointers.
typedef std::map<ImagePurpose, gamegraphics::TilesetPtr> TilesetCollection;

/// Shared pointer to a Tileset collection.
typedef boost::shared_ptr<TilesetCollection> TilesetCollectionPtr;

/// Primary interface to a map file.
/**
 * This class represents a map file.  Its functions are used to edit the map.
 *
 * @note Multithreading: Only call one function in this class at a time.
 */
class Map: virtual public Metadata
{
	public:
		/// Attribute attached to this map.
		/**
		 * Attributes are configuration options that apply to particular map files,
		 * such as a default background colour or which song to play as background
		 * music in the level.
		 *
		 * Attributes should reflect data contained in the map file itself, so for
		 * example, if the map file doesn't store some value that controls which
		 * tileset is used to draw the level, then the tileset filename shouldn't be
		 * exposed as an attribute (because if it was changed, the new value
		 * couldn't be saved back into the map file.)
		 *
		 * Attributes should reflect properties of the map that the user can and may
		 * wish to change.
		 */
		struct Attribute {
			enum Type {
				Integer,         ///< One number within a given range
				Enum,            ///< One choice from a list of static values
				Filename,        ///< A filename of the given file type
				Text,            ///< A text string
			};
			Type type;         ///< What type this attribute is
			std::string name;  ///< Short name of this attribute
			std::string desc;  ///< Description of this attribute

			int integerValue;    ///< Integer type: current value
			int integerMinValue; ///< Integer type: minimum allowed value (set min and max to 0 for unlimited)
			int integerMaxValue; ///< Integer type: maximum allowed value (set min and max to 0 for unlimited)

			unsigned int enumValue;                  ///< Enum type: current value
			std::vector<std::string> enumValueNames; ///< Enum type: permitted values

			/// Filename type: current filename
			/**
			 * Filenames should be specified here as map attributes (as opposed to
			 * supplementary items) if the files are not required to load the map.
			 *
			 * Parts of the actual map (like layer data or sprite positions) should
			 * be listed as supp data because the map will be incomplete if those
			 * files are not available, but things like tileset filenames are not
			 * required to load the map (e.g. if all you want to do is find out the
			 * map dimensions) so those optional files should be listed as attributes.
			 */
			std::string filenameValue;

			/// Filename type: valid filename extensions
			/**
			 * Any files that match this specification will be listed as valid choices
			 * for this attribute value.  An empty string means there is no
			 * restriction on file extension.
			 */
			std::string filenameValidExtension;

			std::string textValue; ///< Text type: the text value
			int textMaxLength;     ///< Text type: maximum string length, in chars
		};

		/// Attribute list.
		typedef std::vector<Attribute> Attributes;

		/// List of attributes (if any) in this map.
		/**
		 * This member is public to avoid having const and non-const getters.
		 * Attributes should be populated at constructor time, so there's no need
		 * to do any work in a getter function.
		 */
		Attributes attributes;

		/// Information about a graphics file used to render this map.
		struct GraphicsFilename {
			std::string filename;  ///< Actual filename
			std::string type;      ///< Type code (e.g. "tls-blah")
		};

		/// Map between each graphics file and what it's used for.
		typedef std::map<ImagePurpose, GraphicsFilename> GraphicsFilenames;

		/// List of tilesets and what they are used for in this map.
		GraphicsFilenames graphicsFilenames;

		inline Map(const Attributes& attributes, const GraphicsFilenames& graphicsFilenames)
			:	attributes(attributes),
				graphicsFilenames(graphicsFilenames)
		{
		}
};

/// Shared pointer to a Map.
typedef boost::shared_ptr<Map> MapPtr;

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_HPP_
