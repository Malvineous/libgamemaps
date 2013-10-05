/**
 * @file   test-map2d.hpp
 * @brief  Generic test code for Map2D class descendents.
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

#ifndef MAP_FIRST_CODE_X_L4
#define MAP_FIRST_CODE_X_L4 0
#define MAP_FIRST_CODE_Y_L4 0
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

	FIXTURE_NAME()
		:	base(new stream::string())
	{
		this->base << makeString(INITIALSTATE_NAME);

		#ifdef MAP_HAS_SUPPDATA_LAYER1
			ADD_SUPPITEM(Layer1);
		#endif
		#ifdef MAP_HAS_SUPPDATA_LAYER2
			ADD_SUPPITEM(Layer2);
		#endif
		#ifdef MAP_HAS_SUPPDATA_LAYER3
			ADD_SUPPITEM(Layer3);
		#endif
		#ifdef MAP_HAS_SUPPDATA_EXTRA1
			ADD_SUPPITEM(Extra1);
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

	FIXTURE_NAME(int i)
		:	base(new stream::string())
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

#ifndef MAP_DETECTION_UNCERTAIN
ISINSTANCE_TEST(c00, INITIALSTATE_NAME, gm::MapType::DefinitelyYes);
#endif


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

	unsigned int layerCount = this->map2d->getLayerCount();
	unsigned int width, height;
	this->map2d->getMapSize(&width, &height);

	unsigned int x, y;
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
		targetX = MAP_FIRST_CODE_X_L ## LAYER; \
		targetY = MAP_FIRST_CODE_Y_L ## LAYER; \
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
		unsigned int targetX = -1, targetY = -1;
		for (gm::Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			(i != items->end()) && (!foundFirstTile);
			i++
		) {
			switch (l) {
#if MAP_LAYER_COUNT >= 1
#ifndef MAP_FIRST_CODE_L1
#error MAP_FIRST_CODE_L1 must be defined for this map format
#endif
CHECK_FIRST_TILE_IN_LAYER(1)
#endif
#if MAP_LAYER_COUNT >= 2
#ifndef MAP_FIRST_CODE_L2
#error MAP_FIRST_CODE_L2 must be defined for this map format
#endif
CHECK_FIRST_TILE_IN_LAYER(2)
#endif
#if MAP_LAYER_COUNT >= 3
#ifndef MAP_FIRST_CODE_L3
#error MAP_FIRST_CODE_L3 must be defined for this map format
#endif
CHECK_FIRST_TILE_IN_LAYER(3)
#endif
#if MAP_LAYER_COUNT >= 4
#ifndef MAP_FIRST_CODE_L4
#error MAP_FIRST_CODE_L4 must be defined for this map format
#endif
CHECK_FIRST_TILE_IN_LAYER(4)
#endif
			}
		}
		BOOST_REQUIRE_MESSAGE(foundFirstTile == true,
			"Unable to find first tile in layer " << l << " at position "
			<< targetX << "," << targetY);
		BOOST_TEST_MESSAGE("Found first tile in layer " << l);
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

#ifdef MAP_HAS_SUPPDATA_LAYER2
	BOOST_CHECK_MESSAGE(
		is_supp_equal(camoto::SuppItem::Layer2, makeString(TEST_RESULT(initialstate_Layer2))),
		"Error writing map to a file - data is different to original in supp::layer2"
	);
#endif

#ifdef MAP_HAS_SUPPDATA_LAYER3
	BOOST_CHECK_MESSAGE(
		is_supp_equal(camoto::SuppItem::Layer3, makeString(TEST_RESULT(initialstate_Layer3))),
		"Error writing map to a file - data is different to original in supp::layer3"
	);
#endif

#ifdef MAP_HAS_SUPPDATA_EXTRA1
	BOOST_CHECK_MESSAGE(
		is_supp_equal(camoto::SuppItem::Extra1, makeString(TEST_RESULT(initialstate_Extra1))),
		"Error writing map to a file - data is different to original in supp::extra1"
	);
#endif
}

BOOST_AUTO_TEST_CASE(TEST_NAME(codelist))
{
	BOOST_TEST_MESSAGE("Checking map codes are all in allowed tile list");
	for (int l = 0; l < MAP_LAYER_COUNT; l++) {
		gm::Map2D::LayerPtr layer = map2d->getLayer(l);
		const gm::Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		const gm::Map2D::Layer::ItemPtrVectorPtr allowed = layer->getValidItemList();
		for (gm::Map2D::Layer::ItemPtrVector::const_iterator
			i = items->begin(); i != items->end(); i++
		) {
			bool found = false;
			for (gm::Map2D::Layer::ItemPtrVector::const_iterator
				j = allowed->begin(); j != allowed->end(); j++
			) {
				if ((*i)->code == (*j)->code) {
					found = true;
					break;
				}
			}
			BOOST_REQUIRE_MESSAGE(found == true,
				"Map code " << std::hex << (int)(*i)->code
				<< " was not found in the list of permitted tiles for layer "
				<< std::dec << l);
		}
	}
}

BOOST_AUTO_TEST_CASE(TEST_NAME(codelist_valid))
{
	BOOST_TEST_MESSAGE("Checking allowed tile list is set up correctly");
	for (int l = 0; l < MAP_LAYER_COUNT; l++) {
		gm::Map2D::LayerPtr layer = map2d->getLayer(l);
		const gm::Map2D::Layer::ItemPtrVectorPtr allowed = layer->getValidItemList();
		for (gm::Map2D::Layer::ItemPtrVector::const_iterator
			i = allowed->begin(); i != allowed->end(); i++
		) {
			// Coordinates must be zero, otherwise UI selections from the tile list
			// will be off
			BOOST_REQUIRE_EQUAL((*i)->x, 0);
			BOOST_REQUIRE_EQUAL((*i)->y, 0);

			// Type must be a valid Map2D::Layer::Item::Type value
			BOOST_REQUIRE_LE((*i)->type, 0x000F);
		}
	}
}

BOOST_AUTO_TEST_SUITE_END()

#define TEST_CONVERSION(name, data_in, data_out)	\
	BOOST_FIXTURE_TEST_CASE(readwrite_ ## name, FIXTURE_NAME) \
{ \
	BOOST_TEST_MESSAGE("Read+write map codes"); \
\
	this->base->truncate(0); \
	this->base << makeString(data_in); \
\
	BOOST_REQUIRE_NO_THROW( \
		gm::ManagerPtr pManager = gm::getManager(); \
		this->pTestType = pManager->getMapTypeByCode(MAP_TYPE); \
	); \
	BOOST_REQUIRE_MESSAGE(pTestType, "Could not find map type " MAP_TYPE); \
\
	this->map = this->pTestType->open(this->base, this->suppData); \
	this->map2d = boost::dynamic_pointer_cast<gm::Map2D>(this->map); \
	BOOST_REQUIRE_MESSAGE(this->map2d, "Could not create map class"); \
\
	this->base->truncate(0); \
\
	this->pTestType->write(this->map, this->base, this->suppData); \
\
	BOOST_CHECK_MESSAGE( \
		is_equal(makeString(data_out)), \
		"Error writing map - data is different to expected" \
	); \
}
