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
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_CORE_HPP_
