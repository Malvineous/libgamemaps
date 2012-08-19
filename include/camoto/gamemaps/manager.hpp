/**
 * @file   gamemaps/manager.hpp
 * @brief  Manager class, used for accessing the various map format readers.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_GAMEMAPS_MANAGER_HPP_
#define _CAMOTO_GAMEMAPS_MANAGER_HPP_

#include <boost/shared_ptr.hpp>
#include <camoto/gamemaps/maptype.hpp>

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

namespace camoto {
namespace gamemaps {

/// Top-level class to manage map types.
/**
 * This class provides access to the different map file formats supported
 * by the library.
 *
 * In order to open a map, this class must be used to access an instance
 * of the map type.  This MapType instance is then used to create an
 * Map instance around a particular file.  It is this Map instance that
 * is then used to access the map file itself.
 *
 * @note Use the free function getManager() to obtain a pointer to an instance
 *   of an object implementing the Manager interface.
 */
class Manager
{
	public:
		/// Get an MapType instance for a supported file format.
		/**
		 * This can be used to enumerate all available file formats.
		 *
		 * @param iIndex
		 *   Index of the format, starting from 0.
		 *
		 * @return A shared pointer to a MapType for the given index, or an empty
		 *   pointer once iIndex goes out of range.
		 */
		virtual const MapTypePtr getMapType(unsigned int iIndex) const = 0;

		/// Get an MapType instance by its code.
		/**
		 * @param strCode
		 *   %Map code (e.g. "grp-duke3d")
		 *
		 * @return A shared pointer to an MapType for the given code, or an empty
		 *   pointer on an invalid code.
		 */
		virtual const MapTypePtr getMapTypeByCode(const std::string& strCode)
			const = 0;
};

/// Shared pointer to a Manager.
typedef boost::shared_ptr<Manager> ManagerPtr;

/// Library entry point.
/**
 * All further functionality is provided by calling functions in the Manager
 * class.
 *
 * @return A shared pointer to a Manager instance.
 */
const ManagerPtr DLL_EXPORT getManager(void);

} // namespace gamemaps
} // namespace camoto

#endif // _CAMOTO_GAMEMAPS_MANAGER_HPP_
