/**
 * @file   test-map2d.hpp
 * @brief  Generic test code for Map2D class descendents.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/gamemaps.hpp>
#include <iostream>
#include <iomanip>

#include "tests.hpp"

// Local headers that will not be installed
#include <camoto/segmented_stream.hpp>

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

// Allow a string constant to be passed around with embedded nulls
#define makeString(x)  std::string((x), sizeof((x)) - 1)

#include <camoto/gamemaps/map2d.hpp>

struct FIXTURE_NAME: public default_sample {

	typedef boost::shared_ptr<std::stringstream> sstr_ptr;

	sstr_ptr baseData;
	void *_do; // unused var, but allows a statement to run in constructor init
	camoto::iostream_sptr baseStream;
	gm::MapPtr map;
	gm::Map2DPtr map2d;
	camoto::SuppData suppData;
	std::map<camoto::SuppItem::Type, sstr_ptr> suppBase;
	gm::MapTypePtr pTestType;

	FIXTURE_NAME() :
		baseData(new std::stringstream),
		_do((*this->baseData) << makeString(INITIALSTATE_NAME)),
		baseStream(this->baseData)
	{
		#ifdef HAS_FAT
		{
			boost::shared_ptr<std::stringstream> suppSS(new std::stringstream);
			suppSS->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
			(*suppSS) << makeString(TEST_RESULT(FAT_initialstate));
			camoto::iostream_sptr suppStream(suppSS);
			gm::SuppItem si;
			si.stream = suppStream;
			si.fnTruncate = boost::bind<void>(stringStreamTruncate, suppSS.get(), _1);
			this->suppData[camoto::SuppItem::FAT] = si;
			this->suppBase[camoto::SuppItem::FAT] = suppSS;
		}
		#endif

		BOOST_REQUIRE_NO_THROW(
			this->baseData->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

			gm::ManagerPtr pManager = gm::getManager();
			this->pTestType = pManager->getMapTypeByCode(MAP_TYPE);
		);
		BOOST_REQUIRE_MESSAGE(pTestType, "Could not find map type " MAP_TYPE);

		// Create an instance of the initialstate data
		this->map = this->pTestType->open(this->baseStream, this->suppData);
		this->map2d = boost::dynamic_pointer_cast<gm::Map2D>(this->map);
		BOOST_REQUIRE_MESSAGE(this->map2d, "Could not create map class");
	}

	FIXTURE_NAME(int i) :
		baseData(new std::stringstream),
		baseStream(this->baseData)
	{
		this->baseData->exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		return this->default_sample::is_equal(strExpected, this->baseData->str());
	}

	boost::test_tools::predicate_result is_supp_equal(camoto::SuppItem::Type type, const std::string& strExpected)
	{
		return this->default_sample::is_equal(strExpected, this->suppBase[type]->str());
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
		boost::shared_ptr<std::stringstream> psstrBase(new std::stringstream); \
		(*psstrBase) << makeString(d); \
		camoto::iostream_sptr psBase = boost::dynamic_pointer_cast<std::iostream>(psstrBase); \
		\
		BOOST_CHECK_EQUAL(pTestType->isInstance(psBase), r); \
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
		camoto::iostream_sptr suppStream(suppSS); \
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
		camoto::iostream_sptr psBase(psstrBase); \
		\
		camoto::SuppData suppData; \
		INVALIDDATA_FATCODE(f) \
		\
		BOOST_CHECK_THROW( \
			gm::MapPtr pMap(pTestType->open(psBase, suppData)), \
			std::ios::failure \
		); \
	}

BOOST_AUTO_TEST_CASE(TEST_NAME(getsize))
{
	BOOST_TEST_MESSAGE("Getting map size");

	int layerCount = this->map2d->getLayerCount();
	int width = -1, height = -1;
	if (this->map2d->getCaps() & gm::Map2D::HasGlobalSize) {
		this->map2d->getMapSize(&width, &height);
		if (this->map2d->getCaps() & gm::Map2D::HasGlobalTileSize) {
			int x, y;
			this->map2d->getTileSize(&x, &y);
			width *= x;
			height *= y;
		}
	} else {
		for (int i = 0; i < layerCount; i++) {
			gm::Map2D::LayerPtr layer = this->map2d->getLayer(i);
			int layerCaps = layer->getCaps();
			int lwidth, lheight;
			if (layerCaps & gm::Map2D::Layer::HasOwnSize) {
				layer->getLayerSize(&lwidth, &lheight);
			}
			if (layerCaps & gm::Map2D::Layer::HasOwnTileSize) {
				int x, y;
				layer->getTileSize(&x, &y);
				lwidth *= x;
				lheight *= y;
			}
			if (lwidth > width) width = lwidth;
			if (lheight > height) height = lheight;
		}
	}

	BOOST_REQUIRE_EQUAL(width, MAP_WIDTH_PIXELS);
	BOOST_REQUIRE_EQUAL(height, MAP_HEIGHT_PIXELS);
	BOOST_REQUIRE_EQUAL(layerCount, MAP_LAYER_COUNT);
}

BOOST_AUTO_TEST_CASE(TEST_NAME(read))
{
	BOOST_TEST_MESSAGE("Reading map codes");

	for (int l = 0; l < MAP_LAYER_COUNT; l++) {
		gm::Map2D::LayerPtr layer = map2d->getLayer(l);
		const gm::Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		bool foundFirstTile = false;
		for (gm::Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			i != items->end();
			i++
		) {
			if (((*i)->x == 0) && ((*i)->y == 0)) {
				BOOST_REQUIRE_EQUAL((*i)->code, MAP_FIRST_CODE);
				foundFirstTile = true;
				break;
			}
		}
		BOOST_REQUIRE_EQUAL(foundFirstTile, true);
		BOOST_TEST_MESSAGE("Found first tile in this layer");
	}
}

BOOST_AUTO_TEST_CASE(TEST_NAME(write))
{
	BOOST_TEST_MESSAGE("Write map codes");

	this->baseData->seekp(0);
	this->baseData->str("");
	this->pTestType->write(this->map, this->baseData, this->suppData);

	BOOST_CHECK_MESSAGE(
		is_equal(makeString(INITIALSTATE_NAME)),
		"Error writing map to a file - data is different to original"
	);
}

BOOST_AUTO_TEST_SUITE_END()
