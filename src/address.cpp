//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "address.hpp"

#include "mailbox.hpp"
#include "mailboxGroup.hpp"

#include "parserHelpers.hpp"


namespace vmime
{


address::address()
{
}


/*

 RFC #2822:
 3.4.  ADDRESS SPECIFICATION

   Addresses occur in several message header fields to indicate senders
   and recipients of messages.  An address may either be an individual
   mailbox, or a group of mailboxes.

address         =       mailbox / group

mailbox         =       name-addr / addr-spec

name-addr       =       [display-name] angle-addr

angle-addr      =       [CFWS] "<" addr-spec ">" [CFWS] / obs-angle-addr

group           =       display-name ":" [mailbox-list / CFWS] ";"
                        [CFWS]

display-name    =       phrase

mailbox-list    =       (mailbox *("," mailbox)) / obs-mbox-list

address-list    =       (address *("," address)) / obs-addr-list

*/

address* address::parseNext(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	bool escaped = false;
	bool quoted = false;
	bool quotedRFC2047 = false;
	bool inRouteAddr = false;
	bool isGroup = false;
	bool stop = false;

	string::size_type pos = position;

	while (pos < end && isspace(buffer[pos]))
		++pos;

	const string::size_type start = pos;

	while (!stop && pos < end)
	{
		if (escaped)
		{
			escaped = false;
		}
		else
		{
			switch (buffer[pos])
			{
			case '\\':
				escaped = true;
				break;
			case '"':
				quoted = !quoted;
				break;
			case '<':
				inRouteAddr = true;
				break;
			case '>':
				inRouteAddr = false;
				break;
			case '=':

				if (pos + 1 < end && buffer[pos + 1] == '?')
				{
					++pos;
					quotedRFC2047 = true;
				}

				break;

			case '?':

				if (quotedRFC2047 && pos + 1 < end && buffer[pos + 1] == '=')
				{
					++pos;
					quotedRFC2047 = false;
				}

				break;

			default:
			{
				if (!quoted && !quotedRFC2047 && !inRouteAddr)
				{
					switch (buffer[pos])
					{
					case ';':

						if (isGroup)
						{
							if (pos + 1 < end && buffer[pos + 1] == ',')
								++pos;
						}

						stop = true;
						break;

					case ':':

						isGroup = true;
						break;

					case ',':

						if (!isGroup) stop = true;
						break;
					}
				}

				break;
			}

			}
		}

		if (!stop)
			++pos;
	}

	if (newPosition)
	{
		if (pos == end)
			*newPosition = end;
		else
			*newPosition = pos + 1;  // ',' or ';'
	}

	// Parse extracted address (mailbox or group)
	if (pos != start)
	{
		address* parsedAddress = isGroup
			? static_cast<address*>(new mailboxGroup)
			: static_cast<address*>(new mailbox);

		try
		{
			parsedAddress->parse(buffer, start, pos, NULL);
			return (parsedAddress);
		}
		catch (std::exception&)
		{
			delete (parsedAddress);
			throw;
		}
	}

	return (NULL);
}


address& address::operator=(const address& addr)
{
	copyFrom(addr);
	return (*this);
}


} // vmime
