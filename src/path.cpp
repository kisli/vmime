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

#include "vmime/path.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime
{


path::path()
{
}


path::path(const string& localPart, const string& domain)
	: m_localPart(localPart), m_domain(domain)
{
}


path::path(const path& p)
	: headerFieldValue(), m_localPart(p.m_localPart), m_domain(p.m_domain)
{
}


const string& path::getLocalPart() const
{
	return (m_localPart);
}


void path::setLocalPart(const string& localPart)
{
	m_localPart = localPart;
}


const string& path::getDomain() const
{
	return (m_domain);
}


void path::setDomain(const string& domain)
{
	m_domain = domain;
}


bool path::operator==(const path& p) const
{
	return (m_localPart == p.m_localPart &&
	        m_domain == p.m_domain);
}


bool path::operator!=(const path& p) const
{
	return (m_localPart != p.m_localPart ||
	        m_domain != p.m_domain);
}


void path::copyFrom(const component& other)
{
	const path& p = dynamic_cast <const path&>(other);

	m_localPart = p.m_localPart;
	m_domain = p.m_domain;
}


ref <component> path::clone() const
{
	return vmime::create <path>(*this);
}


path& path::operator=(const path& other)
{
	copyFrom(other);
	return (*this);
}


const std::vector <ref <component> > path::getChildComponents()
{
	return std::vector <ref <component> >();
}


void path::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	string::size_type pos = position;

	while (pos < end && parserHelpers::isSpace(buffer[pos]))
		++pos;

	string addrSpec;

	if (pos < end && buffer[pos] == '<')
	{
		// Skip '<'
		++pos;

		while (pos < end && parserHelpers::isSpace(buffer[pos]))
			++pos;

		const string::size_type addrStart = pos;

		while (pos < end && buffer[pos] != '>')
			++pos;

		string::size_type addrEnd = pos;

		while (addrEnd > addrStart && parserHelpers::isSpace(buffer[addrEnd - 1]))
			addrEnd--;

		addrSpec = string(buffer.begin() + addrStart, buffer.begin() + addrEnd);
	}
	else
	{
		addrSpec = string(buffer.begin() + position, buffer.begin() + end);
	}

	const string::size_type at = addrSpec.find_first_of('@');

	if (at != string::npos)
	{
		m_localPart = string(addrSpec.begin(), addrSpec.begin() + at);
		m_domain = string(addrSpec.begin() + at + 1, addrSpec.end());
	}
	else
	{
		m_localPart.clear();
		m_domain = addrSpec;
	}

	if (newPosition != NULL)
		*newPosition = end;
}


void path::generateImpl(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	if (m_localPart.empty() && m_domain.empty())
	{
		os << "<>";

		if (newLinePos)
			*newLinePos = curLinePos + 2;
	}
	else
	{
		os << "<" << m_localPart << "@" << m_domain << ">";

		if (newLinePos)
			*newLinePos = curLinePos + m_localPart.length() + m_domain.length() + 3;
	}
}


} // vmime
