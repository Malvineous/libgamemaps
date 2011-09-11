/**
 * @file   test-map-harry.cpp
 * @brief  Test code for Halloween Harry maps.
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

/// 16 empty bytes
#define empty_16 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/// 256 empty bytes
#define empty_256 \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 

#define testdata_initialstate \
	"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00" \
	"\x00" \
	empty_256 empty_256 empty_256 \
	empty_256 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x01\x00" \
	"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16\
	\
	"\x04\x00" "\x04\x00" \
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	\
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \

#define MAP_WIDTH_PIXELS  (4*16)
#define MAP_HEIGHT_PIXELS (4*16)
#define MAP_LAYER_COUNT   3
#define MAP_FIRST_CODE_L1 0x01 // 0x00 is empty tile and thus skipped
#define MAP_FIRST_CODE_L2 0x01 // 0x00 is empty tile and thus skipped
#define MAP_FIRST_CODE_L3 0x01 // 0x00 is empty tile and thus skipped

#define MAP_CLASS fmt_map_harry
#define MAP_TYPE  "map-harry"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.

// Too small
ISINSTANCE_TEST(c01,
	"\x11SubZero Game File"
	,
	gm::MapType::DefinitelyNo
);

// Bad signature
ISINSTANCE_TEST(c02,
	"\x11SubZero Lame File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00" \
	"\x00" \
	empty_256 empty_256 empty_256 \
	empty_256 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x01\x00" \
	"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16\
	\
	"\x04\x00" "\x04\x00" \
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	\
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	,
	gm::MapType::DefinitelyNo
);

// Palette out of range
ISINSTANCE_TEST(c03,
	"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00" \
	"\x00" \
	"\x41\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 \
	empty_256 empty_256 \
	empty_256 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x01\x00" \
	"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16\
	\
	"\x04\x00" "\x04\x00" \
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	\
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	,
	gm::MapType::DefinitelyNo
);

// Flags are out of range
ISINSTANCE_TEST(c04,
	"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00" \
	"\x00" \
	empty_256 empty_256 empty_256 \
	"\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x01\x00" \
	"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16\
	\
	"\x04\x00" "\x04\x00" \
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	\
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	,
	gm::MapType::DefinitelyNo
);

// Actor data runs past EOF
ISINSTANCE_TEST(c05,
	"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00" \
	"\x00" \
	empty_256 empty_256 empty_256 \
	empty_256 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x20" \
	"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16\
	\
	"\x04\x00" "\x04\x00" \
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	\
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00"
	,
	gm::MapType::DefinitelyNo
);

// BG or FG layer cut short
#define TOO_SHORT \
	"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00" \
	"\x00" \
	empty_256 empty_256 empty_256 \
	empty_256 \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x01\x00" \
	"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	empty_16 empty_16 empty_16 empty_16 empty_16 empty_16 empty_16\
	\
	"\x04\x00" "\x04\x00" \
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f\x00" \
	\
	"\x01\x02\x03\x04" \
	"\x05\x06\x07\x08" \
	"\x09\x0a\x0b\x0c" \
	"\x0d\x0e\x0f"
ISINSTANCE_TEST(c06a,
	TOO_SHORT
	,
	gm::MapType::DefinitelyNo
);
ISINSTANCE_TEST(c06b,
	TOO_SHORT "\x00"
	,
	gm::MapType::DefinitelyYes
);
#undef TOO_SHORT
