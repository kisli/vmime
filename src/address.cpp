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

#include "vmime/address.hpp"

#include "vmime/mailbox.hpp"
#include "vmime/mailboxGroup.hpp"

#include "vmime/parserHelpers.hpp"


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

ref <address> address::parseNext(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	bool escaped = false;
	bool quoted = false;
	bool quotedRFC2047 = false;
	bool inRouteAddr = false;
	bool isGroup = false;
	bool stop = false;

	string::size_type pos = position;

	while (pos < end && parserHelpers::isSpace(buffer[pos]))
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

				if (!quoted && !quotedRFC2047 && pos + 1 < end && buffer[pos + 1] == '?')
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
		ref <address> parsedAddress = isGroup
			? create <mailboxGroup>().dynamicCast <address>()
			: create <mailbox>().dynamicCast <address>();

		parsedAddress->parse(buffer, start, pos, NULL);
		parsedAddress->setParsedBounds(start, pos);

		return (parsedAddress);
	}

	return (NULL);
}


} // vmime
