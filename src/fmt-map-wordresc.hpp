/**
 * @file   fmt-map-wordresc.hpp
 * @brief  MapType and Map2D implementation for Word Rescue levels.
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

#ifndef _CAMOTO_GAMEMAPS_MAP_WORDRESC_HPP_
#define _CAMOTO_GAMEMAPS_MAP_WORDRESC_HPP_

#include <camoto/gamemaps/maptype.hpp>

namespace camoto {
namespace gamemaps {

/// Word Rescue level reader/writer.
class WordRescueMapType: virtual public MapType {

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

class WordRescueBackgroundLayer: virtual public Map2D::Layer {

	public:
		WordRescueBackgroundLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems);

		virtual gamegraphics::ImagePtr imageFromCode(unsigned int code,
			camoto::gamegraphics::VC_TILESET& tileset);

};

class WordRescueObjectLayer: virtual public Map2D::Layer {

	public:
		WordRescueObjectLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems);

		virtual gamegraphics::ImagePtr imageFromCode(unsigned int code,
			camoto::gamegraphics::VC_TILESET& tileset);

		virtual bool tilePermittedAt(unsigned int code, unsigned int x,
			unsigned int y, unsigned int *maxCount);

};

class WordRescueAttributeLayer: virtual public Map2D::Layer {

	public:
		WordRescueAttributeLayer(ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems);

		virtual gamegraphics::ImagePtr imageFromCode(unsigned int code,
			camoto::gamegraphics::VC_TILESET& tileset);

		virtual bool tilePermittedAt(unsigned int code, unsigned int x,
			unsigned int y, unsigned int *maxCount);

};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_WORDRESC_HPP_
