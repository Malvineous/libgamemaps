/**
 * @file  map-core.cpp
 * @brief Implementation of Map functions inherited by most format handlers.
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

#include "map-core.hpp"

namespace camoto {
namespace gamemaps {

const char* toString(ImagePurpose p)
{
	switch (p) {
		case ImagePurpose::GenericTileset1: return "GenericTileset1";
		case ImagePurpose::GenericTileset2: return "GenericTileset2";
		case ImagePurpose::GenericTileset3: return "GenericTileset3";
		case ImagePurpose::GenericTileset4: return "GenericTileset4";
		case ImagePurpose::GenericTileset5: return "GenericTileset5";
		case ImagePurpose::GenericTileset6: return "GenericTileset6";
		case ImagePurpose::GenericTileset7: return "GenericTileset7";
		case ImagePurpose::GenericTileset8: return "GenericTileset8";
		case ImagePurpose::GenericTileset9: return "GenericTileset9";
		case ImagePurpose::BackgroundTileset1: return "BackgroundTileset1";
		case ImagePurpose::BackgroundTileset2: return "BackgroundTileset2";
		case ImagePurpose::BackgroundTileset3: return "BackgroundTileset3";
		case ImagePurpose::BackgroundTileset4: return "BackgroundTileset4";
		case ImagePurpose::BackgroundTileset5: return "BackgroundTileset5";
		case ImagePurpose::BackgroundTileset6: return "BackgroundTileset6";
		case ImagePurpose::BackgroundTileset7: return "BackgroundTileset7";
		case ImagePurpose::BackgroundTileset8: return "BackgroundTileset8";
		case ImagePurpose::BackgroundTileset9: return "BackgroundTileset9";
		case ImagePurpose::ForegroundTileset1: return "ForegroundTileset1";
		case ImagePurpose::ForegroundTileset2: return "ForegroundTileset2";
		case ImagePurpose::ForegroundTileset3: return "ForegroundTileset3";
		case ImagePurpose::ForegroundTileset4: return "ForegroundTileset4";
		case ImagePurpose::ForegroundTileset5: return "ForegroundTileset5";
		case ImagePurpose::ForegroundTileset6: return "ForegroundTileset6";
		case ImagePurpose::ForegroundTileset7: return "ForegroundTileset7";
		case ImagePurpose::ForegroundTileset8: return "ForegroundTileset8";
		case ImagePurpose::ForegroundTileset9: return "ForegroundTileset9";
		case ImagePurpose::SpriteTileset1: return "SpriteTileset1";
		case ImagePurpose::SpriteTileset2: return "SpriteTileset2";
		case ImagePurpose::SpriteTileset3: return "SpriteTileset3";
		case ImagePurpose::SpriteTileset4: return "SpriteTileset4";
		case ImagePurpose::SpriteTileset5: return "SpriteTileset5";
		case ImagePurpose::SpriteTileset6: return "SpriteTileset6";
		case ImagePurpose::SpriteTileset7: return "SpriteTileset7";
		case ImagePurpose::SpriteTileset8: return "SpriteTileset8";
		case ImagePurpose::SpriteTileset9: return "SpriteTileset9";
		case ImagePurpose::FontTileset1: return "FontTileset1";
		case ImagePurpose::FontTileset2: return "FontTileset2";
		case ImagePurpose::FontTileset3: return "FontTileset3";
		case ImagePurpose::FontTileset4: return "FontTileset4";
		case ImagePurpose::FontTileset5: return "FontTileset5";
		case ImagePurpose::FontTileset6: return "FontTileset6";
		case ImagePurpose::FontTileset7: return "FontTileset7";
		case ImagePurpose::FontTileset8: return "FontTileset8";
		case ImagePurpose::FontTileset9: return "FontTileset9";
		case ImagePurpose::BackgroundImage: return "BackgroundImage";
		case ImagePurpose::ImagePurposeCount: break; // prevent compiler warning
	}
	return "<unknown ImagePurpose>";
}

MapCore::~MapCore()
{
}

} // namespace gamemaps
} // namespace camoto
