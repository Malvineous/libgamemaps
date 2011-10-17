/**
 * @file   fmt-map-ccomic.hpp
 * @brief  MapType and Map2D implementation for Captain Comic maps.
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

#ifndef _CAMOTO_GAMEMAPS_MAP_CCOMIC_HPP_
#define _CAMOTO_GAMEMAPS_MAP_CCOMIC_HPP_

#include <camoto/gamemaps/maptype.hpp>

namespace camoto {
namespace gamemaps {

/// Captain Comic level reader/writer.
class CComicMapType: virtual public MapType {

	public:

		virtual std::string getMapCode() const
			throw ();

		virtual std::string getFriendlyName() const
			throw ();

		virtual std::vector<std::string> getFileExtensions() const
			throw ();

		virtual std::vector<std::string> getGameList() const
			throw ();

		virtual Certainty isInstance(stream::input_sptr psMap) const
			throw (stream::error);

		virtual MapPtr create(SuppData& suppData) const
			throw (stream::error);

		virtual MapPtr open(stream::input_sptr input, SuppData& suppData) const
			throw (stream::error);

		virtual unsigned long write(MapPtr map, stream::output_sptr output, SuppData& suppData) const
			throw (stream::error);

};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_CCOMIC_HPP_
