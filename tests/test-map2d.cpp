/**
 * @file   test-map2d.cpp
 * @brief  Generic test code for Map2D class descendents.
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

#include <functional>
#include <iomanip>
#include <camoto/util.hpp>
#include "test-map2d.hpp"

using namespace camoto;
using namespace camoto::gamemaps;

/// Check whether a supp item is present and if so that the content is correct.
#define CHECK_ALL_SUPP_ITEMS(check_func, msg) \
	for (unsigned int i = 0; i < (unsigned int)SuppItem::MaxValue; i++) { \
		auto s = (SuppItem)i; \
		if ( \
			(this->suppResult.find(s) != this->suppResult.end()) \
			&& (this->suppResult[s]->written) \
		) { \
			BOOST_CHECK_MESSAGE( \
				this->is_supp_equal(s, \
					this->suppResult[s]->check_func()), \
				"[SuppItem::" << camoto::suppToString(s) << "] " msg \
			); \
		} \
	}

test_map2d::test_map2d()
	:	numIsInstanceTests(0),
		numInvalidContentTests(1),
		numConversionTests(1)
{
	this->pxSize = {-1, -1};
	this->numLayers = -1;
	for (unsigned int i = 0; i < MAP2D_MAX_LAYERS; i++) {
		this->mapCode[i].pos = {-1, -1};
		this->mapCode[i].code = -1;
	}
	this->written = true;
}

void test_map2d::addTests()
{
	ADD_MAP2D_TEST(false, &test_map2d::test_isinstance_others);
	ADD_MAP2D_TEST(false, &test_map2d::test_getsize);
	ADD_MAP2D_TEST(false, &test_map2d::test_read);
	ADD_MAP2D_TEST(false, &test_map2d::test_write);
	ADD_MAP2D_TEST(false, &test_map2d::test_codelist);
	ADD_MAP2D_TEST(false, &test_map2d::test_codelist_valid);
	//if (this->create) {
		// TODO
	//}
	return;
}

void test_map2d::addBoundTest(bool empty, boost::function<void()> fnTest,
	boost::unit_test::const_string name)
{
	std::function<void()> fnTestWrapper = std::bind(&test_map2d::runTest,
		this, empty, fnTest);
	this->ts->add(boost::unit_test::make_test_case(
		boost::unit_test::callback0<>(fnTestWrapper),
		createString(name << '[' << this->basename << ']')
	));
	return;
}

void test_map2d::runTest(bool empty, boost::function<void()> fnTest)
{
	this->map.reset();
	this->prepareTest(empty);
	BOOST_REQUIRE_MESSAGE(
		this->map.unique(),
		"Map has multiple references (" << this->map.use_count()
			<< ", expected 1) before use - this shouldn't happen!"
	);
	fnTest();
	if (this->map) {
		BOOST_REQUIRE_MESSAGE(
			this->map.unique(),
			"Map left with " << this->map.use_count()
				<< " references after test (should be only 1)"
		);
	}
	return;
}

void test_map2d::prepareTest(bool empty)
{
	auto mapType = MapManager::byCode(this->type);
	BOOST_REQUIRE_MESSAGE(mapType, "Could not find map type " + this->type);

	// Make this->suppData valid
	this->resetSuppData(empty);
	this->populateSuppData();

	this->base = std::make_unique<stream::string>();

	std::shared_ptr<Map> basemap;
	if (empty) {
		BOOST_TEST_CHECKPOINT("About to create new empty instance of "
			+ this->basename);
		// This should really use BOOST_REQUIRE_NO_THROW but the message is more
		// informative without it.
		//BOOST_REQUIRE_NO_THROW(
			basemap = mapType->create(stream_wrap(this->base), this->suppData);
		//);
	} else {
		*this->base << this->initialstate();
		BOOST_TEST_CHECKPOINT("About to open " + this->basename
			+ " initialstate as an archive");
		// This should really use BOOST_REQUIRE_NO_THROW but the message is more
		// informative without it.
		//BOOST_REQUIRE_NO_THROW(
		basemap = mapType->open(stream_wrap(this->base), this->suppData);
		//);
	}
	this->map = std::dynamic_pointer_cast<Map2D>(basemap);
	BOOST_REQUIRE_MESSAGE(this->map, "Could not create map class");

	return;
}

void test_map2d::resetSuppData(bool emptyImage)
{
	this->suppBase.clear();
	for (auto& i : this->suppResult) {
		auto& item = i.first;
		if (!i.second) {
			std::cout << "Warning: " << this->basename << " sets empty "
				<< suppToString(item) << " suppitem, ignoring.\n";
			continue;
		}
		auto suppSS = std::make_shared<stream::string>();
		if (!emptyImage) {
			// Populate the suppitem with its initial state
			*suppSS << i.second->initialstate();
		}
		this->suppBase[item] = suppSS;
	}
	return;
}

void test_map2d::populateSuppData()
{
	this->suppData.clear();
	for (auto& i : this->suppBase) {
		auto& item = i.first;
		auto& suppSS = i.second;
		// Wrap this in a substream to get a unique pointer, with an independent
		// seek position.
		this->suppData[item] = stream_wrap(suppSS);
	}
	return;
}

void test_map2d::isInstance(MapType::Certainty result,
	const std::string& content)
{
	std::function<void()> fnTest = std::bind(&test_map2d::test_isInstance,
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

	auto pTestType = MapManager::byCode(this->type);
	BOOST_REQUIRE_MESSAGE(pTestType,
		createString("Could not find map type " << this->type));

	stream::string ss;
	ss << content;

	BOOST_CHECK_EQUAL(pTestType->isInstance(ss), result);
	return;
}

void test_map2d::invalidContent(const std::string& content)
{
	auto fnTest = std::bind(&test_map2d::test_invalidContent,
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
	BOOST_TEST_MESSAGE("invalidContent check (" << this->basename
		<< "; " << std::setfill('0') << std::setw(2) << testNumber << ")");

	auto pTestType = MapManager::byCode(this->type);
	BOOST_REQUIRE_MESSAGE(pTestType,
		createString("Could not find map type " << this->type));

	auto ss = std::make_unique<stream::string>();
	*ss << content;

	// Make sure isInstance reports this is valid
	BOOST_CHECK_EQUAL(pTestType->isInstance(*ss), MapType::DefinitelyYes);

	// But that we get an error when trying to open the file
	BOOST_CHECK_THROW(
		std::shared_ptr<Map> map2d(pTestType->open(std::move(ss), this->suppData)),
		stream::error
	);

	return;
}

void test_map2d::conversion(const std::string& input,
	const std::string& output)
{
	this->addBoundTest(false,
		std::bind(
			&test_map2d::test_conversion, this, input, output,
			this->numConversionTests
		),
		createString("test_map2d[" << this->basename << "]::conversion_"
			<< std::setfill('0') << std::setw(2) << this->numConversionTests)
	);
	this->numConversionTests++;
	return;
}

void test_map2d::test_conversion(const std::string& input,
	const std::string& output, unsigned int testNumber)
{
	BOOST_TEST_MESSAGE(createString("conversion check (" << this->basename
		<< "; " << std::setfill('0') << std::setw(2) << testNumber << ")"));

	this->base->truncate(0);
	this->map->flush();

	BOOST_CHECK_MESSAGE(
		this->is_content_equal(output),
		"Error writing map - data is different to expected"
	);

	return;
}

boost::test_tools::predicate_result test_map2d::is_content_equal(
	const std::string& exp)
{
	return this->is_equal(exp, this->base->data);
}

boost::test_tools::predicate_result test_map2d::is_supp_equal(
	camoto::SuppItem type, const std::string& strExpected)
{
	// Use the supp's test-class' own comparison function, as this will use its
	// preferred outputWidth value, which might be different to the main file's.
	return this->suppResult[type]->is_equal(strExpected,
		this->suppBase[type]->data);
}

void test_map2d::test_isinstance_others()
{
	// Check all file formats except this one to avoid any false positives
	BOOST_TEST_MESSAGE("isInstance check for other formats (not " << this->type
		<< ")");
	for (auto& testType : MapManager::formats()) {
		// Don't check our own type, that's done by the other isinstance_* tests
		std::string otherType = testType->code();
		if (otherType.compare(this->type) == 0) continue;

		// Skip any formats known to produce false detections unavoidably
		if (
			std::find(
				this->skipInstDetect.begin(), this->skipInstDetect.end(), otherType
			) != this->skipInstDetect.end()) continue;

		BOOST_TEST_MESSAGE("Checking " << this->type
			<< " content against isInstance() for " << otherType);

		BOOST_CHECK_MESSAGE(
			testType->isInstance(*this->base) < MapType::Certainty::DefinitelyYes,
			"isInstance() for " << otherType << " incorrectly recognises content for "
			<< this->type);
	}
	return;
}

void test_map2d::test_getsize()
{
	BOOST_TEST_MESSAGE("Getting map size");

	auto layerCount = this->map->layers().size();
	auto dims = this->map->mapSize();

	auto tileSize = this->map->tileSize();
	dims.x *= tileSize.x;
	dims.y *= tileSize.y;

	BOOST_REQUIRE_EQUAL(dims.x, this->pxSize.x);
	BOOST_REQUIRE_EQUAL(dims.y, this->pxSize.y);
	BOOST_REQUIRE_EQUAL(layerCount, this->numLayers);
}

void test_map2d::test_read()
{
	BOOST_TEST_MESSAGE("Reading map codes");

	unsigned int l = 0;
	for (auto& layer : this->map->layers()) {
		bool foundFirstTile = false;
		Point target = {-1, -1};
		for (auto& i : layer->items()) {
			target = this->mapCode[l].pos;
			if (
				(i.pos.x == target.x)
				&& (i.pos.y == target.y)
			) {
				foundFirstTile = true;
				BOOST_REQUIRE_EQUAL(i.code, this->mapCode[l].code);
			}
		}
		BOOST_REQUIRE_MESSAGE(foundFirstTile == true,
			"Unable to find first tile in layer " << l
			<< " (counting from layer 0) at position " << target.x << "," << target.y);
		BOOST_TEST_MESSAGE("Found first tile in layer " << l);
		l++;
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
		auto s = (SuppItem)i;
		if (this->suppResult.find(s) != this->suppResult.end()) {
			this->suppData[s]->truncate(0);
		}
	}

	this->map->flush();

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
	unsigned int l = 0;
	for (auto& layer : this->map->layers()) {
		auto allowed = layer->availableItems();
		for (auto& i : layer->items()) {
			bool found = false;
			for (auto& j : allowed) {
				if (i.code == j.code) {
					found = true;
					break;
				}
			}
			BOOST_REQUIRE_MESSAGE(found == true,
				"Map code " << std::hex << (int)i.code
				<< " was not found in the list of permitted tiles for layer "
				<< std::dec << (l+1));
		}
		l++;
	}
}

void test_map2d::test_codelist_valid()
{
	BOOST_TEST_MESSAGE("Checking allowed tile list is set up correctly");
	for (auto& layer : this->map->layers()) {
		for (auto& i : layer->availableItems()) {
			// Coordinates must be zero, otherwise UI selections from the tile list
			// will be off
			BOOST_REQUIRE_EQUAL(i.pos.x, 0);
			BOOST_REQUIRE_EQUAL(i.pos.y, 0);

			// Type must be a valid Map2D::Layer::Item::Type value
			BOOST_REQUIRE_LE((int)i.type, 0x001F);
		}
	}
}
