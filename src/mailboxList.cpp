//
// VMime library (http://vmime.sourceforge.net)
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
	: addressList(mboxList)
{
}


void mailboxList::appendMailbox(mailbox* mbox)
{
	addressList::appendAddress(mbox);
}


void mailboxList::insertMailboxBefore(mailbox* beforeMailbox, mailbox* mbox)
{
	try
	{
		addressList::insertAddressBefore(beforeMailbox, mbox);
	}
	catch (exceptions::no_such_address&)
	{
		throw exceptions::no_such_mailbox();
	}
}


void mailboxList::insertMailboxBefore(const int pos, mailbox* mbox)
{
	addressList::insertAddressBefore(pos, mbox);
}


void mailboxList::insertMailboxAfter(mailbox* afterMailbox, mailbox* mbox)
{
	try
	{
		addressList::insertAddressAfter(afterMailbox, mbox);
	}
	catch (exceptions::no_such_address&)
	{
		throw exceptions::no_such_mailbox();
	}
}


void mailboxList::insertMailboxAfter(const int pos, mailbox* mbox)
{
	addressList::insertAddressAfter(pos, mbox);
}


void mailboxList::removeMailbox(mailbox* mbox)
{
	try
	{
		addressList::removeAddress(mbox);
	}
	catch (exceptions::no_such_address&)
	{
		throw exceptions::no_such_mailbox();
	}
}


void mailboxList::removeMailbox(const int pos)
{
	addressList::removeAddress(pos);
}


void mailboxList::removeAllMailboxes()
{
	addressList::removeAllAddresses();
}


const int mailboxList::getMailboxCount() const
{
	return (addressList::getAddressCount());
}


const bool mailboxList::isEmpty() const
{
	return (addressList::isEmpty());
}


mailbox* mailboxList::getMailboxAt(const int pos)
{
	return static_cast <mailbox*>(addressList::getAddressAt(pos));
}


const mailbox* const mailboxList::getMailboxAt(const int pos) const
{
	return static_cast <const mailbox*>(addressList::getAddressAt(pos));
}


const std::vector <const mailbox*> mailboxList::getMailboxList() const
{
	const std::vector <const address*> addrList = addressList::getAddressList();
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
	const std::vector <address*> addrList = addressList::getAddressList();
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


} // vmime
