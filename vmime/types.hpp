//
// VMime library (http://www.vmime.org)
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

#ifndef VMIME_TYPES_HPP_INCLUDED
#define VMIME_TYPES_HPP_INCLUDED


#include <limits>
#include <string>

#include "vmime/config.hpp"
#include "vmime/utility/smartPtr.hpp"


namespace vmime
{
	typedef std::string string;
#if VMIME_WIDE_CHAR_SUPPORT
	typedef std::wstring wstring;
#endif

	typedef unsigned short port_t;

	typedef int char_t;

	typedef vmime_uint8 byte;

	// Some aliases
	namespace utils = utility;

	using vmime::utility::ref;
	using vmime::utility::weak_ref;
	using vmime::utility::null_ref;

	extern const null_ref null;

	// For compatibility with versions <= 0.7.1 (deprecated)
	namespace net { }
	namespace messaging = net;
}


// This is here because 'vmime::ref' need to be declared...
#include "vmime/object.hpp"


#endif // VMIME_TYPES_HPP_INCLUDED
