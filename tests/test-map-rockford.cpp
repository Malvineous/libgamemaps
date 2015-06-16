/**
 * @file   test-map-rockford.cpp
 * @brief  Test code for Rockford maps.
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

class test_map_rockford: public test_map2d
{
	public:
		test_map_rockford()
		{
			this->type = "map-rockford";
			this->pxSize = {40 * 16, 22 * 16};
			this->numLayers = 1;
			this->mapCode[0].pos.x = 1;
			this->mapCode[0].pos.y = 0;
			this->mapCode[0].code = 0x28;
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x28\x28\x28\x28\x28\x28\x28\x28\x28"
			) + std::string(30 + 40*21, '\x00'));

			// c02: Tilecode out of range
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\xFF\x28\x28\x28\x28\x28\x28\x28\x28\x28"
			) + std::string(30 + 40*21, '\x00'));

			// c03: Map too wide
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00" "\x00\xf0" "\x00\x00"

				"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00"
				"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0'));

			// c04: Too many actors
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00" "\x00\x40" "\x00\xf0"

				"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00"
				"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0'));

			// c05: More actors than space in the file
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00" "\x00\x40" "\x00\x10"

				"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00"
				"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0'));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x00\x28\x28\x28\x28\x28\x28\x28\x28\x28"
			) + std::string(30 + 40*21, '\x00');
		}
};

IMPLEMENT_TESTS(map_rockford);
