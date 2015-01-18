/**
 * @file   test-map-wordresc.cpp
 * @brief  Test code for Word Rescue maps.
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

class test_map_wordresc: public test_map2d
{
	public:
		test_map_wordresc()
		{
			this->type = "map-wordresc";
			this->pxWidth = 3 * 16;
			this->pxHeight = 5 * 16;
			this->numLayers = 4;
			this->mapCode[0].x = 0;
			this->mapCode[0].y = 0;
			this->mapCode[0].code = 0x02;
			this->mapCode[1].x = 1;
			this->mapCode[1].y = 0;
			this->mapCode[1].code = 0x73;
			this->mapCode[2].x = 0;
			this->mapCode[2].y = 4;
			this->mapCode[2].code = 0x01; // WR_CODE_GRUZZLE
			this->mapCode[3].x = 1;
			this->mapCode[3].y = 4;
			this->mapCode[3].code = 0x02; // WR_CODE_SLIME
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Header too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x03\x00" "\x05\x00"
				"\x04\x00"
				"\x02\x00"
				"\x06\x00"
				"\x01\x00" "\x02\x00"
				"\x03\x00" "\x04\x00"
				"\x01\x00" /* Gruzzle count */
				"\x00\x00" "\x04\x00"
			));

			// c02: Item count out of range
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x03\x00" "\x05\x00"
				"\x04\x00"
				"\x02\x00"
				"\x06\x00"
				"\x01\x00" "\x02\x00"
				"\x03\x00" "\x04\x00"
				"\xF0\x00" /* Gruzzle count */
				"\x00\x00" "\x04\x00"
				"\x00\x00"
				"\x01\x00" /* Slime bucket count */
				"\x01\x00" "\x04\x00"
				"\x02\x00" /* Book count */
				"\x02\x00" "\x04\x00"
				"\x02\x00" "\x03\x00"
				"\x00\x00" "\x00\x00" /* Letter 1 */
				"\x01\x00" "\x00\x00"
				"\x02\x00" "\x00\x00"
				"\x00\x00" "\x01\x00"
				"\x01\x00" "\x01\x00"
				"\x02\x00" "\x01\x00"
				"\x00\x00" "\x02\x00"
				"\x00\x00" /* Anim count */
				"\x00\x00" /* end */
				"\x01\x02\x01\x01\x01\x00"
				"\x01\x12\x01\x11\x01\x10"
				"\x03\x22"
				"\x03\xFF"
				"\x02\x42\x01\x40"
			));

			// c03: Background layer too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x03\x00" "\x05\x00"
				"\x04\x00"
				"\x02\x00"
				"\x06\x00"
				"\x01\x00" "\x02\x00"
				"\x03\x00" "\x04\x00"
				"\x01\x00" /* Gruzzle count */
				"\x00\x00" "\x04\x00"
				"\x00\x00"
				"\x01\x00" /* Slime bucket count */
				"\x01\x00" "\x04\x00"
				"\x02\x00" /* Book count */
				"\x02\x00" "\x04\x00"
				"\x02\x00" "\x03\x00"
				"\x00\x00" "\x00\x00" /* Letter 1 */
				"\x01\x00" "\x00\x00"
				"\x02\x00" "\x00\x00"
				"\x00\x00" "\x01\x00"
				"\x01\x00" "\x01\x00"
				"\x02\x00" "\x01\x00"
				"\x00\x00" "\x02\x00"
				"\x00\x00" /* Anim count */
				"\x00\x00" /* end */
				"\x01\x02\x01\x01\x01\x00"
				"\x01\x12\x01\x11\x01\x10"
				"\x03\x22"
				"\x03\xFF"
				"\x02\x42\x01"
			));

			// c04: Background tile out of range
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x03\x00" "\x05\x00"
				"\x04\x00"
				"\x02\x00"
				"\x03\x00"
				"\x02\x00" "\x04\x00"
				"\x06\x00" "\x08\x00"
				"\x01\x00" /* Gruzzle count */
				"\x00\x00" "\x04\x00"
				"\x01\x00" /* Drip count */
				"\x02\x00" "\x04\x00" "\x44\x00"
				"\x01\x00" /* Slime bucket count */
				"\x01\x00" "\x04\x00"
				"\x02\x00" /* Book count */
				"\x02\x00" "\x04\x00"
				"\x02\x00" "\x03\x00"
				"\x00\x00" "\x00\x00" /* Letter 1 */
				"\x01\x00" "\x00\x00"
				"\x02\x00" "\x00\x00"
				"\x00\x00" "\x01\x00"
				"\x01\x00" "\x01\x00"
				"\x02\x00" "\x01\x00"
				"\x00\x00" "\x02\x00"
				"\x01\x00" /* Anim count */
				"\x01\x00" "\x01\x00"
				"\x01\x00" /* FG tiles */
				"\x02\x00" "\x02\x00"
				"\x01\x02\x01\x01\x01\x00"
				"\x01\xFE\x01\x11\x01\x10"
				"\x03\x22"
				"\x03\xFF"
				"\x02\x42\x01\x40"
				"\x0a\x73" /* attribute layer */
				"\x01\x74\x01\x00\x01\x01\x01\x02\x01\x03\x01\x04\x01\x05\x01\x06\x01\x20\x02\x73"
				"\x09\x20"
				"\x01\x73\x08\x74\x02\x73"
				"\x08\x74\x01\x73"
				"\x0a\x20"
			));

			// c05: Map size is zero
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\x00" "\x05\x00"
				"\x04\x00"
				"\x02\x00"
				"\x03\x00"
				"\x02\x00" "\x04\x00"
				"\x06\x00" "\x08\x00"
				"\x01\x00" /* Gruzzle count */
				"\x00\x00" "\x04\x00"
				"\x01\x00" /* Drip count */
				"\x02\x00" "\x04\x00" "\x44\x00"
				"\x01\x00" /* Slime bucket count */
				"\x01\x00" "\x04\x00"
				"\x02\x00" /* Book count */
				"\x02\x00" "\x04\x00"
				"\x02\x00" "\x03\x00"
				"\x00\x00" "\x00\x00" /* Letter 1 */
				"\x01\x00" "\x00\x00"
				"\x02\x00" "\x00\x00"
				"\x00\x00" "\x01\x00"
				"\x01\x00" "\x01\x00"
				"\x02\x00" "\x01\x00"
				"\x00\x00" "\x02\x00"
				"\x01\x00" /* Anim count */
				"\x01\x00" "\x01\x00"
				"\x01\x00" /* FG tiles */
				"\x02\x00" "\x02\x00"
				"\x01\x02\x01\x01\x01\x00"
				"\x01\x12\x01\x11\x01\x10"
				"\x03\x22"
				"\x03\xFF"
				"\x02\x42\x01\x40"
				"\x0a\x73" /* attribute layer */
				"\x01\x74\x01\x00\x01\x01\x01\x02\x01\x03\x01\x04\x01\x05\x01\x06\x01\x20\x02\x73"
				"\x09\x20"
				"\x01\x73\x08\x74\x02\x73"
				"\x08\x74\x01\x73"
				"\x0a\x20"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x03\x00" "\x05\x00"
				"\x04\x00"
				"\x02\x00"
				"\x03\x00"
				"\x02\x00" "\x04\x00"
				"\x06\x00" "\x08\x00"
				"\x01\x00" /* Gruzzle count */
				"\x00\x00" "\x04\x00"
				"\x01\x00" /* Drip count */
				"\x02\x00" "\x04\x00" "\x44\x00"
				"\x01\x00" /* Slime bucket count */
				"\x01\x00" "\x04\x00"
				"\x02\x00" /* Book count */
				"\x02\x00" "\x04\x00"
				"\x02\x00" "\x03\x00"
				"\x00\x00" "\x00\x00" /* Letter 1 */
				"\x01\x00" "\x00\x00"
				"\x02\x00" "\x00\x00"
				"\x00\x00" "\x01\x00"
				"\x01\x00" "\x01\x00"
				"\x02\x00" "\x01\x00"
				"\x00\x00" "\x02\x00"
				"\x01\x00" /* Anim count */
				"\x01\x00" "\x01\x00"
				"\x01\x00" /* FG tiles */
				"\x02\x00" "\x02\x00"
				"\x01\x02\x01\x01\x01\x00"
				"\x01\x12\x01\x11\x01\x10"
				"\x03\x22"
				"\x03\xFF"
				"\x02\x42\x01\x40"
				"\x0a\x73" /* attribute layer */
				"\x01\x74\x01\x00\x01\x01\x01\x02\x01\x03\x01\x04\x01\x05\x01\x06\x01\x20\x02\x73"
				"\x09\x20"
				"\x01\x73\x08\x74\x02\x73"
				"\x08\x74\x01\x73"
				"\x0a\x20"
			);
		}
};

IMPLEMENT_TESTS(map_wordresc);
