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

#include "defaultField.hpp"
#include "text.hpp"


namespace vmime
{


defaultField::defaultField()
{
}


void defaultField::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_text = string(buffer.begin() + position, buffer.begin() + end);

	if (newPosition)
		*newPosition = end;
}


void defaultField::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	headerField::generate(os, maxLineLength, pos, &pos);

	encodeAndFoldText(os, vmime::text(word(m_text, charset())), maxLineLength,
		pos, newLinePos, encodeAndFoldFlags::forceNoEncoding);
}


defaultField& defaultField::operator=(const string& text)
{
	m_text = text;
	return (*this);
}


void defaultField::copyFrom(const headerField& field)
{
	const defaultField& source = dynamic_cast<const defaultField&>(field);
	m_text = source.m_text;

	headerField::copyFrom(field);
}


} // vmime
