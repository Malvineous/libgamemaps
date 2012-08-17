/**
 * @file   fmt-map-xargon.hpp
 * @brief  MapType and Map2D implementation for Jill of the Jungle and Xargon.
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

#ifndef _CAMOTO_GAMEMAPS_MAP_XARGON_HPP_
#define _CAMOTO_GAMEMAPS_MAP_XARGON_HPP_

#include <map>
#include <camoto/gamemaps/maptype.hpp>

namespace camoto {
namespace gamemaps {

/// Generic level reader/writer for games based on Tim Sweeney's Jill engine.
class SweeneyMapType: virtual public MapType {

	public:
		virtual Certainty isInstance(stream::input_sptr psMap) const;

		virtual MapPtr create(SuppData& suppData) const;

		virtual MapPtr open(stream::input_sptr input, SuppData& suppData) const;

		virtual stream::len write(MapPtr map, stream::output_sptr output, SuppData& suppData) const;

		typedef std::map<uint16_t, uint16_t> image_map;
		typedef boost::shared_ptr<image_map> image_map_sptr;

	protected:
		unsigned int lenSavedata;
};

/// Jill of the Jungle level reader/writer.
class JillMapType: virtual public SweeneyMapType {

	public:
		JillMapType();

		virtual std::string getMapCode() const;

		virtual std::string getFriendlyName() const;

		virtual std::vector<std::string> getFileExtensions() const;

		virtual std::vector<std::string> getGameList() const;

		virtual SuppFilenames getRequiredSupps(const std::string& filenameMap) const;
};

/// Xargon level reader/writer.
class XargonMapType: virtual public SweeneyMapType {

	public:
		XargonMapType();

		virtual std::string getMapCode() const;

		virtual std::string getFriendlyName() const;

		virtual std::vector<std::string> getFileExtensions() const;

		virtual std::vector<std::string> getGameList() const;

		virtual SuppFilenames getRequiredSupps(const std::string& filenameMap) const;
};


class SweeneyBackgroundLayer: virtual public Map2D::Layer {

	public:
		SweeneyBackgroundLayer(ItemPtrVectorPtr& items,
			SweeneyMapType::image_map_sptr imgMap, ItemPtrVectorPtr& validItems);

		virtual ~SweeneyBackgroundLayer();

		gamegraphics::ImagePtr imageFromCode(unsigned int code,
			camoto::gamegraphics::VC_TILESET& tileset);

		gamegraphics::PaletteTablePtr getPalette(gamegraphics::VC_TILESET& tileset);

	protected:
		SweeneyMapType::image_map_sptr imgMap;

};

class SweeneyObjectLayer: virtual public Map2D::Layer {

	public:
		SweeneyObjectLayer(ItemPtrVectorPtr& items,
			SweeneyMapType::image_map_sptr imgMap, ItemPtrVectorPtr& validItems);

		virtual ~SweeneyObjectLayer();

		gamegraphics::ImagePtr imageFromCode(unsigned int code,
			camoto::gamegraphics::VC_TILESET& tileset);

		gamegraphics::PaletteTablePtr getPalette(gamegraphics::VC_TILESET& tileset);

	protected:
		SweeneyMapType::image_map_sptr imgMap;

};

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAP_XARGON_HPP_
