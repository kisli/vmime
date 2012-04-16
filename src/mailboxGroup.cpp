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


void mailboxGroup::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* const pstart = buffer.data() + position;
	const string::value_type* p = pstart;

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


	string::size_type pos = position + (p - pstart);

	while (pos < end)
	{
		ref <address> parsedAddress = address::parseNext(buffer, pos, end, &pos);

		if (parsedAddress)
		{
			if (parsedAddress->isGroup())
			{
				ref <mailboxGroup> group = parsedAddress.staticCast <mailboxGroup>();

				// Sub-groups are not allowed in mailbox groups: so, we add all
				// the contents of the sub-group into this group...
				for (int i = 0 ; i < group->getMailboxCount() ; ++i)
				{
					m_list.push_back(group->getMailboxAt(i)->clone().staticCast <mailbox>());
				}
			}
			else
			{
				m_list.push_back(parsedAddress.staticCast <mailbox>());
			}
		}
	}

	text::decodeAndUnfold(name, &m_name);

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void mailboxGroup::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	// We have to encode the name:
	//   - if it contains characters in a charset different from "US-ASCII",
	//   - and/or if it contains one or more of these special chars:
	//        SPACE  TAB  "  ;  ,  <  >  (  )  @  /  ?  .  =  :

	// Check whether there are words that are not "US-ASCII"
	// and/or contain the special chars.
	bool forceEncode = false;

	for (int w = 0 ; !forceEncode && w < m_name.getWordCount() ; ++w)
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

	string::size_type pos = curLinePos;

	m_name.encodeAndFold(os, maxLineLength - 2, pos, &pos,
		forceEncode ? text::FORCE_ENCODING : 0);

	os << ":";
	++pos;

	for (std::vector <ref <mailbox> >::const_iterator it = m_list.begin() ;
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

		(*it)->generate(os, maxLineLength - 2, pos, &pos);
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

	for (std::vector <ref <mailbox> >::const_iterator it = source.m_list.begin() ;
	     it != source.m_list.end() ; ++it)
	{
		m_list.push_back((*it)->clone().staticCast <mailbox>());
	}
}


ref <component> mailboxGroup::clone() const
{
	return vmime::create <mailboxGroup>(*this);
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


void mailboxGroup::appendMailbox(ref <mailbox> mbox)
{
	m_list.push_back(mbox);
}


void mailboxGroup::insertMailboxBefore(ref <mailbox> beforeMailbox, ref <mailbox> mbox)
{
	const std::vector <ref <mailbox> >::iterator it = std::find
		(m_list.begin(), m_list.end(), beforeMailbox);

	if (it == m_list.end())
		throw exceptions::no_such_mailbox();

	m_list.insert(it, mbox);
}


void mailboxGroup::insertMailboxBefore(const int pos, ref <mailbox> mbox)
{
	m_list.insert(m_list.begin() + pos, mbox);
}


void mailboxGroup::insertMailboxAfter(ref <mailbox> afterMailbox, ref <mailbox> mbox)
{
	const std::vector <ref <mailbox> >::iterator it = std::find
		(m_list.begin(), m_list.end(), afterMailbox);

	if (it == m_list.end())
		throw exceptions::no_such_mailbox();

	m_list.insert(it + 1, mbox);
}


void mailboxGroup::insertMailboxAfter(const int pos, ref <mailbox> mbox)
{
	m_list.insert(m_list.begin() + pos + 1, mbox);
}


void mailboxGroup::removeMailbox(ref <mailbox> mbox)
{
	const std::vector <ref <mailbox> >::iterator it = std::find
		(m_list.begin(), m_list.end(), mbox);

	if (it == m_list.end())
		throw exceptions::no_such_mailbox();

	m_list.erase(it);
}


void mailboxGroup::removeMailbox(const int pos)
{
	const std::vector <ref <mailbox> >::iterator it = m_list.begin() + pos;

	m_list.erase(it);
}


void mailboxGroup::removeAllMailboxes()
{
	m_list.clear();
}


int mailboxGroup::getMailboxCount() const
{
	return (m_list.size());
}


ref <mailbox> mailboxGroup::getMailboxAt(const int pos)
{
	return (m_list[pos]);
}


const ref <const mailbox> mailboxGroup::getMailboxAt(const int pos) const
{
	return (m_list[pos]);
}


const std::vector <ref <const mailbox> > mailboxGroup::getMailboxList() const
{
	std::vector <ref <const mailbox> > list;

	list.reserve(m_list.size());

	for (std::vector <ref <mailbox> >::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <ref <mailbox> > mailboxGroup::getMailboxList()
{
	return (m_list);
}


const std::vector <ref <component> > mailboxGroup::getChildComponents()
{
	std::vector <ref <component> > list;

	copy_vector(m_list, list);

	return (list);

}


} // vmime
