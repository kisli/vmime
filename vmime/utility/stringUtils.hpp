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
	static bool isStringEqualNoCase(const string& s1, const char* s2, const string::size_type n);

	/** Test two strings for equality (case insensitive).
	  * \warning Use this with ASCII-only strings.
	  *
	  * @param s1 first string
	  * @param s2 second string
	  * @return true if the two strings compare equally, false otherwise
	  */
	static bool isStringEqualNoCase(const string& s1, const string& s2);

	/** Test two strings for equality (case insensitive).
	  * \warning Use this with ASCII-only strings.
	  *
	  * @param begin start position of the first string
	  * @param end end position of the first string
	  * @param s second string (must be in lower-case!)
	  * @param n length of the second string
	  * @return true if the two strings compare equally, false otherwise
	  */
	static bool isStringEqualNoCase(const string::const_iterator begin, const string::const_iterator end, const char* s, const string::size_type n);

	/** Transform all the characters in a string to lower-case.
	  * \warning Use this with ASCII-only strings.
	  *
	  * @param str the string to transform
	  * @return a new string in lower-case
	  */
	static const string toLower(const string& str);

	/** Transform all the characters in a string to upper-case.
	  * \warning Use this with ASCII-only strings.
	  *
	  * @param str the string to transform
	  * @return a new string in upper-case
	  */
	static const string toUpper(const string& str);

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
	static string::size_type countASCIIchars(const string::const_iterator begin, const string::const_iterator end);

	/** Returns the position of the first non 7-bit US-ASCII character in a string.
	  *
	  * @param begin start position
	  * @param end end position
	  * @return position since begin, or string::npos
	  */
	static string::size_type findFirstNonASCIIchar(const string::const_iterator begin, const string::const_iterator end);

	/** Convert the specified value to a string value.
	  *
	  * @param value to convert
	  * @return value converted from type 'TYPE'
	  */
	template <class TYPE>
	static const string toString(const TYPE& value)
	{
		std::ostringstream oss;
		oss.imbue(std::locale::classic());

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
		iss.imbue(std::locale::classic());

		iss >> ret;

		return (ret);
	}

	/** Unquote the specified string and transform escaped characters.
	  *
	  * @param string from which to remove quotes
	  * @return unquoted string
	  */
	static const string unquote(const string& str);
};


} // utility
} // vmime


#endif // VMIME_UTILITY_STRINGUTILS_HPP_INCLUDED
