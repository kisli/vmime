//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

class VMIME_EXPORT outputStream : public stream
{
public:

	/** Write data to the stream.
	  *
	  * @param data buffer containing data to write
	  * @param count number of bytes to write
	  */
	void write(const byte_t* const data, const size_t count);

	/** Write data to the stream.
	  *
	  * @param data buffer containing data to write
	  * @param count number of bytes to write
	  */
	void write(const char* const data, const size_t count);

	/** Write data to the stream.
	  *
	  * @param data buffer containing data to write
	  * @param N number of bytes to write, including terminating
	  * null (value is induced by compiler)
	  */
	template <int N>
	void write(const char (&data)[N])
	{
		write(data, N - 1);
	}

	/** Flush this output stream and forces any buffered output
	  * bytes to be written out to the stream.
	  */
	virtual void flush() = 0;

protected:

	/** Write data to the stream.
	  * This is the method to be implemented is subclasses.
	  *
	  * @param data buffer containing data to write
	  * @param count number of bytes to write
	  */
	virtual void writeImpl(const byte_t* const data, const size_t count) = 0;
};


// Helpers functions

VMIME_EXPORT outputStream& operator<<(outputStream& os, const string& str);
VMIME_EXPORT outputStream& operator<<(outputStream& os, const byte_t c);


#if defined(_MSC_VER) && (_MSC_VER <= 1200)  // Internal compiler error with VC++6

inline outputStream& operator<<(outputStream& os, const char* str)
{
	os.write(reinterpret_cast <const byte_t*>(str), ::strlen(str));
	return (os);
}

#else

template <int N>
outputStream& operator<<(outputStream& os, const char (&str)[N])
{
	os.write(reinterpret_cast <const byte_t*>(str), N - 1);
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

