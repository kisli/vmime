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

#include "vmime/messaging/urlUtils.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime {
namespace messaging {


const string urlUtils::encode(const string& s)
{
	string result;
	result.reserve(s.length());

	for (string::const_iterator it = s.begin() ; it != s.end() ; ++it)
	{
		const char_t c = *it;

		if (parserHelpers::isPrint(c) && c != '%')
		{
			result += c;
		}
		else
		{
			char hex[4];

			hex[0] = '%';
			hex[1] = c / 16;
			hex[2] = c % 16;
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
		const char_t c = *it;

		switch (c)
		{
		case '%':
		{
			const char_t p = (++it != s.end() ? *it : 0);
			const char_t q = (++it != s.end() ? *it : 0);

			result += static_cast <string::value_type>(p * 16 + q);

			if (it != s.end())
				++it;

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


} // messaging
} // vmime
