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

#include "messageIdField.hpp"


namespace vmime
{


messageIdField::messageIdField()
{
}


void messageIdField::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_id.parse(buffer, position, end, newPosition);
}


void messageIdField::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	headerField::generate(os, maxLineLength, pos, &pos);

 	m_id.generate(os, maxLineLength, pos, newLinePos);
}


messageIdField& messageIdField::operator=(const messageId& mid)
{
	m_id = mid;
	return (*this);
}


void messageIdField::copyFrom(const headerField& field)
{
	const messageIdField& source = dynamic_cast<const messageIdField&>(field);
	m_id = source.m_id;

	headerField::copyFrom(field);
}


} // vmime
