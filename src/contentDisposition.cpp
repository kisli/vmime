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

#include "vmime/contentDisposition.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


contentDisposition::contentDisposition()
	: m_name(contentDispositionTypes::INLINE)
{
}


contentDisposition::contentDisposition(const string& name)
	: m_name(utility::stringUtils::toLower(name))
{
}


contentDisposition::contentDisposition(const contentDisposition& type)
	: component(), m_name(type.m_name)
{
}


void contentDisposition::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_name = utility::stringUtils::toLower
		(string(buffer.begin() + position, buffer.begin() + end));

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void contentDisposition::generate(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << m_name;

	if (newLinePos)
		*newLinePos = curLinePos + m_name.length();
}


contentDisposition& contentDisposition::operator=(const string& name)
{
	m_name = utility::stringUtils::toLower(name);
	return (*this);
}


const bool contentDisposition::operator==(const contentDisposition& value) const
{
	return (utility::stringUtils::toLower(m_name) == value.m_name);
}


const bool contentDisposition::operator!=(const contentDisposition& value) const
{
	return !(*this == value);
}


contentDisposition* contentDisposition::clone() const
{
	return new contentDisposition(*this);
}


void contentDisposition::copyFrom(const component& other)
{
	const contentDisposition& d = dynamic_cast <const contentDisposition&>(other);

	m_name = d.m_name;
}


contentDisposition& contentDisposition::operator=(const contentDisposition& other)
{
	copyFrom(other);
	return (*this);
}


const string& contentDisposition::getName() const
{
	return (m_name);
}


void contentDisposition::setName(const string& name)
{
	m_name = name;
}


const std::vector <const component*> contentDisposition::getChildComponents() const
{
	return std::vector <const component*>();
}


} // vmime
