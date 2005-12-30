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
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/utility/stringUtils.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime {
namespace utility {


const bool stringUtils::isStringEqualNoCase
	(const string& s1, const char* s2, const string::size_type n)
{
	// 'n' is the number of characters to compare
	// 's2' must be in lowercase letters only
	if (s1.length() < n)
		return (false);

	bool equal = true;

	for (string::size_type i = 0 ; equal && i < n ; ++i)
		equal = (std::tolower(s1[i], std::locale()) == s2[i]);

	return (equal);
}


const bool stringUtils::isStringEqualNoCase(const string& s1, const string& s2)
{
	if (s1.length() != s2.length())
		return (false);

	bool equal = true;
	const string::const_iterator end = s1.end();

	for (string::const_iterator i = s1.begin(), j = s2.begin(); i != end ; ++i, ++j)
		equal = (std::tolower(*i, std::locale()) == std::tolower(*j, std::locale()));

	return (equal);
}


const bool stringUtils::isStringEqualNoCase
	(const string::const_iterator begin, const string::const_iterator end,
	 const char* s, const string::size_type n)
{
	if (static_cast <string::size_type>(end - begin) < n)
		return (false);

	bool equal = true;
	char* c = const_cast<char*>(s);
	string::size_type r = n;

	for (string::const_iterator i = begin ; equal && r && *c ; ++i, ++c, --r)
		equal = (std::tolower(*i, std::locale()) == *c);

	return (r == 0 && equal);
}


const string stringUtils::toLower(const string& str)
{
	string out;
	out.resize(str.size());

	for (string::size_type i = 0, len = str.length() ; i < len ; ++i)
		out[i] = std::tolower(str[i], std::locale());

	return out;
}


const string stringUtils::toUpper(const string& str)
{
	string out;
	out.resize(str.size());

	for (string::size_type i = 0, len = str.length() ; i < len ; ++i)
		out[i] = std::toupper(str[i], std::locale());

	return out;
}


const string stringUtils::trim(const string& str)
{
	string::const_iterator b = str.begin();
	string::const_iterator e = str.end();

	if (b != e)
	{
		for ( ; b != e && parserHelpers::isSpace(*b) ; ++b);
		for ( ; e != b && parserHelpers::isSpace(*(e - 1)) ; --e);
	}

	return (string(b, e));
}


const string::size_type stringUtils::countASCIIchars
	(const string::const_iterator begin, const string::const_iterator end)
{
	string::size_type count = 0;

	for (string::const_iterator i = begin ; i != end ; ++i)
	{
		if (parserHelpers::isAscii(*i))
		{
			if (*i != '=' || *(i + 1) != '?') // To avoid bad behaviour...
				++count;
		}
	}

	return (count);
}


} // utility
} // vmime
