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


void addressList::parseImpl
	(const parsingContext& ctx, const string& buffer, const size_t position,
	 const size_t end, size_t* newPosition)
{
	removeAllAddresses();

	size_t pos = position;

	while (pos < end)
	{
		shared_ptr <address> parsedAddress = address::parseNext(ctx, buffer, pos, end, &pos, NULL);

		if (parsedAddress != NULL)
			m_list.push_back(parsedAddress);
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void addressList::generateImpl
	(const generationContext& ctx, utility::outputStream& os,
	 const size_t curLinePos, size_t* newLinePos) const
{
	size_t pos = curLinePos;

	generationContext tmpCtx(ctx);
	tmpCtx.setMaxLineLength(tmpCtx.getMaxLineLength() - 2);

	if (!m_list.empty())
	{
		for (std::vector <shared_ptr <address> >::const_iterator i = m_list.begin() ; ; )
		{
			(*i)->generate(ctx, os, pos, &pos);

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

	for (std::vector <shared_ptr <address> >::const_iterator it = addrList.m_list.begin() ;
	     it != addrList.m_list.end() ; ++it)
	{
		m_list.push_back(vmime::clone(*it));
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

	for (size_t i = 0 ; i < other.getMailboxCount() ; ++i)
		m_list.push_back(dynamicCast <address>(other.getMailboxAt(i)->clone()));

	return (*this);
}


shared_ptr <component> addressList::clone() const
{
	return make_shared <addressList>(*this);
}


void addressList::appendAddress(shared_ptr <address> addr)
{
	m_list.push_back(addr);
}


void addressList::insertAddressBefore(shared_ptr <address> beforeAddress, shared_ptr <address> addr)
{
	const std::vector <shared_ptr <address> >::iterator it = std::find
		(m_list.begin(), m_list.end(), beforeAddress);

	if (it == m_list.end())
		throw std::out_of_range("Invalid position");

	m_list.insert(it, addr);
}


void addressList::insertAddressBefore(const size_t pos, shared_ptr <address> addr)
{
	if (pos >= m_list.size())
		throw std::out_of_range("Invalid position");

	m_list.insert(m_list.begin() + pos, addr);
}


void addressList::insertAddressAfter(shared_ptr <address> afterAddress, shared_ptr <address> addr)
{
	const std::vector <shared_ptr <address> >::iterator it = std::find
		(m_list.begin(), m_list.end(), afterAddress);

	if (it == m_list.end())
		throw std::out_of_range("Invalid position");

	m_list.insert(it + 1, addr);
}


void addressList::insertAddressAfter(const size_t pos, shared_ptr <address> addr)
{
	if (pos >= m_list.size())
		throw std::out_of_range("Invalid position");

	m_list.insert(m_list.begin() + pos + 1, addr);
}


void addressList::removeAddress(shared_ptr <address> addr)
{
	const std::vector <shared_ptr <address> >::iterator it = std::find
		(m_list.begin(), m_list.end(), addr);

	if (it == m_list.end())
		throw std::out_of_range("Invalid position");

	m_list.erase(it);
}


void addressList::removeAddress(const size_t pos)
{
	if (pos >= m_list.size())
		throw std::out_of_range("Invalid position");

	const std::vector <shared_ptr <address> >::iterator it = m_list.begin() + pos;

	m_list.erase(it);
}


void addressList::removeAllAddresses()
{
	m_list.clear();
}


size_t addressList::getAddressCount() const
{
	return (m_list.size());
}


bool addressList::isEmpty() const
{
	return (m_list.empty());
}


shared_ptr <address> addressList::getAddressAt(const size_t pos)
{
	return (m_list[pos]);
}


const shared_ptr <const address> addressList::getAddressAt(const size_t pos) const
{
	return (m_list[pos]);
}


const std::vector <shared_ptr <const address> > addressList::getAddressList() const
{
	std::vector <shared_ptr <const address> > list;

	list.reserve(m_list.size());

	for (std::vector <shared_ptr <address> >::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <shared_ptr <address> > addressList::getAddressList()
{
	return (m_list);
}


const std::vector <shared_ptr <component> > addressList::getChildComponents()
{
	std::vector <shared_ptr <component> > list;

	copy_vector(m_list, list);

	return (list);
}


shared_ptr <mailboxList> addressList::toMailboxList() const
{
	shared_ptr <mailboxList> res = make_shared <mailboxList>();

	for (std::vector <shared_ptr <address> >::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		shared_ptr <const address> addr = *it;

		if (addr->isGroup())
		{
			const std::vector <shared_ptr <const mailbox> > mailboxes =
				dynamicCast <const mailboxGroup>(addr)->getMailboxList();

			for (std::vector <shared_ptr <const mailbox> >::const_iterator jt = mailboxes.begin() ;
			     jt != mailboxes.end() ; ++jt)
			{
				res->appendMailbox(vmime::clone(*jt));
			}
		}
		else
		{
			res->appendMailbox(dynamicCast <mailbox>(addr->clone()));
		}
	}

	return res;
}


} // vmime
