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

#include "vmime/mailboxList.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


mailboxList::mailboxList()
{
}


mailboxList::mailboxList(const mailboxList& mboxList)
	: component(), m_list(mboxList.m_list)
{
}


void mailboxList::appendMailbox(mailbox* mbox)
{
	m_list.appendAddress(mbox);
}


void mailboxList::insertMailboxBefore(mailbox* beforeMailbox, mailbox* mbox)
{
	try
	{
		m_list.insertAddressBefore(beforeMailbox, mbox);
	}
	catch (exceptions::no_such_address&)
	{
		throw exceptions::no_such_mailbox();
	}
}


void mailboxList::insertMailboxBefore(const int pos, mailbox* mbox)
{
	m_list.insertAddressBefore(pos, mbox);
}


void mailboxList::insertMailboxAfter(mailbox* afterMailbox, mailbox* mbox)
{
	try
	{
		m_list.insertAddressAfter(afterMailbox, mbox);
	}
	catch (exceptions::no_such_address&)
	{
		throw exceptions::no_such_mailbox();
	}
}


void mailboxList::insertMailboxAfter(const int pos, mailbox* mbox)
{
	m_list.insertAddressAfter(pos, mbox);
}


void mailboxList::removeMailbox(mailbox* mbox)
{
	try
	{
		m_list.removeAddress(mbox);
	}
	catch (exceptions::no_such_address&)
	{
		throw exceptions::no_such_mailbox();
	}
}


void mailboxList::removeMailbox(const int pos)
{
	m_list.removeAddress(pos);
}


void mailboxList::removeAllMailboxes()
{
	m_list.removeAllAddresses();
}


const int mailboxList::getMailboxCount() const
{
	return (m_list.getAddressCount());
}


const bool mailboxList::isEmpty() const
{
	return (m_list.isEmpty());
}


mailbox* mailboxList::getMailboxAt(const int pos)
{
	return static_cast <mailbox*>(m_list.getAddressAt(pos));
}


const mailbox* mailboxList::getMailboxAt(const int pos) const
{
	return static_cast <const mailbox*>(m_list.getAddressAt(pos));
}


const std::vector <const mailbox*> mailboxList::getMailboxList() const
{
	const std::vector <const address*> addrList = m_list.getAddressList();
	std::vector <const mailbox*> res;

	for (std::vector <const address*>::const_iterator it = addrList.begin() ;
	     it != addrList.end() ; ++it)
	{
		const mailbox* mbox = dynamic_cast <const mailbox*>(*it);

		if (mbox != NULL)
			res.push_back(mbox);
	}

	return (res);
}


const std::vector <mailbox*> mailboxList::getMailboxList()
{
	const std::vector <address*> addrList = m_list.getAddressList();
	std::vector <mailbox*> res;

	for (std::vector <address*>::const_iterator it = addrList.begin() ;
	     it != addrList.end() ; ++it)
	{
		mailbox* mbox = dynamic_cast <mailbox*>(*it);

		if (mbox != NULL)
			res.push_back(mbox);
	}

	return (res);
}


mailboxList* mailboxList::clone() const
{
	return new mailboxList(*this);
}


void mailboxList::copyFrom(const component& other)
{
	const mailboxList& mboxList = dynamic_cast <const mailboxList&>(other);

	m_list = mboxList.m_list;
}


mailboxList& mailboxList::operator=(const mailboxList& other)
{
	copyFrom(other);
	return (*this);
}


const std::vector <const component*> mailboxList::getChildComponents() const
{
	return (m_list.getChildComponents());
}


void mailboxList::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	m_list.parse(buffer, position, end, newPosition);
}


void mailboxList::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	m_list.generate(os, maxLineLength, curLinePos, newLinePos);
}


} // vmime
