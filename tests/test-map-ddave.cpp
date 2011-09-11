/**
 * @file   test-map-ddave.cpp
 * @brief  Test code for Dangerous Dave maps.
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

/// 10 empty tiles in a line
#define empty_10x1 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/// 90 empty tiles in a line
#define empty_90x1 \
	empty_10x1 empty_10x1 empty_10x1 empty_10x1 empty_10x1 \
	empty_10x1 empty_10x1 empty_10x1 empty_10x1

/// 100 empty tiles in a line (one entire map row)
#define empty_100x1 \
	empty_90x1 empty_10x1

/// 100x8 empty tiles
#define empty_100x8 \
	empty_100x1 empty_100x1 empty_100x1 empty_100x1 \
	empty_100x1 empty_100x1 empty_100x1 empty_100x1

/// 24 bytes of padding
#define tile_padding \
	"\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0"

/// Short path with only two elements + end code
#define initial_path \
	"\x03\x05\xfd\xfb\xea\xea\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \

#define testdata_initialstate \
	initial_path \
	"\x01\x03\x04\x02\x05\x08\x00\x00\x00\x00" empty_90x1 \
	empty_100x8 \
	empty_100x1 \
	tile_padding

#define testdata_first_8x1_copied \
	initial_path \
	"\x01\x03\x04\x02\x05\x08\x06\x07\x00\x00" empty_90x1 \
	empty_100x8 \
	empty_90x1 "\x01\x03\x04\x02\x05\x08\x06\x07\x00\x00" \
	tile_padding

#define MAP_WIDTH_PIXELS  (100*16)
#define MAP_HEIGHT_PIXELS (10*16)
#define MAP_LAYER_COUNT   1
#define MAP_FIRST_CODE_L1 0x01

#define MAP_CLASS fmt_map_ddave
#define MAP_TYPE  "map-ddave"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.

// Wrong length
ISINSTANCE_TEST(c01,
	initial_path
	"\x12"
	"\x01\x03\x04\x02\x05\x08\x00\x00\x00\x00" empty_90x1
	empty_100x8
	empty_100x1
	tile_padding
	,
	gm::MapType::DefinitelyNo
);

// First tile byte is out of range
ISINSTANCE_TEST(c02,
	initial_path
	"\xff\x03\x04\x02\x05\x08\x00\x00\x00\x00" empty_90x1
	empty_100x8
	empty_100x1
	tile_padding
	,
	gm::MapType::DefinitelyNo
);
