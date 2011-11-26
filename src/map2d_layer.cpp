/**
 * @file   map2d_layer.cpp
 * @brief  Generic layer in a 2D grid-based map.
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

namespace camoto {
namespace gamemaps {

Map2D::Layer::Layer(const std::string& title, int caps, unsigned int width,
	unsigned int height, unsigned int tileWidth, unsigned int tileHeight,
	ItemPtrVectorPtr& items, ItemPtrVectorPtr& validItems)
	throw () :
		title(title),
		caps(caps),
		width(width), height(height),
		tileWidth(tileWidth), tileHeight(tileHeight),
		items(items),
		validItems(validItems)
{
}

Map2D::Layer::~Layer()
	throw ()
{
}

const std::string& Map2D::Layer::getTitle()
	throw ()
{
	return this->title;
}

int Map2D::Layer::getCaps()
	throw ()
{
	return this->caps;
}

void Map2D::Layer::getLayerSize(unsigned int *x, unsigned int *y)
	throw ()
{
	assert(this->getCaps() & HasOwnSize);

	*x = this->width;
	*y = this->height;
	return;
}

void Map2D::Layer::setLayerSize(unsigned int x, unsigned int y)
	throw ()
{
	assert(this->getCaps() & CanResize);

	this->width = x;
	this->height = y;
	return;
}

void Map2D::Layer::getTileSize(unsigned int *x, unsigned int *y)
	throw ()
{
	assert(this->getCaps() & HasOwnTileSize);

	*x = this->tileWidth;
	*y = this->tileHeight;
	return;
}

void Map2D::Layer::setTileSize(unsigned int x, unsigned int y)
	throw ()
{
	assert(this->getCaps() & ChangeTileSize);

	this->tileWidth = x;
	this->tileHeight = y;
	return;
}

const Map2D::Layer::ItemPtrVectorPtr Map2D::Layer::getAllItems()
	throw ()
{
	return this->items;
}

gamegraphics::ImagePtr Map2D::Layer::imageFromCode(unsigned int code,
	gamegraphics::VC_TILESET& tileset)
	throw ()
{
	// Default implementation to return an empty tile.
	return gamegraphics::ImagePtr();
}

bool Map2D::Layer::tilePermittedAt(unsigned int code, unsigned int x,
	unsigned int y, unsigned int *maxCount)
	throw ()
{
	assert(maxCount);

	// Defaults
	*maxCount = 0; // unlimited
	return true; // permitted here
}

gamegraphics::PaletteTablePtr Map2D::Layer::getPalette(
	camoto::gamegraphics::VC_TILESET& tileset)
	throw ()
{
	assert(this->getCaps() & HasPalette);
	std::cerr << "BUG: Map2D::Layer reported having a palette but didn't "
		"implement getPalette()" << std::endl;
	return gamegraphics::PaletteTablePtr();
}

const Map2D::Layer::ItemPtrVectorPtr Map2D::Layer::getValidItemList()
	throw ()
{
	return this->validItems;
}

Map2D::Layer::Item::~Item()
	throw ()
{
}

Map2D::Layer::Item::Text::~Text()
	throw ()
{
}

Map2D::Layer::Item::Movable::~Movable()
	throw ()
{
}

} // namespace gamemaps
} // namespace camoto
