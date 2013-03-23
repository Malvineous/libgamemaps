/**
 * @file   test-map-cosmo.cpp
 * @brief  Test code for Cosmo's Cosmic Adventures maps.
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

/// 16 empty tiles in a line
#define empty_16x1 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/// 64x4 empty tiles in a line (four entire map rows)
#define empty_64x4 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1

/// 511 empty map rows
#define empty_64x511 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 empty_64x4 \
	empty_64x4 empty_64x4 empty_64x4 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1 \
	empty_16x1 empty_16x1 empty_16x1 empty_16x1

#define testdata_initialstate \
	"\x00\x00" "\x40\x00" "\x03\x00" \
	\
	"\x01\x00" "\x00\x00" "\x00\x00" \
	\
	"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00" \
	"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00" \
	empty_16x1 empty_16x1 empty_16x1 \
	empty_64x511

#define MAP_WIDTH_PIXELS  (64*8)
#define MAP_HEIGHT_PIXELS (512*8)
#define MAP_LAYER_COUNT   2
#define MAP_FIRST_CODE_L1 0x01 // 0x00 is empty tile and thus skipped
#define MAP_FIRST_CODE_L2 0x01

#define MAP_CLASS fmt_map_cosmo
#define MAP_TYPE  "map-cosmo"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.


// Too small
#define TOO_SHORT \
	"\x00\x00" "\x40\x00" "\x00\x00" \
	\
	"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00" \
	"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00" \
	empty_16x1 empty_16x1 empty_16x1
ISINSTANCE_TEST(c01a,
	TOO_SHORT
	,
	gm::MapType::DefinitelyNo
);
// Just large enough
ISINSTANCE_TEST(c01b,
	TOO_SHORT empty_64x511
	,
	gm::MapType::DefinitelyYes
);
#undef TOO_SHORT

// Map too wide
ISINSTANCE_TEST(c02,
	"\x00\x00" "\x00\xf0" "\x00\x00" \
	\
	"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00" \
	"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00" \
	empty_16x1 empty_16x1 empty_16x1 \
	empty_64x511
	,
	gm::MapType::DefinitelyNo
);

// Too many actors
ISINSTANCE_TEST(c03,
	"\x00\x00" "\x00\x40" "\x00\xf0" \
	\
	"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00" \
	"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00" \
	empty_16x1 empty_16x1 empty_16x1 \
	empty_64x511
	,
	gm::MapType::DefinitelyNo
);

// More actors than space in the file
ISINSTANCE_TEST(c04,
	"\x00\x00" "\x00\x40" "\x00\x10" \
	\
	"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00" \
	"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00" \
	empty_16x1 empty_16x1 empty_16x1 \
	empty_64x511
	,
	gm::MapType::DefinitelyNo
);
