/**
 * @file   test-map-sagent.cpp
 * @brief  Test code for Secret Agent maps.
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

// copied from fmt-map-sagent.cpp
/// Create a tile number from a tileset number and an index into the tileset.
#define ST(tileset, tile) (((tileset) << 8) | (tile))

class test_map_sagent: public test_map2d
{
	public:
		test_map_sagent()
		{
			this->type = "map-sagent";
			this->pxWidth = 40 * 16;
			this->pxHeight = 4 * 16;
			this->numLayers = 2;
			this->mapCode[0].code = ST(5, 14);
			this->mapCode[1].x = 3;
			this->mapCode[1].y = 2;
			this->mapCode[1].code = ST(5, 29);

			this->outputWidth = 42;

			// This test identifies our initialstate as its own type, which is
			// technically correct because the formats are the same, only the
			// tile mapping is different.  So we skip the test to avoid an error.
			this->skipInstDetect.push_back("map-sagent-world");
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c00a: Extra stuff we currently ignore
			this->isInstance(MapType::DefinitelyYes, STRING_WITH_NULLS(
				"667                                    *\x0D\x0A"
				"2 22222222222222222222222222222222222222\x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
				"O                                      O\x0D\x0A"
				"* d                                    O\x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
				"\x0D\x0A"
			) + std::string(42 - 2, '\0')
				+ std::string(42 * (48 - 7), '\0')
			);

			// c01: File is wrong size
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"667                                     \x0D\x0A"
				"                                        \x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
				"O                                      O\x0D\x0A"
				"* d                                     \x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
			) + std::string(42 * (48 - 6) - 1, '\0')
			);

			// c02: Invalid tile code
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"667                                     \x0D\x0A"
				"                                        \x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
				"\xFF                                      O\x0D\x0A"
				"* d                                     \x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
			) + std::string(42 * (48 - 6), '\0')
			);

			// c03: No CRLF at end of row
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"667                                     \x0D\x0A"
				"                                        \x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x20\x0A"
				"O                                      O\x0D\x0A"
				"* d                                     \x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
			) + std::string(42 * (48 - 6), '\0')
			);
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"667                                     \x0D\x0A"
				"                                        \x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
				"O    " "  "    "                                O\x0D\x0A"
				"O 567" " \xD2" "                                O\x0D\x0A"
				"*  d " " f"    "                                 \x0D\x0A"
				"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\x0D\x0A"
			) + std::string(42 * (48 - 7), '\0');
		}
};

IMPLEMENT_TESTS(map_sagent);
