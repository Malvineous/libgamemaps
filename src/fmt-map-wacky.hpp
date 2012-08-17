/**
 * @file   fmt-map-wacky.hpp
 * @brief  MapType and Map2D implementation for Wacky Wheels levels.
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

#ifndef _CAMOTO_GAMEMAPS_MAP_WACKY_HPP_
#define _CAMOTO_GAMEMAPS_MAP_WACKY_HPP_

#include <boost/shared_ptr.hpp>
#include <exception>
#include <iostream>
#include <sstream>
#include <vector>

#include <camoto/gamemaps/maptype.hpp>
#include <camoto/gamemaps/map2d.hpp>
#include <camoto/stream.hpp>
#include <stdint.h>

namespace camoto {
namespace gamemaps {

/// Wacky Wheels level reader/writer.
class WackyMapType: virtual public MapType {

	public:

		virtual std::string getMapCode() const;

		virtual std::string getFriendlyName() const;

		virtual std::vector<std::string> getFileExtensions() const;

		virtual std::vector<std::string> getGameList() const;

		virtual Certainty isInstance(stream::input_sptr psMap) const;

		virtual MapPtr create(SuppData& suppData) const;

		virtual MapPtr open(stream::input_sptr input, SuppData& suppData) const;

		virtual stream::len write(MapPtr map, stream::output_sptr output, SuppData& suppData) const;

		virtual SuppFilenames getRequiredSupps(const std::string& filenameMap) const;

};

class WackyBackgroundLayer: virtual public Map2D::Layer {

	public:
		WackyBackgroundLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems);

		virtual gamegraphics::ImagePtr imageFromCode(unsigned int code,
			camoto::gamegraphics::VC_TILESET& tileset);

};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_WACKY_HPP_
