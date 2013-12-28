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

#ifndef VMIME_TYPES_HPP_INCLUDED
#define VMIME_TYPES_HPP_INCLUDED


#include <limits>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <utility>

#include "vmime/config.hpp"


#ifndef VMIME_BUILDING_DOC

#if VMIME_SHARED_PTR_USE_CXX
	// If we are compiling with C++11, use shared_ptr<> from the standard lib
	#include <memory>

	#define VMIME_SHARED_PTR_NAMESPACE std
#elif VMIME_SHARED_PTR_USE_BOOST
	// Else, use boost's shared_ptr<>
	#include <boost/shared_ptr.hpp>
	#include <boost/weak_ptr.hpp>
	#include <boost/make_shared.hpp>
	#include <boost/enable_shared_from_this.hpp>
	#include <boost/shared_ptr.hpp>

	#define VMIME_SHARED_PTR_NAMESPACE boost
#else
	#error Either VMIME_SHAREDPTR_USE_CXX or VMIME_SHAREDPTR_USE_BOOST must be set to ON
#endif

namespace vmime
{
	using VMIME_SHARED_PTR_NAMESPACE::shared_ptr;
	using VMIME_SHARED_PTR_NAMESPACE::weak_ptr;
	using VMIME_SHARED_PTR_NAMESPACE::make_shared;
	using VMIME_SHARED_PTR_NAMESPACE::enable_shared_from_this;
	using VMIME_SHARED_PTR_NAMESPACE::dynamic_pointer_cast;
	using VMIME_SHARED_PTR_NAMESPACE::const_pointer_cast;

	/** Custom deleter to be used with shared_ptr.
	  * This is does not actually delete the pointer, and is used
	  * only for the singleton classes allocated on the stack.
	  */
	template <typename T>
	struct noop_shared_ptr_deleter
	{
		void operator()(T*) const {}
	};
}

#undef VMIME_SHARED_PTR_NAMESPACE

#endif // VMIME_BUILDING_DOC


namespace vmime
{
	typedef std::string string;

	typedef unsigned short port_t;

	typedef int char_t;

	typedef vmime_uint8 byte_t;
	typedef std::vector <byte_t> byteArray;

	typedef std::size_t size_t;

	// For compatibility with versions <= 0.7.1 (deprecated)
	namespace net { }
	namespace messaging = net;

	// For (minimal) compatibility with legacy smart pointers (<= 0.9.1)
	// Your compiler must have support for C++11
#if VMIME_COMPAT_LEGACY_SMART_POINTERS
	template <typename T> using ref = shared_ptr <T>;
	class creator {}; // unused
	template <typename T, typename... Args>
	inline shared_ptr <T> create(Args&&... args) { return make_shared <T>(args...); }
#endif

}


#include "vmime/object.hpp"


#endif // VMIME_TYPES_HPP_INCLUDED
