/**
 * @file   test-map-ddave.cpp
 * @brief  Test code for Dangerous Dave maps.
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

class test_map_ddave: public test_map2d
{
	public:
		test_map_ddave()
		{
			this->type = "map2d-ddave";
			this->pxSize = {100 * 16, 10 * 16};
			this->numLayers = 1;
			this->mapCode[0].pos = {0, 0};
			this->mapCode[0].code = 0x01;

			this->initialPath = STRING_WITH_NULLS(
				"\x03\x05\xfd\xfb\xea\xea\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
				"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
			);
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Wrong length
			this->isInstance(MapType::DefinitelyNo,
				this->initialPath + STRING_WITH_NULLS(
				"\x12"
				"\x01\x03\x04\x02\x05\x08\x00\x00\x00\x00"
				) + std::string(90 + 100 * 9 + 24, '\0')
			);

			// c02: First tile byte is out of range
			this->isInstance(MapType::DefinitelyNo,
				this->initialPath + STRING_WITH_NULLS(
				"\xff\x03\x04\x02\x05\x08\x00\x00\x00\x00"
				) + std::string(90 + 100 * 9 + 24, '\0')
			);
		}

		virtual std::string initialstate()
		{
			return
				this->initialPath + STRING_WITH_NULLS(
				"\x01\x03\x04\x02\x05\x08\x00\x00\x00\x00"
				) + std::string(90 + 100 * 9 + 24, '\0')
			;
		}

	protected:
		std::string initialPath;
};

IMPLEMENT_TESTS(map_ddave);
