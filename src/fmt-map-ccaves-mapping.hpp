/**
 * @file   fmt-map-ccaves-mapping.hpp
 * @brief  Static mapping between tile codes and tile images.
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

// This is not a header file, but code for fmt-map-ccaves.cpp placed
// here to stop that file from becoming excessively long.

/// Create a tile number from a tileset number and an index into the tileset.
#define CCT(tileset, tile) (((tileset) << 8) | (tile))

#define ___________     -1 ///< No tile here
#define IBEAM_L         -9 ///< I-beam left end
#define IBEAM_M         -8 ///< I-beam left end
#define IBEAM_R         -7 ///< I-beam left end
#define IS_IBEAM(x)     (((x) >= IBEAM_L) && ((x) <= IBEAM_R)) ///< Is this an i-beam tile?
#define CCT_IBEAM(b, x) ((b) + ((x) - IBEAM_L)) ///< Get i-beam tile index from base (ibeam colour) and position flag (IBEAM_*)

#define BLOCK_TL        -20 ///< solid block top-left (0)
#define BLOCK_TM        -19 ///< solid block top-mid (1)
#define BLOCK_TR        -18 ///< solid block top-right (2)
#define BLOCK_BL        -16 ///< solid block bottom-left (4)
#define BLOCK_BM        -15 ///< solid block bottom-mid (5)
#define BLOCK_BR        -14 ///< solid block bottom-right (6)
#define BLOCK_ML        -12 ///< solid block mid-left (8)
#define BLOCK_MM        -11 ///< solid block mid-mid (9)
#define BLOCK_MR        -10 ///< solid block mid-right (10)
#define IS_BLOCK(x)     (((x) >= BLOCK_TL) && ((x) <= BLOCK_MR)) ///< Is this a block tile?
#define CCT_BLOCK(b, x) ((b) + ((x) - BLOCK_TL)) ///< Get block tile index from base (block colour) and position flag (BLOCK_*)

#define CCT_USCORE      -30 ///< underscore

#define CCTF_MV_NONE  0 ///< No special flags
#define CCTF_MV_VERT  1 ///< This block moves up and down (e.g. wall lasers)
#define CCTF_MV_HORZ  2 ///< This block moves left and right (e.g. moving platform)
#define CCTF_MV_DROP  3 ///< This block drops (e.g. danger sign)

typedef struct {
	uint8_t code;
	int tileIndexBG[4];
	int tileIndexFG;
	unsigned int flags;
} TILE_MAP;

typedef struct {
	uint8_t code;
	int tileIndexMid;
	int tileIndexEnd;
	unsigned int flags;
} TILE_MAP_VINE;

typedef struct {
	uint8_t code1;
	uint8_t code2;
	int tileIndexBG[16];
	unsigned int flags;
} TILE_MAP_SIGN;

typedef struct {
	uint8_t code;
	int tileIndexBG;
	int tileIndexFG;
} TILE_REVMAP_BLOCKS;

TILE_MAP tileMap[] = {
	{0x21, {CCT(13,  0), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // blue dripping pipe
	{0x22, {CCT(12, 30), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green stuff hanging down from block 2
	{0x23, {CCT( 2, 24), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // spider
	{0x24, {CCT(17, 10), ___________, CCT(17, 14), ___________}, ___________, CCTF_MV_NONE}, // air compressor
	{0x25, {CCT(10, 39), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, vert
	{0x26, {CCT(13, 12), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // robot enemy
// 0x27 invalid
	{0x28, {CCT( 3, 34), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // brown stalactites 1
	{0x29, {CCT( 3, 35), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // brown stalactites 2
	{0x2A, {CCT( 2,  4), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // brown walking ball enemy
	{0x2B, {CCT(12,  1), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // yellow gem
	{0x2C, {CCT(10, 37), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, down exit, left join
	{0x2D, {CCT(10, 36), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, horiz
	{0x2E, {CCT(10, 38), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, down exit, right join
	{0x2F, {CCT( 9, 31), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // flying bone enemy
	{0x30, {CCT( 0, 43), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // large chain end
// 0x31 invalid
// 0x32 invalid, although used on the first row of many levels(?)
// 0x33 invalid
	{0x34, {BLOCK_ML,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 8
	{0x35, {BLOCK_MM,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 9
	{0x36, {BLOCK_MR,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 10
// 0x37 invalid
	{0x38, {CCT( 0, 34), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // large chain
	{0x39, {CCT( 1, 46), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // mine cart
	{0x3A, {CCT(12, 29), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green stuff hanging down from block 1
// 0x3B invalid
// 0x3C invalid
	{0x3D, {CCT(13, 44), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // purple wall enemy, attacking to left
// 0x3E invalid
	{0x3F, {CCT( 1, 12), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green stripy enemy
	{0x40, {CCT( 3, 40), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // tornado
	{0x41, {CCT(17, 32), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green fish enemy
	{0x42, {CCT( 0,  6), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // ice block  @todo could be 0,19 also, tiles are identical
	{0x43, {CCT(21,  0), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // random concrete blocks  @todo only in certain levels, figure out trigger
	{0x44, {IBEAM_L,     IBEAM_R,     ___________, ___________}, ___________, CCTF_MV_NONE}, // I-beam left
	{0x45, {CCT(13, 35), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // purple wall enemy, attacking to right
	{0x46, {CCT( 9, 22), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // flame
	{0x47, {CCT( 5, 48), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // gun/ammo
	{0x48, {CCT(11, 40), ___________, ___________, ___________}, ___________, CCTF_MV_HORZ}, // horiz moving platform, always on
	{0x49, {CCT( 4, 37), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // popup floor spike
	{0x4A, {CCT( 9,  3), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // flame tower
	{0x4B, {CCT(21,  0), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // concrete block 0
	{0x4C, {CCT(21,  1), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // concrete block 1
	{0x4D, {CCT( 6,  0), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // emu enemy
	{0x4E, {CCT(12,  9), ___________, ___________, ___________}, ___________, CCTF_MV_HORZ}, // moon
// 0x4F invalid
// 0x50 invalid
// 0x51 invalid
	{0x52, {CCT(12,  0), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // red gem
	{0x53, {CCT( 3,  4), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // purple snake enemy
	{0x54, {CCT( 9, 24), CCT( 9, 25), ___________, ___________}, ___________, CCTF_MV_NONE}, // hammer guide
	{0x55, {CCT( 9, 10), CCT( 9, 11), CCT( 9, 14), CCT( 9, 15)}, ___________, CCTF_MV_NONE}, // hammer top
	{0x56, {CCT(11, 44), ___________, ___________, ___________}, ___________, CCTF_MV_VERT}, // vert moving platform
// 0x57 is a sign
	{0x58, {CCT(11, 12), CCT(11, 20), CCT(11, 16), CCT(11, 24)}, ___________, CCTF_MV_NONE}, // level exit
	{0x59, {CCT( 5,  0), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // player start
	{0x5A, {CCT( 4, 32), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // random map horizon/hill/light
// 0x5B is for signs
// 0x5C invalid
	{0x5D, {CCT( 5, 49), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // P powerup
	{0x5E, {CCT( 4,  5), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // bird enemy
	{0x5F, {CCT_USCORE,  ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // underscore platform (colour dependent on level)
// 0x60 invalid
	{0x61, {CCT(11, 10), ___________, ___________, ___________}, ___________, CCTF_MV_VERT}, // left-facing laser, moving vertically, always on
	{0x62, {CCT(12,  2), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green gem
	{0x63, {CCT(12,  3), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // blue gem
	{0x64, {IBEAM_M,     IBEAM_R,     ___________, ___________}, ___________, CCTF_MV_NONE}, // I-beam mid
// 0x65 invalid
	{0x66, {BLOCK_BL,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 4
	{0x67, {BLOCK_BM,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 5
	{0x68, {BLOCK_BR,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 6
	{0x69, {CCT(13, 24), ___________, CCT(13, 25), ___________}, ___________, CCTF_MV_NONE}, // stop sign face
	{0x6A, {CCT(12,  5), CCT(12,  6), ___________, ___________}, ___________, CCTF_MV_NONE}, // inverted rubble pile, mid
	{0x6B, {CCT(21,  2), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // concrete 2
	{0x6C, {CCT(21,  3), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // concrete 3
	{0x6D, {CCT(12,  8), ___________, ___________, ___________}, ___________, CCTF_MV_HORZ}, // earth
// 0x6E is the "fill" tile handled differently
	{0x6F, {CCT( 2,  0), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // dormant brown walking ball enemy
	{0x70, {CCT(12,  4), CCT(12,  6), ___________, ___________}, ___________, CCTF_MV_NONE}, // inverted rubble pile, left + end
	{0x71, {CCT(11, 10), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // left-facing laser, static, always on
	{0x72, {BLOCK_TL,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 0
	{0x73, {CCT(11,  9), ___________, ___________, ___________}, ___________, CCTF_MV_VERT}, // right-facing laser, moving vertically, always on
	{0x74, {BLOCK_TM,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 1
	{0x75, {CCT(14, 20), CCT(14, 21), ___________, ___________}, ___________, CCTF_MV_NONE}, // volcano eruption
	{0x76, {CCT(11, 30), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // horizontal switch, off
	{0x77, {CCT(11,  9), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // right-facing laser, static, always on
	{0x78, {CCT( 0, 12), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // level entrance
	{0x79, {BLOCK_TR,    ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // fgtile + 2
	{0x7A, {CCT( 6, 30), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // invisible blocking tile (made up mapping)
// 0x7B invalid
	{0x7C, {CCT( 3, 34), ___________, ___________, ___________}, ___________, CCTF_MV_DROP}, // brown stalactites 1 (same as 0x28) - maybe these fall?
// 0x7D invalid
	{0x7E, {CCT( 4, 12), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // bat enemy
// 0x7F invalid
	{0x80, {CCT(17,  8), CCT(17,  9), CCT(17, 12), CCT(17, 13)}, ___________, CCTF_MV_NONE}, // sector alpha sign
	{0x81, {CCT(11, 10), ___________, ___________, ___________}, CCT( 8, 12), CCTF_MV_VERT}, // left-facing laser, moving vertically, switched
	{0x82, {CCT(11, 10), ___________, ___________, ___________}, CCT( 8, 12), CCTF_MV_NONE}, // left-facing laser, static, switched
	{0x83, {CCT(11,  9), ___________, ___________, ___________}, CCT( 8, 12), CCTF_MV_VERT}, // right-facing laser, moving vertically, switched
	{0x84, {CCT(11,  9), ___________, ___________, ___________}, CCT( 8, 12), CCTF_MV_NONE}, // right-facing laser, static, switched
// 0x85 is a vine
// 0x86 is a vine
// 0x87 is a vine
// 0x88 is a vine
	{0x89, {CCT(11,  8), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // tear revealing horizontal bar
	{0x8A, {CCT(11,  4), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // tear revealing vertical bar
	{0x8B, {CCT( 0,  3), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // candle
	{0x8C, {CCT(11, 37), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // G powerup
// 0x8D invalid
	{0x8E, {CCT(14,  9), CCT(14, 10), ___________, ___________}, ___________, CCTF_MV_NONE}, // volcano top
// 0x8F is a 4x1 code
	{0x90, {CCT( 7, 36), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // funnel tube stem
	{0x91, {CCT( 9, 49), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // slime with two chunks
	{0x92, {CCT( 9, 26), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // slime with two bones
	{0x93, {CCT( 9, 27), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // slime with helmet
	{0x94, {CCT( 7, 28), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // golden handrail left
	{0x95, {CCT( 7, 29), CCT( 7, 30), ___________, ___________}, ___________, CCTF_MV_NONE}, // golden handrail mid + right
	{0x96, {CCT( 7, 32), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // wooden handrail left
	{0x97, {CCT( 7, 33), CCT( 7, 34), ___________, ___________}, ___________, CCTF_MV_NONE}, // wooden handrail mid + right
	{0x98, {IBEAM_L,     IBEAM_R,     ___________, ___________}, CCT(12, 36), CCTF_MV_NONE}, // hidden gem in I-beam left-end (following right-end tile has no hidden gem)
	{0x99, {IBEAM_M,     IBEAM_R,     ___________, ___________}, CCT(12, 36), CCTF_MV_NONE}, // hidden gem in I-beam midsection (following right-end tile has no hidden gem)
	{0x9A, {IBEAM_R,     ___________, ___________, ___________}, CCT(12, 36), CCTF_MV_NONE}, // hidden gem in I-beam right-end
// 0x9B invalid
// 0x9C invalid
// 0x9D invalid
// 0x9E invalid
	{0x9F, {CCT(17, 24), CCT(17, 25), CCT(17, 28), CCT(17, 29)}, ___________, CCTF_MV_NONE}, // large fan blades
	{0xA0, {CCT( 8, 16), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // red switch
	{0xA1, {CCT( 8, 20), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green switch
	{0xA2, {CCT( 8, 18), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // blue switch
	{0xA3, {CCT( 8, 24), ___________, CCT( 8, 28), ___________}, ___________, CCTF_MV_NONE}, // red door
	{0xA4, {CCT( 8, 26), ___________, CCT( 8, 30), ___________}, ___________, CCTF_MV_NONE}, // green door
	{0xA5, {CCT( 8, 25), ___________, CCT( 8, 29), ___________}, ___________, CCTF_MV_NONE}, // blue door
	{0xA6, {CCT( 8, 13), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // light switch, off (level starts in the dark)
	{0xA7, {CCT(12, 49), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // treasure chest
	{0xA8, {CCT(12, 43), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // key for treasure chest
	{0xA9, {CCT(12, 46), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // purple-spotted white egg
	{0xAA, {CCT(12, 48), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // blue mushroom
	{0xAB, {CCT(12, 44), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // red mushroom
	{0xAC, {CCT(12, 45), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green mushroom
// 0xAD invalid
// 0xAE invalid
// 0xAF invalid
	{0xB0, {CCT( 0,  2), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // hidden block revealed by head-butting
	{0xB1, {CCT(11, 48), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // thick horizontal wooden post
	{0xB2, {CCT(11, 36), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // thick vertical wooden post
	{0xB3, {CCT(17,  6), CCT(17,  7), ___________, ___________}, ___________, CCTF_MV_NONE}, // vertical thin wooden post (left)
// 0xB4 invalid
// 0xB5 invalid
// 0xB6 invalid
// 0xB7 invalid
// 0xB8 invalid
// 0xB9 invalid
	{0xBA, {CCT(11, 33), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // vertical thick metal support, middle
	{0xBB, {CCT(11, 38), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // purple mushroom
	{0xBC, {CCT(11, 39), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // tuft of grass
	{0xBD, {CCT(11, 29), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // \ ledge
	{0xBE, {CCT(11, 28), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // / ledge
	{0xBF, {CCT(10, 40), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, left/down join
	{0xC0, {CCT(10, 44), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, top/right join
	{0xC1, {CCT(10, 46), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, top/left/right join
	{0xC2, {CCT(10, 47), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, bottom/left/right join
	{0xC3, {CCT(21,  7), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // red vine, top
	{0xC4, {CCT(21, 11), ___________, CCT(21, 15), ___________}, ___________, CCTF_MV_NONE}, // red vine, mid + bottom
	{0xC5, {CCT(10, 36), ___________, ___________, ___________}, CCT(10, 39), CCTF_MV_NONE}, // green pipe, top/bottom/left/right join
	{0xC6, {CCT( 8, 42), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // down arrow
	{0xC7, {CCT( 8, 43), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // up arrow
	{0xC8, {CCT(16, 48), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // barrier to contain green fish thing
// 0xC9 invalid
	{0xCA, {CCT(11, 34), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // vertical thick metal support, bottom
	{0xCB, {CCT(11, 32), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // vertical thick metal support, top
	{0xCC, {CCT( 8, 15), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // brown lump on ceiling
	{0xCD, {CCT(11, 43), ___________, ___________, ___________}, CCT( 8, 12), CCTF_MV_NONE}, // horiz moving platform, switched
	{0xCE, {CCT(21,  6), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // low grass
	{0xCF, {CCT(10, 43), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, bottom exit
	{0xD0, {CCT(11,  7), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // control panel
	{0xD1, {CCT(10, 42), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, top exit
// 0xD2 invalid
// 0xD3 invalid
// 0xD4 invalid
	{0xD5, {CCT(12, 39), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // flame
	{0xD6, {CCT(11, 47), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // vert moving platform, stationary
	{0xD7, {CCT(11, 47), ___________, ___________, ___________}, CCT( 8, 12), CCTF_MV_VERT}, // vert moving platform, switched
	{0xD8, {CCT(11, 31), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // horizontal switch, on
	{0xD9, {CCT(10, 45), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, top/left join
	{0xDA, {CCT(10, 41), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // green pipe, right/down join
	{0xDB, {CCT(12,  8), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // earth (intro)
	{0xDC, {CCT(12,  9), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // moving moon (intro)
// 0xDD invalid
// 0xDE invalid
// 0xDF invalid
	{0xE0, {CCT(17,  0), CCT(17,  1), CCT(17,  4), CCT(17,  5)}, ___________, CCTF_MV_NONE}, // on/off funnel machine
// 0xE1 invalid
// 0xE2 invalid
// 0xE3 invalid
// 0xE4 invalid
// 0xE5 invalid
// 0xE6 invalid
	{0xE7, {CCT(10, 48), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // thick purple post
	{0xE8, {CCT( 7, 44), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // corrugated pipe vert
	{0xE9, {CCT( 7, 45), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // corrugated pipe horiz
	{0xEA, {CCT( 7, 46), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // corrugated pipe L-bend
	{0xEB, {CCT( 7, 47), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // corrugated pipe backwards-L-bend
	{0xEC, {CCT( 7, 48), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // corrugated pipe backwards-r-bend
	{0xED, {CCT( 7, 49), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // corrugated pipe r-bend
// 0xEE invalid
// 0xEF invalid
	{0xF0, {CCT(17,  2), CCT(17,  3), ___________, ___________}, ___________, CCTF_MV_NONE}, // wooden Y beam (left)
// 0xF1 invalid
	{0xF2, {CCT(19, 32), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // dinosaur enemy (feet)
	{0xF3, {CCT(20, 44), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // blue ball enemy
	{0xF4, {CCT( 0,  8), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // pick
	{0xF5, {CCT( 0, 10), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // shovel
	{0xF6, {CCT( 3, 32), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // stalagmite 1
	{0xF7, {CCT( 3, 33), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // stalagmite 2
	{0xF8, {CCT(17, 16), CCT(17, 17), CCT(17, 20), CCT(17, 21)}, ___________, CCTF_MV_NONE}, // round glass thing
	{0xF9, {CCT(11,  3), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // clean barrel
	{0xFA, {CCT(11, 11), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // barrel leaking with green
	{0xFB, {CCT(11, 35), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // exploded barrel with red
	{0xFC, {CCT( 9, 48), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // slime with three chunks in
	{0xFD, {CCT( 9, 49), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // slime with two chunks in (duplicate of 0x91)
	{0xFE, {CCT( 9, 26), ___________, ___________, ___________}, ___________, CCTF_MV_NONE}, // slime with two bones in (duplicate of 0x92)
// 0xFF invalid
};

TILE_MAP tileMap4x1[] = {
	{0x8F, {CCT(14, 12), CCT(14, 13), CCT(14, 14), CCT(14, 15)}, ___________, CCTF_MV_NONE}, // volcano bottom
};

TILE_MAP_VINE tileMapVine[] = {
	{0x85, CCT( 8, 27), CCT( 8, 31), CCTF_MV_NONE}, // hanging single-chain with hook
	{0x86, CCT( 8, 22), CCT( 8, 23), CCTF_MV_NONE}, // hanging double-chain
	{0x87, CCT( 0,  0), CCT( 0,  4), CCTF_MV_NONE}, // purple vine
	{0x88, CCT( 0,  1), CCT( 0,  5), CCTF_MV_NONE}, // green vine
};

TILE_MAP_SIGN tileMapSign[] = {
	// Exhaust suckers
	{0x57, 0x4C, {CCT( 3, 29), CCT( 3,  2), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // left facing exhaust sucker
	{0x57, 0x52, {CCT( 3,  0), CCT( 3, 29), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // right facing exhaust sucker

	// Actual signs
// 0x5B: 0x00 - 0x22 are invalid
	{0x5B, 0x23, {CCT(14,  0), CCT(14,  1), ___________, ___________,
	              CCT(14,  4), CCT(14,  5), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // air vent grill
// 0x5B: 0x24 invalid
// 0x5B: 0x25 invalid
// 0x5B: 0x26 invalid
// 0x5B: 0x27 invalid
// 0x5B: 0x28 invalid
// 0x5B: 0x29 invalid
	{0x5B, 0x2A, {CCT( 8, 32), CCT( 8, 33), ___________, ___________,
	              CCT( 8, 36), CCT( 8, 37), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // cog
// 0x5B: 0x2B invalid
// 0x5B: 0x2C invalid
	{0x5B, 0x2D, {CCT( 1, 40), CCT( 1, 41), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // brown metal supports holding red I-beam
// 0x5B: 0x2E invalid
// 0x5B: 0x2F invalid
// 0x5B: 0x30 invalid
	{0x5B, 0x31, {CCT( 9,  4), CCT( 9,  5), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // reverse gravity
	{0x5B, 0x32, {CCT( 9,  6), CCT( 9,  7), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // low gravity
	{0x5B, 0x33, {CCT( 9, 12), CCT( 9, 13), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // kilroy was here
	{0x5B, 0x34, {CCT( 9,  0), CCT( 9,  1), CCT( 9,  2), ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // win (ners, don't)
	{0x5B, 0x35, {CCT(10, 24), CCT(10, 25), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // trading post
// 0x5B: 0x36 invalid
// 0x5B: 0x37 invalid
// 0x5B: 0x38 invalid
// 0x5B: 0x39 invalid
	{0x5B, 0x3A, {CCT(21,  4), CCT(21,  5), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // mario-style funnel top
	{0x5B, 0x3B, {CCT(21,  8), CCT(21,  9), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // mario-style funnel shaft
// 0x5B: 0x3C invalid
	{0x5B, 0x3D, {CCT(16, 24), CCT(16, 31), CCT(16, 32), ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // red dinosaur
// 0x5B: 0x3E invalid
// 0x5B: 0x3F invalid
// 0x5B: 0x40 invalid
	{0x5B, 0x41, {CCT(10, 10), CCT(10, 11), ___________, ___________,
	              CCT(10, 14), CCT(10, 15), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // crystal caves planetoid
	{0x5B, 0x42, {CCT(10, 26), CCT(10, 27), ___________, ___________,
	              CCT(10, 14), CCT(10, 15), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // trading post planetoid
// 0x5B: 0x43 invalid
	{0x5B, 0x44, {CCT( 9,  8), CCT( 9,  9), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_DROP}, // danger sign
	{0x5B, 0x45, {CCT(20,  8), CCT(20,  0), CCT(20, 16), ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // wide-eyed green enemy
// 0x5B: 0x46 invalid
// 0x5B: 0x47 invalid
// 0x5B: 0x48 invalid
// 0x5B: 0x49 invalid
// 0x5B: 0x4A invalid
// 0x5B: 0x4B invalid
// 0x5B: 0x4C invalid
// 0x5B: 0x4D invalid
// 0x5B: 0x4E invalid
	{0x5B, 0x4F, {CCT( 8, 34), CCT( 8, 35), ___________, ___________,
	              CCT( 8, 38), CCT( 8, 39), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // large sewer outlet, no slime
	{0x5B, 0x50, {CCT(14, 28), CCT(14, 29), CCT(14, 30), CCT(14, 31),
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // spiky green multi-segment caterpillar enemy
	{0x5B, 0x51, {CCT( 8, 34), CCT( 8, 35), ___________, ___________,
	              CCT( 8, 40), CCT( 8, 41), ___________, ___________,
	              CCT( 8, 44), CCT( 8, 45), ___________, ___________,
	              CCT( 8, 49), CCT( 8, 48), ___________, ___________}, CCTF_MV_NONE}, // large sewer outlet, with slime
// 0x5B: 0x52 invalid
// 0x5B: 0x53 invalid
	{0x5B, 0x54, {CCT( 7, 38), CCT( 7, 37), CCT( 7, 39), ___________,
	              ___________, CCT( 7, 36), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // funnel tube
// 0x5B: 0x55 invalid
// 0x5B: 0x56 invalid
// 0x5B: 0x57 invalid
// 0x5B: 0x58 invalid
// 0x5B: 0x59 invalid
// 0x5B: 0x5A invalid
// 0x5B: 0x5B invalid
// 0x5B: 0x5C invalid
	{0x5B, 0x5D, {CCT( 2, 44), CCT( 2, 45), ___________, ___________,
	              CCT( 2, 48), CCT( 2, 49), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // window into space
	{0x5B, 0x5E, {CCT( 1, 42), CCT( 1, 43), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // ^ shaped brown metal supports
// 0x5B: 0x5F invalid
// 0x5B: 0x60 invalid
// 0x5B: 0x61 invalid
	{0x5B, 0x62, {CCT( 8,  0), CCT( 8,  1), CCT( 8,  2), CCT( 8,  3),
	              CCT( 8,  4), CCT( 8,  5), CCT( 8,  6), CCT( 8,  7),
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // green wood box with yellow frame, 4x2
	{0x5B, 0x63, {CCT( 6, 36), CCT( 6, 37), ___________, ___________,
	              CCT( 6, 40), CCT( 6, 41), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // yellow/black hazard box
	{0x5B, 0x64, {CCT( 9,  8), CCT( 9,  9), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // danger sign (does not fall)
// 0x5B: 0x65 invalid
	{0x5B, 0x66, {CCT( 2, 42), CCT( 2, 43), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // falling rocks sign
	{0x5B, 0x67, {CCT( 8,  0), CCT( 8,  2), CCT( 8,  3), ___________,
	              CCT( 8,  4), CCT( 8,  6), CCT( 8,  7), ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // green wood box with yellow frame, 3x2
// 0x5B: 0x68 invalid
// 0x5B: 0x69 invalid
// 0x5B: 0x6A invalid
// 0x5B: 0x6B invalid
// 0x5B: 0x6C invalid
	{0x5B, 0x6D, {CCT( 4, 43), CCT( 4, 44), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // mine-> sign
// 0x5B: 0x6E invalid
// 0x5B: 0x6F invalid
// 0x5B: 0x70 invalid
// 0x5B: 0x71 invalid
	{0x5B, 0x72, {CCT( 2, 40), CCT( 2, 41), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // boarded up box
// 0x5B: 0x73 invalid
// 0x5B: 0x74 invalid
// 0x5B: 0x75 invalid
// 0x5B: 0x76 invalid
// 0x5B: 0x77 invalid
	{0x5B, 0x78, {CCT( 3, 36), CCT( 3, 37), ___________, ___________,
	              CCT( 3, 38), CCT( 3, 39), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // grey X box
	{0x5B, 0x79, {CCT( 8,  0), CCT( 8,  3), ___________, ___________,
	              CCT( 8,  4), CCT( 8,  7), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // green wood box with yellow frame, 2x2
// 0x5B: 0x7A invalid
// 0x5B: 0x7B invalid
	{0x5B, 0x7C, {CCT( 0, 17), CCT( 0, 18), ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________,
	              ___________, ___________, ___________, ___________}, CCTF_MV_NONE}, // || shaped brown metal supports
// 0x5B: 0x7D-0xFF invalid
};

#define SOLID_TILESET(t, x) \
	{0x72, CCT(t, (x) + 0), ___________},  /* fgtile + 0 */ \
	{0x74, CCT(t, (x) + 1), ___________},  /* fgtile + 1 */ \
	{0x79, CCT(t, (x) + 2), ___________},  /* fgtile + 2 */ \
	{0x66, CCT(t, (x) + 4), ___________},  /* fgtile + 4 */ \
	{0x67, CCT(t, (x) + 5), ___________},  /* fgtile + 5 */ \
	{0x68, CCT(t, (x) + 6), ___________},  /* fgtile + 6 */ \
	{0x34, CCT(t, (x) + 8), ___________},  /* fgtile + 8 */ \
	{0x35, CCT(t, (x) + 9), ___________},  /* fgtile + 9 */ \
	{0x36, CCT(t, (x) + 10), ___________}, /* fgtile + 10 */

/// Reverse mapping just for blocks (since many different tiles will
/// map back to the same codes, depending on the level tileset)
TILE_REVMAP_BLOCKS tileRevMapBlocks[] = {
	SOLID_TILESET(15, 24) // black solid
	SOLID_TILESET(19, 20) // blue rock
	SOLID_TILESET(21, 12) // green wavy
	SOLID_TILESET(21, 20) // cyan solid
	SOLID_TILESET(21, 32) // brown solid
	SOLID_TILESET(22,  0) // purple solid
	SOLID_TILESET(22, 12) // blue solid

	// I-beam blue
	{0x44, CCT(19,  3 + 0), ___________}, // I-beam left
	{0x64, CCT(19,  4 + 0), ___________}, // I-beam mid
	{0x6E, CCT(19,  5 + 0), ___________}, // I-beam right

	// I-beam red
	{0x44, CCT(19,  3 + 3), ___________}, // I-beam left
	{0x64, CCT(19,  4 + 3), ___________}, // I-beam mid
	{0x6E, CCT(19,  5 + 3), ___________}, // I-beam right

	// I-beam green
	{0x44, CCT(19,  3 + 6), ___________}, // I-beam left
	{0x64, CCT(19,  4 + 6), ___________}, // I-beam mid
	{0x6E, CCT(19,  5 + 6), ___________}, // I-beam right

	// I-beam blue (with hidden gem)
	{0x98, CCT(19,  3 + 0), CCT(12, 36)}, // I-beam left
	{0x99, CCT(19,  4 + 0), CCT(12, 36)}, // I-beam mid
	{0x9A, CCT(19,  5 + 0), CCT(12, 36)}, // I-beam right

	// I-beam red (with hidden gem)
	{0x98, CCT(19,  3 + 3), CCT(12, 36)}, // I-beam left
	{0x99, CCT(19,  4 + 3), CCT(12, 36)}, // I-beam mid
	{0x9A, CCT(19,  5 + 3), CCT(12, 36)}, // I-beam right

	// I-beam green (with hidden gem)
	{0x98, CCT(19,  3 + 6), CCT(12, 36)}, // I-beam left
	{0x99, CCT(19,  4 + 6), CCT(12, 36)}, // I-beam mid
	{0x9A, CCT(19,  5 + 6), CCT(12, 36)}, // I-beam right

	{0x5F, CCT(19,  0 + 0), ___________}, // underscore (blue)
	{0x5F, CCT(19,  0 + 1), ___________}, // underscore (red)
	{0x5F, CCT(19,  0 + 2), ___________}, // underscore (green)
};
