/**
 * @file   maptype.hpp
 * @brief  MapType class, used to identify and open an instance of a
 *         particular map format.
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

#ifndef _CAMOTO_GAMEMAPS_MAPTYPE_HPP_
#define _CAMOTO_GAMEMAPS_MAPTYPE_HPP_

#include <vector>
#include <map>

#include <camoto/types.hpp>
#include <camoto/gamemaps/map.hpp>

/// Main namespace
namespace camoto {
/// Namespace for this library
namespace gamemaps {

/// Type of supplemental file.
enum E_SUPPTYPE {
	/// Compression dictionary is external
	EST_DICT,
};

/// Supplementary item for a map.
/**
 * This class contains data about a supplementary item required to open a
 * map file.
 *
 * @see MapType::getRequiredSupps()
 */
struct SuppItem {
	/// The stream containing the supplemental data.
	iostream_sptr stream;
	/// The truncate callback (required)
	FN_TRUNCATE fnTruncate;
};

/// A list of required supplemental files and their filenames.  The filenames
/// may contain a path (especially if the main map file also contains a
/// path.)
typedef std::map<E_SUPPTYPE, std::string> MP_SUPPLIST;

/// A list of the supplemental file types mapped to open file streams.
typedef std::map<E_SUPPTYPE, SuppItem> MP_SUPPDATA;

/// Interface to a particular map format.
class MapType {

	public:

		/// Confidence level when guessing a file format.
		enum Certainty {
			DefinitelyNo,  ///< Definitely not in this format
			Unsure,        ///< The checks were inconclusive, it could go either way
			PossiblyYes,   ///< Everything checked out OK, but there's no signature
			DefinitelyYes, ///< This format has a signature and it matched
		};

		/// Get a short code to identify this file format, e.g. "map-xargon"
		/**
		 * This can be useful for command-line arguments.
		 *
		 * @return The map short name/ID.
		 */
		virtual std::string getMapCode() const
			throw () = 0;

		/// Get the map name, e.g. "Xargon map"
		/**
		 * @return The map name.
		 */
		virtual std::string getFriendlyName() const
			throw () = 0;

		/// Get a list of the known file extensions for this format.
		/**
		 * @note Can be empty for embedded maps with no filenames.
		 *
		 * @return A vector of file extensions, e.g. "xr1"
		 */
		virtual std::vector<std::string> getFileExtensions() const
			throw () = 0;

		/// Get a list of games using this format.
		/**
		 * @return A vector of game names, such as "Major Stryker", "Cosmo's Cosmic
		 *         Adventures", "Duke Nukem II"
		 */
		virtual std::vector<std::string> getGameList() const
			throw () = 0;

		/// Check a stream to see if it's in this map format.
		/**
		 * @param psMap
		 *   A C++ iostream of the file to test.
		 *
		 * @return A single confidence value from \ref E_CERTAINTY.
		 */
		virtual Certainty isInstance(istream_sptr psMap) const
			throw (std::ios::failure) = 0;

		/// Create a blank map in this format.
		/**
		 * This function creates an empty map in the given format.
		 *
		 * @param suppData
		 *   Any supplemental data required by this format (see getRequiredSupps()).
		 *
		 * @return A shared pointer to an instance of the Map class.
		 */
		virtual MapPtr create(MP_SUPPDATA& suppData) const
			throw (std::ios::failure) = 0;

		/// Open a map file.
		/**
		 * @pre Recommended that isInstance() has returned > EC_DEFINITELY_NO.
		 *
		 * @param input
		 *   The map file.
		 *
		 * @param suppData
		 *   Any supplemental data required by this format (see getRequiredSupps())
		 *
		 * @return A shared pointer to an instance of the Map class.
		 *
		 * @note Will throw an exception if the data is invalid (likely if
		 *   isInstance() returned EC_DEFINITELY_NO) however it will try its best
		 *   to read the data anyway, to make it possible to "force" a file to be
		 *   opened by a particular format handler.
		 */
		virtual MapPtr open(istream_sptr input, MP_SUPPDATA& suppData) const
			throw (std::ios::failure) = 0;

		/// Write a map out to a file in this format.
		/**
		 * @param map
		 *   The map to write out to the stream.
		 *
		 * @param output
		 *   The stream where the raw map data will be written to.
		 *
		 * @param suppData
		 *   Any supplemental data required by this format (see getRequiredSupps())
		 *
		 * @return The amount of data written.  The caller should ensure the output
		 *   stream is truncated to this length if necessary.
		 */
		virtual unsigned long write(MapPtr map, ostream_sptr output,
			MP_SUPPDATA& suppData) const
			throw (std::ios::failure) = 0;

		/// Get a list of any required supplemental files.
		/**
		 * For some map formats, data is stored externally to the map file
		 * itself (for example the filenames may be stored in a different file than
		 * the actual file data.)  This function obtains a list of these
		 * supplementary files, so the caller can open them and pass them along
		 * to the map manipulation classes.
		 *
		 * Note to format implementors: This function only needs to be overridden
		 * if there is supplementary data, otherwise the default implementation
		 * returns an empty vector, indicating there is no supp data.
		 *
		 * @param  filenameMap  The filename of the map (no path.)  This is
		 *         for supplemental files which share the same base name as the
		 *         map, but a different filename extension.
		 *         This can be empty for embedded map files with no filenames.
		 *
		 * @return A (possibly empty) map associating required supplemental file
		 *         types with their filenames.  For each returned value the file
		 *         should be opened and placed in a SuppItem instance.  The
		 *         SuppItem is then added to an \ref MP_SUPPDATA map where it can
		 *         be passed to newMap() or open().  Note that the filenames
		 *         returned can have relative paths, and may even have an absolute
		 *         path, if one was passed in with filenameMap.
		 */
		virtual MP_SUPPLIST getRequiredSupps(const std::string& filenameMap) const
			throw ();

};

/// Shared pointer to a MapType.
typedef boost::shared_ptr<MapType> MapTypePtr;

/// Vector of MapType shared pointers.
typedef std::vector<MapTypePtr> VC_MAPTYPE;

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MAPTYPE_HPP_
