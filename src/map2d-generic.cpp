/**
 * @file   map2d-generic.cpp
 * @brief  Generic implementation of a Map2D interface.
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

#include <cassert>
#include "map2d-generic.hpp"

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

GenericMap2D::GenericMap2D(const Attributes& attributes,
	const GraphicsFilenames& graphicsFilenames, unsigned int caps,
	unsigned int viewportWidth, unsigned int viewportHeight,
	unsigned int width, unsigned int height,
	unsigned int tileWidth, unsigned int tileHeight,
	const LayerPtrVector& layers, PathPtrVectorPtr paths)
	:	Map2D(attributes, graphicsFilenames, caps, viewportWidth, viewportHeight),
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

GenericMap2D::~GenericMap2D()
{
}

void GenericMap2D::getMapSize(unsigned int *x, unsigned int *y) const
{
	*x = this->width;
	*y = this->height;
	return;
}

void GenericMap2D::setMapSize(unsigned int x, unsigned int y)
{
	assert(this->caps & CanResize);

	this->width = x;
	this->height = y;
	return;
}

void GenericMap2D::getTileSize(unsigned int *x, unsigned int *y) const
{
	*x = this->tileWidth;
	*y = this->tileHeight;
	return;
}

void GenericMap2D::setTileSize(unsigned int x, unsigned int y)
{
	assert(this->caps & ChangeTileSize);

	this->tileWidth = x;
	this->tileHeight = y;
	return;
}

unsigned int GenericMap2D::getLayerCount() const
{
	return this->layers.size();
}

Map2D::LayerPtr GenericMap2D::getLayer(unsigned int index)
{
	assert(index < this->getLayerCount());
	return this->layers[index];
}

Map2D::PathPtrVectorPtr GenericMap2D::getPaths()
{
	return this->paths;
}

Map2D::ImageAttachment GenericMap2D::getBackgroundImage(
	const TilesetCollectionPtr& tileset, ImagePtr *outImage,
	PaletteEntry *outColour) const
{
	return Map2D::NoBackground;
}

} // namespace gamemaps
} // namespace camoto
