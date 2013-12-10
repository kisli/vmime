//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

#include "vmime/emailAddress.hpp"

#include "vmime/platform.hpp"

#include "vmime/parserHelpers.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime
{


emailAddress::emailAddress()
{
}


emailAddress::emailAddress(const emailAddress& eml)
	: component(), m_localName(eml.m_localName), m_domainName(eml.m_domainName)
{
}


emailAddress::emailAddress(const string& email)
{
	parse(email);
}


emailAddress::emailAddress(const char* email)
{
	parse(email);
}


emailAddress::emailAddress(const string& localName, const string& domainName)
	: component(), m_localName(word(localName, vmime::charsets::UTF_8)),
	  m_domainName(word(domainName, vmime::charsets::UTF_8))
{
}


emailAddress::emailAddress(const word& localName, const word& domainName)
	: component(), m_localName(localName), m_domainName(domainName)
{
}


void emailAddress::parseImpl
	(const parsingContext& /* ctx */, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	const char* const pend = buffer.data() + end;
	const char* const pstart = buffer.data() + position;
	const char* p = pstart;

	enum ParserStates
	{
		State_Before,
		State_LocalPartStart,
		State_LocalPartMiddle,
		State_LocalPartComment,
		State_LocalPartQuoted,
		State_DomainPartStart,
		State_DomainPartMiddle,
		State_DomainPartComment,
		State_End,
		State_Error
	} state = State_Before;

	std::ostringstream localPart;
	std::ostringstream domainPart;

	bool escapeNext = false;  // for quoting
	bool prevIsDot = false;
	bool atFound = false;
	bool stop = false;
	int commentLevel = 0;

	while (p < pend && !stop)
	{
		const char c = *p;

		if ((localPart.str().length() + domainPart.str().length()) >= 256)
		{
			state = State_Error;
			break;
		}

		switch (state)
		{
		case State_Before:

			if (parserHelpers::isSpace(c))
				++p;
			else
				state = State_LocalPartStart;

		case State_LocalPartStart:

			if (c == '"')
			{
				state = State_LocalPartQuoted;
				++p;
			}
			else if (c == '(')
			{
				state = State_LocalPartComment;
				++commentLevel;
				++p;
			}
			else
			{
				state = State_LocalPartMiddle;
				localPart << c;
				++p;
			}

			break;

		case State_LocalPartComment:

			if (escapeNext)
			{
				escapeNext = false;
				++p;
			}
			else if (c == '\\')
			{
				escapeNext = true;
				++p;
			}
			else if (c == '(')
			{
				++commentLevel;
				++p;
			}
			else if (c == ')')
			{
				if (--commentLevel == 0)
				{
					// End of comment
					state = State_LocalPartMiddle;
				}

				++p;
			}
			else
			{
				// Comment continues
				++p;
			}

			break;

		case State_LocalPartQuoted:

			if (escapeNext)
			{
				escapeNext = false;

				if (c == '"' || c == '\\')
				{
					localPart << c;
					++p;
				}
				else
				{
					// This char cannot be escaped
					state = State_Error;
				}
			}
			else if (c == '"')
			{
				// End of quoted string
				state = State_LocalPartMiddle;
				++p;
			}
			else if (c == '\\')
			{
				escapeNext = true;
				++p;
			}
			else
			{
				localPart << c;
				++p;
			}

			break;

		case State_LocalPartMiddle:

			if (c == '.')
			{
				prevIsDot = true;
				localPart << c;
				++p;
			}
			else if (c == '"' && prevIsDot)
			{
				prevIsDot = false;
				state = State_LocalPartQuoted;
				++p;
			}
			else if (c == '(')
			{
				// By allowing comments anywhere in the local part,
				// we are more permissive than RFC-2822
				state = State_LocalPartComment;
				++commentLevel;
				++p;
			}
			else if (c == '@')
			{
				atFound = true;
				state = State_DomainPartStart;
				++p;
			}
			else if (parserHelpers::isSpace(c))
			{
				// Allow not specifying domain part
				state = State_End;
			}
			else
			{
				prevIsDot = false;
				localPart << c;
				++p;
			}

			break;

		case State_DomainPartStart:

			if (c == '(')
			{
				state = State_DomainPartComment;
				++commentLevel;
				++p;
			}
			else
			{
				state = State_DomainPartMiddle;
				domainPart << c;
				++p;
			}

			break;

		case State_DomainPartMiddle:

			if (parserHelpers::isSpace(c))
			{
				state = State_End;
			}
			else if (c == '(')
			{
				// By allowing comments anywhere in the domain part,
				// we are more permissive than RFC-2822
				state = State_DomainPartComment;
				++commentLevel;
				++p;
			}
			else
			{
				domainPart << c;
				++p;
			}

			break;

		case State_DomainPartComment:

			if (escapeNext)
			{
				escapeNext = false;
				++p;
			}
			else if (c == '\\')
			{
				escapeNext = true;
				++p;
			}
			else if (c == '(')
			{
				++commentLevel;
				++p;
			}
			else if (c == ')')
			{
				if (--commentLevel == 0)
				{
					// End of comment
					state = State_DomainPartMiddle;
				}

				++p;
			}
			else
			{
				// Comment continues
				++p;
			}

			break;

		case State_End:
		case State_Error:

			stop = true;
			break;
		}
	}

	if (p == pend && state != State_Error)
	{
		if (state == State_DomainPartMiddle)
			state = State_End;
		else if (state == State_LocalPartMiddle)
			state = State_End;  // allow not specifying domain part
	}

	if (state != State_End)
	{
		m_localName = word("invalid", vmime::charsets::UTF_8);
		m_domainName = word("invalid", vmime::charsets::UTF_8);
	}
	else
	{
		// If the domain part is missing, use local host name
		if (domainPart.str().empty() && !atFound)
			domainPart << platform::getHandler()->getHostName();

		m_localName = word(localPart.str(), vmime::charsets::UTF_8);
		m_domainName = word(domainPart.str(), vmime::charsets::UTF_8);
	}

	setParsedBounds(position, p - pend);

	if (newPosition)
		*newPosition = p - pend;
}


static const string domainNameToIDNA(const string& domainName)
{
	std::ostringstream idnaDomain;
	size_t p = 0;

	for (size_t n = domainName.find('.', p) ;
	     (n = domainName.find('.', p)) != string::npos ; p = n + 1)
	{
		string idnaPart;
		charset::convert(string(domainName.begin() + p, domainName.begin() + n),
			idnaPart, vmime::charsets::UTF_8, vmime::charsets::IDNA);

		idnaDomain << idnaPart << '.';
	}

	if (p < domainName.length())
	{
		string idnaPart;
		charset::convert(string(domainName.begin() + p, domainName.end()),
			idnaPart, vmime::charsets::UTF_8, vmime::charsets::IDNA);

		idnaDomain << idnaPart;
	}

	return idnaDomain.str();
}


void emailAddress::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t curLinePos, size_t* newLinePos) const
{
	string localPart, domainPart;

	if (ctx.getInternationalizedEmailSupport() &&
	    (!utility::stringUtils::is7bit(m_localName.getBuffer()) ||
	     !utility::stringUtils::is7bit(m_domainName.getBuffer())))
	{
		// Local part
		string localPartUTF8(m_localName.getConvertedText(vmime::charsets::UTF_8));
		word localPartWord(localPartUTF8, vmime::charsets::UTF_8);

		vmime::utility::outputStreamStringAdapter os(localPart);
		localPartWord.generate(ctx, os, 0, NULL, text::FORCE_NO_ENCODING | text::QUOTE_IF_NEEDED, NULL);

		// Domain part
		domainPart = m_domainName.getConvertedText(vmime::charsets::UTF_8);
	}
	else
	{
		// Local part
		vmime::utility::outputStreamStringAdapter os(localPart);
		m_localName.generate(ctx, os, 0, NULL, text::QUOTE_IF_NEEDED, NULL);

		// Domain part as IDNA
		domainPart = domainNameToIDNA(m_domainName.getConvertedText(vmime::charsets::UTF_8));
	}

	os << localPart
	   << "@"
	   << domainPart;

	if (newLinePos)
	{
		*newLinePos = curLinePos
			+ localPart.length()
			+ 1 // @
			+ domainPart.length();
	}
}


bool emailAddress::operator==(const class emailAddress& eml) const
{
	return (m_localName == eml.m_localName &&
	        m_domainName == eml.m_domainName);
}


bool emailAddress::operator!=(const class emailAddress& eml) const
{
	return !(*this == eml);
}


void emailAddress::copyFrom(const component& other)
{
	const emailAddress& source = dynamic_cast <const emailAddress&>(other);

	m_localName = source.m_localName;
	m_domainName = source.m_domainName;
}


emailAddress& emailAddress::operator=(const emailAddress& other)
{
	copyFrom(other);
	return (*this);
}


shared_ptr <component>emailAddress::clone() const
{
	return make_shared <emailAddress>(*this);
}


const word& emailAddress::getLocalName() const
{
	return m_localName;
}


void emailAddress::setLocalName(const word& localName)
{
	m_localName = localName;
}


const word& emailAddress::getDomainName() const
{
	return m_domainName;
}


void emailAddress::setDomainName(const word& domainName)
{
	m_domainName = domainName;
}


const std::vector <shared_ptr <component> > emailAddress::getChildComponents()
{
	return std::vector <shared_ptr <component> >();
}


bool emailAddress::isEmpty() const
{
	return m_localName.isEmpty();
}


const string emailAddress::toString() const
{
	std::ostringstream oss;
	utility::outputStreamAdapter adapter(oss);

	generationContext ctx(generationContext::getDefaultContext());
	ctx.setMaxLineLength(lineLengthLimits::infinite);

	generateImpl(ctx, adapter, 0, NULL);

	return oss.str();
}


const text emailAddress::toText() const
{
	text txt;
	txt.appendWord(make_shared <vmime::word>(m_localName));
	txt.appendWord(make_shared <vmime::word>("@", vmime::charsets::US_ASCII));
	txt.appendWord(make_shared <vmime::word>(m_domainName));

	return txt;
}


} // vmime
