//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2007 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/encoding.hpp"
#include "vmime/encoderFactory.hpp"
#include "vmime/contentHandler.hpp"

#include <algorithm>


namespace vmime
{


encoding::encoding()
	: m_name(encodingTypes::SEVEN_BIT)
{
}


encoding::encoding(const string& name)
	: m_name(utility::stringUtils::toLower(name))
{
}


encoding::encoding(const encoding& enc)
	: headerFieldValue(), m_name(enc.m_name)
{
}


void encoding::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_name = utility::stringUtils::toLower(utility::stringUtils::trim
		(utility::stringUtils::unquote(utility::stringUtils::trim
			(string(buffer.begin() + position, buffer.begin() + end)))));

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void encoding::generate(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << m_name;

	if (newLinePos)
		*newLinePos = curLinePos + m_name.length();
}


ref <encoder> encoding::getEncoder() const
{
	return (encoderFactory::getInstance()->create(generate()));
}


encoding& encoding::operator=(const encoding& other)
{
	copyFrom(other);
	return (*this);
}


encoding& encoding::operator=(const string& name)
{
	m_name = utility::stringUtils::toLower(name);
	return (*this);
}


const bool encoding::operator==(const encoding& value) const
{
	return (utility::stringUtils::toLower(m_name) == value.m_name);
}


const bool encoding::operator!=(const encoding& value) const
{
	return !(*this == value);
}


const encoding encoding::decide
	(const string::const_iterator begin, const string::const_iterator end)
{
	const string::difference_type length = end - begin;
	const string::difference_type count = std::count_if
		(begin, end, std::bind2nd(std::less<unsigned char>(), 127));

	// All is in 7-bit US-ASCII --> 7-bit (or Quoted-Printable...)
	if (length == count)
	{
		// Now, we check if there is any line with more than
		// "lineLengthLimits::convenient" characters (7-bit requires that)
		string::const_iterator p = begin;

		const string::size_type maxLen = lineLengthLimits::convenient;
		string::size_type len = 0;

		for ( ; p != end && len <= maxLen ; )
		{
			if (*p == '\n')
			{
				len = 0;
				++p;

				// May or may not need to be encoded, we don't take
				// any risk (avoid problems with SMTP)
				if (p != end && *p == '.')
					len = maxLen + 1;
			}
			else
			{
				++len;
				++p;
			}
		}

		if (len > maxLen)
			return (encoding(encodingTypes::QUOTED_PRINTABLE));
		else
			return (encoding(encodingTypes::SEVEN_BIT));
	}
	// Less than 20% non US-ASCII --> Quoted-Printable
	else if ((length - count) <= length / 5)
	{
		return (encoding(encodingTypes::QUOTED_PRINTABLE));
	}
	// Otherwise --> Base64
	else
	{
		return (encoding(encodingTypes::BASE64));
	}
}


const encoding encoding::decide(ref <const contentHandler> /* data */)
{
	// TODO: a better solution to do that?
	return (encoding(encodingTypes::BASE64));
}


ref <component> encoding::clone() const
{
	return vmime::create <encoding>(*this);
}


void encoding::copyFrom(const component& other)
{
	const encoding& e = dynamic_cast <const encoding&>(other);

	m_name = e.m_name;
}


const string& encoding::getName() const
{
	return (m_name);
}


void encoding::setName(const string& name)
{
	m_name = name;
}


const std::vector <ref <const component> > encoding::getChildComponents() const
{
	return std::vector <ref <const component> >();
}


} // vmime
