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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/pop3/POP3Utils.hpp"
#include "vmime/net/pop3/POP3Response.hpp"

#include <sstream>


namespace vmime {
namespace net {
namespace pop3 {


// static
void POP3Utils::parseMultiListOrUidlResponse(shared_ptr <POP3Response> response, std::map <int, string>& result)
{
	std::map <int, string> ids;

	for (size_t i = 0, n = response->getLineCount() ; i < n ; ++i)
	{
		string line = response->getLineAt(i);
		string::iterator it = line.begin();

		while (it != line.end() && (*it == ' ' || *it == '\t'))
			++it;

		if (it != line.end())
		{
			int number = 0;

			while (it != line.end() && (*it >= '0' && *it <= '9'))
			{
				number = (number * 10) + (*it - '0');
				++it;
			}

			while (it != line.end() && !(*it == ' ' || *it == '\t')) ++it;
			while (it != line.end() && (*it == ' ' || *it == '\t')) ++it;

			if (it != line.end())
			{
				result.insert(std::map <int, string>::value_type(number, string(it, line.end())));
			}
		}
	}
}



class POP3MessageSetEnumerator : public messageSetEnumerator
{
public:

	void enumerateNumberMessageRange(const vmime::net::numberMessageRange& range)
	{
		for (int i = range.getFirst(), last = range.getLast() ; i <= last ; ++i)
			list.push_back(i);
	}

	void enumerateUIDMessageRange(const vmime::net::UIDMessageRange& /* range */)
	{
		// Not supported
	}

public:

	std::vector <int> list;
};


// static
const std::vector <int> POP3Utils::messageSetToNumberList(const messageSet& msgs)
{
	POP3MessageSetEnumerator en;
	msgs.enumerate(en);

	return en.list;
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

