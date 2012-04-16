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

#include "vmime/encoding.hpp"
#include "vmime/contentHandler.hpp"

#include "vmime/utility/outputStreamStringAdapter.hpp"
#include "vmime/utility/encoder/encoderFactory.hpp"

#include <algorithm>


namespace vmime
{


encoding::encoding()
	: m_name(encodingTypes::SEVEN_BIT),
	  m_usage(USAGE_UNKNOWN)
{
}


encoding::encoding(const string& name)
	: m_name(utility::stringUtils::toLower(name)),
	  m_usage(USAGE_UNKNOWN)
{
}


encoding::encoding(const string& name, const EncodingUsage usage)
	: m_name(utility::stringUtils::toLower(name)),
	  m_usage(usage)
{
}


encoding::encoding(const encoding& enc)
	: headerFieldValue(), m_name(enc.m_name), m_usage(enc.m_usage)
{
}


void encoding::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_usage = USAGE_UNKNOWN;

	m_name = utility::stringUtils::toLower(utility::stringUtils::trim
		(utility::stringUtils::unquote(utility::stringUtils::trim
			(string(buffer.begin() + position, buffer.begin() + end)))));

	if (m_name.empty())
		m_name = encodingTypes::SEVEN_BIT;   // assume default "7-bit"

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void encoding::generateImpl(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << m_name;

	if (newLinePos)
		*newLinePos = curLinePos + m_name.length();
}


ref <utility::encoder::encoder> encoding::getEncoder() const
{
	ref <utility::encoder::encoder> encoder =
		utility::encoder::encoderFactory::getInstance()->create(generate());

	// FIXME: this should not be here (move me into QP encoder instead?)
	if (m_usage == USAGE_TEXT && m_name == encodingTypes::QUOTED_PRINTABLE)
		encoder->getProperties()["text"] = true;

	return encoder;
}


encoding& encoding::operator=(const encoding& other)
{
	copyFrom(other);
	return (*this);
}


encoding& encoding::operator=(const string& name)
{
	m_name = utility::stringUtils::toLower(name);
	m_usage = USAGE_UNKNOWN;
	return (*this);
}


bool encoding::operator==(const encoding& value) const
{
	return (utility::stringUtils::toLower(m_name) == value.m_name);
}


bool encoding::operator!=(const encoding& value) const
{
	return !(*this == value);
}


const encoding encoding::decideImpl
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


const encoding encoding::decide
	(ref <const contentHandler> data, const EncodingUsage usage)
{
	encoding enc;

	if (usage == USAGE_TEXT && data->isBuffered() &&
	    data->getLength() > 0 && data->getLength() < 32768)
	{
		// Extract data into temporary buffer
		string buffer;
		utility::outputStreamStringAdapter os(buffer);

		data->extract(os);
		os.flush();

		enc = decideImpl(buffer.begin(), buffer.end());
	}
	else
	{
		enc = encoding(encodingTypes::BASE64);
	}

	enc.setUsage(usage);

	return enc;
}


const encoding encoding::decide(ref <const contentHandler> data,
	const charset& chset, const EncodingUsage usage)
{
	if (usage == USAGE_TEXT)
	{
		encoding recEncoding;

		if (chset.getRecommendedEncoding(recEncoding))
		{
			recEncoding.setUsage(usage);
			return recEncoding;
		}
	}

	return decide(data, usage);
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


encoding::EncodingUsage encoding::getUsage() const
{
	return m_usage;
}


void encoding::setUsage(const EncodingUsage usage)
{
	m_usage = usage;
}


const std::vector <ref <component> > encoding::getChildComponents()
{
	return std::vector <ref <component> >();
}


} // vmime
