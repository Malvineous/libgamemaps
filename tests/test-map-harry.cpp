/**
 * @file   test-map-harry.cpp
 * @brief  Test code for Halloween Harry maps.
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

class test_map_harry: public test_map2d
{
	public:
		test_map_harry()
		{
			this->type = "map-harry";
			this->pxSize = {4 * 16, 4 * 16};
			this->numLayers = 3;
			this->mapCode[0].pos = {0, 0};
			this->mapCode[0].code = 0x01; // 0x00 is empty tile and thus skipped
			this->mapCode[1].pos = {0, 0};
			this->mapCode[1].code = 0x01; // 0x00 is empty tile and thus skipped
			this->mapCode[2].pos = {0, 0};
			this->mapCode[2].code = 0x01; // 0x00 is empty tile and thus skipped
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x11SubZero Game File"
			));

			// c02: Bad signature
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x11SubZero Lame File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00"
				"\x00"
			)
			+ makePalette()
			+ makeTileFlags()
			+ std::string(10, '\0')
			+ STRING_WITH_NULLS(
				"\x01\x00"
				"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			)
			+ std::string(112, '\0')
			+ STRING_WITH_NULLS(
				"\x04\x00" "\x04\x00"
				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"

				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"
			));

			// c03: Palette out of range
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00"
				"\x00"
				"\x41\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			)
			+ std::string(768 - 16, '\0')
			+ makeTileFlags()
			+ STRING_WITH_NULLS(
				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"\x01\x00"
				"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			)
			+ std::string(112, '\0')
			+ STRING_WITH_NULLS(
				"\x04\x00" "\x04\x00"
				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"

				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"
			));

			// c04: Flags out of range
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00"
				"\x00"
			)
			+ makePalette()
			+ STRING_WITH_NULLS(
				"\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			)
			+ std::string(256 - 16, '\0')
			+ STRING_WITH_NULLS(
				"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				"\x01\x00"
				"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			)
			+ std::string(112, '\0')
			+ STRING_WITH_NULLS(
				"\x04\x00" "\x04\x00"
				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"

				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"
			));

			// c05: Actor data runs past EOF
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00"
				"\x00"
			)
			+ makePalette()
			+ makeTileFlags()
			+ std::string(10, '\0')
			+ STRING_WITH_NULLS(
				"\x00\x20"
				"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			)
			+ std::string(112, '\0')
			+ STRING_WITH_NULLS(
				"\x04\x00" "\x04\x00"
				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"

				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"
			));

			std::string tooShort = STRING_WITH_NULLS(
				"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00"
				"\x00"
			)
			+ makePalette()
			+ makeTileFlags()
			+ std::string(10, '\0')
			+ STRING_WITH_NULLS(
				"\x01\x00"
				"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			)
			+ std::string(112, '\0')
			+ STRING_WITH_NULLS(
				"\x04\x00" "\x04\x00"
				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"

				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f"
			);

			// c06a: BG or FG layer cut short
			this->isInstance(MapType::DefinitelyNo, tooShort);

			// c06b: Prev test plus one byte works
			this->isInstance(MapType::DefinitelyYes, tooShort + std::string(1, '\0'));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x11SubZero Game File" "\x00\x00\x00\x00" "\x02\x00\x03\x00" "\x00\x00"
				"\x01"
			)
			+ makePalette()
			+ makeTileFlags()
			+ std::string(10, '\0')
			// Actors
			+ STRING_WITH_NULLS(
				"\x01\x00"
				"\x01" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
			)
			+ std::string(112, '\0')
			// BG/FG
			+ STRING_WITH_NULLS(
				"\x04\x00" "\x04\x00"
				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"

				"\x01\x02\x03\x04"
				"\x05\x06\x07\x08"
				"\x09\x0a\x0b\x0c"
				"\x0d\x0e\x0f\x00"
			);
		}

		std::string makePalette()
		{
			std::string pal(768, '\0');
			pal[3] = pal[4] = pal[5] = 10;
			pal[6] = pal[7] = pal[8] = 20;
			return pal;
		}

		std::string makeTileFlags()
		{
			std::string flags(256, '\0');
			flags[3] = flags[4] = flags[5] = 1;
			flags[6] = flags[7] = flags[8] = 1;
			return flags;
		}
};

IMPLEMENT_TESTS(map_harry);
