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

#ifndef VMIME_UTILITY_STRINGUTILS_HPP_INCLUDED
#define VMIME_UTILITY_STRINGUTILS_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/base.hpp"

#include <sstream>


namespace vmime {
namespace utility {


/** Miscellaneous functions related to strings.
  */

class stringUtils
{
public:

	/** Test two strings for equality (case insensitive).
	  * \warning Use this with ASCII-only strings.
	  *
	  * @param s1 first string
	  * @param s2 second string (must be in lower-case!)
	  * @param n length of the second string
	  * @return true if the two strings compare equally, false otherwise
	  */
	static const bool isStringEqualNoCase(const string& s1, const char* s2, const string::size_type n);

	/** Test two strings for equality (case insensitive).
	  * \warning Use this with ASCII-only strings.
	  *
	  * @param s1 first string
	  * @param s2 second string
	  * @return true if the two strings compare equally, false otherwise
	  */
	static const bool isStringEqualNoCase(const string& s1, const string& s2);

	/** Test two strings for equality (case insensitive).
	  * \warning Use this with ASCII-only strings.
	  *
	  * @param begin start position of the first string
	  * @param end end position of the first string
	  * @param s second string (must be in lower-case!)
	  * @param n length of the second string
	  * @return true if the two strings compare equally, false otherwise
	  */
	static const bool isStringEqualNoCase(const string::const_iterator begin, const string::const_iterator end, const char* s, const string::size_type n);

	/** Transform all the characters in a string to lower-case.
	  * \warning Use this with ASCII-only strings.
	  *
	  * @param str the string to transform
	  * @return a new string in lower-case
	  */
	static const string toLower(const string& str);

	/** Strip the space characters (SPC, TAB, CR, LF) at the beginning
	  * and at the end of the specified string.
	  *
	  * @param str string in which to strip spaces
	  * @return a new string with space characters removed
	  */
	static const string trim(const string& str);

	/** Return the number of 7-bit US-ASCII characters in a string.
	  *
	  * @param begin start position
	  * @param end end position
	  * @return number of ASCII characters
	  */
	static const string::size_type countASCIIchars(const string::const_iterator begin, const string::const_iterator end);

	/** Convert the specified value to a string value.
	  *
	  * @param value to convert
	  * @return value converted from type 'TYPE'
	  */
	template <class TYPE>
	static const string toString(const TYPE& value)
	{
		std::ostringstream oss;
		oss << value;

		return (oss.str());
	}

	/** Convert the specified string value to a value of
	  * the specified type.
	  *
	  * @param value value to convert
	  * @return value converted into type 'TYPE'
	  */
	template <class TYPE>
	static const TYPE fromString(const string& value)
	{
		TYPE ret;

		std::istringstream iss(value);
		iss >> ret;

		return (ret);
	}
};


} // utility
} // vmime


#endif // VMIME_UTILITY_STRINGUTILS_HPP_INCLUDED
