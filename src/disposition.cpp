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

#include "disposition.hpp"


namespace vmime
{


disposition::disposition()
	: m_name(dispositionTypes::INLINE)
{
}


disposition::disposition(const string& name)
	: m_name(toLower(name))
{
}


disposition::disposition(const disposition& type)
	: component(), m_name(type.m_name)
{
}


void disposition::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_name = toLower(string(buffer.begin() + position, buffer.begin() + end));

	if (newPosition)
		*newPosition = end;
}


void disposition::generate(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << m_name;

	if (newLinePos)
		*newLinePos = curLinePos + m_name.length();
}


disposition& disposition::operator=(const disposition& source)
{
	m_name = source.m_name;
	return (*this);
}


disposition& disposition::operator=(const string& name)
{
	m_name = toLower(name);
	return (*this);
}


const bool disposition::operator==(const disposition& value) const
{
	return (toLower(m_name) == value.m_name);
}


const bool disposition::operator!=(const disposition& value) const
{
	return !(*this == value);
}


} // vmime
