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

#ifndef VMIME_UTILITY_SEEKABLEINPUTSTREAM_HPP_INCLUDED
#define VMIME_UTILITY_SEEKABLEINPUTSTREAM_HPP_INCLUDED


#include "vmime/utility/inputStream.hpp"


namespace vmime {
namespace utility {


/** An input stream that allows seeking within the input.
  */

class seekableInputStream : public inputStream
{
public:

	/** Returns the current position in this stream.
	  *
	  * @return the offset from the beginning of the stream, in bytes,
	  * at which the next read occurs
	  */
	virtual size_type getPosition() const = 0;

	/** Sets the position, measured from the beginning of this stream,
	  * at which the next read occurs.
	  *
	  * @param pos the offset position, measured in bytes from the
	  * beginning of the stream, at which to set the stream pointer.
	  */
	virtual void seek(const size_type pos) = 0;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_SEEKABLEINPUTSTREAM_HPP_INCLUDED

