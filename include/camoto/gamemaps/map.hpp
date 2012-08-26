/**
 * @file   map.hpp
 * @brief  Declaration of top-level Map class, for accessing files
 *         storing game map data.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

namespace camoto {
namespace gamemaps {

/// Generic "invalid map format" exception.
class EInvalidFormat: virtual public error {
};

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
		};

		/// Shared pointer to an Attribute.
		typedef boost::shared_ptr<Attribute> AttributePtr;

		/// Vector of Attribute shared pointers.
		typedef std::vector<AttributePtr> AttributePtrVector;

		/// Shared pointer to Attribute vector.
		typedef boost::shared_ptr<AttributePtrVector> AttributePtrVectorPtr;

		/// Get a list of attributes that can be set in this map.
		virtual AttributePtrVectorPtr getAttributes() = 0;

		/// Const-accessible version
		virtual const AttributePtrVectorPtr getAttributes() const = 0;

		/// Information about a graphics file used to render this map.
		struct GraphicsFilename {
			enum Purpose {
				Tileset,             ///< Normal tileset
				BackgroundImage,     ///< Image to appear behind all tiles
			};
			Purpose purpose;
			std::string filename;  ///< Actual filename
			std::string type;      ///< Type code (e.g. "tls-blah")
		};
		typedef std::vector<GraphicsFilename> FilenameVector;
		typedef boost::shared_ptr<FilenameVector> FilenameVectorPtr;

		/// Get a list of additional files needed to render the map.
		/**
		 * This function returns a list of filenames and format types needed to
		 * render the map.  Tilesets, background images, etc.  These values may
		 * change as map attributes are altered.
		 */
		virtual FilenameVectorPtr getGraphicsFilenames() const = 0;
};

/// Shared pointer to a Map.
typedef boost::shared_ptr<Map> MapPtr;

/// Vector of Map shared pointers.
typedef std::vector<MapPtr> MapVector;

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_HPP_
