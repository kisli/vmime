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

#include "vmime/messageId.hpp"
#include "vmime/utility/random.hpp"
#include "vmime/platformDependant.hpp"
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
	: component(), m_left(mid.m_left), m_right(mid.m_right)
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

void messageId::parse(const string& buffer, const string::size_type position,
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

	if (p < pend)
	{
		// Extract left part
		const string::size_type leftStart = position + (p - pstart);

		while (p < pend && *p != '@') ++p;

		m_left = string(buffer.begin() + leftStart,
		                buffer.begin() + position + (p - pstart));

		if (p < pend)
		{
			// Skip '@'
			++p;

			// Extract right part
			const string::size_type rightStart = position + (p - pstart);

			while (p < pend && *p != '>') ++p;

			m_right = string(buffer.begin() + rightStart,
			                 buffer.begin() + position + (p - pstart));
		}
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


const string messageId::getId() const
{
	return (m_left + '@' + m_right);
}


void messageId::generate(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	os << '<' << m_left << '@' << m_right << '>';

	if (newLinePos)
		*newLinePos = curLinePos + m_left.length() + m_right.length() + 3;
}


messageId& messageId::operator=(const string& id)
{
	parse(id);
	return (*this);
}


messageId messageId::generateId()
{
	std::ostringstream left;

	left << "vmime";
	left << '.';
	left << std::hex << utility::random::getTime();
	left << '.';
	left << std::hex << utility::random::getProcess();
	left << '.';
	left << std::hex << utility::random::getNext();
	left << std::hex << utility::random::getNext();

	return (messageId(left.str(), platformDependant::getHandler()->getHostName()));
}


const bool messageId::operator==(const messageId& mid) const
{
	return (m_left == mid.m_left && m_right == mid.m_right);
}


const bool messageId::operator!=(const messageId& mid) const
{
	return !(*this == mid);
}


messageId* messageId::clone() const
{
	return new messageId(*this);
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


const std::vector <const component*> messageId::getChildComponents() const
{
	return std::vector <const component*>();
}


} // vmime
