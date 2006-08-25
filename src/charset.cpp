//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2006 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/charset.hpp"
#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"

#include "vmime/utility/stringUtils.hpp"

#include "vmime/charsetConverter.hpp"



namespace vmime
{


charset::charset()
	: m_name(charsets::US_ASCII)
{
}


charset::charset(const string& name)
	: m_name(name)
{
}


charset::charset(const char* name)
	: m_name(name)
{
}


void charset::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_name = utility::stringUtils::trim
		(string(buffer.begin() + position, buffer.begin() + end));

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void charset::generate(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << m_name;

	if (newLinePos)
		*newLinePos = curLinePos + m_name.length();
}


void charset::convert(utility::inputStream& in, utility::outputStream& out,
	const charset& source, const charset& dest)
{
	charsetConverter conv(source, dest);
	conv.convert(in, out);
}


void charset::convert(const string& in, string& out, const charset& source, const charset& dest)
{
	charsetConverter conv(source, dest);
	conv.convert(in, out);
}


const charset charset::getLocaleCharset()
{
	return (platformDependant::getHandler()->getLocaleCharset());
}


charset& charset::operator=(const charset& other)
{
	copyFrom(other);
	return (*this);
}


const bool charset::operator==(const charset& value) const
{
	return (utility::stringUtils::isStringEqualNoCase(m_name, value.m_name));
}


const bool charset::operator!=(const charset& value) const
{
	return !(*this == value);
}


ref <component> charset::clone() const
{
	return vmime::create <charset>(m_name);
}


const string& charset::getName() const
{
	return (m_name);
}


void charset::copyFrom(const component& other)
{
	m_name = dynamic_cast <const charset&>(other).m_name;
}


const std::vector <ref <const component> > charset::getChildComponents() const
{
	return std::vector <ref <const component> >();
}


} // vmime
