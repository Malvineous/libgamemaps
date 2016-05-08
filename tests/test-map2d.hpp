/**
 * @file   test-map2d.hpp
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

#ifndef _CAMOTO_GAMEMAPS_TEST_MAP2D_HPP_
#define _CAMOTO_GAMEMAPS_TEST_MAP2D_HPP_

#include <map>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <camoto/stream_string.hpp>
#include <camoto/gamemaps.hpp>
#include "tests.hpp"

// This header will only be used by test implementations.
using namespace camoto;
using namespace camoto::gamemaps;

/// Maximum number of layers the tests will handle.  Just increase this if ever
/// a map format has more layers.
#define MAP2D_MAX_LAYERS 5

class test_map2d: public test_main
{
	public:
		/// Constructor sets some default values.
		test_map2d();

		/// Add all the standard tests.
		/**
		 * This can be overridden by descendent classes to add more tests for
		 * particular file formats.  If this is done, remember to call this
		 * function from the overridden one or the standard tests won't get run.
		 */
		virtual void addTests();

		/// Reset map back to a known state.
		/**
		 * @param empty
		 *   true resets to an empty map (via MapType::create()) while
		 *   false resets to initialstate() and calls MapType::open().
		 */
		virtual void prepareTest(bool empty);

		void test_isinstance_others();
		void test_getsize();
		void test_read();
		void test_write();
		void test_codelist();
		void test_codelist_valid();
		void test_attributes();

	protected:
		/// Initial state.
		/**
		 * This is the base state loaded into a format handler and then
		 * modified to produce the states checked by the functions below.
		 */
		virtual std::string initialstate() = 0;

		/// Add a test to the suite.  Used by ADD_MAP2D_TEST().
		void addBoundTest(bool empty, boost::function<void()> fnTest,
			boost::unit_test::const_string file, std::size_t line,
			boost::unit_test::const_string name);

		/// Reset the map content to the initial state and run the given test.
		/**
		 * @param empty
		 *   true resets to an empty map (via MapType::create()) while
		 *   false resets to initialstate() and calls MapType::open().
		 *
		 * @param fnTest
		 *   Function to call once map stream is back to initial state.
		 */
		void runTest(bool empty, boost::function<void()> fnTest);

		/// Populate suppBase with default content.
		/**
		 * This may be called mid-test if the suppBase content should be reset to
		 * the initial state.
		 */
		void resetSuppData(bool empty);

		/// Populate suppData with data loaded from suppBase.
		/**
		 * This may be called mid-test if new suppData structures are needed to
		 * create a new Image instance, since the original ones have been
		 * std::move()'d to the original Image instance and won't be available to
		 * create a new Image with.
		 *
		 * This repopulates suppData from the existing suppBase content, so it is
		 * possible to access modified data this way.  If you don't want suppData
		 * that may have been modified by a previous Image instance, call
		 * resetSuppData() first to return everything to the initialstate.
		 */
		void populateSuppData();

		/// Check if main content and all supp data streams match expected values.
		/**
		 * @param fnExpected
		 *   test_map2d member function to call (on the suppitem instance) to
		 *   get the expected data, e.g. &test_map2d::initialstate
		 *
		 * @param msg
		 *   Error message in case of data mismatch.
		 */
		void checkData(std::function<std::string(test_map2d&)> fnExpected,
			const std::string& msg);

		/// Add an isInstance check to run later.
		/**
		 * @param result
		 *   Expected result when opening the content.
		 *
		 * @param content
		 *   Content to pass as a map to Map2D::isInstance().
		 */
		void isInstance(MapType::Certainty result, const std::string& content);

		/// Perform an isInstance check now.
		void test_isInstance(MapType::Certainty result,
			const std::string& content, unsigned int testNumber);

		/// Add an invalidContent check to run later.
		/**
		 * These checks make sure files that are in the correct format
		 * don't cause segfaults or infinite loops if the data is corrupted.
		 *
		 * @param content
		 *   Content to pass as a map to Map2D::isInstance() where
		 *   it will be reported as a valid instance, then passed to
		 *   Map2D::read(), where an exception should be thrown.
		 */
		void invalidContent(const std::string& content);

		/// Perform an invalidContent check now.
		void test_invalidContent(const std::string& content,
			unsigned int testNumber);

		/// Add a conversion check to run later.
		/**
		 * These checks make sure files that are read with certain semi-valid values
		 * are written out with better (different) values.  These would fail the
		 * normal read/write tests because the output won't be identical to the
		 * input.
		 *
		 * @param input
		 *   Content to pass as a map to Map2D::isInstance() where
		 *   it will be reported as a valid instance, then passed to
		 *   Map2D::read(), where an exception should be thrown.
		 *
		 * @param output
		 *   Expected output after the read/write process.
		 */
		void conversion(const std::string& input, const std::string& output);

		/// Perform a conversion check now.
		void test_conversion(const std::string& input, const std::string& output,
			unsigned int testNumber);

		/// Add a changeAttribute check to run later.
		/**
		 * These checks make sure attribute alterations work correctly.
		 *
		 * @param attributeIndex
		 *   Zero-based index of the attribute to change.
		 *
		 * @param newValue
		 *   New content for the attribute.
		 *
		 * @param content
		 *   Expected result after taking the initialstate() and changing the
		 *   given attribute as specified.
		 */
		void changeAttribute(unsigned int attributeIndex,
			const std::string& newValue, const std::string& content);

		void changeAttribute(unsigned int attributeIndex,
			unsigned int newValue, const std::string& content);

		/// Perform a changeAttribute<std::string> check now.
		void test_changeAttribute(unsigned int attributeIndex,
			const std::string& newValue, const std::string& content,
			unsigned int testNumber);

		/// Perform a changeAttribute<int> check now.
		void test_changeAttribute(unsigned int attributeIndex,
			int newValue, const std::string& content,
			unsigned int testNumber);

		/// Does the map content match the parameter?
		boost::test_tools::predicate_result is_content_equal(const std::string& exp);

		/// Does the given supplementary item content match the parameter?
		boost::test_tools::predicate_result is_supp_equal(
			camoto::SuppItem type, const std::string& strExpected);

	protected:
		/// Underlying data stream containing map file content.
		std::shared_ptr<stream::string> base;

		/// Pointer to the active map instance.
		std::shared_ptr<camoto::gamemaps::Map2D> map;

		/// Pointers to the underlying storage used for suppitems.
		std::map<SuppItem, std::shared_ptr<stream::string>> suppBase;

		/// Supplementary data for the archive, populated by streams sitting on
		/// top of suppBase.
		camoto::SuppData suppData;

	private:
		/// Number of isInstance tests, used to number them sequentially.
		unsigned int numIsInstanceTests;

		/// Number of invalidData tests, used to number them sequentially.
		unsigned int numInvalidContentTests;

		/// Number of conversion tests, used to number them sequentially.
		unsigned int numConversionTests;

		/// Number of attribute-change tests, used to number them sequentially.
		unsigned int numChangeAttributeTests;

	public:
		/// File type code for this format.
		std::string type;

		/// Any formats here identify us as an instance of that type, and it
		/// cannot be avoided.
		/**
		 * If "otherformat" is listed here then we will not pass our initialstate
		 * to otherformat's isInstance function.  This is kind of backwards but is
		 * is the way the test functions are designed.
		 */
		std::vector<std::string> skipInstDetect;

		/// Width and height of the entire map, in pixels.
		camoto::gamemaps::Point pxSize;

		/// Number of layers in the map.
		int numLayers;

		/// Map codes to inspect, one per layer.
		struct {
			camoto::gamemaps::Point pos;
			int code;
		} mapCode[MAP2D_MAX_LAYERS];

		/// List of attributes this format supports, can be empty.
		std::vector<camoto::Attribute> attributes;

		/// Link between supplementary items and the class containing the expected
		/// content for each test case.
		std::map<camoto::SuppItem, std::shared_ptr<test_map2d>> suppResult;

		/// Set to false if this instance is of a supp item and it is not written
		/// out when saving a map.
		bool written;
};

/// Add a test_map2d member function to the test suite
#define ADD_MAP2D_TEST(create, fn) \
	this->test_map2d::addBoundTest( \
		create, \
		std::bind(fn, this), \
		__FILE__, __LINE__, \
		BOOST_TEST_STRINGIZE(fn) \
	);

#endif // _CAMOTO_GAMEMAPS_TEST_MAP2D_HPP_
