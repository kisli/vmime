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

#include "vmime/mailboxList.hpp"
#include "vmime/mailboxGroup.hpp"
#include "vmime/exception.hpp"


namespace vmime
{


mailboxList::mailboxList()
{
}


mailboxList::mailboxList(const mailboxList& mboxList)
	: headerFieldValue(), m_list(mboxList.m_list)
{
}


void mailboxList::appendMailbox(shared_ptr <mailbox> mbox)
{
	m_list.appendAddress(mbox);
}


void mailboxList::insertMailboxBefore(shared_ptr <mailbox> beforeMailbox, shared_ptr <mailbox> mbox)
{
	m_list.insertAddressBefore(beforeMailbox, mbox);
}


void mailboxList::insertMailboxBefore(const size_t pos, shared_ptr <mailbox> mbox)
{
	m_list.insertAddressBefore(pos, mbox);
}


void mailboxList::insertMailboxAfter(shared_ptr <mailbox> afterMailbox, shared_ptr <mailbox> mbox)
{
	m_list.insertAddressAfter(afterMailbox, mbox);
}


void mailboxList::insertMailboxAfter(const size_t pos, shared_ptr <mailbox> mbox)
{
	m_list.insertAddressAfter(pos, mbox);
}


void mailboxList::removeMailbox(shared_ptr <mailbox> mbox)
{
	m_list.removeAddress(mbox);
}


void mailboxList::removeMailbox(const size_t pos)
{
	m_list.removeAddress(pos);
}


void mailboxList::removeAllMailboxes()
{
	m_list.removeAllAddresses();
}


size_t mailboxList::getMailboxCount() const
{
	return (m_list.getAddressCount());
}


bool mailboxList::isEmpty() const
{
	return (m_list.isEmpty());
}


shared_ptr <mailbox> mailboxList::getMailboxAt(const size_t pos)
{
	return dynamicCast <mailbox>(m_list.getAddressAt(pos));
}


const shared_ptr <const mailbox> mailboxList::getMailboxAt(const size_t pos) const
{
	return dynamicCast <const mailbox>(m_list.getAddressAt(pos));
}


const std::vector <shared_ptr <const mailbox> > mailboxList::getMailboxList() const
{
	const std::vector <shared_ptr <const address> > addrList = m_list.getAddressList();
	std::vector <shared_ptr <const mailbox> > res;

	for (std::vector <shared_ptr <const address> >::const_iterator it = addrList.begin() ;
	     it != addrList.end() ; ++it)
	{
		const shared_ptr <const mailbox> mbox = dynamicCast <const mailbox>(*it);

		if (mbox != NULL)
			res.push_back(mbox);
	}

	return (res);
}


const std::vector <shared_ptr <mailbox> > mailboxList::getMailboxList()
{
	const std::vector <shared_ptr <address> > addrList = m_list.getAddressList();
	std::vector <shared_ptr <mailbox> > res;

	for (std::vector <shared_ptr <address> >::const_iterator it = addrList.begin() ;
	     it != addrList.end() ; ++it)
	{
		const shared_ptr <mailbox> mbox = dynamicCast <mailbox>(*it);

		if (mbox != NULL)
			res.push_back(mbox);
	}

	return (res);
}


shared_ptr <component> mailboxList::clone() const
{
	return make_shared <mailboxList>(*this);
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


const std::vector <shared_ptr <component> > mailboxList::getChildComponents()
{
	return (m_list.getChildComponents());
}


void mailboxList::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	m_list.removeAllAddresses();

	size_t pos = position;

	while (pos < end)
	{
		shared_ptr <address> parsedAddress = address::parseNext(ctx, buffer, pos, end, &pos, NULL);

		if (parsedAddress != NULL)
		{
			if (parsedAddress->isGroup())
			{
				shared_ptr <mailboxGroup> group = dynamicCast <mailboxGroup>(parsedAddress);

				for (size_t i = 0 ; i < group->getMailboxCount() ; ++i)
				{
					m_list.appendAddress(group->getMailboxAt(i));
				}
			}
			else
			{
				m_list.appendAddress(parsedAddress);
			}
		}
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void mailboxList::generateImpl(const generationContext& ctx, utility::outputStream& os,
	const size_t curLinePos, size_t* newLinePos) const
{
	m_list.generate(ctx, os, curLinePos, newLinePos);
}


shared_ptr <addressList> mailboxList::toAddressList() const
{
	return vmime::clone(m_list);
}


} // vmime

