//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2012 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_UTILITY_SEEKABLEINPUTSTREAMREGIONADAPTER_HPP_INCLUDED
#define VMIME_UTILITY_SEEKABLEINPUTSTREAMREGIONADAPTER_HPP_INCLUDED


#include "vmime/utility/seekableInputStream.hpp"


namespace vmime {
namespace utility {


/** An adapter for reading a limited region of a seekable input stream.
  */

class seekableInputStreamRegionAdapter : public seekableInputStream
{
public:

	/** Creates a new adapter for a seekableInputStream.
	  *
	  * @param stream source stream
	  * @param begin start position in source stream
	  * @param length region length in source stream
	  */
	seekableInputStreamRegionAdapter(ref <seekableInputStream> stream,
		const size_type begin, const size_type length);

	bool eof() const;
	void reset();
	size_type read(value_type* const data, const size_type count);
	size_type skip(const size_type count);
	size_type getPosition() const;
	void seek(const size_type pos);

private:

	ref <seekableInputStream> m_stream;
	size_type m_begin;
	size_type m_length;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_SEEKABLEINPUTSTREAMREGIONADAPTER_HPP_INCLUDED

