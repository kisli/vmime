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

#include "vmime/mailboxField.hpp"
#include "vmime/mailboxGroup.hpp"


namespace vmime
{


mailboxField::mailboxField()
{
}


mailboxField::mailboxField(const mailboxField&)
	: headerField(), genericField <mailbox>()
{
}


void mailboxField::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	getValue().clear();

	// Here, we cannot simply call "m_mailbox.parse()" because it
	// may have more than one address specified (even if this field
	// should contain only one). We are never too much careful...
	address* parsedAddress = address::parseNext(buffer, position, end, newPosition);

	if (parsedAddress)
	{
		if (parsedAddress->isGroup())
		{
			// If it is a group of mailboxes, take the first
			// mailbox of the group
			mailboxGroup* group = static_cast <mailboxGroup*>(parsedAddress);

			if (!group->isEmpty())
				getValue() = *(group->getMailboxAt(0));
		}
		else
		{
			// Parse only if it is a mailbox
			getValue() = *static_cast <mailbox*>(parsedAddress);
		}
	}

	delete (parsedAddress);

	getValue().setParsedBounds(position, end);

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


} // vmime
