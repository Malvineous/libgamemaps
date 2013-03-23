/**
 * @file   test-map-ccomic.cpp
 * @brief  Test code for Captain Comic maps.
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

#define testdata_initialstate \
	"\x03\x00" "\x05\x00" \
	"\x02\x01\x00" \
	"\x12\x11\x10" \
	"\x22\x21\x20" \
	"\x32\x31\x30" \
	"\x42\x41\x40"

#define MAP_WIDTH_PIXELS  (3*16)
#define MAP_HEIGHT_PIXELS (5*16)
#define MAP_LAYER_COUNT   1
#define MAP_FIRST_CODE_L1 0x02

#define MAP_CLASS fmt_map_ccomic
#define MAP_TYPE  "map-ccomic"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.

// Too short
ISINSTANCE_TEST(c01,
	"\x03\x00" "\x05"
	,
	gm::MapType::DefinitelyNo
);

// Dimensions too large for available data
ISINSTANCE_TEST(c02,
	"\x03\x00" "\x06\x00"
	"\x02\x01\x00"
	"\x12\x11\x10"
	"\x22\x21\x20"
	"\x32\x31\x30"
	"\x42\x41\x40"
	,
	gm::MapType::DefinitelyNo
);

// First tile byte is out of range
ISINSTANCE_TEST(c03,
	"\x03\x00" "\x05\x00"
	"\xFF\x01\x00"
	"\x12\x11\x10"
	"\x22\x21\x20"
	"\x32\x31\x30"
	"\x42\x41\x40"
	,
	gm::MapType::DefinitelyNo
);
