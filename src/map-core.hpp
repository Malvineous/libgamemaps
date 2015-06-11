/**
 * @file  map-core.hpp
 * @brief Implementation of Map functions inherited by most format handlers.
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

#ifndef _CAMOTO_GAMEMAPS_MAP_CORE_HPP_
#define _CAMOTO_GAMEMAPS_MAP_CORE_HPP_

#include <camoto/gamemaps/map.hpp>

namespace camoto {
namespace gamemaps {

class MapCore: virtual public Map
{
	public:
		virtual ~MapCore();

		/// Get a list of attributes for this map.
		virtual std::vector<Attribute> attributes() const;

		/// Change one of the map's integer/enum attributes.
		/**
		 * @param index
		 *   Index into vector returned by attributes().  0 is the first item.
		 *
		 * @param newValue
		 *   The new value to set for the attribute.
		 */
		virtual void attribute(unsigned int index, int newValue);

		/// Change one of the map's string/filename attributes.
		/**
		 * @param index
		 *   Index into vector returned by attributes().  0 is the first item.
		 *
		 * @param newValue
		 *   The new value to set for the attribute.
		 */
		virtual void attribute(unsigned int index, const std::string& newValue);

	protected:
		/// List of attributes (if any) in this map.
		/**
		 * Descendent classes should add their attributes to this in their
		 * constructor.
		 */
		std::vector<Attribute> attr;
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_CORE_HPP_
