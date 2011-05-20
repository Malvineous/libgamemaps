/**
 * @file   main.cpp
 * @brief  Main entry point for libgamemaps.
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

#include <string>
#include <camoto/gamemaps.hpp>
#include <camoto/debug.hpp>

// Include all the file formats for the Manager to load
#include "fmt-map-ddave.hpp"
#include "fmt-map-ccaves.hpp"
#include "fmt-map-cosmo.hpp"
#include "fmt-map-harry.hpp"
#include "fmt-map-wacky.hpp"
#include "fmt-map-xargon.hpp"

namespace camoto {
namespace gamemaps {

ManagerPtr getManager()
	throw ()
{
	return ManagerPtr(new Manager());
}

Manager::Manager()
	throw ()
{
	this->vcTypes.push_back(MapTypePtr(new DDaveMapType()));
	this->vcTypes.push_back(MapTypePtr(new CCavesMapType()));
	this->vcTypes.push_back(MapTypePtr(new CosmoMapType()));
	this->vcTypes.push_back(MapTypePtr(new HarryMapType()));
	this->vcTypes.push_back(MapTypePtr(new WackyMapType()));
	this->vcTypes.push_back(MapTypePtr(new XargonMapType()));
}

Manager::~Manager()
	throw ()
{
}

MapTypePtr Manager::getMapType(int iIndex)
	throw ()
{
	if (iIndex >= this->vcTypes.size()) return MapTypePtr();
	return this->vcTypes[iIndex];
}

MapTypePtr Manager::getMapTypeByCode(const std::string& strCode)
	throw ()
{
	for (VC_MAPTYPE::const_iterator i = this->vcTypes.begin(); i != this->vcTypes.end(); i++) {
		if ((*i)->getMapCode().compare(strCode) == 0) return *i;
	}
	return MapTypePtr();
}

} // namespace gamemaps
} // namespace camoto
