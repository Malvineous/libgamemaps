/**
 * @file   test-map-bash.cpp
 * @brief  Test code for Monster Bash maps.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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
	"bk1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"fg1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"bon1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"sgl1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"main_r\0\0"    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"bash.snd"      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" \
	"UNNAMED\0"     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"

#define testdata_initialstate_Layer1 \
	"\x10\x02" "\x20\x00" "\x00\x01" "\x20\x00" \
	\
	"\x01\x00" "\x00\x00" "\x02\x00" "\x03\x00" \
	"\x04\x00" "\x05\x00" "\x06\x00" "\x07\x00" \
	"\x08\x00" "\x09\x00" "\x0a\x00" "\x0b\x00" \
	"\x0c\x00" "\x0d\x00" "\x0e\x00" "\x0f\x00" \
	\
	"\x10\x00" "\x11\x00" "\x12\x00" "\x13\x00" \
	"\x14\x00" "\x15\x00" "\x16\x00" "\x17\x00" \
	"\x18\x00" "\x19\x00" "\x1a\x00" "\x1b\x00" \
	"\x1c\x00" "\x1d\x00" "\x1e\x00" "\x1f\x00" \

#define testdata_initialstate_Layer2 \
	"\x10\x00" \
	\
	"\x01" "\x00" "\x02" "\x03" \
	"\x04" "\x05" "\x06" "\x07" \
	"\x08" "\x09" "\x0a" "\x0b" \
	"\x0c" "\x0d" "\x0e" "\x0f" \
	\
	"\x10" "\x11" "\x12" "\x13" \
	"\x14" "\x15" "\x16" "\x17" \
	"\x18" "\x19" "\x1a" "\x1b" \
	"\x1c" "\x1d" "\x1e" "\x1f" \

#define MAP_WIDTH_PIXELS  (16*16)
#define MAP_HEIGHT_PIXELS (2*16)
#define MAP_LAYER_COUNT   2
#define MAP_FIRST_CODE_L1 0x01 // code at (0,0)
#define MAP_FIRST_CODE_L2 0x01 // code at (0,0)

#define MAP_HAS_SUPPDATA_LAYER1
#define MAP_HAS_SUPPDATA_LAYER2

// Skip isInstance checks
#define MAP_DETECTION_UNCERTAIN

#define MAP_CLASS fmt_map_bash
#define MAP_TYPE  "map-bash"
#include "test-map2d.hpp"
