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

#include "mailboxField.hpp"
#include "mailboxGroup.hpp"


namespace vmime
{


mailboxField::mailboxField()
{
}


void mailboxField::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_mailbox.clear();

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

			if (!group->empty())
				m_mailbox = *(group->begin());
		}
		else
		{
			// Parse only if it is a mailbox
			m_mailbox = *static_cast <mailbox*>(parsedAddress);
		}
	}

	delete (parsedAddress);

	if (newPosition)
		*newPosition = end;
}


void mailboxField::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	headerField::generate(os, maxLineLength, pos, &pos);

	m_mailbox.generate(os, maxLineLength, pos, newLinePos);
}


mailboxField& mailboxField::operator=(const class mailbox& mailbox)
{
	m_mailbox = mailbox;
	return (*this);
}


void mailboxField::copyFrom(const headerField& field)
{
	const mailboxField& source = dynamic_cast<const mailboxField&>(field);
	m_mailbox = source.m_mailbox;

	headerField::copyFrom(field);
}


} // vmime
