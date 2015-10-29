/**
 * @file   test-map-vinyl.cpp
 * @brief  Test code for Vinyl Goddess From Mars maps.
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

class test_map_vinyl: public test_map2d
{
	public:
		test_map_vinyl()
		{
			this->type = "map2d-vinyl";
			this->pxSize = {5 * 16, 4 * 16};
			this->numLayers = 2;

			this->mapCode[0].pos = {0, 0};
			this->mapCode[0].code = 0x0001;

			this->mapCode[1].pos = {0, 0};
			this->mapCode[1].code = 0x11;
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too small
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00\x00"
			));

			// c02: Truncated
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x04\x00\x05\x00"
				"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00"
				"\x11\x00\x12\x00\x13\x00\x14\x00\x15\x00"
				"\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00"
				"\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00"
				"\x11\x12\x13\x14\x15"
				"\x21\x22\x23\x24\x25"
				"\x31\x32\x33\x34\x35"
			));

			// c03: BG tile code out of range
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x04\x00\x05\x00"
				"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00"
				"\x11\x00\x12\x00\x13\x00\x14\x00\x15\x00"
				"\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00"
				"\x31\x00\x32\x00\x33\x00\x34\x00\x35\xFF"
				"\x11\x12\x13\x14\x15"
				"\x21\x22\x23\x24\x25"
				"\x31\x32\x33\x34\x35"
				"\x41\x42\x43\x44\x45"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x04\x00\x05\x00"
				"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00"
				"\x11\x00\x12\x00\x13\x00\x14\x00\x15\x00"
				"\x21\x00\x22\x00\x23\x00\x24\x00\x25\x00"
				"\x31\x00\x32\x00\x33\x00\x34\x00\x35\x00"
				"\x11\x12\x13\x14\x15"
				"\x21\x22\x23\x24\x25"
				"\x31\x32\x33\x34\x35"
				"\x41\x42\x43\x44\x45"
			);
		}
};

IMPLEMENT_TESTS(map_vinyl);
