/**
 * @file   map.hpp
 * @brief  Declaration of top-level Map class, for accessing files
 *         storing game map data.
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

#ifndef _CAMOTO_GAMEMAPS_MAP_HPP_
#define _CAMOTO_GAMEMAPS_MAP_HPP_

#include <boost/shared_ptr.hpp>
#include <exception>
#include <vector>

#include <camoto/stream.hpp>
#include <stdint.h>
#include <camoto/metadata.hpp>

namespace camoto {
namespace gamemaps {

/// Generic "invalid map format" exception.
class EInvalidFormat: public std::exception {
};

/// Primary interface to a map file.
/**
 * This class represents a map file.  Its functions are used to edit the map.
 *
 * @note Multithreading: Only call one function in this class at a time.
 */
class Map: virtual public Metadata {

	public:

		/// Attribute attached to this map.
		struct Attribute {
			enum Type {
				Integer,         ///< Implements IntAttribute
				Enum,            ///< Implements EnumAttribute
			};
			Type type;         ///< What type this attribute is
			std::string name;  ///< Short name of this attribute
			std::string desc;  ///< Description of this attribute

			virtual ~Attribute() throw ();
		};

		/// Shared pointer to an Attribute.
		typedef boost::shared_ptr<Attribute> AttributePtr;

		/// Vector of Attribute shared pointers.
		typedef std::vector<AttributePtr> AttributePtrVector;

		/// Shared pointer to Attribute vector.
		typedef boost::shared_ptr<AttributePtrVector> AttributePtrVectorPtr;

		/// Attribute that can be a number, within a given range.
		struct IntAttribute: public Attribute {
			int value;

			int minValue;
			int maxValue;

			virtual ~IntAttribute() throw ();
		};

		/// Attribute that can be a single value from a list of permitted values.
		struct EnumAttribute: public Attribute {
			unsigned int value;

			std::vector<std::string> values;

			virtual ~EnumAttribute() throw ();
		};


		Map(AttributePtrVectorPtr attributes)
			throw ();

		virtual ~Map()
			throw ();

		virtual AttributePtrVectorPtr getAttributes()
			throw ();

	protected:
		AttributePtrVectorPtr attributes;

};

/// Shared pointer to a Map.
typedef boost::shared_ptr<Map> MapPtr;

/// Vector of Map shared pointers.
typedef std::vector<MapPtr> VC_MAP;

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_HPP_
