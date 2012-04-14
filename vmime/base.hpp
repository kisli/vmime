//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_BASE_HPP_INCLUDED
#define VMIME_BASE_HPP_INCLUDED


#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cctype>
#include <locale>

#include "vmime/config.hpp"
#include "vmime/types.hpp"
#include "vmime/constants.hpp"
#include "vmime/utility/smartPtr.hpp"


namespace vmime
{
	class text;
	class word;
	class charset;


	// "Null" strings
	extern const string NULL_STRING;

	extern const text NULL_TEXT;
	extern const word NULL_WORD;


	//
	// Library name and version
	//

	const string libname();
	const string libversion();
	const string libapi();


	//
	// Helpful functions used for array -> iterator conversion
	//

	template <typename T, size_t N>
	inline T const* begin(T const (&array)[N])
	{
		return (array);
	}

	template <typename T, size_t N>
	inline T const* end(T const (&array)[N])
	{
		return (array + N);
	}

	template <typename T, size_t N>
	inline size_t count(T const (&array)[N])
	{
		return (N);
	}


	// Copy one vector to another, with type conversion

	template <class T1, class T2>
	void copy_vector(const T1& v1, T2& v2)
	{
		const typename T1::size_type count = v1.size();

		v2.resize(count);

		for (typename T1::size_type i = 0 ; i < count ; ++i)
			v2[i] = v1[i];
	}


	/*

	RFC#2822
	2.1.1. Line Length Limits

	There are two limits that this standard places on the number of
	characters in a line. Each line of characters MUST be no more than
	998 characters, and SHOULD be no more than 78 characters, excluding
	the CRLF.

	The 998 character limit is due to limitations in many implementations
	which send, receive, or store Internet Message Format messages that
	simply cannot handle more than 998 characters on a line. Receiving
	implementations would do well to handle an arbitrarily large number
	of characters in a line for robustness sake. However, there are so
	many implementations which (in compliance with the transport
	requirements of [RFC2821]) do not accept messages containing more
	than 1000 character including the CR and LF per line, it is important
	for implementations not to create such messages.

	The more conservative 78 character recommendation is to accommodate
	the many implementations of user interfaces that display these
	messages which may truncate, or disastrously wrap, the display of
	more than 78 characters per line, in spite of the fact that such
	implementations are non-conformant to the intent of this specification
	(and that of [RFC2821] if they actually cause information to be lost).
	Again, even though this limitation is put on messages, it is encumbant
	upon implementations which display messages to handle an arbitrarily
	large number of characters in a line (certainly at least up to the 998
	character limit) for the sake of robustness.
	*/

	namespace lineLengthLimits
	{
		extern const string::size_type infinite;

		enum
		{
			max = 998,
			convenient = 78
		};
	}


	// New line sequence to be used when folding header fields.
	extern const string NEW_LINE_SEQUENCE;
	extern const string::size_type NEW_LINE_SEQUENCE_LENGTH;


	// CR-LF sequence
	extern const string CRLF;


	// Mime version
	extern const string SUPPORTED_MIME_VERSION;

	/** Utility classes. */
	namespace utility { }


#ifndef VMIME_BUILDING_DOC
	/** Work-around for friend template functions.
	  *
	  * Make this class a friend if you want to be able to use
	  * vmime::create() with private/protected constructors.
	  */
	class creator
	{
	public:

		template <class T>
		static ref <T> create() { return ref <T>::fromPtr(new T); }

		template <class T, class P0>
		static ref <T> create(const P0& p0) { return ref <T>::fromPtr(new T(p0)); }

		template <class T, class P0, class P1>
		static ref <T> create(const P0& p0, const P1& p1) { return ref <T>::fromPtr(new T(p0, p1)); }

		template <class T, class P0, class P1, class P2>
		static ref <T> create(const P0& p0, const P1& p1, const P2& p2) { return ref <T>::fromPtr(new T(p0, p1, p2)); }

		template <class T, class P0, class P1, class P2, class P3>
		static ref <T> create(const P0& p0, const P1& p1, const P2& p2, const P3& p3) { return ref <T>::fromPtr(new T(p0, p1, p2, p3)); }

		template <class T, class P0, class P1, class P2, class P3, class P4>
		static ref <T> create(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4) { return ref <T>::fromPtr(new T(p0, p1, p2, p3, p4)); }
	};
#endif // VMIME_BUILDING_DOC

	/** Create a new object and return a reference to it.
	  * @return reference to the new object
	  */
	template <class T>
	static ref <T> create() { return creator::create <T>(); }

	/** Create a new object and return a reference to it.
	  * @return reference to the new object
	  */
	template <class T, class P0>
	static ref <T> create(const P0& p0) { return creator::create <T, P0>(p0); }

	/** Create a new object and return a reference to it.
	  * @return reference to the new object
	  */
	template <class T, class P0, class P1>
	static ref <T> create(const P0& p0, const P1& p1) { return creator::create <T, P0, P1>(p0, p1); }

	/** Create a new object and return a reference to it.
	  * @return reference to the new object
	  */
	template <class T, class P0, class P1, class P2>
	static ref <T> create(const P0& p0, const P1& p1, const P2& p2) { return creator::create <T, P0, P1, P2>(p0, p1, p2); }

	/** Create a new object and return a reference to it.
	  * @return reference to the new object
	  */
	template <class T, class P0, class P1, class P2, class P3>
	static ref <T> create(const P0& p0, const P1& p1, const P2& p2, const P3& p3) { return creator::create <T, P0, P1, P2, P3>(p0, p1, p2, p3); }

	/** Create a new object and return a reference to it.
	  * @return reference to the new object
	  */
	template <class T, class P0, class P1, class P2, class P3, class P4>
	static ref <T> create(const P0& p0, const P1& p1, const P2& p2, const P3& p3, const P4& p4) { return creator::create <T, P0, P1, P2, P3, P4>(p0, p1, p2, p3, p4); }


	/** Clone helper.
	  * Use "vmime::clone(obj)" instead of "obj->clone().cast <objtype>()".
	  */
	template <class T>
	ref <T> clone(ref <const T> x)
	{
		return x->clone().template dynamicCast <T>();
	}

	/** Clone helper.
	  * Use "vmime::clone(obj)" instead of "obj.clone().cast <objtype>()".
	  */
	template <class T>
	ref <T> clone(const T& x)
	{
		return x.clone().template dynamicCast <T>();
	}


	/** Downcast helper.
	  * Usage: vmime::dynamicCast <DerivedType>(obj), where 'obj' is of
	  * type Type, and DerivedType is derived from Type.
	  */
	template <class X, class Y>
	ref <X> dynamicCast(ref <Y> y)
	{
		return y.dynamicCast <X>();
	}

	/** Inherit from this class to indicate the subclass is not copyable,
	  * ie. you want to prohibit copy construction and copy assignment.
	  */
	class noncopyable
	{
	protected:

		noncopyable() { }
		virtual ~noncopyable() { }

	private:

		noncopyable(const noncopyable&);
		void operator=(const noncopyable&);
	};

} // vmime


#include "vmime/utility/stream.hpp"


#endif // VMIME_BASE_HPP_INCLUDED
