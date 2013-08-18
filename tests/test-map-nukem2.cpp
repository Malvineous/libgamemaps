/**
 * @file   test-map-nukem2.cpp
 * @brief  Test code for Duke Nukem II maps.
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

/// 46x1 empty tiles in a line (trailing end in 64x511 map to pad to 32750 bytes)
#define empty_46x1 \
	empty_16x1 empty_16x1 "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" \
	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

/// 510 empty map rows
#define empty_64x510 \
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
	empty_46x1

#define testdata_initialstate \
	"\x35\x00" \
	"czone1.mni  \0" \
	"drop1.mni   \0" \
	"demosong.imf\0" \
	"\x01\x02\x00\x00" \
	"\x03\x00" /* Actor ints */ \
	"\x02\x00\x00\x00\x00\x00" \
	"\x40\x00" /* Map width */ \
	"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0" \
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0" \
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0" \
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0" \
	empty_16x1 empty_16x1 \
	empty_64x510 \
	"\x0B\x00" /* Extra length */ \
	"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67" \
	"\x00\x00" \
	"attrfile.mni\0" \
	"tile.mni\0\0\0\0\0" \
	"maskfile.mni\0"

#define MAP_WIDTH_PIXELS  (64*8)
#define MAP_HEIGHT_PIXELS (511*8)
#define MAP_LAYER_COUNT   3
#define MAP_FIRST_CODE_L1 0x01 // 0x00 is empty tile and thus skipped
#define MAP_FIRST_CODE_X_L1 0
#define MAP_FIRST_CODE_Y_L1 0
#define MAP_FIRST_CODE_L2 0x70
#define MAP_FIRST_CODE_X_L2 0
#define MAP_FIRST_CODE_Y_L2 0
#define MAP_FIRST_CODE_L3 0x02
#define MAP_FIRST_CODE_X_L3 0
#define MAP_FIRST_CODE_Y_L3 0

#define MAP_CLASS fmt_map_nukem2
#define MAP_TYPE  "map-nukem2"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.


// Too short
ISINSTANCE_TEST(c01,
	"\x35\x00"
	"czone1.mni  \0"
	"drop1.mni   \0"
	"demosong.imf\0"
	"\x01\x02\x00\x00"
	,
	gm::MapType::DefinitelyNo
);

// Offset past EOF
ISINSTANCE_TEST(c02,
	"\x00\xF0"
	"czone1.mni  \0"
	"drop1.mni   \0"
	"demosong.imf\0"
	"\x01\x02\x00\x00"
	"\x03\x00" /* Actor ints */
	"\x02\x00\x00\x00\x00\x00"
	"\x40\x00" /* Map width */
	"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	empty_16x1 empty_16x1
	empty_64x510
	"\x0B\x00" /* Extra length */
	"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
	"\x00\x00"
	"attrfile.mni\0"
	"tile.mni\0\0\0\0\0"
	"maskfile.mni\0"
	,
	gm::MapType::DefinitelyNo
);

// Actors go past EOF
ISINSTANCE_TEST(c03,
	"\x35\x00"
	"czone1.mni  \0"
	"drop1.mni   \0"
	"demosong.imf\0"
	"\x01\x02\x00\x00"
	"\x00\xFF" /* Actor ints */
	"\x02\x00\x00\x00\x00\x00"
	"\x40\x00" /* Map width */
	"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	empty_16x1 empty_16x1
	empty_64x510
	"\x0B\x00" /* Extra length */
	"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
	"\x00\x00"
	"attrfile.mni\0"
	"tile.mni\0\0\0\0\0"
	"maskfile.mni\0"
	,
	gm::MapType::DefinitelyNo
);

// Extra data too long
ISINSTANCE_TEST(c04,
	"\x35\x00"
	"czone1.mni  \0"
	"drop1.mni   \0"
	"demosong.imf\0"
	"\x01\x02\x00\x00"
	"\x03\x00" /* Actor ints */
	"\x02\x00\x00\x00\x00\x00"
	"\x40\x00" /* Map width */
	"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	empty_16x1 empty_16x1
	empty_64x510
	"\x00\xF0" /* Extra length */
	"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
	"\x00\x00"
	"attrfile.mni\0"
	"tile.mni\0\0\0\0\0"
	"maskfile.mni\0"
	,
	gm::MapType::DefinitelyNo
);

// Missing optional fields
ISINSTANCE_TEST(c05,
	"\x35\x00"
	"czone1.mni  \0"
	"drop1.mni   \0"
	"demosong.imf\0"
	"\x01\x02\x00\x00"
	"\x03\x00" /* Actor ints */
	"\x02\x00\x00\x00\x00\x00"
	"\x40\x00" /* Map width */
	"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
	empty_16x1 empty_16x1
	empty_64x510
	"\x0B\x00" /* Extra length */
	"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
	"\x00\x00"
	"attrfile.mni\0"
	"tile.mni\0\0\0\0\0"
	,
	gm::MapType::PossiblyYes
);
