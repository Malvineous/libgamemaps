/**
 * @file   test-map-ccaves.cpp
 * @brief  Test code for Crystal Caves maps.
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

/// 10 empty tiles in a line
#define empty_10x1 \
	"\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"

/// 40 empty tiles in a line + len (one entire map row)
#define empty_40x1 \
	"\x28" empty_10x1 empty_10x1 empty_10x1 empty_10x1

/// 40x2 empty tiles
#define empty_40x2 \
	empty_40x1 empty_40x1

#define testdata_initialstate \
	"\x28" "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A" \
		empty_10x1 empty_10x1 empty_10x1 \
	empty_40x2

#define MAP_WIDTH_PIXELS  (40*16)
#define MAP_HEIGHT_PIXELS (3*16)
#define MAP_LAYER_COUNT   1
#define MAP_FIRST_CODE    0x01

#define MAP_CLASS fmt_map_ccaves
#define MAP_TYPE  "map-ccaves"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.

// Too small
ISINSTANCE_TEST(c01,
	"\x28" "\x01\x02\x03\x04\x05\x06\x07\x08\x09"
	,
	gm::MapType::DefinitelyNo
);

// Wrong row length
ISINSTANCE_TEST(c02,
	"\x29" "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B"
		empty_10x1 empty_10x1 empty_10x1
	empty_40x2
	,
	gm::MapType::DefinitelyNo
);

// Incomplete row
ISINSTANCE_TEST(c03,
	empty_40x2
	"\x28" "\x01\x02\x03\x04\x05\x06\x07\x08\x09"
		empty_10x1 empty_10x1 empty_10x1
	,
	gm::MapType::DefinitelyNo
);

// Invalid tile code
ISINSTANCE_TEST(c04,
	empty_40x2
	"\x28" "\x01\xFF\x03\x04\x05\x06\x07\x08\x09\x0A"
		empty_10x1 empty_10x1 empty_10x1
	,
	gm::MapType::DefinitelyNo
);

/// 40x10 empty tiles
#define empty_40x10 \
	empty_40x2 empty_40x2 empty_40x2 empty_40x2 empty_40x2

// Map too tall
ISINSTANCE_TEST(c05,
	empty_40x10 empty_40x10 empty_40x10 empty_40x10
	empty_40x10 empty_40x10 empty_40x10 empty_40x10
	empty_40x10 empty_40x10
	empty_40x1
	,
	gm::MapType::DefinitelyNo
);
