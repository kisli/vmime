//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef VMIME_UTILITY_STRINGPROXY_HPP_INCLUDED
#define VMIME_UTILITY_STRINGPROXY_HPP_INCLUDED


#include <limits>

#include "vmime/types.hpp"
#include "vmime/utility/stream.hpp"


namespace vmime {
namespace utility {


/** This class is a proxy for the string class. This takes
  * advantage of the COW (copy-on-write) system that might
  * be used in "std::string" implementation.
  */

class stringProxy
{
public:

	typedef string::size_type size_type;
	typedef string string_type;


	// Consruction
	stringProxy();
	stringProxy(const stringProxy& s);
	stringProxy(const string_type& s, const size_type start = 0, const size_type end = std::numeric_limits <size_type>::max());

	// Assignment
	void set(const string_type& s, const size_type start = 0, const size_type end = std::numeric_limits <size_type>::max());
	void detach();

	stringProxy& operator=(const stringProxy& s);
	stringProxy& operator=(const string_type& s);

	// Extract some portion (or whole) of the string
	// and output it into a stream.
	void extract(outputStream& os, const size_type start = 0, const size_type end = std::numeric_limits <size_type>::max()) const;

	// Return the "virtual" length of the string
	const size_type length() const;

	// Return the boundaries of the "virtual" string
	const size_type start() const;
	const size_type end() const;

	string::const_iterator it_begin() const { return (m_buffer.begin() + m_start); }
	string::const_iterator it_end() const { return (m_buffer.begin() + m_end); }

private:

	string_type m_buffer;

	size_type m_start;
	size_type m_end;
};


std::ostream& operator<<(std::ostream& os, const stringProxy& s);
outputStream& operator<<(outputStream& os, const stringProxy& s);


} // utility
} // vmime


#endif // VMIME_UTILITY_STRINGPROXY_HPP_INCLUDED
