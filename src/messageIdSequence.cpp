//
// VMime library (http://www.vmime.org)
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

#include "vmime/messageIdSequence.hpp"
#include "vmime/exception.hpp"
#include <algorithm>


namespace vmime
{




messageIdSequence::messageIdSequence()
{
}


messageIdSequence::~messageIdSequence()
{
	removeAllMessageIds();
}


messageIdSequence::messageIdSequence(const messageIdSequence& midSeq)
	: component()
{
	copyFrom(midSeq);
}


messageIdSequence* messageIdSequence::clone() const
{
	return new messageIdSequence(*this);
}


void messageIdSequence::copyFrom(const component& other)
{
	const messageIdSequence& midSeq = dynamic_cast <const messageIdSequence&>(other);

	removeAllMessageIds();

	for (unsigned int i = 0 ; i < midSeq.m_list.size() ; ++i)
		m_list.push_back(midSeq.m_list[i]->clone());
}


messageIdSequence& messageIdSequence::operator=(const messageIdSequence& other)
{
	copyFrom(other);
	return (*this);
}


const std::vector <const component*> messageIdSequence::getChildComponents() const
{
	std::vector <const component*> res;

	copy_vector(m_list, res);

	return (res);
}


void messageIdSequence::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	removeAllMessageIds();

	string::size_type pos = position;

	while (pos < end)
	{
		messageId* parsedMid = messageId::parseNext(buffer, pos, end, &pos);

		if (parsedMid != NULL)
			m_list.push_back(parsedMid);
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void messageIdSequence::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	if (!m_list.empty())
	{
		for (std::vector <messageId*>::const_iterator it = m_list.begin() ; ; )
		{
			(*it)->generate(os, maxLineLength - 2, pos, &pos);

			if (++it == m_list.end())
				break;

			os << " ";
			pos++;
		}
	}

	if (newLinePos)
		*newLinePos = pos;
}


void messageIdSequence::appendMessageId(messageId* mid)
{
	m_list.push_back(mid);
}


void messageIdSequence::insertMessageIdBefore(messageId* beforeMid, messageId* mid)
{
	const std::vector <messageId*>::iterator it = std::find
		(m_list.begin(), m_list.end(), beforeMid);

	if (it == m_list.end())
		throw exceptions::no_such_message_id();

	m_list.insert(it, mid);
}


void messageIdSequence::insertMessageIdBefore(const int pos, messageId* mid)
{
	m_list.insert(m_list.begin() + pos, mid);
}


void messageIdSequence::insertMessageIdAfter(messageId* afterMid, messageId* mid)
{
	const std::vector <messageId*>::iterator it = std::find
		(m_list.begin(), m_list.end(), afterMid);

	if (it == m_list.end())
		throw exceptions::no_such_message_id();

	m_list.insert(it + 1, mid);
}


void messageIdSequence::insertMessageIdAfter(const int pos, messageId* mid)
{
	m_list.insert(m_list.begin() + pos + 1, mid);
}


void messageIdSequence::removeMessageId(messageId* mid)
{
	const std::vector <messageId*>::iterator it = std::find
		(m_list.begin(), m_list.end(), mid);

	if (it == m_list.end())
		throw exceptions::no_such_message_id();

	delete (*it);

	m_list.erase(it);
}


void messageIdSequence::removeMessageId(const int pos)
{
	const std::vector <messageId*>::iterator it = m_list.begin() + pos;

	delete (*it);

	m_list.erase(it);
}


void messageIdSequence::removeAllMessageIds()
{
	free_container(m_list);
}


const int messageIdSequence::getMessageIdCount() const
{
	return (m_list.size());
}


const bool messageIdSequence::isEmpty() const
{
	return (m_list.empty());
}


messageId* messageIdSequence::getMessageIdAt(const int pos)
{
	return (m_list[pos]);
}


const messageId* messageIdSequence::getMessageIdAt(const int pos) const
{
	return (m_list[pos]);
}


const std::vector <const messageId*> messageIdSequence::getMessageIdList() const
{
	std::vector <const messageId*> list;

	list.reserve(m_list.size());

	for (std::vector <messageId*>::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <messageId*> messageIdSequence::getMessageIdList()
{
	return (m_list);
}


} // vmime
