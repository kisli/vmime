//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_UTILITY_PREFIXEDINPUTSTREAM_HPP_INCLUDED
#define VMIME_UTILITY_PREFIXEDINPUTSTREAM_HPP_INCLUDED


#include "vmime/utility/inputStream.hpp"


namespace vmime {
namespace utility {


/** An input stream which reads the specified data (prefix) first,
  * then data from another input stream.
  *
  * As long as the prefix has not been read completely, read() and skip()
  * only return data from the prefix, so that the underlying stream is not
  * accessed (and no prefix data can be lost if it throws an exception).
  */
class VMIME_EXPORT prefixedInputStream : public inputStream {

public:

	/** Creates a new prefixed input stream.
	  *
	  * @param prefix data to read before data from the underlying stream
	  * @param is underlying stream (it must remain valid as long as
	  * this object is used)
	  */
	prefixedInputStream(const string& prefix, inputStream& is);

	bool eof() const;
	void reset();
	size_t read(byte_t* const data, const size_t count);
	size_t skip(const size_t count);

	size_t getBlockSize();

private:

	prefixedInputStream(const prefixedInputStream&);

	const string m_prefix;
	size_t m_pos;

	inputStream& m_stream;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_PREFIXEDINPUTSTREAM_HPP_INCLUDED
