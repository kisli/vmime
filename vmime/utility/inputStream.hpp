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

#ifndef VMIME_UTILITY_INPUTSTREAM_HPP_INCLUDED
#define VMIME_UTILITY_INPUTSTREAM_HPP_INCLUDED


#include "vmime/utility/stream.hpp"


namespace vmime {
namespace utility {


/** Simple input stream.
  */

class inputStream : public stream
{
public:

	/** Test for end of stream (no more data to read).
	  *
	  * @return true if we have reached the end of stream, false otherwise
	  */
	virtual bool eof() const = 0;

	/** Set the read pointer to the beginning of the stream.
	  *
	  * @warning WARNING: this may not work for all stream types.
	  */
	virtual void reset() = 0;

	/** Read data from the stream.
	  *
	  * @param data will receive the data read
	  * @param count maximum number of bytes to read
	  * @return number of bytes read
	  */
	virtual size_type read(value_type* const data, const size_type count) = 0;

	/** Skip a number of bytes.
	  *
	  * @param count maximum number of bytes to ignore
	  * @return number of bytes skipped
	  */
	virtual size_type skip(const size_type count) = 0;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_INPUTSTREAM_HPP_INCLUDED

