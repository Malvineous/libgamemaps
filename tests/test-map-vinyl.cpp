/**
 * @file   test-map-vinyl.cpp
 * @brief  Test code for Vinyl Goddess From Mars maps.
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
	"\x04\x00\x05\x00" \
	"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00" \
	"\x11\x00\x12\x00\x13\x00\x14\x00\x15\x00" \
	"\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00" \
	"\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00" \
	"\x11\x12\x13\x14\x15" \
	"\x21\x22\x23\x24\x25" \
	"\x31\x32\x33\x34\x35" \
	"\x41\x42\x43\x44\x45"

#define MAP_WIDTH_PIXELS  (5*16)
#define MAP_HEIGHT_PIXELS (4*16)
#define MAP_LAYER_COUNT   2
#define MAP_FIRST_CODE_L1 0x0001
#define MAP_FIRST_CODE_L2 0x11

#define MAP_CLASS fmt_map_vinyl
#define MAP_TYPE  "map-vinyl"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.

// Too small
ISINSTANCE_TEST(c01,
	"\x00\x00\x00"
	,
	gm::MapType::DefinitelyNo
);

// Truncated
ISINSTANCE_TEST(c02,
	"\x04\x00\x05\x00"
	"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00"
	"\x11\x00\x12\x00\x13\x00\x14\x00\x15\x00"
	"\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00"
	"\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00"
	"\x11\x12\x13\x14\x15"
	"\x21\x22\x23\x24\x25"
	"\x31\x32\x33\x34\x35"
	,
	gm::MapType::DefinitelyNo
);

// BG tile code out of range
ISINSTANCE_TEST(c03,
	"\x04\x00\x05\x00"
	"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00"
	"\x11\x00\x12\x00\x13\x00\x14\x00\x15\x00"
	"\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00"
	"\x31\x00\x32\x00\x33\x00\x34\x00\x35\xFF"
	"\x11\x12\x13\x14\x15"
	"\x21\x22\x23\x24\x25"
	"\x31\x32\x33\x34\x35"
	"\x41\x42\x43\x44\x45"
	,
	gm::MapType::DefinitelyNo
);
