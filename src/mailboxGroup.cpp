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

#include "vmime/mailboxGroup.hpp"
#include "vmime/parserHelpers.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


mailboxGroup::mailboxGroup()
{
}


mailboxGroup::mailboxGroup(const mailboxGroup& mboxGroup)
	: address()
{
	copyFrom(mboxGroup);
}


mailboxGroup::mailboxGroup(const text& name)
	: m_name(name)
{
}


mailboxGroup::~mailboxGroup()
{
	removeAllMailboxes();
}


void mailboxGroup::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	const char* const pend = buffer.data() + end;
	const char* const pstart = buffer.data() + position;
	const char* p = pstart;

	while (p < pend && parserHelpers::isSpace(*p))
		++p;

	string name;

	while (p < pend && *p != ':')
	{
		name += *p;
		++p;
	}

	if (p < pend && *p == ':')
		++p;


	size_t pos = position + (p - pstart);
	bool isLastAddressOfGroup = false;

	while (pos < end && !isLastAddressOfGroup)
	{
		shared_ptr <address> parsedAddress = address::parseNext(ctx, buffer, pos, end, &pos, &isLastAddressOfGroup);

		if (parsedAddress)
		{
			if (parsedAddress->isGroup())
			{
				shared_ptr <mailboxGroup> group = dynamicCast <mailboxGroup>(parsedAddress);

				// Sub-groups are not allowed in mailbox groups: so, we add all
				// the contents of the sub-group into this group...
				for (size_t i = 0 ; i < group->getMailboxCount() ; ++i)
				{
					m_list.push_back(vmime::clone(group->getMailboxAt(i)));
				}
			}
			else
			{
				m_list.push_back(dynamicCast <mailbox>(parsedAddress));
			}
		}
	}

	text::decodeAndUnfold(ctx, utility::stringUtils::trim(name), &m_name);

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void mailboxGroup::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t curLinePos, size_t* newLinePos) const
{
	// We have to encode the name:
	//   - if it contains characters in a charset different from "US-ASCII",
	//   - and/or if it contains one or more of these special chars:
	//        SPACE  TAB  "  ;  ,  <  >  (  )  @  /  ?  .  =  :

	// Check whether there are words that are not "US-ASCII"
	// and/or contain the special chars.
	bool forceEncode = false;

	for (size_t w = 0 ; !forceEncode && w < m_name.getWordCount() ; ++w)
	{
		if (m_name.getWordAt(w)->getCharset() == charset(charsets::US_ASCII))
		{
			const string& buffer = m_name.getWordAt(w)->getBuffer();

			for (string::const_iterator c = buffer.begin() ;
			     !forceEncode && c != buffer.end() ; ++c)
			{
				switch (*c)
				{
				case ' ':
				case '\t':
				case ';':
				case ',':
				case '<': case '>':
				case '(': case ')':
				case '@':
				case '/':
				case '?':
				case '.':
				case '=':
				case ':':

					forceEncode = true;
					break;
				}
			}
		}
	}

	size_t pos = curLinePos;

	generationContext tmpCtx(ctx);
	tmpCtx.setMaxLineLength(ctx.getMaxLineLength() - 2);

	m_name.encodeAndFold(ctx, os, pos, &pos,
		forceEncode ? text::FORCE_ENCODING : 0);

	os << ":";
	++pos;

	for (std::vector <shared_ptr <mailbox> >::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		if (it != m_list.begin())
		{
			os << ", ";
			pos += 2;
		}
		else
		{
			os << " ";
			++pos;
		}

		(*it)->generate(tmpCtx, os, pos, &pos);
	}

	os << ";";
	pos++;

	if (newLinePos)
		*newLinePos = pos;
}


void mailboxGroup::copyFrom(const component& other)
{
	const mailboxGroup& source = dynamic_cast <const mailboxGroup&>(other);

	m_name = source.m_name;

	removeAllMailboxes();

	for (std::vector <shared_ptr <mailbox> >::const_iterator it = source.m_list.begin() ;
	     it != source.m_list.end() ; ++it)
	{
		m_list.push_back(vmime::clone(*it));
	}
}


shared_ptr <component> mailboxGroup::clone() const
{
	return make_shared <mailboxGroup>(*this);
}


mailboxGroup& mailboxGroup::operator=(const component& other)
{
	copyFrom(other);
	return (*this);
}


const text& mailboxGroup::getName() const
{
	return (m_name);
}


void mailboxGroup::setName(const text& name)
{
	m_name = name;
}


bool mailboxGroup::isGroup() const
{
	return (true);
}


bool mailboxGroup::isEmpty() const
{
	return (m_list.empty());
}


void mailboxGroup::appendMailbox(shared_ptr <mailbox> mbox)
{
	m_list.push_back(mbox);
}


void mailboxGroup::insertMailboxBefore(shared_ptr <mailbox> beforeMailbox, shared_ptr <mailbox> mbox)
{
	const std::vector <shared_ptr <mailbox> >::iterator it = std::find
		(m_list.begin(), m_list.end(), beforeMailbox);

	if (it == m_list.end())
		throw std::out_of_range("Invalid position");

	m_list.insert(it, mbox);
}


void mailboxGroup::insertMailboxBefore(const size_t pos, shared_ptr <mailbox> mbox)
{
	if (pos >= m_list.size())
		throw std::out_of_range("Invalid position");

	m_list.insert(m_list.begin() + pos, mbox);
}


void mailboxGroup::insertMailboxAfter(shared_ptr <mailbox> afterMailbox, shared_ptr <mailbox> mbox)
{
	const std::vector <shared_ptr <mailbox> >::iterator it = std::find
		(m_list.begin(), m_list.end(), afterMailbox);

	if (it == m_list.end())
		throw std::out_of_range("Invalid position");

	m_list.insert(it + 1, mbox);
}


void mailboxGroup::insertMailboxAfter(const size_t pos, shared_ptr <mailbox> mbox)
{
	if (pos >= m_list.size())
		throw std::out_of_range("Invalid position");

	m_list.insert(m_list.begin() + pos + 1, mbox);
}


void mailboxGroup::removeMailbox(shared_ptr <mailbox> mbox)
{
	const std::vector <shared_ptr <mailbox> >::iterator it = std::find
		(m_list.begin(), m_list.end(), mbox);

	if (it == m_list.end())
		throw std::out_of_range("Invalid position");

	m_list.erase(it);
}


void mailboxGroup::removeMailbox(const size_t pos)
{
	if (pos >= m_list.size())
		throw std::out_of_range("Invalid position");

	const std::vector <shared_ptr <mailbox> >::iterator it = m_list.begin() + pos;

	m_list.erase(it);
}


void mailboxGroup::removeAllMailboxes()
{
	m_list.clear();
}


size_t mailboxGroup::getMailboxCount() const
{
	return (m_list.size());
}


shared_ptr <mailbox> mailboxGroup::getMailboxAt(const size_t pos)
{
	return (m_list[pos]);
}


const shared_ptr <const mailbox> mailboxGroup::getMailboxAt(const size_t pos) const
{
	return (m_list[pos]);
}


const std::vector <shared_ptr <const mailbox> > mailboxGroup::getMailboxList() const
{
	std::vector <shared_ptr <const mailbox> > list;

	list.reserve(m_list.size());

	for (std::vector <shared_ptr <mailbox> >::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <shared_ptr <mailbox> > mailboxGroup::getMailboxList()
{
	return (m_list);
}


const std::vector <shared_ptr <component> > mailboxGroup::getChildComponents()
{
	std::vector <shared_ptr <component> > list;

	copy_vector(m_list, list);

	return (list);

}


} // vmime
