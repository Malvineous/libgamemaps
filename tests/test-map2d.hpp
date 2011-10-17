/**
 * @file   test-map2d.hpp
 * @brief  Generic test code for Map2D class descendents.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/iostreams/copy.hpp>
#include <boost/bind.hpp>
#include <camoto/stream_string.hpp>
#include <camoto/gamemaps.hpp>
//#include <iomanip>

#include "tests.hpp"

using namespace camoto;
namespace gm = camoto::gamemaps;

// Defines to allow code reuse
#define COMBINE_CLASSNAME_EXP(c, n)  c ## _ ## n
#define COMBINE_CLASSNAME(c, n)  COMBINE_CLASSNAME_EXP(c, n)

#define TEST_VAR(n)        COMBINE_CLASSNAME(MAP_CLASS, n)
#define TEST_NAME(n)       TEST_VAR(n)
#define TEST_RESULT(n)     testdata_ ## n

#define FIXTURE_NAME       TEST_VAR(sample)
#define EMPTY_FIXTURE_NAME TEST_VAR(sample_empty)
#define SUITE_NAME         TEST_VAR(suite)
#define EMPTY_SUITE_NAME   TEST_VAR(suite_empty)
#define INITIALSTATE_NAME  TEST_RESULT(initialstate)

#ifndef MAP_FIRST_CODE_X_L1
/// Coordinates of first file.  Normally (0,0) but can be changed for maps
/// where (0,0) is not a valid location.
#define MAP_FIRST_CODE_X_L1 0
#define MAP_FIRST_CODE_Y_L1 0
#endif

#ifndef MAP_FIRST_CODE_X_L2
#define MAP_FIRST_CODE_X_L2 0
#define MAP_FIRST_CODE_Y_L2 0
#endif

#ifndef MAP_FIRST_CODE_X_L3
#define MAP_FIRST_CODE_X_L3 0
#define MAP_FIRST_CODE_Y_L3 0
#endif

// Add a new constant as a supplementary data item
#define ADD_SUPPITEM(suppitem) \
	{ \
		stream::string_sptr suppSS(new stream::string()); \
		suppSS << makeString(TEST_RESULT(initialstate_ ## suppitem)); \
		this->suppData[camoto::SuppItem::suppitem] = suppSS; \
	}

#include <camoto/gamemaps/map2d.hpp>

struct FIXTURE_NAME: public default_sample {

	stream::string_sptr base;
	gm::MapPtr map;
	gm::Map2DPtr map2d;
	camoto::SuppData suppData;
	gm::MapTypePtr pTestType;

	FIXTURE_NAME() :
		base(new stream::string())
	{
		this->base << makeString(INITIALSTATE_NAME);

		#ifdef MAP_HAS_SUPPDATA_LAYER1
			ADD_SUPPITEM(Layer1);
		#endif
		#ifdef MAP_HAS_SUPPDATA_LAYER2
			ADD_SUPPITEM(Layer2);
		#endif

		BOOST_REQUIRE_NO_THROW(
			gm::ManagerPtr pManager = gm::getManager();
			this->pTestType = pManager->getMapTypeByCode(MAP_TYPE);
		);
		BOOST_REQUIRE_MESSAGE(pTestType, "Could not find map type " MAP_TYPE);

		// Create an instance of the initialstate data
		this->map = this->pTestType->open(this->base, this->suppData);
		this->map2d = boost::dynamic_pointer_cast<gm::Map2D>(this->map);
		BOOST_REQUIRE_MESSAGE(this->map2d, "Could not create map class");
	}

	FIXTURE_NAME(int i) :
		base(new stream::string())
	{
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		return this->default_sample::is_equal(strExpected, this->base->str());
	}

	boost::test_tools::predicate_result is_supp_equal(camoto::SuppItem::Type type, const std::string& strExpected)
	{
		stream::string_sptr ss = boost::dynamic_pointer_cast<stream::string>(this->suppData[type]);
		assert(ss);
		return this->default_sample::is_equal(strExpected, ss->str());
	}

};

BOOST_FIXTURE_TEST_SUITE(SUITE_NAME, FIXTURE_NAME)

// Define an ISINSTANCE_TEST macro which we use to confirm the initial state
// is a valid instance of this format.  This is defined as a macro so the
// format-specific code can reuse it later to test various invalid formats.
#define ISINSTANCE_TEST(c, d, r) \
	BOOST_AUTO_TEST_CASE(TEST_NAME(isinstance_ ## c)) \
	{ \
		BOOST_TEST_MESSAGE("isInstance check (" MAP_TYPE "; " #c ")"); \
		\
		gm::ManagerPtr pManager(gm::getManager()); \
		gm::MapTypePtr pTestType(pManager->getMapTypeByCode(MAP_TYPE)); \
		BOOST_REQUIRE_MESSAGE(pTestType, "Could not find map type " MAP_TYPE); \
		\
		stream::string_sptr ss(new stream::string()); \
		ss << makeString(d); \
		\
		BOOST_CHECK_EQUAL(pTestType->isInstance(ss), r); \
	}

ISINSTANCE_TEST(c00, INITIALSTATE_NAME, gm::MapType::DefinitelyYes);


// Define an INVALIDDATA_TEST macro which we use to confirm the reader correctly
// rejects a file with invalid data.  This is defined as a macro so the
// format-specific code can reuse it later to test various invalid formats.
#ifdef HAS_FAT
#	define INVALIDDATA_FATCODE(d) \
	{ \
		boost::shared_ptr<std::stringstream> suppSS(new std::stringstream); \
		suppSS->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit); \
		(*suppSS) << makeString(d); \
		camoto::stream::inout_sptr suppStream(suppSS); \
		gm::SuppItem si; \
		si.stream = suppStream; \
		si.fnTruncate = boost::bind<void>(stringStreamTruncate, suppSS.get(), _1); \
		suppData[camoto::SuppItem::FAT] = si; \
	}
#else
#	define INVALIDDATA_FATCODE(d)
#endif

#define INVALIDDATA_TEST(c, d) \
	INVALIDDATA_TEST_FULL(c, d, 0)

#define INVALIDDATA_TEST_FAT(c, d, f) \
	INVALIDDATA_TEST_FULL(c, d, f)

#define INVALIDDATA_TEST_FULL(c, d, f) \
	/* Run an isInstance test first to make sure the data is accepted */ \
	ISINSTANCE_TEST(invaliddata_ ## c, d, gm::EC_DEFINITELY_YES); \
	\
	BOOST_AUTO_TEST_CASE(TEST_NAME(invaliddata_ ## c)) \
	{ \
		BOOST_TEST_MESSAGE("invalidData check (" MAP_TYPE "; " #c ")"); \
		\
		boost::shared_ptr<gm::Manager> pManager(gm::getManager()); \
		gm::MapTypePtr pTestType(pManager->getMapTypeByCode(MAP_TYPE)); \
		\
		/* Prepare an invalid map */ \
		boost::shared_ptr<std::stringstream> psstrBase(new std::stringstream); \
		(*psstrBase) << makeString(d); \
		camoto::stream::inout_sptr psBase(psstrBase); \
		\
		camoto::SuppData suppData; \
		INVALIDDATA_FATCODE(f) \
		\
		BOOST_CHECK_THROW( \
			gm::MapPtr pMap(pTestType->open(psBase, suppData)), \
			stream::error \
		); \
	}

BOOST_AUTO_TEST_CASE(TEST_NAME(getsize))
{
	BOOST_TEST_MESSAGE("Getting map size");

	int layerCount = this->map2d->getLayerCount();
	int width, height;
	this->map2d->getMapSize(&width, &height);

	int x, y;
	this->map2d->getTileSize(&x, &y);
	width *= x;
	height *= y;

	BOOST_REQUIRE_EQUAL(width, MAP_WIDTH_PIXELS);
	BOOST_REQUIRE_EQUAL(height, MAP_HEIGHT_PIXELS);
	BOOST_REQUIRE_EQUAL(layerCount, MAP_LAYER_COUNT);
}

BOOST_AUTO_TEST_CASE(TEST_NAME(read))
{
	BOOST_TEST_MESSAGE("Reading map codes");
#define CHECK_FIRST_TILE_IN_LAYER(LAYER) \
	case (LAYER-1): \
		if ( \
			((*i)->x == MAP_FIRST_CODE_X_L ## LAYER) \
			&& ((*i)->y == MAP_FIRST_CODE_Y_L ## LAYER) \
		) { \
			foundFirstTile = true; \
			BOOST_REQUIRE_EQUAL((*i)->code, MAP_FIRST_CODE_L ## LAYER); \
		} \
		break;

	for (int l = 0; l < MAP_LAYER_COUNT; l++) {
		gm::Map2D::LayerPtr layer = map2d->getLayer(l);
		const gm::Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		bool foundFirstTile = false;
		for (gm::Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			(i != items->end()) && (!foundFirstTile);
			i++
		) {
			switch (l) {
#ifdef MAP_FIRST_CODE_L1
CHECK_FIRST_TILE_IN_LAYER(1)
#endif
#ifdef MAP_FIRST_CODE_L2
CHECK_FIRST_TILE_IN_LAYER(2)
#endif
#ifdef MAP_FIRST_CODE_L3
CHECK_FIRST_TILE_IN_LAYER(3)
#endif
			}
		}
		BOOST_REQUIRE_EQUAL(foundFirstTile, true);
		BOOST_TEST_MESSAGE("Found first tile in this layer");
	}
}

#define ERASE_SUPPITEM(suppitem) \
	this->suppData[camoto::SuppItem::suppitem]->truncate(0);

BOOST_AUTO_TEST_CASE(TEST_NAME(write))
{
	BOOST_TEST_MESSAGE("Write map codes");

	this->base->truncate(0);

#ifdef MAP_HAS_SUPPDATA_LAYER1
	ERASE_SUPPITEM(Layer1);
#endif
#ifdef MAP_HAS_SUPPDATA_LAYER2
	ERASE_SUPPITEM(Layer2);
#endif

	this->pTestType->write(this->map, this->base, this->suppData);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(INITIALSTATE_NAME)),
		"Error writing map to a file - data is different to original"
	);

#ifdef MAP_HAS_SUPPDATA_LAYER1
	BOOST_CHECK_MESSAGE(
		is_supp_equal(camoto::SuppItem::Layer1, makeString(TEST_RESULT(initialstate_Layer1))),
		"Error writing map to a file - data is different to original in supp::layer1"
	);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
