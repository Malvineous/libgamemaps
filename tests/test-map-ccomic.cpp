/**
 * @file   test-map-ccomic.cpp
 * @brief  Test code for Captain Comic maps.
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

class test_map_ccomic: public test_map2d
{
	public:
		test_map_ccomic()
		{
			this->type = "map-ccomic";
			this->pxSize = {3 * 16, 5 * 16};
			this->numLayers = 1;
			this->mapCode[0].code = 0x02;
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x03\x00" "\x05"
			));

			// c02: Dimensions too large for available data
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x03\x00" "\x06\x00"
				"\x02\x01\x00"
				"\x12\x11\x10"
				"\x22\x21\x20"
				"\x32\x31\x30"
				"\x42\x41\x40"
			));

			// c03: First tile byte is out of range
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x03\x00" "\x05\x00"
				"\xFF\x01\x00"
				"\x12\x11\x10"
				"\x22\x21\x20"
				"\x32\x31\x30"
				"\x42\x41\x40"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x03\x00" "\x05\x00"
				"\x02\x01\x00"
				"\x12\x11\x10"
				"\x22\x21\x20"
				"\x32\x31\x30"
				"\x42\x41\x40"
			);
		}
};

IMPLEMENT_TESTS(map_ccomic);
