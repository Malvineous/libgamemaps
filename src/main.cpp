/**
 * @file   libgamemaps/src/main.cpp
 * @brief  Main entry point for libgamemaps.
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

// Include all the file formats for the Manager to load
#include "fmt-map-bash.hpp"
#include "fmt-map-ccaves.hpp"
#include "fmt-map-ccomic.hpp"
#include "fmt-map-cosmo.hpp"
#include "fmt-map-darkages.hpp"
#include "fmt-map-ddave.hpp"
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

namespace camoto {
namespace gamemaps {

class ActualManager: virtual public Manager
{
	private:
		/// List of available map types.
		MapTypeVector vcTypes;

	public:
		ActualManager();
		~ActualManager();

		virtual const MapTypePtr getMapType(unsigned int iIndex) const;
		virtual const MapTypePtr getMapTypeByCode(const std::string& strCode) const;
};

const ManagerPtr getManager()
{
	return ManagerPtr(new ActualManager());
}

ActualManager::ActualManager()
{
	this->vcTypes.push_back(MapTypePtr(new BashMapType()));
	this->vcTypes.push_back(MapTypePtr(new CCavesMapType()));
	this->vcTypes.push_back(MapTypePtr(new CComicMapType()));
	this->vcTypes.push_back(MapTypePtr(new CosmoMapType()));
	this->vcTypes.push_back(MapTypePtr(new DarkAgesMapType()));
	this->vcTypes.push_back(MapTypePtr(new DDaveMapType()));
	this->vcTypes.push_back(MapTypePtr(new GOTMapType()));
	this->vcTypes.push_back(MapTypePtr(new HarryMapType()));
	this->vcTypes.push_back(MapTypePtr(new HocusMapType()));
	this->vcTypes.push_back(MapTypePtr(new Nukem2MapType()));
	this->vcTypes.push_back(MapTypePtr(new JillMapType()));
	this->vcTypes.push_back(MapTypePtr(new RockfordMapType()));
	this->vcTypes.push_back(MapTypePtr(new SAgentMapType()));
	this->vcTypes.push_back(MapTypePtr(new SAgentWorldMapType()));
	this->vcTypes.push_back(MapTypePtr(new VinylMapType()));
	this->vcTypes.push_back(MapTypePtr(new WackyMapType()));
	this->vcTypes.push_back(MapTypePtr(new WordRescueMapType()));
	this->vcTypes.push_back(MapTypePtr(new XargonMapType()));
	this->vcTypes.push_back(MapTypePtr(new Zone66MapType()));
}

ActualManager::~ActualManager()
{
}

const MapTypePtr ActualManager::getMapType(unsigned int iIndex) const
{
	if (iIndex >= this->vcTypes.size()) return MapTypePtr();
	return this->vcTypes[iIndex];
}

const MapTypePtr ActualManager::getMapTypeByCode(const std::string& strCode)
	const
{
	for (MapTypeVector::const_iterator i = this->vcTypes.begin(); i != this->vcTypes.end(); i++) {
		if ((*i)->getMapCode().compare(strCode) == 0) return *i;
	}
	return MapTypePtr();
}

} // namespace gamemaps
} // namespace camoto
