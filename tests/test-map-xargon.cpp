/**
 * @file   test-map-xargon.cpp
 * @brief  Test code for Xargon maps.
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
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/// 128x1 empty tiles in a line (one entire map rows)
#define empty_128x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1

/// 63 empty map rows
#define empty_128x63 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	\
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1 empty_128x1 \
	empty_128x1 empty_128x1 empty_128x1

/// 97 bytes of nothing
#define empty_savedata \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00"

#define testdata_initialstate \
	"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	empty_16x1 empty_16x1 empty_16x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1 \
	empty_128x63 \
	"\x01\x00" \
	"\x01" "\x00\x00" "\x00\x00" \
	"\x00\x00" "\x00\x00" \
	"\x00\x00" "\x00\x00" \
	"\x00\x00" "\x00\x00" \
	"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00" \
	"\x00\x00" "\x00\x00" \
	empty_savedata

#define MAP_WIDTH_PIXELS  (128*16)
#define MAP_HEIGHT_PIXELS (64*16)
#define MAP_LAYER_COUNT   2
#define MAP_FIRST_CODE_L1 0x01
#define MAP_FIRST_CODE_L2 0x01

#define MAP_CLASS fmt_map_xargon
#define MAP_TYPE  "map-xargon"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.


// Too small
ISINSTANCE_TEST(c01,
	"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
	,
	gm::MapType::DefinitelyNo
);

// Exact size w/ no text section
ISINSTANCE_TEST(c02,
	"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	empty_16x1 empty_16x1 empty_16x1
	empty_16x1 empty_16x1 empty_16x1 empty_16x1
	empty_128x63
	"\x01\x00"
	"\x01" "\x10\x00" "\x10\x00"
	"\x00\x00" "\x00\x00"
	"\x00\x00" "\x00\x00"
	"\x00\x00" "\x00\x00"
	"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
	"\x00\x00" "\x00\x00"
	empty_savedata
	,
	gm::MapType::DefinitelyYes
);

// Truncated object layer
ISINSTANCE_TEST(c03,
	"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	empty_16x1 empty_16x1 empty_16x1
	empty_16x1 empty_16x1 empty_16x1 empty_16x1
	empty_128x63
	"\x01\x00"
	"\x01"
	,
	gm::MapType::DefinitelyNo
);
