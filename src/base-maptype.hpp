/**
 * @file  base-maptype.hpp
 * @brief Shared functionality for all MapType classes.
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

#ifndef _CAMOTO_GAMEMAPS_BASE_MAPTYPE_HPP_
#define _CAMOTO_GAMEMAPS_BASE_MAPTYPE_HPP_

#include <camoto/gamemaps/maptype.hpp>

namespace camoto {
namespace gamemaps {

/// SuppData equivalent but with expanding output streams instead.
typedef std::map<SuppItem::Type, stream::expanding_output_sptr> ExpandingSuppData;

/// Standard functionality used by all map types
class BaseMapType: virtual public MapType
{
	public:
		BaseMapType();
		virtual ~BaseMapType();

		virtual void write(MapPtr map, stream::output_sptr output,
			SuppData& suppData) const;

		virtual void write(MapPtr map, stream::expanding_output_sptr output,
			ExpandingSuppData& suppData) const = 0;
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_BASE_MAPTYPE_HPP_
