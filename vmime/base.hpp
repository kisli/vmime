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

#ifndef VMIME_BASE_HPP_INCLUDED
#define VMIME_BASE_HPP_INCLUDED


#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <locale>

#include "vmime/config.hpp"
#include "vmime/types.hpp"
#include "vmime/constants.hpp"
#include "vmime/utility/stream.hpp"


namespace vmime
{
	class text;
	class charset;


	// "Null" strings
	extern const string NULL_STRING;
#if VMIME_WIDE_CHAR_SUPPORT
	extern const wstring NULL_WSTRING;
#endif

	extern const text NULL_TEXT;


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


	// Free the pointer elements in a STL container and empty the container

	template <class CONTAINER>
	void free_container(CONTAINER& c)
	{
		for (typename CONTAINER::iterator it = c.begin() ; it != c.end() ; ++it)
			delete (*it);

		c.clear();
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

} // vmime


#endif // VMIME_BASE_HPP_INCLUDED
