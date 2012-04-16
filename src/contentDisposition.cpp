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
	: headerFieldValue(), m_name(type.m_name)
{
}


void contentDisposition::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_name = utility::stringUtils::trim(utility::stringUtils::toLower
		(string(buffer.begin() + position, buffer.begin() + end)));

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void contentDisposition::generateImpl(utility::outputStream& os, const string::size_type /* maxLineLength */,
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


bool contentDisposition::operator==(const contentDisposition& value) const
{
	return (utility::stringUtils::toLower(m_name) == value.m_name);
}


bool contentDisposition::operator!=(const contentDisposition& value) const
{
	return !(*this == value);
}


ref <component> contentDisposition::clone() const
{
	return vmime::create <contentDisposition>(*this);
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


const std::vector <ref <component> > contentDisposition::getChildComponents()
{
	return std::vector <ref <component> >();
}


} // vmime
