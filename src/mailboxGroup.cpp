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

#include "mailboxGroup.hpp"
#include "parserHelpers.hpp"


namespace vmime
{


mailboxGroup::mailboxGroup()
{
}


mailboxGroup::mailboxGroup(const class mailboxGroup& mailboxGroup)
	: address()
{
	copyFrom(mailboxGroup);
}


mailboxGroup::mailboxGroup(const text& name)
	: m_name(name)
{
}


mailboxGroup::~mailboxGroup()
{
	clear();
}


void mailboxGroup::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* const pstart = buffer.data() + position;
	const string::value_type* p = pstart;

	while (p < pend && isspace(*p))
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
		address* parsedAddress = address::parseNext(buffer, pos, end, &pos);

		if (parsedAddress)
		{
			if (parsedAddress->isGroup())
			{
				mailboxGroup* group = static_cast <mailboxGroup*>(parsedAddress);

				// Sub-groups are not allowed in mailbox groups: so, we add all
				// the contents of the sub-group into this group...
				for (mailboxGroup::const_iterator
				     it = group->begin() ; it != group->end() ; ++it)
				{
					m_list.push_back(static_cast <mailbox*>((*it).clone()));
				}

				delete (parsedAddress);
			}
			else
			{
				m_list.push_back(static_cast <mailbox*>(parsedAddress));
			}
		}
	}

	decodeAndUnfoldText(name, m_name);

	if (newPosition)
		*newPosition = end;
}


void mailboxGroup::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	// We have to encode the name:
	//   - if it contains characters in a charset different from "US-ASCII",
	//   - and/or if it contains one or more of these special chars:
	//        SPACE  TAB  "  ;  ,  <  >  (  )  @  /  ?  .  =  :

	// Check whether there are words that are not "US-ASCII"
	// and/or contain the special chars.
	bool forceEncode = false;

	for (text::const_iterator w = m_name.begin() ; !forceEncode && w != m_name.end() ; ++w)
	{
		if ((*w).charset() == charset(charsets::US_ASCII))
		{
			const string& buffer = (*w).buffer();

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

	encodeAndFoldText(os, m_name, maxLineLength - 2, pos, &pos,
		forceEncode ? encodeAndFoldFlags::forceEncoding : encodeAndFoldFlags::none);

	os << ":";
	++pos;

	for (const_iterator it = m_list.begin() ; it != m_list.end() ; ++it)
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

		(*it).generate(os, maxLineLength - 2, pos, &pos);
	}

	os << ";";
	pos++;

	if (newLinePos)
		*newLinePos = pos;
}


address* mailboxGroup::clone() const
{
	return new mailboxGroup(*this);
}


// Mailbox insertion
void mailboxGroup::append(const mailbox& field)
{
	m_list.push_back(static_cast<mailbox*>(field.clone()));
}


void mailboxGroup::insert(const iterator it, const mailbox& field)
{
	m_list.insert(it.m_iterator, static_cast<mailbox*>(field.clone()));
}


// Mailbox removing
void mailboxGroup::erase(const iterator it)
{
	delete (*it.m_iterator);
	m_list.erase(it.m_iterator);
}


void mailboxGroup::clear()
{
	free_container(m_list);
}


void mailboxGroup::copyFrom(const address& addr)
{
	const mailboxGroup& source = dynamic_cast<const mailboxGroup&>(addr);

	m_name = source.m_name;

	clear();

	for (std::vector <mailbox*>::const_iterator i = source.m_list.begin() ; i != source.m_list.end() ; ++i)
		m_list.push_back(static_cast<mailbox*>((*i)->clone()));
}


const bool mailboxGroup::isGroup() const
{
	return (true);
}


} // vmime
