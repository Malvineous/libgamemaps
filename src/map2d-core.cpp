/**
 * @file  map2d-core.cpp
 * @brief Implementation of Map2D functions inherited by most format handlers.
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
#include "map2d-core.hpp"

namespace camoto {
namespace gamemaps {

using namespace camoto::gamegraphics;

Map2DCore::~Map2DCore()
{
}

Point Map2DCore::viewport() const
{
	// If this assertion fails, it means the map format returned a caps() value
	// suggesting it has a viewport, but didn't override viewport() to provide
	// the actual viewport dimensions.
	assert(!(this->caps() & Caps::HasViewport));

	// If this assertion fails, it means the caps() value reported no viewport
	// available, but the caller tried to retrieve the viewport value anyway.
	// This is a bug - if caps() reports no viewport, you cannot call this
	// function.
	assert(false);
}

Point Map2DCore::mapSize() const
{
	// If this assertion fails, it means the map format returned a caps() value
	// suggesting it has a global size, but didn't override mapSize() to provide
	// the actual map dimensions.
	assert(!(this->caps() & Caps::HasMapSize));

	// If this assertion fails, it means the caps() value reported no global
	// map size, but the caller tried to retrieve the value anyway.
	// This is a bug - if caps() reports no map size, you cannot call this
	// function.
	assert(false);
	return {};
}

void Map2DCore::mapSize(const Point& newSize)
{
	// If this assertion fails, it means the map format returned a caps() value
	// suggesting it can be resized, but didn't override mapSize() to provide
	// an actual resize implementation.
	assert(!(this->caps() & Caps::SetMapSize));

	// If this assertion fails, it means the caps() value reported that the map
	// cannot be resized, but the caller tried to resize it anyway.
	// This is a bug - if caps() reports no resize, you cannot call this
	// function.
	assert(false);
}

Point Map2DCore::tileSize() const
{
	// If this assertion fails, it means the map format returned a caps() value
	// suggesting it has a global tile size, but didn't override tileSize() to
	// provide the actual tile dimensions.
	assert(!(this->caps() & Caps::HasTileSize));

	// If this assertion fails, it means the caps() value reported no global
	// tile size, but the caller tried to retrieve the value anyway.
	// This is a bug - if caps() reports no tile size, you cannot call this
	// function.
	assert(false);
	return {};
}

void Map2DCore::tileSize(const Point& newSize)
{
	// If this assertion fails, it means the map format returned a caps() value
	// suggesting the tiles can be resized, but didn't override tileSize() to
	// provide an actual resize implementation.
	assert(!(this->caps() & Caps::SetTileSize));

	// If this assertion fails, it means the caps() value reported that the map
	// cannot be resized, but the caller tried to resize it anyway.
	// This is a bug - if caps() reports no resize, you cannot call this
	// function.
	assert(false);
}

std::vector<std::shared_ptr<Map2D::Layer>> Map2DCore::layers()
{
	return this->v_layers;
}

std::vector<std::shared_ptr<const Map2D::Layer>> Map2DCore::layers() const
{
	return std::vector<std::shared_ptr<const Map2D::Layer>>(
		this->v_layers.begin(), this->v_layers.end());
}

std::vector<std::shared_ptr<Map2D::Path>>& Map2DCore::paths()
{
	return this->v_paths;
}

Map2D::Background Map2DCore::background(const TilesetCollection& tileset)
	const
{
	Background bg;
	bg.att = Background::Attachment::NoBackground;
	return bg;
}

Map2D::Background Map2DCore::backgroundFromTilecode(
	const TilesetCollection& tileset, unsigned int code) const
{
	Layer::Item item;
	item.code = code;
	auto imgInfo = this->v_layers[0]->imageFromCode(item, tileset);

	Background bg;
	if (imgInfo.type == Layer::ImageFromCodeInfo::ImageType::Supplied) {
		// Got the image for the default tile, use that
		bg.att = Background::Attachment::SingleImageTiled;
		bg.img = imgInfo.img;
	} else {
		// Couldn't get the tile image for some reason, use transparent BG
		bg.att = Background::Attachment::NoBackground;
	}
	return bg;
}

Map2D::Background Map2DCore::backgroundUseBGImage(
	const TilesetCollection& tileset) const
{
	Background bg;
	bg.att = Background::Attachment::NoBackground;

	auto t = tileset.find(ImagePurpose::BackgroundImage);
	if (t != tileset.end()) {
		auto images = t->second->files();
		if (images.size() > 0) {
			// Just open the first image, it will have been whatever was supplied
			// by this->graphicsFilenames[BackgroundImage]
			bg.att = Background::Attachment::SingleImageCentred;
			bg.img = t->second->openImage(images[0]);
		}
	}
	return bg;
}

Map2DCore::LayerCore::~LayerCore()
{
}

Point Map2DCore::LayerCore::layerSize() const
{
	assert(this->caps() & Map2D::Layer::Caps::HasOwnSize);
	return v_layerSize;
}

void Map2DCore::LayerCore::layerSize(const Point& newSize)
{
	assert(this->caps() & Map2D::Layer::Caps::SetOwnSize);
	this->v_layerSize = newSize;
	return;
}

Point Map2DCore::LayerCore::tileSize() const
{
	assert(this->caps() & Map2D::Layer::Caps::HasOwnTileSize);
	return v_tileSize;
}

void Map2DCore::LayerCore::tileSize(const Point& newSize)
{
	assert(this->caps() & Map2D::Layer::Caps::SetOwnTileSize);
	this->v_tileSize = newSize;
	return;
}

std::vector<Map2D::Layer::Item>& Map2DCore::LayerCore::items()
{
	return this->v_allItems;
}

std::vector<Map2D::Layer::Item> Map2DCore::LayerCore::items() const
{
	return this->v_allItems;
}

Map2D::Layer::ImageFromCodeInfo Map2DCore::LayerCore::imageFromCode(
	const Map2D::Layer::Item& item, const TilesetCollection& tileset) const
{
	// Default implementation to return a question-mark/unknown tile.
	ImageFromCodeInfo info;
	//info.type = Map2D::Layer::Item::ImageFromCodeInfo::Unknown;
	info.type = ImageFromCodeInfo::ImageType::Unknown;
	return info;
}

bool Map2DCore::LayerCore::tilePermittedAt(const Map2D::Layer::Item& item,
	const Point& pos, unsigned int *maxCount) const
{
	assert(maxCount);

	// Defaults
	*maxCount = 0; // unlimited
	return true; // permitted here
}

std::shared_ptr<const gamegraphics::Palette> Map2DCore::LayerCore::palette(
	const TilesetCollection& tileset) const
{
	assert(this->caps() & Map2D::Layer::Caps::HasPalette);

	throw camoto::error("BUG: Map2D::Layer implementation reported having a "
		"palette but didn't implement getPalette()!");
}

} // namespace gamemaps
} // namespace camoto
