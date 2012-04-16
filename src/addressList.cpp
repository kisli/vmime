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

#include "vmime/addressList.hpp"
#include "vmime/parserHelpers.hpp"
#include "vmime/exception.hpp"
#include "vmime/mailboxList.hpp"
#include "vmime/mailboxGroup.hpp"


namespace vmime
{


addressList::addressList()
{
}


addressList::addressList(const addressList& addrList)
	: headerFieldValue()
{
	copyFrom(addrList);
}


addressList::~addressList()
{
	removeAllAddresses();
}


void addressList::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	removeAllAddresses();

	string::size_type pos = position;

	while (pos < end)
	{
		ref <address> parsedAddress = address::parseNext(buffer, pos, end, &pos);

		if (parsedAddress != NULL)
			m_list.push_back(parsedAddress);
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void addressList::generateImpl(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	string::size_type pos = curLinePos;

	if (!m_list.empty())
	{
		for (std::vector <ref <address> >::const_iterator i = m_list.begin() ; ; )
		{
			(*i)->generate(os, maxLineLength - 2, pos, &pos);

			if (++i == m_list.end())
				break;

			os << ", ";
			pos += 2;
		}
	}

	if (newLinePos)
		*newLinePos = pos;
}


void addressList::copyFrom(const component& other)
{
	const addressList& addrList = dynamic_cast <const addressList&>(other);

	removeAllAddresses();

	for (std::vector <ref <address> >::const_iterator it = addrList.m_list.begin() ;
	     it != addrList.m_list.end() ; ++it)
	{
		m_list.push_back((*it)->clone().dynamicCast <address>());
	}
}


addressList& addressList::operator=(const addressList& other)
{
	copyFrom(other);
	return (*this);
}


addressList& addressList::operator=(const mailboxList& other)
{
	removeAllAddresses();

	for (int i = 0 ; i < other.getMailboxCount() ; ++i)
		m_list.push_back(other.getMailboxAt(i)->clone().dynamicCast <address>());

	return (*this);
}


ref <component> addressList::clone() const
{
	return vmime::create <addressList>(*this);
}


void addressList::appendAddress(ref <address> addr)
{
	m_list.push_back(addr);
}


void addressList::insertAddressBefore(ref <address> beforeAddress, ref <address> addr)
{
	const std::vector <ref <address> >::iterator it = std::find
		(m_list.begin(), m_list.end(), beforeAddress);

	if (it == m_list.end())
		throw exceptions::no_such_address();

	m_list.insert(it, addr);
}


void addressList::insertAddressBefore(const int pos, ref <address> addr)
{
	m_list.insert(m_list.begin() + pos, addr);
}


void addressList::insertAddressAfter(ref <address> afterAddress, ref <address> addr)
{
	const std::vector <ref <address> >::iterator it = std::find
		(m_list.begin(), m_list.end(), afterAddress);

	if (it == m_list.end())
		throw exceptions::no_such_address();

	m_list.insert(it + 1, addr);
}


void addressList::insertAddressAfter(const int pos, ref <address> addr)
{
	m_list.insert(m_list.begin() + pos + 1, addr);
}


void addressList::removeAddress(ref <address> addr)
{
	const std::vector <ref <address> >::iterator it = std::find
		(m_list.begin(), m_list.end(), addr);

	if (it == m_list.end())
		throw exceptions::no_such_address();

	m_list.erase(it);
}


void addressList::removeAddress(const int pos)
{
	const std::vector <ref <address> >::iterator it = m_list.begin() + pos;

	m_list.erase(it);
}


void addressList::removeAllAddresses()
{
	m_list.clear();
}


int addressList::getAddressCount() const
{
	return (m_list.size());
}


bool addressList::isEmpty() const
{
	return (m_list.empty());
}


ref <address> addressList::getAddressAt(const int pos)
{
	return (m_list[pos]);
}


const ref <const address> addressList::getAddressAt(const int pos) const
{
	return (m_list[pos]);
}


const std::vector <ref <const address> > addressList::getAddressList() const
{
	std::vector <ref <const address> > list;

	list.reserve(m_list.size());

	for (std::vector <ref <address> >::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <ref <address> > addressList::getAddressList()
{
	return (m_list);
}


const std::vector <ref <component> > addressList::getChildComponents()
{
	std::vector <ref <component> > list;

	copy_vector(m_list, list);

	return (list);
}


ref <mailboxList> addressList::toMailboxList() const
{
	ref <mailboxList> res = vmime::create <mailboxList>();

	for (std::vector <ref <address> >::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		ref <const address> addr = *it;

		if (addr->isGroup())
		{
			const std::vector <ref <const mailbox> > mailboxes =
				addr.dynamicCast <const mailboxGroup>()->getMailboxList();

			for (std::vector <ref <const mailbox> >::const_iterator jt = mailboxes.begin() ;
			     jt != mailboxes.end() ; ++jt)
			{
				res->appendMailbox(vmime::clone(*jt));
			}
		}
		else
		{
			res->appendMailbox(addr->clone().dynamicCast <mailbox>());
		}
	}

	return res;
}


} // vmime
