/**
 * @file   test-map-wordresc.cpp
 * @brief  Test code for Word Rescue maps.
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
	"\x04\x00" \
	"\x02\x00" \
	"\x03\x00" \
	"\x02\x00" "\x04\x00" \
	"\x06\x00" "\x08\x00" \
	"\x01\x00" /* Gruzzle count */  \
	"\x00\x00" "\x04\x00" \
	"\x00\x00" \
	"\x01\x00" /* Slime bucket count */  \
	"\x01\x00" "\x04\x00" \
	"\x02\x00" /* Book count */  \
	"\x02\x00" "\x04\x00" \
	"\x02\x00" "\x03\x00" \
	"\x00\x00" "\x00\x00" /* Letter 1 */ \
	"\x01\x00" "\x00\x00" \
	"\x02\x00" "\x00\x00" \
	"\x00\x00" "\x01\x00" \
	"\x01\x00" "\x01\x00" \
	"\x02\x00" "\x01\x00" \
	"\x00\x00" "\x02\x00" \
	"\x00\x00" /* Anim count */ \
	"\x00\x00" /* end */ \
	"\x01\x02\x01\x01\x01\x00" \
	"\x01\x12\x01\x11\x01\x10" \
	"\x03\x22" \
	"\x03\xFF" \
	"\x02\x42\x01\x40" \
	"\x0a\x73" /* attribute layer */ \
	"\x01\x74\x01\x00\x01\x01\x01\x02\x01\x03\x01\x04\x01\x05\x01\x06\x01\x20\x02\x73" \
	"\x09\x20" \
	"\x01\x73\x08\x74\x02\x73" \
	"\x08\x74\x01\x73" \
	"\x0a\x20"

#define MAP_WIDTH_PIXELS  (3*16)
#define MAP_HEIGHT_PIXELS (5*16)
#define MAP_LAYER_COUNT   3
#define MAP_FIRST_CODE_L1 0x02
#define MAP_FIRST_CODE_L2 0x73
#define MAP_FIRST_CODE_L3 0x06

#define MAP_FIRST_CODE_X_L2 1
#define MAP_FIRST_CODE_Y_L2 0

#define MAP_CLASS fmt_map_wordresc
#define MAP_TYPE  "map-wordresc"
#include "test-map2d.hpp"

// Test some invalid formats to make sure they're not identified as valid
// archives.  Note that they can still be opened though (by 'force'), this
// only checks whether they look like valid files or not.

// The "c00" test has already been performed in test-map.hpp to ensure the
// initial state is correctly identified as a valid archive.

// Header too short
ISINSTANCE_TEST(c01,
	"\x03\x00" "\x05\x00"
	"\x04\x00"
	"\x02\x00"
	"\x06\x00"
	"\x01\x00" "\x02\x00"
	"\x03\x00" "\x04\x00"
	"\x01\x00" /* Gruzzle count */
	"\x00\x00" "\x04\x00"
	,
	gm::MapType::DefinitelyNo
);

// Item count out of range
ISINSTANCE_TEST(c02,
	"\x03\x00" "\x05\x00"
	"\x04\x00"
	"\x02\x00"
	"\x06\x00"
	"\x01\x00" "\x02\x00"
	"\x03\x00" "\x04\x00"
	"\xF0\x00" /* Gruzzle count */
	"\x00\x00" "\x04\x00"
	"\x00\x00"
	"\x01\x00" /* Slime bucket count */
	"\x01\x00" "\x04\x00"
	"\x02\x00" /* Book count */
	"\x02\x00" "\x04\x00"
	"\x02\x00" "\x03\x00"
	"\x00\x00" "\x00\x00" /* Letter 1 */
	"\x01\x00" "\x00\x00"
	"\x02\x00" "\x00\x00"
	"\x00\x00" "\x01\x00"
	"\x01\x00" "\x01\x00"
	"\x02\x00" "\x01\x00"
	"\x00\x00" "\x02\x00"
	"\x00\x00" /* Anim count */
	"\x00\x00" /* end */
	"\x01\x02\x01\x01\x01\x00"
	"\x01\x12\x01\x11\x01\x10"
	"\x03\x22"
	"\x03\xFF"
	"\x02\x42\x01\x40"
	,
	gm::MapType::DefinitelyNo
);

// Background layer too short
ISINSTANCE_TEST(c03,
	"\x03\x00" "\x05\x00"
	"\x04\x00"
	"\x02\x00"
	"\x06\x00"
	"\x01\x00" "\x02\x00"
	"\x03\x00" "\x04\x00"
	"\x01\x00" /* Gruzzle count */
	"\x00\x00" "\x04\x00"
	"\x00\x00"
	"\x01\x00" /* Slime bucket count */
	"\x01\x00" "\x04\x00"
	"\x02\x00" /* Book count */
	"\x02\x00" "\x04\x00"
	"\x02\x00" "\x03\x00"
	"\x00\x00" "\x00\x00" /* Letter 1 */
	"\x01\x00" "\x00\x00"
	"\x02\x00" "\x00\x00"
	"\x00\x00" "\x01\x00"
	"\x01\x00" "\x01\x00"
	"\x02\x00" "\x01\x00"
	"\x00\x00" "\x02\x00"
	"\x00\x00" /* Anim count */
	"\x00\x00" /* end */
	"\x01\x02\x01\x01\x01\x00"
	"\x01\x12\x01\x11\x01\x10"
	"\x03\x22"
	"\x03\xFF"
	"\x02\x42\x01"
	,
	gm::MapType::DefinitelyNo
);

// Background tile out of range
ISINSTANCE_TEST(c04,
	"\x03\x00" "\x05\x00"
	"\x04\x00"
	"\x02\x00"
	"\x06\x00"
	"\x01\x00" "\x02\x00"
	"\x03\x00" "\x04\x00"
	"\x01\x00" /* Gruzzle count */
	"\x00\x00" "\x04\x00"
	"\x00\x00"
	"\x01\x00" /* Slime bucket count */
	"\x01\x00" "\x04\x00"
	"\x02\x00" /* Book count */
	"\x02\x00" "\x04\x00"
	"\x02\x00" "\x03\x00"
	"\x00\x00" "\x00\x00" /* Letter 1 */
	"\x01\x00" "\x00\x00"
	"\x02\x00" "\x00\x00"
	"\x00\x00" "\x01\x00"
	"\x01\x00" "\x01\x00"
	"\x02\x00" "\x01\x00"
	"\x00\x00" "\x02\x00"
	"\x00\x00" /* Anim count */
	"\x00\x00" /* end */
	"\x01\x02\x01\x01\x01\x00"
	"\x01\xFE\x01\x11\x01\x10"
	"\x03\x22"
	"\x03\xFF"
	"\x02\x42\x01\x40"
	,
	gm::MapType::DefinitelyNo
);
