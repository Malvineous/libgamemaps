/**
 * @file   map-generic.cpp
 * @brief  Generic implementation of Map interface.
 *
 * Copyright (C) 2010-2013 Adam Nielsen <malvineous@shikadi.net>
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

#include "map-generic.hpp"

namespace camoto {
namespace gamemaps {

GenericMap::GenericMap(AttributePtrVectorPtr attributes,
	GraphicsFilenamesCallback fnGfxFiles)
	:	attributes(attributes),
		fnGfxFiles(fnGfxFiles)
{
}

GenericMap::~GenericMap()
{
}

Map::AttributePtrVectorPtr GenericMap::getAttributes()
{
	return this->attributes;
}

const Map::AttributePtrVectorPtr GenericMap::getAttributes() const
{
	return this->attributes;
}

Map::GraphicsFilenamesPtr GenericMap::getGraphicsFilenames() const
{
	if (this->fnGfxFiles) return this->fnGfxFiles(this);
	return Map::GraphicsFilenamesPtr();
}

} // namespace gamemaps
} // namespace camoto
