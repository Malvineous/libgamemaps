/**
 * @file  camoto/gamemaps/manager.hpp
 * @brief Manager class, used for accessing the various map format readers.
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

#ifndef _CAMOTO_GAMEMAPS_MANAGER_HPP_
#define _CAMOTO_GAMEMAPS_MANAGER_HPP_

#include <camoto/formatenum.hpp>
#include <camoto/gamemaps/maptype.hpp>

#ifndef CAMOTO_GAMEMAPS_API
#define CAMOTO_GAMEMAPS_API
#endif

namespace camoto {
namespace gamemaps {

typedef FormatEnumerator<MapType> MapManager;

} // namespace gamegraphics
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MANAGER_HPP_
