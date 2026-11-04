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

#ifndef VMIME_UTILITY_DETAILS_CLASSICISTRINGSTREAM_HPP_INCLUDED
#define VMIME_UTILITY_DETAILS_CLASSICISTRINGSTREAM_HPP_INCLUDED


#include <locale>
#include <sstream>
#include <string>


namespace vmime {
namespace utility {
namespace details {


/** Input string stream which always uses the classic "C" locale,
  * so that numbers are parsed in the same way whatever the global
  * locale set by the application (eg. without digit grouping).
  *
  * VMime uses it instead of std::istringstream. This is an internal
  * class: it is not part of the public API.
  */
class classicIStringStream : public std::istringstream {

public:

	explicit classicIStringStream(const std::string& str)
		: std::istringstream(str) {

		imbue(std::locale::classic());
	}
};


} // details
} // utility
} // vmime


#endif // VMIME_UTILITY_DETAILS_CLASSICISTRINGSTREAM_HPP_INCLUDED
