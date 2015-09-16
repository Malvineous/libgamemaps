/**
 * @file   test-map-bash.cpp
 * @brief  Test code for Monster Bash maps.
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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

#include "test-map2d.hpp"

/// Layer1: Background tiles
class test_suppl1_map_bash: public test_map2d
{
	public:
		test_suppl1_map_bash()
		{
			this->type = "map2d-bash.l1";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x10\x02" "\x20\x00" "\x00\x01" "\x20\x00"

				"\x01\x28" "\x00\x00" "\x02\x00" "\x03\x10"
				"\x04\x00" "\x05\x00" "\x06\x00" "\x07\x20"
				"\x08\x20" "\x09\x20" "\x0a\x20" "\x0b\x40"
				"\x0c\x00" "\x0d\x00" "\x0e\x00" "\x0f\x80"

				"\x10\x00" "\x11\x00" "\x12\x00" "\x13\x08"
				"\x14\x00" "\x15\x00" "\x16\x00" "\x17\x04"
				"\x18\x00" "\x19\x00" "\x1a\x00" "\x1b\x02"
				"\x1c\x00" "\x1d\x00" "\x1e\x00" "\x1f\x20"
			);
		}
};

/// Layer2: Foreground tiles
class test_suppl2_map_bash: public test_map2d
{
	public:
		test_suppl2_map_bash()
		{
			this->type = "map2d-bash.l2";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x10\x00"

				"\x01" "\x00" "\x02" "\x03"
				"\x04" "\x05" "\x06" "\x07"
				"\x08" "\x09" "\x0a" "\x0b"
				"\x0c" "\x0d" "\x0e" "\x0f"

				"\x10" "\x11" "\x12" "\x13"
				"\x14" "\x15" "\x16" "\x17"
				"\x18" "\x19" "\x1a" "\x1b"
				"\x1c" "\x1d" "\x1e" "\xff"
			);
		}
};

/// Layer3: Sprites
class test_suppl3_map_bash: public test_map2d
{
	public:
		test_suppl3_map_bash()
		{
			this->type = "map2d-bash.l3";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\xFE\xFF"

				"\x34" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"

				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00"

				"main_r\x00\x00"

				"\x32" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00"
				"\x02" "\x00" "\x00" "\x00"
				"\x01" "\x00" "\x00" "\x00"

				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00" "\x00" "\x00"
				"\x00" "\x00"

				"zb_l\x00\x00"
			);
		}
};

/// Extra1: Sprite list
class test_suppx1_map_bash: public test_map2d
{
	public:
		test_suppx1_map_bash()
		{
			this->type = "map2d-bash.x1";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"arrows\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"axe\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"blank\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"bomb\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"bone1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"bone2\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"bone3\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"border\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"border2\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"break_screen\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"cat\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"chunk\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"crack\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"crawlleft\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"crawlright\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"dirt_l\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"dirt_r\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"dog\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"flag\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"float100\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"guage\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"heart\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"leaf\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"main_die\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"main_exit\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"main_hat_l\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"main_hat_r\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"main_l\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"main_meter\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"main_r\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"main_stars\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"plank\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"plank_r\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"rock\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"score\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"skullw\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"splat\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"white\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"zb_l\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"zb_r\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"zbh_l\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"zbh_r\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"zhead\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"zhead_r\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			);
		}
};

/// Extra2: Background tile properties
class test_suppx2_map_bash: public test_map2d
{
	public:
		test_suppx2_map_bash()
		{
			this->type = "map2d-bash.x2";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
			);
		}
};

/// Extra3: Foreground tile properties
class test_suppx3_map_bash: public test_map2d
{
	public:
		test_suppx3_map_bash()
		{
			this->type = "map2d-bash.x3";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
			);
		}
};

/// Extra4: Bonus tile properties
class test_suppx4_map_bash: public test_map2d
{
	public:
		test_suppx4_map_bash()
		{
			this->type = "map2d-bash.x4";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"00 00 00 00 00 00 00 10 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
				"00 00 00 00 00 00 00 00 "
			);
		}
};

/// Extra5: Full list of sprites and dependencies
class test_suppx5_map_bash: public test_map2d
{
	public:
		test_suppx5_map_bash()
		{
			this->type = "map2d-bash.x5";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"*=arrows\n"
				"*=axe\n"
				"*=blank\n"
				"*=bomb\n"
				"*=bone1\n"
				"*=bone2\n"
				"*=bone3\n"
				"*=border\n"
				"*=border2\n"
				"*=cat\n"
				"*=chunk\n"
				"*=dog\n"
				"*=flag\n"
				"*=float100\n"
				"*=guage\n"
				"*=heart\n"
				"*=leaf\n"
				"*=plank\n"
				"*=plank_r\n"
				"*=rock\n"
				"*=score\n"
				"*=skullw\n"
				"*=splat\n"
				"*=white\n"
				"main_r=break_screen\n"
				"main_r=crack\n"
				"main_r=crawlleft\n"
				"main_r=crawlright\n"
				"main_r=dirt_l\n"
				"main_r=dirt_r\n"
				"main_r=main_die\n"
				"main_r=main_exit\n"
				"main_r=main_hat_l\n"
				"main_r=main_hat_r\n"
				"main_r=main_l\n"
				"main_r=main_meter\n"
				"main_r=main_stars\n"
				"zb_l=zb_r\n"
				"zb_l=zbh_l\n"
				"zb_l=zbh_r\n"
				"zb_l=zhead\n"
				"zb_l=zhead_r\n"
			);
		}
};

class test_map_bash: public test_map2d
{
	public:
		test_map_bash()
		{
			this->type = "map2d-bash";
			this->pxSize = {16 * 16, 2 * 16};
			this->numLayers = 4;

			// BG
			this->mapCode[0].code = 0x01;
			this->mapCode[0].pos = {0, 0};
			// FG
			this->mapCode[1].code = 0x01;
			this->mapCode[1].pos = {0, 0};
			// AT
			this->mapCode[2].code = 0x08;
			this->mapCode[2].pos = {3, 0};
			// SP
			this->mapCode[3].code = 1000025;
			this->mapCode[3].pos = {0, 0};

			this->suppResult[SuppItem::Layer1].reset(new test_suppl1_map_bash());
			this->suppResult[SuppItem::Layer2].reset(new test_suppl2_map_bash());
			this->suppResult[SuppItem::Layer3].reset(new test_suppl3_map_bash());
			this->suppResult[SuppItem::Extra1].reset(new test_suppx1_map_bash());
			this->suppResult[SuppItem::Extra2].reset(new test_suppx2_map_bash());
			this->suppResult[SuppItem::Extra3].reset(new test_suppx3_map_bash());
			this->suppResult[SuppItem::Extra4].reset(new test_suppx4_map_bash());
			this->suppResult[SuppItem::Extra5].reset(new test_suppx5_map_bash());
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"bk1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"fg1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bon1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"sgl1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"main_r\0\0"    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bash.snd"      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			));

			// c02: Too long
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"bk1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"fg1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bon1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"sgl1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"main_r\0\0"    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bash.snd"      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"UNNAMED\0"     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"extra"
			));

			// c03: Chars after the terminating null
			this->isInstance(MapType::PossiblyYes, STRING_WITH_NULLS(
				"bk1\0\0\0\0\0" "\0\0\0\0\0\0\0x\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"fg1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bon1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"sgl1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"main_r\0\0"    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bash.snd"      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"UNNAMED\0"     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			));

			// c04: Bad char in filename
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"bk1\xFF\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"fg1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bon1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"sgl1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"main_r\0\0"    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bash.snd"      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"UNNAMED\0"     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			));

			// c05: Entry without terminating null
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"bk1jjjjj"      "jjjjjjjjjjjjjjjjjjjjjjj"
				"fg1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bon1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"sgl1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"main_r\0\0"    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bash.snd"      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"UNNAMED\0"     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"bk1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"fg1\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bon1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"sgl1\0\0\0\0"  "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"main_r\0\0"    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"bash.snd"      "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"UNNAMED\0"     "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			);
		}
};

IMPLEMENT_TESTS(map_bash);
