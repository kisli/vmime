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

#include "dateField.hpp"


namespace vmime
{


dateField::dateField()
{
}


void dateField::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_datetime.parse(buffer, position, end, newPosition);
}


void dateField::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	headerField::generate(os, maxLineLength, pos, &pos);

	m_datetime.generate(os, maxLineLength, pos, newLinePos);
}


dateField& dateField::operator=(const class datetime& datetime)
{
	m_datetime = datetime;
	return (*this);
}


void dateField::copyFrom(const headerField& field)
{
	const dateField& source = dynamic_cast<const dateField&>(field);
	m_datetime = source.m_datetime;

	headerField::copyFrom(field);
}


} // vmime
