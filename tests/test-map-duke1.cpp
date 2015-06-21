/**
 * @file   test-map-duke1.cpp
 * @brief  Test code for Duke Nukem I maps.
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

class test_map_duke1: public test_map2d
{
	public:
		test_map_duke1()
		{
			this->type = "map-duke1";
			this->pxSize = {128 * 16, 90 * 16};
			this->numLayers = 1;
			this->mapCode[0].pos = {0, 0};
			this->mapCode[0].code = 0x01;
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::PossiblyYes, this->initialstate());

			// c01: Wrong size
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x00\x03\x00\x04\x00"
			) + std::string((128 * 90 - 4) * 2, '\x00'));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x01\x00\x02\x00\x03\x00\x04\x00"
			) + std::string((128 * 90 - 4) * 2, '\x00');
		}
};

IMPLEMENT_TESTS(map_duke1);
