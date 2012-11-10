/**
 * @file   map2d_layer.cpp
 * @brief  Generic layer in a 2D grid-based map.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#include "map2d-generic.hpp"

namespace camoto {
namespace gamemaps {

GenericMap2D::Layer::Layer(const std::string& title, int caps, unsigned int width,
	unsigned int height, unsigned int tileWidth, unsigned int tileHeight,
	ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
	:	title(title),
		caps(caps),
		width(width), height(height),
		tileWidth(tileWidth), tileHeight(tileHeight),
		items(items),
		validItems(validItems)
{
}

GenericMap2D::Layer::~Layer()
{
}

const std::string& GenericMap2D::Layer::getTitle()
{
	return this->title;
}

int GenericMap2D::Layer::getCaps()
{
	return this->caps;
}

void GenericMap2D::Layer::getLayerSize(unsigned int *x, unsigned int *y)
{
	assert(this->getCaps() & HasOwnSize);

	*x = this->width;
	*y = this->height;
	return;
}

void GenericMap2D::Layer::setLayerSize(unsigned int x, unsigned int y)
{
	assert(this->getCaps() & CanResize);

	this->width = x;
	this->height = y;
	return;
}

void GenericMap2D::Layer::getTileSize(unsigned int *x, unsigned int *y)
{
	assert(this->getCaps() & HasOwnTileSize);

	*x = this->tileWidth;
	*y = this->tileHeight;
	return;
}

void GenericMap2D::Layer::setTileSize(unsigned int x, unsigned int y)
{
	assert(this->getCaps() & ChangeTileSize);

	this->tileWidth = x;
	this->tileHeight = y;
	return;
}

const Map2D::Layer::ItemPtrVectorPtr GenericMap2D::Layer::getAllItems()
{
	return this->items;
}

gamegraphics::ImagePtr GenericMap2D::Layer::imageFromCode(
	const Map2D::Layer::ItemPtr& item,
	const TilesetCollectionPtr& tileset)
{
	// Default implementation to return an empty tile.
	return gamegraphics::ImagePtr();
}

bool GenericMap2D::Layer::tilePermittedAt(const Map2D::Layer::ItemPtr& item,
	unsigned int x, unsigned int y, unsigned int *maxCount)
{
	assert(maxCount);

	// Defaults
	*maxCount = 0; // unlimited
	return true; // permitted here
}

gamegraphics::PaletteTablePtr GenericMap2D::Layer::getPalette(
	const TilesetCollectionPtr& tileset)
{
	assert(this->getCaps() & HasPalette);
	std::cerr << "BUG: GenericMap2D::Layer reported having a palette but didn't "
		"implement getPalette()" << std::endl;
	return gamegraphics::PaletteTablePtr();
}

const Map2D::Layer::ItemPtrVectorPtr GenericMap2D::Layer::getValidItemList()
{
	return this->validItems;
}

} // namespace gamemaps
} // namespace camoto
