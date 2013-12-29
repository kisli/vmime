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
	: headerFieldValue()
{
	copyFrom(midSeq);
}


shared_ptr <component> messageIdSequence::clone() const
{
	return make_shared <messageIdSequence>(*this);
}


void messageIdSequence::copyFrom(const component& other)
{
	const messageIdSequence& midSeq = dynamic_cast <const messageIdSequence&>(other);

	removeAllMessageIds();

	for (unsigned int i = 0 ; i < midSeq.m_list.size() ; ++i)
		m_list.push_back(vmime::clone(midSeq.m_list[i]));
}


messageIdSequence& messageIdSequence::operator=(const messageIdSequence& other)
{
	copyFrom(other);
	return (*this);
}


const std::vector <shared_ptr <component> > messageIdSequence::getChildComponents()
{
	std::vector <shared_ptr <component> > res;

	copy_vector(m_list, res);

	return (res);
}


void messageIdSequence::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	removeAllMessageIds();

	size_t pos = position;

	while (pos < end)
	{
		shared_ptr <messageId> parsedMid = messageId::parseNext(ctx, buffer, pos, end, &pos);

		if (parsedMid != NULL)
			m_list.push_back(parsedMid);
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void messageIdSequence::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t curLinePos, size_t* newLinePos) const
{
	size_t pos = curLinePos;

	if (!m_list.empty())
	{
		generationContext tmpCtx(ctx);
		tmpCtx.setMaxLineLength(ctx.getMaxLineLength() - 2);

		for (std::vector <shared_ptr <messageId> >::const_iterator it = m_list.begin() ; ; )
		{
			(*it)->generate(ctx, os, pos, &pos);

			if (++it == m_list.end())
				break;

			os << " ";
			pos++;
		}
	}

	if (newLinePos)
		*newLinePos = pos;
}


void messageIdSequence::appendMessageId(shared_ptr <messageId> mid)
{
	m_list.push_back(mid);
}


void messageIdSequence::insertMessageIdBefore(shared_ptr <messageId> beforeMid, shared_ptr <messageId> mid)
{
	const std::vector <shared_ptr <messageId> >::iterator it = std::find
		(m_list.begin(), m_list.end(), beforeMid);

	if (it == m_list.end())
		throw exceptions::no_such_message_id();

	m_list.insert(it, mid);
}


void messageIdSequence::insertMessageIdBefore(const size_t pos, shared_ptr <messageId> mid)
{
	m_list.insert(m_list.begin() + pos, mid);
}


void messageIdSequence::insertMessageIdAfter(shared_ptr <messageId> afterMid, shared_ptr <messageId> mid)
{
	const std::vector <shared_ptr <messageId> >::iterator it = std::find
		(m_list.begin(), m_list.end(), afterMid);

	if (it == m_list.end())
		throw exceptions::no_such_message_id();

	m_list.insert(it + 1, mid);
}


void messageIdSequence::insertMessageIdAfter(const size_t pos, shared_ptr <messageId> mid)
{
	m_list.insert(m_list.begin() + pos + 1, mid);
}


void messageIdSequence::removeMessageId(shared_ptr <messageId> mid)
{
	const std::vector <shared_ptr <messageId> >::iterator it = std::find
		(m_list.begin(), m_list.end(), mid);

	if (it == m_list.end())
		throw exceptions::no_such_message_id();

	m_list.erase(it);
}


void messageIdSequence::removeMessageId(const size_t pos)
{
	const std::vector <shared_ptr <messageId> >::iterator it = m_list.begin() + pos;

	m_list.erase(it);
}


void messageIdSequence::removeAllMessageIds()
{
	m_list.clear();
}


size_t messageIdSequence::getMessageIdCount() const
{
	return (m_list.size());
}


bool messageIdSequence::isEmpty() const
{
	return (m_list.empty());
}


const shared_ptr <messageId> messageIdSequence::getMessageIdAt(const size_t pos)
{
	return (m_list[pos]);
}


const shared_ptr <const messageId> messageIdSequence::getMessageIdAt(const size_t pos) const
{
	return (m_list[pos]);
}


const std::vector <shared_ptr <const messageId> > messageIdSequence::getMessageIdList() const
{
	std::vector <shared_ptr <const messageId> > list;

	list.reserve(m_list.size());

	for (std::vector <shared_ptr <messageId> >::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <shared_ptr <messageId> > messageIdSequence::getMessageIdList()
{
	return (m_list);
}


} // vmime
