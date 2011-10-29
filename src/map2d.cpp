/**
 * @file   map2d.cpp
 * @brief  Implementation of base functions in Map2D class.
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

#include <camoto/gamemaps/map2d.hpp>
#include <cassert>

namespace camoto {
namespace gamemaps {

Map2D::Map2D(AttributePtrVectorPtr attributes, int caps,
	unsigned int viewportWidth, unsigned int viewportHeight, unsigned int width,
	unsigned int height, unsigned int tileWidth, unsigned int tileHeight,
	LayerPtrVector& layers, PathPtrVectorPtr paths)
	throw () :
		Map(attributes),
		caps(caps),
		viewportWidth(viewportWidth), viewportHeight(viewportHeight),
		width(width), height(height),
		tileWidth(tileWidth), tileHeight(tileHeight),
		layers(layers),
		paths(paths)
{
	assert(width > 0);
	assert(height > 0);
	assert(tileWidth > 0);
	assert(tileHeight > 0);
}

Map2D::~Map2D()
	throw ()
{
}

int Map2D::getCaps()
	throw ()
{
	return this->caps;
}

void Map2D::getViewport(unsigned int *x, unsigned int *y)
	throw ()
{
	assert(this->getCaps() & HasViewport);

	*x = this->viewportWidth;
	*y = this->viewportHeight;
	return;
}

void Map2D::getMapSize(unsigned int *x, unsigned int *y)
	throw ()
{
	*x = this->width;
	*y = this->height;
	return;
}

void Map2D::setMapSize(unsigned int x, unsigned int y)
	throw ()
{
	assert(this->getCaps() & CanResize);

	this->width = x;
	this->height = y;
	return;
}

void Map2D::getTileSize(unsigned int *x, unsigned int *y)
	throw ()
{
	*x = this->tileWidth;
	*y = this->tileHeight;
	return;
}

void Map2D::setTileSize(unsigned int x, unsigned int y)
	throw ()
{
	assert(this->getCaps() & ChangeTileSize);

	this->tileWidth = x;
	this->tileHeight = y;
	return;
}

unsigned int Map2D::getLayerCount()
	throw ()
{
	return this->layers.size();
}

Map2D::LayerPtr Map2D::getLayer(unsigned int index)
	throw ()
{
	assert(index < this->getLayerCount());
	return this->layers[index];
}

Map2D::PathPtrVectorPtr Map2D::getPaths()
	throw ()
{
	return this->paths;
}

} // namespace gamemaps
} // namespace camoto
