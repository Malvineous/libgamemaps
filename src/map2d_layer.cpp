/**
 * @file   map2d_layer.cpp
 * @brief  Generic layer in a 2D grid-based map.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

Map2D::Layer::Layer(const std::string& title, int caps, int width, int height,
	int tileWidth, int tileHeight, ItemPtrVectorPtr& items,
	FN_IMAGEFROMCODE fnImageFromCode, FN_TILEPERMITTEDAT fnTilePermittedAt)
	throw () :
		title(title),
		caps(caps),
		width(width), height(height),
		tileWidth(tileWidth), tileHeight(tileHeight),
		items(items),
		fnImageFromCode(fnImageFromCode),
		fnTilePermittedAt(fnTilePermittedAt) // may be NULL
{
	assert(fnImageFromCode);
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

void Map2D::Layer::getLayerSize(int *x, int *y)
	throw ()
{
	assert(this->getCaps() & HasOwnSize);

	*x = this->width;
	*y = this->height;
	return;
}

void Map2D::Layer::setLayerSize(int x, int y)
	throw ()
{
	assert(this->getCaps() & CanResize);

	this->width = x;
	this->height = y;
	return;
}

void Map2D::Layer::getTileSize(int *x, int *y)
	throw ()
{
	assert(this->getCaps() & HasOwnTileSize);

	*x = this->tileWidth;
	*y = this->tileHeight;
	return;
}

void Map2D::Layer::setTileSize(int x, int y)
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

camoto::gamegraphics::ImagePtr Map2D::Layer::imageFromCode(unsigned int code,
	camoto::gamegraphics::VC_TILESET& tileset)
	throw ()
{
	return this->fnImageFromCode(code, tileset);
}

bool Map2D::Layer::tilePermittedAt(unsigned int code, unsigned int x,
	unsigned int y, unsigned int *maxCount)
	throw ()
{
	assert(maxCount);
	if (this->fnTilePermittedAt) {
		return this->fnTilePermittedAt(code, x, y, maxCount);
	}

	// Defaults
	*maxCount = 0; // unlimited
	return true; // permitted here
}

Map2D::Layer::Item::~Item()
	throw ()
{
}

Map2D::Layer::Text::~Text()
	throw ()
{
}

} // namespace gamemaps
} // namespace camoto
