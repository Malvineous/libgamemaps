/**
 * @file  fmt-map-harry.hpp
 * @brief MapType and Map2D implementation for Halloween Harry/Alien Carnage.
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

#ifndef _CAMOTO_GAMEMAPS_MAP_HARRY_HPP_
#define _CAMOTO_GAMEMAPS_MAP_HARRY_HPP_

#include <camoto/gamemaps/maptype.hpp>

namespace camoto {
namespace gamemaps {

/// Halloween Harry level reader/writer.
class MapType_Harry: virtual public MapType
{
	public:
		virtual std::string code() const;
		virtual std::string friendlyName() const;
		virtual std::vector<std::string> fileExtensions() const;
		virtual std::vector<std::string> games() const;
		virtual Certainty isInstance(stream::input& content) const;
		virtual std::unique_ptr<Map> create(std::unique_ptr<stream::inout> content,
			SuppData& suppData) const;
		virtual std::unique_ptr<Map> open(std::unique_ptr<stream::inout> content,
			SuppData& suppData) const;
		virtual SuppFilenames getRequiredSupps(stream::input& content,
			const std::string& filename) const;
};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_HARRY_HPP_
