/**
 * @file   test-map-hocus.cpp
 * @brief  Test code for Hocus Pocus maps.
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

class test_map_hocus_layer1: public test_map2d
{
	public:
		test_map_hocus_layer1()
		{
			this->type = "map-hocus.l1";
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x11\x12\x13\x14"
			) + std::string(240 * 60 - 4, '\xFF');
		}
};

class test_map_hocus: public test_map2d
{
	public:
		test_map_hocus()
		{
			this->type = "map-hocus";
			this->pxSize = {240 * 16, 60 * 16};
			this->numLayers = 2;
			this->mapCode[0].pos = {0, 0};
			this->mapCode[0].code = 0x01;
			this->mapCode[1].pos = {0, 0};
			this->mapCode[1].code = 0x11;
			this->suppResult[SuppItem::Layer1] = std::make_shared<test_map_hocus_layer1>();
		}

		void addTests()
		{
			this->test_map2d::addTests();

			// c00: Initial state
			this->isInstance(MapType::PossiblyYes, this->initialstate());

			// c01: Wrong size
			this->isInstance(MapType::DefinitelyNo, STRING_WITH_NULLS(
				"\x02\x03\x04"
			) + std::string(240 * 60 - 4, '\xFF'));
		}

		virtual std::string initialstate()
		{
			return STRING_WITH_NULLS(
				"\x01\x02\x03\x04"
			) + std::string(240 * 60 - 4, '\xFF');
		}
};

IMPLEMENT_TESTS(map_hocus);
