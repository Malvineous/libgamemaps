/**
 * @file  libgamemaps/src/main.cpp
 * @brief Main entry point for libgamemaps.
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

#include <camoto/gamemaps/manager.hpp>

#include "fmt-map-bash.hpp"
#include "fmt-map-ccaves.hpp"
#include "fmt-map-ccomic.hpp"
#include "fmt-map-cosmo.hpp"
#include "fmt-map-darkages.hpp"
#include "fmt-map-ddave.hpp"
#include "fmt-map-duke1.hpp"
#include "fmt-map-got.hpp"
#include "fmt-map-harry.hpp"
#include "fmt-map-hocus.hpp"
#include "fmt-map-nukem2.hpp"
#include "fmt-map-rockford.hpp"
#include "fmt-map-sagent.hpp"
#include "fmt-map-vinyl.hpp"
#include "fmt-map-wacky.hpp"
#include "fmt-map-wordresc.hpp"
#include "fmt-map-xargon.hpp"
#include "fmt-map-zone66.hpp"

using namespace camoto::gamemaps;

namespace camoto {

template <>
const std::vector<std::shared_ptr<const MapType> > CAMOTO_GAMEMAPS_API
	FormatEnumerator<MapType>::formats()
{
	std::vector<std::shared_ptr<const MapType> > list;
	FormatEnumerator<MapType>::addFormat<
		MapType_Bash,
		MapType_CCaves,
		MapType_CComic,
		MapType_Cosmo,
		MapType_DarkAges,
		MapType_DDave,
		MapType_Duke1,
		MapType_GOT,
		MapType_Harry,
		MapType_Hocus,
		MapType_Nukem2,
		MapType_Jill,
		MapType_Rockford,
		MapType_SAgent,
		MapType_SAgentWorld,
		MapType_Vinyl,
		MapType_Wacky,
		MapType_WordRescue,
		MapType_Xargon,
		MapType_Zone66
	>(list);
	return list;
}

namespace gamemaps {

constexpr const char* const MapType::obj_t_name;

} // namespace gamegraphics

} // namespace camoto
