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

#include "vmime/messageId.hpp"
#include "vmime/utility/random.hpp"
#include "vmime/platform.hpp"
#include "vmime/parserHelpers.hpp"


namespace vmime
{


messageId::messageId()
{
}


messageId::messageId(const string& id)
{
	parse(id);
}


messageId::messageId(const messageId& mid)
	: headerFieldValue(), m_left(mid.m_left), m_right(mid.m_right)
{
}


messageId::messageId(const string& left, const string& right)
	: m_left(left), m_right(right)
{
}


/*
	RFC-2822:
	3.6.4. Identification fields

	msg-id          =       [CFWS] "<" id-left "@" id-right ">" [CFWS]
*/

void messageId::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* const pstart = buffer.data() + position;
	const string::value_type* p = pstart;

	m_left.clear();
	m_right.clear();

	unsigned int commentLevel = 0;
	bool escape = false;
	bool stop = false;

	for ( ; !stop && p < pend ; ++p)
	{
		if (escape)
		{
			// Ignore this character
		}
		else
		{
			switch (*p)
			{
			case '(': ++commentLevel; break;
			case ')': --commentLevel; break;
			case '\\': escape = true; break;
			case '<':
			{
				if (commentLevel == 0)
				{
					stop = true;
					break;
				}
			}

			}
		}
	}

	// Fix for message ids without angle brackets (invalid)
	bool hasBrackets = true;

	if (p == pend)  // no opening angle bracket found
	{
		hasBrackets = false;
		p = pstart;

		while (p < pend && parserHelpers::isSpace(*p))
			++p;
	}

	if (p < pend)
	{
		// Extract left part
		const string::size_type leftStart = position + (p - pstart);

		while (p < pend && *p != '@' && *p != '>') ++p;

		m_left = string(buffer.begin() + leftStart,
		                buffer.begin() + position + (p - pstart));

		if (p < pend)
		{
			// Skip '@'
			++p;

			// Extract right part
			const string::size_type rightStart = position + (p - pstart);

			while (p < pend && *p != '>' && (hasBrackets || !parserHelpers::isSpace(*p))) ++p;

			m_right = string(buffer.begin() + rightStart,
			                 buffer.begin() + position + (p - pstart));
		}
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


ref <messageId> messageId::parseNext(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	string::size_type pos = position;

	while (pos < end && parserHelpers::isSpace(buffer[pos]))
		++pos;

	if (pos != end)
	{
		const string::size_type begin = pos;

		while (pos < end && !parserHelpers::isSpace(buffer[pos]))
			++pos;

		ref <messageId> mid = vmime::create <messageId>();
		mid->parse(buffer, begin, pos, NULL);

		if (newPosition != NULL)
			*newPosition = pos;

		return (mid);
	}

	if (newPosition != NULL)
		*newPosition = end;

	return (NULL);
}


const string messageId::getId() const
{
	if (m_right.empty())
		return m_left;

	return (m_left + '@' + m_right);
}


void messageId::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	if (curLinePos + m_left.length() + m_right.length() + 3 > maxLineLength)
	{
		os << NEW_LINE_SEQUENCE;
		pos = NEW_LINE_SEQUENCE_LENGTH;
	}

	os << '<' << m_left;
	if (m_right != "") os << '@' << m_right;
	os << '>';

	if (newLinePos)
		*newLinePos = pos + m_left.length() + m_right.length() + 3;
}


messageId& messageId::operator=(const string& id)
{
	parse(id);
	return (*this);
}


messageId messageId::generateId()
{
	std::ostringstream left;
	left.imbue(std::locale::classic());

	left << "vmime";
	left << '.';
	left << std::hex << utility::random::getTime();
	left << '.';
	left << std::hex << utility::random::getProcess();
	left << '.';
	left << std::hex << utility::random::getNext();
	left << std::hex << utility::random::getNext();

	return (messageId(left.str(), platform::getHandler()->getHostName()));
}


bool messageId::operator==(const messageId& mid) const
{
	return (m_left == mid.m_left && m_right == mid.m_right);
}


bool messageId::operator!=(const messageId& mid) const
{
	return !(*this == mid);
}


ref <component> messageId::clone() const
{
	return vmime::create <messageId>(*this);
}


void messageId::copyFrom(const component& other)
{
	const messageId& mid = dynamic_cast <const messageId&>(other);

	m_left = mid.m_left;
	m_right = mid.m_right;
}


messageId& messageId::operator=(const messageId& other)
{
	copyFrom(other);
	return (*this);
}


const string& messageId::getLeft() const
{
	return (m_left);
}


void messageId::setLeft(const string& left)
{
	m_left = left;
}


const string& messageId::getRight() const
{
	return (m_right);
}


void messageId::setRight(const string& right)
{
	m_right = right;
}


const std::vector <ref <component> > messageId::getChildComponents()
{
	return std::vector <ref <component> >();
}


} // vmime
