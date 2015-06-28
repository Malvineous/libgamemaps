/**
 * @file   test-map-cosmo.cpp
 * @brief  Test code for Cosmo's Cosmic Adventures maps.
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

class test_map_cosmo: public test_map2d
{
	public:
		test_map_cosmo()
		{
			this->type = "map-cosmo";
			this->pxSize = {64 * 8, 512 * 8};
			this->numLayers = 2;
			this->mapCode[0].pos = {1, 0};
			this->mapCode[0].code = 0x08;
			this->mapCode[1].pos = {0, 0};
			this->mapCode[1].code = 0x1F;
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// Attribute 00: Backdrop
			this->changeAttribute(0, 1, 25, STRING_WITH_NULLS(
				"\x39\x09" "\x40\x00" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x00\x00\x08\x00\x10\x00\x18\x00\x20\x00\x28\x00\x30\x00\x38\x00"
				"\x40\x00\x48\x00\x50\x00\x58\x00\x60\x00\x68\x00\x70\x00\x78\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0')
			);

			// Attribute 01: Rain
			this->changeAttribute(1, 1, 0, STRING_WITH_NULLS(
				"\x01\x09" "\x40\x00" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x00\x00\x08\x00\x10\x00\x18\x00\x20\x00\x28\x00\x30\x00\x38\x00"
				"\x40\x00\x48\x00\x50\x00\x58\x00\x60\x00\x68\x00\x70\x00\x78\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0')
			);

			// Attribute 02: Scroll X
			this->changeAttribute(2, 0, 1, STRING_WITH_NULLS(
				"\x61\x09" "\x40\x00" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x00\x00\x08\x00\x10\x00\x18\x00\x20\x00\x28\x00\x30\x00\x38\x00"
				"\x40\x00\x48\x00\x50\x00\x58\x00\x60\x00\x68\x00\x70\x00\x78\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0')
			);

			// Attribute 03: Scroll Y
			this->changeAttribute(3, 0, 1, STRING_WITH_NULLS(
				"\xA1\x09" "\x40\x00" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x00\x00\x08\x00\x10\x00\x18\x00\x20\x00\x28\x00\x30\x00\x38\x00"
				"\x40\x00\x48\x00\x50\x00\x58\x00\x60\x00\x68\x00\x70\x00\x78\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0')
			);

			// Attribute 04: Palette animation
			this->changeAttribute(4, 1, 0, STRING_WITH_NULLS(
				"\x21\x08" "\x40\x00" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x00\x00\x08\x00\x10\x00\x18\x00\x20\x00\x28\x00\x30\x00\x38\x00"
				"\x40\x00\x48\x00\x50\x00\x58\x00\x60\x00\x68\x00\x70\x00\x78\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0')
			);

			// Attribute 05: Music
			this->changeAttribute(5, 1, 18, STRING_WITH_NULLS(
				"\x21\x91" "\x40\x00" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x00\x00\x08\x00\x10\x00\x18\x00\x20\x00\x28\x00\x30\x00\x38\x00"
				"\x40\x00\x48\x00\x50\x00\x58\x00\x60\x00\x68\x00\x70\x00\x78\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0')
			);

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			std::string tooShort = STRING_WITH_NULLS(
				"\x21\x09" "\x40\x00" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00"
				"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00"
				) + std::string((16 * 3) * 2, '\0');

			// c01: Too short
			this->isInstance(MapType::DefinitelyNo, tooShort);

			// c02: Just large enough
			this->isInstance(MapType::DefinitelyYes, tooShort
				+ std::string((64 * 511) * 2, '\0'));

			// c03: Map too wide
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x21\x09" "\x00\xF0" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00"
				"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0'));

			// c04: Too many actors
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x21\x09" "\x40\x00" "\x00\xf0"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00"
				"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0'));

			// c05: More actors than space in the file
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x21\x09" "\x40\x00" "\x00\x10"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x01\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00"
				"\x08\x00\x09\x00\x0a\x00\x0b\x00\x0c\x00\x0d\x00\x0e\x00\x0f\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0'));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x21\x09" "\x40\x00" "\x03\x00"

				"\x1F\x00" "\x00\x00" "\x00\x00"

				"\x00\x00\x08\x00\x10\x00\x18\x00\x20\x00\x28\x00\x30\x00\x38\x00"
				"\x40\x00\x48\x00\x50\x00\x58\x00\x60\x00\x68\x00\x70\x00\x78\x00"
				) + std::string((16 * 3 + 64 * 511) * 2, '\0');
		}
};

IMPLEMENT_TESTS(map_cosmo);
