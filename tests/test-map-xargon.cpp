/**
 * @file   test-map-xargon.cpp
 * @brief  Test code for Xargon maps.
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

class test_suppx1_map_xargon: public test_map2d
{
	public:
		test_suppx1_map_xargon()
		{
			this->type = "map-xargon.x1";
			this->written = false;
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x00\x00\x00\x00\x00\x00\x01" "0"
				"\x01\x00\x00\x00\x00\x00\x03" "one"
				"\x02\x00\x00\x00\x00\x00\x03" "two"
			);
		}
};

class test_map_xargon: public test_map2d
{
	public:
		test_map_xargon()
		{
			this->type = "map-xargon";
			this->pxWidth = 128 * 16;
			this->pxHeight = 64 * 16;
			this->numLayers = 2;
			this->mapCode[0].code = 0x01;
			this->mapCode[1].code = 0x01;
			this->suppResult[SuppItem::Extra1].reset(new test_suppx1_map_xargon());
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
			));

			// c02: Exact size w/ no text section
			this->isInstance(MapType::DefinitelyYes,
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00') + STRING_WITH_NULLS(
					"\x01\x00"
					"\x01" "\x10\x00" "\x10\x00"
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"
				) + std::string(97, '\x00') // empty savedata
			);

			// c03: Truncated object layer
			this->isInstance(MapType::DefinitelyNo,
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00') + STRING_WITH_NULLS(
					"\x01\x00"
					"\x01"
				)
			);
		}

		virtual std::string initialstate()
		{
			return
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00') + STRING_WITH_NULLS(
					"\x01\x00"
					"\x01" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"
				) + std::string(97, '\x00') // empty savedata
			;
		}
};

IMPLEMENT_TESTS(map_xargon);
