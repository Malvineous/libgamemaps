/**
 * @file   test-map-wacky.cpp
 * @brief  Test code for Wacky Wheels maps.
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

#include "test-map2d.hpp"

class test_suppl1_map_wacky: public test_map2d
{
	public:
		test_suppl1_map_wacky()
		{
			this->type = "map-wacky.l1";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x03\x00"
				"\x10\x00" "\x20\x00" "\x20\x00" "\x30\x00" "\xf0\x00\x07\x00" "\x16\x00"
				"\x20\x00" "\x30\x00" "\x30\x00" "\x40\x00" "\xf0\x00\x07\x00" "\x16\x00"
				"\x30\x00" "\x40\x00" "\x00\x00" "\x10\x00" "\xb0\x04\x03\x00" "\x43\x00"
			);
		}
};

class test_map_wacky: public test_map2d
{
	public:
		test_map_wacky()
		{
			this->type = "map-wacky";
			this->pxWidth = 64 * 32;
			this->pxHeight = 64 * 32;
			this->numLayers = 1;
			this->mapCode[0].code = 0x20;
			this->suppResult[SuppItem::Layer1].reset(new test_suppl1_map_wacky());
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too small
			this->isInstance(MapType::DefinitelyNo, std::string(64 * 63, '\x20'));

			// c02: Invalid tile code
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\xFF\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
				) + std::string(16 * 3 + 64 * 63, '\x20')
			);
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20"
			) + std::string(16 * 3 + 64 * 63, '\x20');
		}
};

IMPLEMENT_TESTS(map_wacky);
