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

#ifndef _CAMOTO_GAMEMAPS_TEST_MAP2D_HPP_
#define _CAMOTO_GAMEMAPS_TEST_MAP2D_HPP_

#include <map>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/iostreams/copy.hpp>
#include <boost/bind.hpp>
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

		/// Reset pMap back to a known state.
		virtual void prepareTest();

		void test_isinstance_others();
		void test_getsize();
		void test_read();
		void test_write();
		void test_codelist();
		void test_codelist_valid();

	protected:
		/// Initial state.
		/**
		 * This is the base state loaded into a format handler and then
		 * modified to produce the states checked by the functions below.
		 */
		virtual std::string initialstate() = 0;

		/// Add a test to the suite.  Used by ADD_MAP2D_TEST().
		void addBoundTest(boost::function<void()> fnTest,
			boost::unit_test::const_string name);

		/// Reset the map content to the initial state and run the given test.
		/**
		 * @param fnTest
		 *   Function to call once map stream is back to initial state.
		 */
		void runTest(boost::function<void()> fnTest);

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
		 * @param content
		 *   Content to pass as a map to Map2D::isInstance() where
		 *   it will be reported as a valid instance, then passed to
		 *   Map2D::read(), where an exception should be thrown.
		 */
		void conversion(const std::string& input, const std::string& output);

		/// Perform a conversion check now.
		void test_conversion(const std::string& input, const std::string& output,
			unsigned int testNumber);

		/// Does the map content match the parameter?
		boost::test_tools::predicate_result is_content_equal(const std::string& exp);

		/// Does the given supplementary item content match the parameter?
		boost::test_tools::predicate_result is_supp_equal(
			camoto::SuppItem::Type type, const std::string& strExpected);

	protected:
		/// Underlying data stream containing map file content.
		stream::string_sptr base;

		/// Factory class used to open maps in this format.
		MapTypePtr pMapType;

		/// Pointer to the active map instance.
		Map2DPtr pMap;

		/// Supplementary data for the map.
		camoto::SuppData suppData;

	private:
		/// Have we allocated pMapType yet?
		bool init;

		/// Number of isInstance tests, used to number them sequentially.
		unsigned int numIsInstanceTests;

		/// Number of invalidData tests, used to number them sequentially.
		unsigned int numInvalidContentTests;

		/// Number of conversion tests, used to number them sequentially.
		unsigned int numConversionTests;

	public:
		/// File type code for this format.
		std::string type;

		/// Width of the entire map, in pixels.
		int pxWidth;

		/// Height of the entire map, in pixels.
		int pxHeight;

		/// Number of layers in the map.
		int numLayers;

		/// Map codes to inspect, one per layer.
		struct {
			int x;
			int y;
			int code;
		} mapCode[MAP2D_MAX_LAYERS];

		/// Link between supplementary items and the class containing the expected
		/// content for each test case.
		std::map<camoto::SuppItem::Type, boost::shared_ptr<test_map2d> > suppResult;
};

/// Add a test_map2d member function to the test suite
#define ADD_MAP2D_TEST(fn) {	  \
	boost::function<void()> fnTest = boost::bind(fn, this); \
	this->test_map2d::addBoundTest(fnTest, BOOST_TEST_STRINGIZE(fn)); \
}

#endif // _CAMOTO_GAMEMAPS_TEST_MAP2D_HPP_
