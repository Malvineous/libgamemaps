/**
 * @file   test-map-wacky.cpp
 * @brief  Test code for Wacky Wheels maps.
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

/// 16 empty tiles in a line
#define empty_16x1 \
	"\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"

/// 64 empty tiles in a line (one entire map row)
#define empty_64x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1

/// 10 empty rows
#define empty_64x10 \
	empty_64x1 empty_64x1 empty_64x1 empty_64x1 empty_64x1 \
	empty_64x1 empty_64x1 empty_64x1 empty_64x1 empty_64x1

/// 64x63 empty tiles (empty map except for one row)
#define empty_64x63 \
	empty_64x10 empty_64x10 empty_64x10 empty_64x10 empty_64x10 empty_64x10 \
	empty_64x1 empty_64x1 empty_64x1

#define testdata_initialstate \
	"\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20" \
	empty_16x1 empty_16x1 empty_16x1 \
	empty_64x63

#define MAP_WIDTH_PIXELS  (64*16)
#define MAP_HEIGHT_PIXELS (64*16)
#define MAP_LAYER_COUNT   1
#define MAP_FIRST_CODE    0x20

#define MAP_CLASS fmt_map_wacky
#define MAP_TYPE  "map-wacky"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.

// Too small
ISINSTANCE_TEST(c01,
	empty_64x63
	,
	gm::MapType::DefinitelyNo
);

// Invalid tile code
ISINSTANCE_TEST(c02,
	"\xFF\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
	empty_16x1 empty_16x1 empty_16x1
	empty_64x63
	,
	gm::MapType::DefinitelyNo
);
