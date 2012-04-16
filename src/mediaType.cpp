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

#include "vmime/mediaType.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime
{


mediaType::mediaType()
	: m_type(mediaTypes::APPLICATION), m_subType(mediaTypes::APPLICATION_OCTET_STREAM)
{
}


mediaType::mediaType(const string& type)
{
	parse(type);
}


mediaType::mediaType(const string& type, const string& subType)
	: m_type(utility::stringUtils::toLower(type)),
	  m_subType(utility::stringUtils::toLower(subType))
{
}


void mediaType::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* const pstart = buffer.data() + position;
	const string::value_type* p = pstart;

	// Extract the type
	const string::size_type typeStart = position;

	while (p < pend && *p != '/') ++p;

	m_type = utility::stringUtils::trim(utility::stringUtils::toLower(
		string(buffer.begin() + typeStart,
	            buffer.begin() + position + (p - pstart))));

	if (p < pend)
	{
		// Skip '/' character
		++p;

		// Extract the sub-type
		m_subType = utility::stringUtils::trim(utility::stringUtils::toLower(
			string(buffer.begin() + position + (p - pstart),
		            buffer.begin() + end)));
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void mediaType::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	const string value = m_type + "/" + m_subType;

	if (curLinePos + value.length() > maxLineLength)
	{
		os << NEW_LINE_SEQUENCE;
		os << value;

		if (newLinePos)
			*newLinePos = NEW_LINE_SEQUENCE_LENGTH + value.length();
	}
	else
	{
		os << value;

		if (newLinePos)
			*newLinePos = curLinePos + value.length();
	}
}


bool mediaType::operator==(const mediaType& type) const
{
	return (m_type == type.m_type && m_subType == type.m_subType);
}


bool mediaType::operator!=(const mediaType& type) const
{
	return !(*this == type);
}


mediaType& mediaType::operator=(const string& type)
{
	parse(type);
	return (*this);
}


ref <component> mediaType::clone() const
{
	return vmime::create <mediaType>(m_type, m_subType);
}


void mediaType::copyFrom(const component& other)
{
	const mediaType& mt = dynamic_cast <const mediaType&>(other);

	m_type = mt.m_type;
	m_subType = mt.m_subType;
}


mediaType& mediaType::operator=(const mediaType& other)
{
	copyFrom(other);
	return (*this);
}


const string& mediaType::getType() const
{
	return (m_type);
}


void mediaType::setType(const string& type)
{
	m_type = utility::stringUtils::toLower(type);
}


const string& mediaType::getSubType() const
{
	return (m_subType);
}


void mediaType::setSubType(const string& subType)
{
	m_subType = utility::stringUtils::toLower(subType);
}


void mediaType::setFromString(const string& type)
{
	parse(type);
}


const std::vector <ref <component> > mediaType::getChildComponents()
{
	return std::vector <ref <component> >();
}


} // vmime
