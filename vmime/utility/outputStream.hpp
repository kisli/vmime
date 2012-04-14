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

#ifndef VMIME_UTILITY_OUTPUTSTREAM_HPP_INCLUDED
#define VMIME_UTILITY_OUTPUTSTREAM_HPP_INCLUDED


#include "vmime/utility/stream.hpp"


#if defined(_MSC_VER) && (_MSC_VER <= 1200)  // VC++6
#	include <cstring>
#endif


namespace vmime {
namespace utility {


/** Simple output stream.
  */

class outputStream : public stream
{
public:

	/** Write data to the stream.
	  *
	  * @param data buffer containing data to write
	  * @param count number of bytes to write
	  */
	virtual void write(const value_type* const data, const size_type count) = 0;

	/** Flush this output stream and forces any buffered output
	  * bytes to be written out to the stream.
	  */
	virtual void flush() = 0;
};


// Helpers functions

outputStream& operator<<(outputStream& os, const string& str);
outputStream& operator<<(outputStream& os, const stream::value_type c);


#if defined(_MSC_VER) && (_MSC_VER <= 1200)  // Internal compiler error with VC++6

inline outputStream& operator<<(outputStream& os, const char* str)
{
	os.write(str, ::strlen(str));
	return (os);
}

#else

template <int N>
outputStream& operator<<(outputStream& os, const char (&str)[N])
{
	os.write(str, N - 1);
	return (os);
}

#endif // defined(_MSC_VER) && (_MSC_VER <= 1200)


template <typename T>
outputStream& operator<<(outputStream& os, const T& t)
{
	std::ostringstream oss;
	oss.imbue(std::locale::classic());  // no formatting

	oss << t;

	os << oss.str();

	return (os);
}



} // utility
} // vmime


#endif // VMIME_UTILITY_OUTPUTSTREAM_HPP_INCLUDED

