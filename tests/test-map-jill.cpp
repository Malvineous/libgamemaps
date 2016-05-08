/**
 * @file   test-map-jill.cpp
 * @brief  Test code for Jill of the Jungle maps.
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

// JILL.DMA
class test_suppx1_map_jill: public test_map2d
{
	public:
		test_suppx1_map_jill()
		{
			this->type = "map2d-jill.x1";
			this->written = false;
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x00\x00" "\x00" "\x00" "\x00\x00" "\x01" "0"
				"\x01\x00" "\x00" "\x00" "\x00\x00" "\x03" "one"
				"\x02\x00" "\x00" "\x00" "\x00\x00" "\x03" "two"
				"\x03\x00" "\x00" "\x00" "\x00\x00" "\x05" "three"
				"\x04\x00" "\x00" "\x00" "\x00\x00" "\x04" "four"
				"\x05\x00" "\x00" "\x00" "\x00\x00" "\x04" "five"
				"\x06\x00" "\x00" "\x00" "\x00\x00" "\x03" "six"
				"\x07\x00" "\x00" "\x00" "\x00\x00" "\x05" "seven"
				"\x08\x00" "\x00" "\x00" "\x00\x00" "\x05" "eight"
			);
		}
};

class test_map_jill: public test_map2d
{
	public:
		test_map_jill()
		{
			this->type = "map2d-jill";
			this->pxSize = {128 * 16, 64 * 16};
			this->numLayers = 2;
			this->mapCode[0].pos.x = 0;
			this->mapCode[0].pos.y = 0;
			this->mapCode[0].code = 0x01;
			this->mapCode[1].pos.x = 1;
			this->mapCode[1].pos.y = 2;
			this->mapCode[1].code = 0x01;
			this->suppResult[SuppItem::Extra1].reset(new test_suppx1_map_jill());
			this->skipInstDetect.push_back("map2d-wordresc");

			{
				this->attributes.emplace_back();
				auto& a = this->attributes.back();
				a.type = Attribute::Type::Integer;
				a.integerValue = 3;
			}
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::DefinitelyYes, this->initialstate());

			// c01: Too short
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
			));

			// c02: Make sure the object and savedata layers aren't cut off
			this->isInstance(MapType::DefinitelyNo,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x04\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"
				)
			);

			// c03: Player object must be first
			this->isInstance(MapType::DefinitelyNo,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x33" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x33" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // second string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x02\x00")
				+ std::string(68, '\x00')
				// String list
				+ STRING_WITH_NULLS(
					"\x05\x00" "Hello" "\0"
					"\x07\x00" "Goodbye" "\0"
				)
			);

			// c04: Wrong number of player objects
			this->isInstance(MapType::DefinitelyNo,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x00" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"

					"\x33" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // second string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x02\x00")
				+ std::string(68, '\x00')
				// String list
				+ STRING_WITH_NULLS(
					"\x05\x00" "Hello" "\0"
					"\x07\x00" "Goodbye" "\0"
				)
			);

			// c05: Exact size w/ no string table
			this->isInstance(MapType::DefinitelyYes,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x33" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00" // no string
					"\x00\x00" "\x00\x00"

					"\x33" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00" // no string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x02\x00")
				+ std::string(68, '\x00')
			);

			// c06: String's length bytes are cut
			this->isInstance(MapType::DefinitelyNo,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x33" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"

					"\x33" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // second string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x02\x00")
				+ std::string(68, '\x00')
				// String list
				+ STRING_WITH_NULLS(
					"\x05\x00" // terminating null is cut
				)
			);

			// c07: Empty string
			this->isInstance(MapType::DefinitelyNo,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x33" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"

					"\x33" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // second string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x02\x00")
				+ std::string(68, '\x00')
				// String list
				+ STRING_WITH_NULLS(
					"\x05\x00" "Hello" "\0"
					"\x00\x00\0"
				)
			);

			// c08: String itself is cut
			this->isInstance(MapType::DefinitelyNo,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x33" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"

					"\x33" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // second string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x02\x00")
				+ std::string(68, '\x00')
				// String list
				+ STRING_WITH_NULLS(
					"\x05\x00" "He" // string itself is cut
				)
			);

			// c09: Too many strings
			std::string many;
			for (int i = 0; i < 513; i++) {
				many.append("\x01\x00x\0", 4);
			}
			this->isInstance(MapType::DefinitelyNo,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x33" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"

					"\x33" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // second string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x02\x00")
				+ std::string(68, '\x00')
				// String list (lots of short strings)
				+ many
			);

			// Attribute 00: Savegame level number
			this->changeAttribute(0, 0,
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x01" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"

					"\x01" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x00\x00")
				+ std::string(68, '\x00')
				// String list
				+ STRING_WITH_NULLS(
					"\x05\x00" "Hello" "\0"
					"\x07\x00" "Goodbye" "\0"
				)
			);
		}

		virtual std::string initialstate()
		{
			return
				// Background layer
				STRING_WITH_NULLS(
					"\x01\x00\x02\x00\x03\x00\x04\x00\x05\x00\x06\x00\x07\x00\x08\x00"
					"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
				) + std::string((16 * 7 + 128 * 63) * 2, '\x00')
				// Object layer
				+ STRING_WITH_NULLS(
					"\x03\x00"

					"\x00" "\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x00\x00\x00\x00"
					"\x00\x00" "\x00\x00"

					"\x01" "\x01\x00" "\x02\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"

					"\x01" "\x03\x00" "\x04\x00"
					"\x00\x00" "\x00\x00"
					"\x10\x00" "\x10\x00" /* TODO: Use real width and height */
					"\x00\x00" "\x00\x00"
					"\x00\x00" "\x00\x00" "\x00\x00" "\x01\x00\x00\x00" // first string
					"\x00\x00" "\x00\x00"
				)
				// Empty savedata
				+ STRING_WITH_NULLS("\x03\x00")
				+ std::string(68, '\x00')
				// String list
				+ STRING_WITH_NULLS(
					"\x05\x00" "Hello" "\0"
					"\x07\x00" "Goodbye" "\0"
				)
			;
		}
};

IMPLEMENT_TESTS(map_jill);
