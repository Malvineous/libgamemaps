/**
 * @file   base-maptype.cpp
 * @brief  Shared functionality for all MapType classes.
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

#include <camoto/stream_memory.hpp>
#include "base-maptype.hpp"

namespace camoto {
namespace gamemaps {

typedef std::map<SuppItem::Type, stream::expanding_inout_sptr> ExpandingSuppDataRW;

BaseMapType::BaseMapType()
{
}

BaseMapType::~BaseMapType()
{
}

void BaseMapType::write(MapPtr map, stream::output_sptr output,
	SuppData& suppData) const
{
	stream::memory_sptr expOut(new stream::memory);
	ExpandingSuppData expSuppData;
	ExpandingSuppDataRW expSuppDataRW;
	for (SuppData::const_iterator i = suppData.begin(); i != suppData.end(); i++) {
		stream::memory_sptr s(new stream::memory);
		expSuppDataRW[i->first] = s;  // remember for later
		expSuppData[i->first] = s;    // pass to write()
	}

	// Write out the map to our in-memory buffers
	this->write(map, expOut, expSuppData);

	// Copy the main stream data back to the parent
	stream::len lenOut = expOut->size();
	expOut->seekg(0, stream::start);
	output->truncate(lenOut);
	output->seekp(0, stream::start);
	stream::copy(output, expOut);
	output->flush();

	// Copy the suppData streams back to the parent
	for (SuppData::iterator i = suppData.begin(); i != suppData.end(); i++) {
		stream::input_sptr source = expSuppDataRW[i->first];
		stream::output_sptr target = i->second;
		stream::len lenSource = source->size();
		source->seekg(0, stream::start);
		target->truncate(lenSource);
		target->seekp(0, stream::start);
		stream::copy(target, source);
		target->flush();
	}

	return;
}

} // namespace gamemaps
} // namespace camoto
