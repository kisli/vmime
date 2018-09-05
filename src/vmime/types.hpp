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

#ifndef VMIME_TYPES_HPP_INCLUDED
#define VMIME_TYPES_HPP_INCLUDED


#include <limits>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <utility>
#include <memory>

#include "vmime/config.hpp"


#ifndef VMIME_BUILDING_DOC

namespace vmime {

	using std::shared_ptr;
	using std::weak_ptr;
	using std::make_shared;
	using std::enable_shared_from_this;
	using std::dynamic_pointer_cast;
	using std::const_pointer_cast;

	/** Custom deleter to be used with shared_ptr.
	  * This does not actually delete the pointer, and is used
	  * only for the singleton classes allocated on the stack.
	  */
	template <typename T>
	struct noop_shared_ptr_deleter {
		void operator()(T*) const {}
	};

	template <typename T> using scoped_ptr = std::unique_ptr <T>;
}

#endif // VMIME_BUILDING_DOC


namespace vmime {

	typedef std::string string;

	typedef unsigned short port_t;

	typedef int char_t;

	typedef vmime_uint8 byte_t;
	typedef std::vector <byte_t> byteArray;

	typedef std::size_t size_t;

	// For compatibility with versions <= 0.7.1 (deprecated)
	namespace net { }
	namespace messaging = net;
}


#include "vmime/object.hpp"


#endif // VMIME_TYPES_HPP_INCLUDED
