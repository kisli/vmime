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

#include "vmime/utility/stringUtils.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime {
namespace utility {


bool stringUtils::isStringEqualNoCase
	(const string& s1, const char* s2, const size_t n)
{
	// 'n' is the number of characters to compare
	// 's2' must be in lowercase letters only
	if (s1.length() < n)
		return (false);

	const std::ctype <char>& fac =
		std::use_facet <std::ctype <char> >(std::locale::classic());

	bool equal = true;

	for (size_t i = 0 ; equal && i < n ; ++i)
		equal = (fac.tolower(static_cast <unsigned char>(s1[i])) == s2[i]);

	return (equal);
}


bool stringUtils::isStringEqualNoCase(const string& s1, const string& s2)
{
	if (s1.length() != s2.length())
		return (false);

	const std::ctype <char>& fac =
		std::use_facet <std::ctype <char> >(std::locale::classic());

	bool equal = true;
	const string::const_iterator end = s1.end();

	for (string::const_iterator i = s1.begin(), j = s2.begin(); i != end ; ++i, ++j)
		equal = (fac.tolower(static_cast <unsigned char>(*i)) == fac.tolower(static_cast <unsigned char>(*j)));

	return (equal);
}


bool stringUtils::isStringEqualNoCase
	(const string::const_iterator begin, const string::const_iterator end,
	 const char* s, const size_t n)
{
	if (static_cast <size_t>(end - begin) < n)
		return (false);

	const std::ctype <char>& fac =
		std::use_facet <std::ctype <char> >(std::locale::classic());

	bool equal = true;
	char* c = const_cast<char*>(s);
	size_t r = n;

	for (string::const_iterator i = begin ; equal && r && *c ; ++i, ++c, --r)
		equal = (fac.tolower(static_cast <unsigned char>(*i)) == static_cast <unsigned char>(*c));

	return (r == 0 && equal);
}


const string stringUtils::toLower(const string& str)
{
	const std::ctype <char>& fac =
		std::use_facet <std::ctype <char> >(std::locale::classic());

	string out;
	out.resize(str.size());

	for (size_t i = 0, len = str.length() ; i < len ; ++i)
		out[i] = fac.tolower(static_cast <unsigned char>(str[i]));

	return out;
}


const string stringUtils::toUpper(const string& str)
{
	const std::ctype <char>& fac =
		std::use_facet <std::ctype <char> >(std::locale::classic());

	string out;
	out.resize(str.size());

	for (size_t i = 0, len = str.length() ; i < len ; ++i)
		out[i] = fac.toupper(static_cast <unsigned char>(str[i]));

	return out;
}


const string stringUtils::trim(const string& str)
{
	string::const_iterator b = str.begin();
	string::const_iterator e = str.end();

	if (b != e)
	{
		for ( ; b != e && parserHelpers::isSpace(*b) ; ++b) {}
		for ( ; e != b && parserHelpers::isSpace(*(e - 1)) ; --e) {}
	}

	return (string(b, e));
}


size_t stringUtils::countASCIIchars
	(const string::const_iterator begin, const string::const_iterator end)
{
	size_t count = 0;

	for (string::const_iterator i = begin ; i != end ; ++i)
	{
		if (parserHelpers::isAscii(*i))
		{
			if (*i != '=' || ((i + 1) != end && *(i + 1) != '?')) // To avoid bad behaviour...
				++count;
		}
	}

	return (count);
}


bool stringUtils::is7bit(const string& str)
{
	return countASCIIchars(str.begin(), str.end()) == str.length();
}


size_t stringUtils::findFirstNonASCIIchar
	(const string::const_iterator begin, const string::const_iterator end)
{
	size_t pos = string::npos;

	for (string::const_iterator i = begin ; i != end ; ++i)
	{
		if (!parserHelpers::isAscii(*i))
		{
			pos = i - begin;
			break;
		}
	}

	return pos;
}


const string stringUtils::unquote(const string& str)
{
	if (str.length() < 2)
		return str;

	if (str[0] != '"' || str[str.length() - 1] != '"')
		return str;

	string res;
	res.reserve(str.length());

	bool escaped = false;

	for (string::const_iterator it = str.begin() + 1, end = str.end() - 1 ; it != end ; ++it)
	{
		const char c = *it;

		if (escaped)
		{
			res += c;
			escaped = false;
		}
		else if (!escaped && c == '\\')
		{
			escaped = true;
		}
		else
		{
			res += c;
		}
	}

	return res;
}


bool stringUtils::needQuoting(const string& str, const string& specialChars)
{
	return str.find_first_of(specialChars.c_str()) != string::npos;
}


string stringUtils::quote
	(const string& str, const string& escapeSpecialChars, const string& escapeChar)
{
	std::ostringstream oss;
	size_t lastPos = 0, pos = 0;

	while ((pos = str.find_first_of(escapeSpecialChars, lastPos)) != string::npos)
	{
		oss << str.substr(lastPos, pos - lastPos)
		    << escapeChar
		    << str[pos];

		lastPos = pos + 1;
	}

	oss << str.substr(lastPos);

	return oss.str();
}


} // utility
} // vmime
