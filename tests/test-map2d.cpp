/**
 * @file   test-map2d.cpp
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

#include <iomanip>
#include <boost/bind.hpp>
#include <camoto/util.hpp>
#include "test-map2d.hpp"

using namespace camoto;
using namespace camoto::gamemaps;

/// Check whether a supp item is present and if so that the content is correct.
#define CHECK_ALL_SUPP_ITEMS(check_func, msg) \
	for (unsigned int i = 0; i < (unsigned int)SuppItem::MaxValue; i++) { \
		SuppItem::Type s = (SuppItem::Type)i; \
		if ((this->suppResult[s]) && (this->suppResult[s]->written)) { \
			BOOST_CHECK_MESSAGE( \
				this->is_supp_equal(s, \
					this->suppResult[s]->check_func()), \
				"[SuppItem::" << camoto::suppToString(s) << "] " msg \
			); \
		} \
	}

test_map2d::test_map2d()
	:	init(false),
		numIsInstanceTests(0),
		numInvalidContentTests(1),
		numConversionTests(1)
{
	this->pxWidth = -1;
	this->pxHeight = -1;
	this->numLayers = -1;
	for (unsigned int i = 0; i < MAP2D_MAX_LAYERS; i++) {
		this->mapCode[i].x = 0;
		this->mapCode[i].y = 0;
		this->mapCode[i].code = -1;
	}
	this->written = true;
}

void test_map2d::addTests()
{
	ADD_MAP2D_TEST(&test_map2d::test_isinstance_others);
	ADD_MAP2D_TEST(&test_map2d::test_getsize);
	ADD_MAP2D_TEST(&test_map2d::test_read);
	ADD_MAP2D_TEST(&test_map2d::test_write);
	ADD_MAP2D_TEST(&test_map2d::test_codelist);
	ADD_MAP2D_TEST(&test_map2d::test_codelist_valid);
	return;
}

void test_map2d::addBoundTest(boost::function<void()> fnTest,
	boost::unit_test::const_string name)
{
	boost::function<void()> fnTestWrapper = boost::bind(&test_map2d::runTest,
		this, fnTest);
	this->ts->add(boost::unit_test::make_test_case(
		boost::unit_test::callback0<>(fnTestWrapper),
		createString(name << '[' << this->basename << ']')
	));
	return;
}

void test_map2d::runTest(boost::function<void()> fnTest)
{
	this->prepareTest();
	fnTest();
	return;
}

void test_map2d::prepareTest()
{
	if (!this->init) {
		ManagerPtr pManager;
		BOOST_REQUIRE_NO_THROW(
			pManager = camoto::gamemaps::getManager();
			this->pMapType = pManager->getMapTypeByCode(this->type);
		);
		BOOST_REQUIRE_MESSAGE(pMapType, "Could not find map type " + this->type);
		this->init = true;
	}

	// Reset all the supp items to the initial state (or blank)
	for (unsigned int i = 0; i < (unsigned int)SuppItem::MaxValue; i++) {
		SuppItem::Type s = (SuppItem::Type)i;
		if (this->suppResult[s]) {
			stream::string_sptr suppSS(new stream::string());
			// Populate the suppitem with its initial state
			suppSS << this->suppResult[s]->initialstate();
			this->suppData[s] = suppSS;
		}
	}

	this->base.reset(new stream::string());
	this->base << this->initialstate();

	// Create an instance of the initialstate data
	MapPtr basemap = this->pMapType->open(this->base, this->suppData);
	this->pMap = boost::dynamic_pointer_cast<Map2D>(basemap);
	BOOST_REQUIRE_MESSAGE(this->pMap, "Could not create map class");

	return;
}

void test_map2d::isInstance(MapType::Certainty result,
	const std::string& content)
{
	boost::function<void()> fnTest = boost::bind(&test_map2d::test_isInstance,
		this, result, content, this->numIsInstanceTests);
	this->ts->add(boost::unit_test::make_test_case(
			boost::unit_test::callback0<>(fnTest),
			createString("test_map2d[" << this->basename << "]::isinstance_c"
				<< std::setfill('0') << std::setw(2) << this->numIsInstanceTests)
		));
	this->numIsInstanceTests++;
	return;
}

void test_map2d::test_isInstance(MapType::Certainty result,
	const std::string& content, unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(createString("isInstance check (" << this->basename
		<< "; " << std::setfill('0') << std::setw(2) << testNumber << ")"));

	ManagerPtr pManager(getManager());
	MapTypePtr pTestType(pManager->getMapTypeByCode(this->type));
	BOOST_REQUIRE_MESSAGE(pTestType,
		createString("Could not find map type " << this->type));

	stream::string_sptr ss(new stream::string());
	ss << content;

	BOOST_CHECK_EQUAL(pTestType->isInstance(ss), result);
	return;
}

void test_map2d::invalidContent(const std::string& content)
{
	boost::function<void()> fnTest = boost::bind(&test_map2d::test_invalidContent,
		this, content, this->numInvalidContentTests);
	this->ts->add(boost::unit_test::make_test_case(
			boost::unit_test::callback0<>(fnTest),
			createString("test_map2d[" << this->basename << "]::invalidcontent_i"
				<< std::setfill('0') << std::setw(2) << this->numInvalidContentTests)
		));
	this->numInvalidContentTests++;
	return;
}

void test_map2d::test_invalidContent(const std::string& content,
	unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(createString("invalidContent check (" << this->basename
		<< "; " << std::setfill('0') << std::setw(2) << testNumber << ")"));

	ManagerPtr pManager(getManager());
	MapTypePtr pTestType(pManager->getMapTypeByCode(this->type));
	BOOST_REQUIRE_MESSAGE(pTestType,
		createString("Could not find map type " << this->type));

	stream::string_sptr ss(new stream::string());
	ss << content;

	// Make sure isInstance reports this is valid
	BOOST_CHECK_EQUAL(pTestType->isInstance(ss), MapType::DefinitelyYes);

	// But that we get an error when trying to open the file
	BOOST_CHECK_THROW(
		MapPtr map2d(pTestType->open(ss, this->suppData)),
		stream::error
	);

	return;
}

void test_map2d::conversion(const std::string& input,
	const std::string& output)
{
	boost::function<void()> fnTest = boost::bind(&test_map2d::test_conversion,
		this, input, output, this->numConversionTests);
	this->ts->add(boost::unit_test::make_test_case(
			boost::unit_test::callback0<>(fnTest),
			createString("test_map2d[" << this->basename << "]::conversion_"
				<< std::setfill('0') << std::setw(2) << this->numConversionTests)
		));
	this->numConversionTests++;
	return;
}

void test_map2d::test_conversion(const std::string& input,
	const std::string& output, unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(createString("conversion check (" << this->basename
		<< "; " << std::setfill('0') << std::setw(2) << testNumber << ")"));

	this->base->truncate(0);
	this->base << input;

	MapPtr basemap = this->pMapType->open(this->base, this->suppData);
	this->pMap = boost::dynamic_pointer_cast<Map2D>(basemap);
	BOOST_REQUIRE_MESSAGE(this->pMap, "Could not create map class");

	this->base->truncate(0);
	this->pMapType->write(this->pMap, this->base, this->suppData);

	BOOST_CHECK_MESSAGE(
		this->is_content_equal(output),
		"Error writing map - data is different to expected"
	);

	return;
}

boost::test_tools::predicate_result test_map2d::is_content_equal(
	const std::string& exp)
{
	return this->is_equal(exp, *(this->base->str()));
}

boost::test_tools::predicate_result test_map2d::is_supp_equal(
	camoto::SuppItem::Type type, const std::string& strExpected)
{
	stream::string_sptr suppBase =
		boost::dynamic_pointer_cast<stream::string>(this->suppData[type]);
	return this->is_equal(strExpected, *(suppBase->str()));
}

void test_map2d::test_isinstance_others()
{
	// Check all file formats except this one to avoid any false positives
	BOOST_TEST_MESSAGE("isInstance check for other formats (not " << this->type
		<< ")");
	ManagerPtr pManager(camoto::gamemaps::getManager());
	int i = 0;
	MapTypePtr pTestType;
	for (int i = 0; (pTestType = pManager->getMapType(i)); i++) {
		// Don't check our own type, that's done by the other isinstance_* tests
		std::string otherType = pTestType->getMapCode();
		if (otherType.compare(this->type) == 0) continue;

		// Skip any formats known to produce false detections unavoidably
		if (
			std::find(
				this->skipInstDetect.begin(), this->skipInstDetect.end(), otherType
			) != this->skipInstDetect.end()) continue;

		BOOST_TEST_MESSAGE("Checking " << this->type
			<< " content against isInstance() for " << otherType);

		BOOST_CHECK_MESSAGE(pTestType->isInstance(base) < MapType::DefinitelyYes,
			"isInstance() for " << otherType << " incorrectly recognises content for "
			<< this->type);
	}
	return;
}

void test_map2d::test_getsize()
{
	BOOST_TEST_MESSAGE("Getting map size");

	unsigned int layerCount = this->pMap->getLayerCount();
	unsigned int width, height;
	this->pMap->getMapSize(&width, &height);

	unsigned int x, y;
	this->pMap->getTileSize(&x, &y);
	width *= x;
	height *= y;

	BOOST_REQUIRE_EQUAL(width, this->pxWidth);
	BOOST_REQUIRE_EQUAL(height, this->pxHeight);
	BOOST_REQUIRE_EQUAL(layerCount, this->numLayers);
}

void test_map2d::test_read()
{
	BOOST_TEST_MESSAGE("Reading map codes");

	for (int l = 0; l < this->numLayers; l++) {
		Map2D::LayerPtr layer = this->pMap->getLayer(l);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		bool foundFirstTile = false;
		unsigned int targetX = -1, targetY = -1;
		for (Map2D::Layer::ItemPtrVector::const_iterator i = items->begin();
			(i != items->end()) && (!foundFirstTile);
			i++
		) {
			targetX = this->mapCode[l].x;
			targetY = this->mapCode[l].y;
			if (
				((*i)->x == targetX)
				&& ((*i)->y == targetY)
			) {
				foundFirstTile = true;
				BOOST_REQUIRE_EQUAL((*i)->code, this->mapCode[l].code);
			}
		}
		BOOST_REQUIRE_MESSAGE(foundFirstTile == true,
			"Unable to find first tile in layer " << l
			<< " (counting from layer 0) at position " << targetX << "," << targetY);
		BOOST_TEST_MESSAGE("Found first tile in layer " << l);
	}
}

#define ERASE_SUPPITEM(suppitem) \
	this->suppData[camoto::SuppItem::suppitem]->truncate(0);

void test_map2d::test_write()
{
	BOOST_TEST_MESSAGE("Write map codes");

	this->base->truncate(0);

	// Erase all the supp items
	for (unsigned int i = 0; i < (unsigned int)SuppItem::MaxValue; i++) {
		SuppItem::Type s = (SuppItem::Type)i;
		if (this->suppResult[s]) {
			this->suppData[s]->truncate(0);
		}
	}

	this->pMapType->write(this->pMap, this->base, this->suppData);

	BOOST_CHECK_MESSAGE(
		this->is_content_equal(this->initialstate()),
		"Error writing map to a file - data is different to original"
	);

	CHECK_ALL_SUPP_ITEMS(initialstate,
		"Error writing map to a file - data is different to original");
}

void test_map2d::test_codelist()
{
	BOOST_TEST_MESSAGE("Checking map codes are all in allowed tile list");
	for (unsigned int l = 0; l < this->numLayers; l++) {
		Map2D::LayerPtr layer = this->pMap->getLayer(l);
		const Map2D::Layer::ItemPtrVectorPtr items = layer->getAllItems();
		const Map2D::Layer::ItemPtrVectorPtr allowed = layer->getValidItemList();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = items->begin(); i != items->end(); i++
		) {
			bool found = false;
			for (Map2D::Layer::ItemPtrVector::const_iterator
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
				<< std::dec << (l+1));
		}
	}
}

void test_map2d::test_codelist_valid()
{
	BOOST_TEST_MESSAGE("Checking allowed tile list is set up correctly");
	for (unsigned int l = 0; l < this->numLayers; l++) {
		Map2D::LayerPtr layer = this->pMap->getLayer(l);
		const Map2D::Layer::ItemPtrVectorPtr allowed = layer->getValidItemList();
		for (Map2D::Layer::ItemPtrVector::const_iterator
			i = allowed->begin(); i != allowed->end(); i++
		) {
			// Coordinates must be zero, otherwise UI selections from the tile list
			// will be off
			BOOST_REQUIRE_EQUAL((*i)->x, 0);
			BOOST_REQUIRE_EQUAL((*i)->y, 0);

			// Type must be a valid Map2D::Layer::Item::Type value
			BOOST_REQUIRE_LE((*i)->type, 0x001F);
		}
	}
}
