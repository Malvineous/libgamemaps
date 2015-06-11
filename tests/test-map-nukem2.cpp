/**
 * @file   test-map-nukem2.cpp
 * @brief  Test code for Duke Nukem II maps.
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

class test_map_nukem2: public test_map2d
{
	public:
		test_map_nukem2()
		{
			this->type = "map-nukem2";
			this->pxSize = {64 * 8, 511 * 8};
			this->numLayers = 3;
			this->mapCode[0].code = 0x01;
			this->mapCode[1].code = 0x70;
			this->mapCode[2].code = 0x02;
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x35\x00"
				"czone1.mni  \0"
				"drop1.mni   \0"
				"demosong.imf\0"
				"\x01\x02\x00\x00"
			));

			// c02: Offset past EOF
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x00\xF0"
				"czone1.mni  \0"
				"drop1.mni   \0"
				"demosong.imf\0"
				"\x01\x02\x00\x00"
				"\x03\x00" /* Actor ints */
				"\x02\x00\x00\x00\x00\x00"
				"\x40\x00" /* Map width */
				"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
			) + std::string((16 * 2 + 64 * 510 + 46) * 2, '\x00') + STRING_WITH_NULLS(
				"\x0B\x00" /* Extra length */
				"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
				"\x00\x00"
				"attrfile.mni\0"
				"tile.mni\0\0\0\0\0"
				"maskfile.mni\0"
			));

			// c03: Actors go past EOF
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x35\x00"
				"czone1.mni  \0"
				"drop1.mni   \0"
				"demosong.imf\0"
				"\x01\x02\x00\x00"
				"\x00\xFF" /* Actor ints */
				"\x02\x00\x00\x00\x00\x00"
				"\x40\x00" /* Map width */
				"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
			) + std::string((16 * 2 + 64 * 510 + 46) * 2, '\x00') + STRING_WITH_NULLS(
				"\x0B\x00" /* Extra length */
				"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
				"\x00\x00"
				"attrfile.mni\0"
				"tile.mni\0\0\0\0\0"
				"maskfile.mni\0"
			));

			// c04: Extra data too long
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x35\x00"
				"czone1.mni  \0"
				"drop1.mni   \0"
				"demosong.imf\0"
				"\x01\x02\x00\x00"
				"\x03\x00" /* Actor ints */
				"\x02\x00\x00\x00\x00\x00"
				"\x40\x00" /* Map width */
				"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
			) + std::string((16 * 2 + 64 * 510 + 46) * 2, '\x00') + STRING_WITH_NULLS(
				"\x00\xF0" /* Extra length */
				"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
				"\x00\x00"
				"attrfile.mni\0"
				"tile.mni\0\0\0\0\0"
				"maskfile.mni\0"
			));

			// c05: Missing optional fields
			this->isInstance(MapType::PossiblyYes, STRING_WITH_NULLS(
				"\x35\x00"
				"czone1.mni  \0"
				"drop1.mni   \0"
				"demosong.imf\0"
				"\x01\x02\x00\x00"
				"\x03\x00" /* Actor ints */
				"\x02\x00\x00\x00\x00\x00"
				"\x40\x00" /* Map width */
				"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
			) + std::string((16 * 2 + 64 * 510 + 46) * 2, '\x00') + STRING_WITH_NULLS(
				"\x0B\x00" /* Extra length */
				"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
				"\x00\x00"
				"attrfile.mni\0"
				"tile.mni\0\0\0\0\0"
			));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x35\x00"
				"czone1.mni  \0"
				"drop1.mni   \0"
				"demosong.imf\0"
				"\x01\x02\x00\x00"
				"\x03\x00" /* Actor ints */
				"\x02\x00\x00\x00\x00\x00"
				"\x40\x00" /* Map width */
				"\x01\xC0\x10\x00\x40\x1F\x20\x00\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
				"\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0\x01\xC0"
			) + std::string((16 * 2 + 64 * 510 + 46) * 2, '\x00') + STRING_WITH_NULLS(
				"\x0B\x00" /* Extra length */
				"\xFF\x03\x02\x01\xFE\x23\x45\x03\x67"
				"\x00\x00"
				"attrfile.mni\0"
				"tile.mni\0\0\0\0\0"
				"maskfile.mni\0"
			);
		}
};

IMPLEMENT_TESTS(map_nukem2);
