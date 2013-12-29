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

#ifndef VMIME_UTILITY_STRINGPROXY_HPP_INCLUDED
#define VMIME_UTILITY_STRINGPROXY_HPP_INCLUDED


#include <limits>

#include "vmime/types.hpp"
#include "vmime/utility/stream.hpp"
#include "vmime/utility/outputStream.hpp"
#include "vmime/utility/progressListener.hpp"


namespace vmime {
namespace utility {


/** This class is a proxy for the string class. This takes
  * advantage of the COW (copy-on-write) system that might
  * be used in "std::string" implementation.
  */

class VMIME_EXPORT stringProxy
{
public:

	// Consruction
	stringProxy();
	stringProxy(const stringProxy& s);
	stringProxy(const string& s, const size_t start = 0, const size_t end = std::numeric_limits <size_t>::max());

	// Assignment
	void set(const string& s, const size_t start = 0, const size_t end = std::numeric_limits <size_t>::max());
	void detach();

	stringProxy& operator=(const stringProxy& s);
	stringProxy& operator=(const string& s);

	// Extract some portion (or whole) of the string
	// and output it into a stream.
	void extract(outputStream& os, const size_t start = 0, const size_t end = std::numeric_limits <size_t>::max(), utility::progressListener* progress = NULL) const;

	// Return the "virtual" length of the string
	size_t length() const;

	// Return the boundaries of the "virtual" string
	size_t start() const;
	size_t end() const;

	string::const_iterator it_begin() const { return (m_buffer.begin() + m_start); }
	string::const_iterator it_end() const { return (m_buffer.begin() + m_end); }

private:

	string m_buffer;

	size_t m_start;
	size_t m_end;
};


VMIME_EXPORT std::ostream& operator<<(std::ostream& os, const stringProxy& s);
VMIME_EXPORT outputStream& operator<<(outputStream& os, const stringProxy& s);


} // utility
} // vmime


#endif // VMIME_UTILITY_STRINGPROXY_HPP_INCLUDED
