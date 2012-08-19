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
		/// Attribute attached to this map
		struct Attribute {
			enum Type {
				Integer,         ///< One number within a given range
				Enum,            ///< One choice from a list of static values
				Filename,        ///< A filename of the given type
			};
			Type type;         ///< What type this attribute is
			std::string name;  ///< Short name of this attribute
			std::string desc;  ///< Description of this attribute

			int integerValue;    ///< Integer type: current value
			int integerMinValue; ///< Integer type: minimum allowed value (set min and max to 0 for unlimited)
			int integerMaxValue; ///< Integer type: maximum allowed value (set min and max to 0 for unlimited)

			unsigned int enumValue;                  ///< Enum type: current value
			std::vector<std::string> enumValueNames; ///< Enum type: permitted values

			std::string filenameValue; ///< Filename type: current filename
			/// Valid filename extensions
			/**
			 * Any files that match this specification will be listed as valid choices
			 * for this attribute value.  An empty string means there is no
			 * restriction on file extension.
			 */
			std::string filenameValidExtensions;
		};

		/// Shared pointer to an Attribute.
		typedef boost::shared_ptr<Attribute> AttributePtr;

		/// Vector of Attribute shared pointers.
		typedef std::vector<AttributePtr> AttributePtrVector;

		/// Shared pointer to Attribute vector.
		typedef boost::shared_ptr<AttributePtrVector> AttributePtrVectorPtr;

		/// Get a list of attributes that can be set in this map.
		virtual AttributePtrVectorPtr getAttributes() = 0;
};

/// Shared pointer to a Map.
typedef boost::shared_ptr<Map> MapPtr;

/// Vector of Map shared pointers.
typedef std::vector<MapPtr> MapVector;

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_HPP_
