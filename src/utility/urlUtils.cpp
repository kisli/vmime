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

#include "vmime/utility/urlUtils.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime {
namespace utility {


const string urlUtils::encode(const string& s)
{
	static const string RESERVED_CHARS =
		/* reserved */ "$&+,/:;=?@"
		/* unsafe */   "<>#%{}[]|\\^\"~`";

	string result;
	result.reserve(s.length());

	for (string::const_iterator it = s.begin() ; it != s.end() ; ++it)
	{
		const char c = *it;

		if (parserHelpers::isPrint(c) && !parserHelpers::isSpace(c) &&
		    static_cast <unsigned char>(c) <= 127 &&
		    RESERVED_CHARS.find(c) == string::npos)
		{
			result += c;
		}
		else
		{
			char hex[4];
			const unsigned char k = static_cast <unsigned char>(c);

			hex[0] = '%';
			hex[1] = "0123456789ABCDEF"[k / 16];
			hex[2] = "0123456789ABCDEF"[k % 16];
			hex[3] = 0;

			result += hex;
		}
	}

	return (result);
}


const string urlUtils::decode(const string& s)
{
	string result;
	result.reserve(s.length());

	for (string::const_iterator it = s.begin() ; it != s.end() ; )
	{
		const char c = *it;

		switch (c)
		{
		case '%':
		{
			++it;  // skip '%'

			const char_t p = (it != s.end() ? *(it++) : 0);
			const char_t q = (it != s.end() ? *(it++) : 0);

			unsigned int r = 0;

			switch (p)
			{
			case 0: r = '%'; break;
			case 'a': case 'A': r = 10; break;
			case 'b': case 'B': r = 11; break;
			case 'c': case 'C': r = 12; break;
			case 'd': case 'D': r = 13; break;
			case 'e': case 'E': r = 14; break;
			case 'f': case 'F': r = 15; break;
			default: r = p - '0'; break;
			}

			if (q != 0)
			{
				r *= 16;

				switch (q)
				{
				case 'a': case 'A': r += 10; break;
				case 'b': case 'B': r += 11; break;
				case 'c': case 'C': r += 12; break;
				case 'd': case 'D': r += 13; break;
				case 'e': case 'E': r += 14; break;
				case 'f': case 'F': r += 15; break;
				default: r += q - '0'; break;
				}
			}

			result += static_cast <char>(r);
			break;
		}
		default:

			result += c;
			++it;
			break;
		}
	}

	return (result);
}


} // utility
} // vmime
