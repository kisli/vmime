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

#include "vmime/addressList.hpp"
#include "vmime/parserHelpers.hpp"
#include "vmime/exception.hpp"
#include "vmime/mailboxList.hpp"


namespace vmime
{


addressList::addressList()
{
}


addressList::addressList(const addressList& addrList)
	: component()
{
	copyFrom(addrList);
}


addressList::~addressList()
{
	removeAllAddresses();
}


void addressList::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	removeAllAddresses();

	string::size_type pos = position;

	while (pos < end)
	{
		address* parsedAddress = address::parseNext(buffer, pos, end, &pos);

		if (parsedAddress != NULL)
			m_list.push_back(parsedAddress);
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void addressList::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	if (!m_list.empty())
	{
		string::size_type pos = curLinePos;
		std::vector <address*>::const_iterator i = m_list.begin();

		for ( ; ; )
		{
			(*i)->generate(os, maxLineLength - 2, pos, &pos);

			if (++i != m_list.end())
			{
				os << ", ";
				pos += 2;
			}
			else
			{
				break;
			}
		}

		if (newLinePos)
			*newLinePos = pos;
	}
}


void addressList::copyFrom(const component& other)
{
	const addressList& addrList = dynamic_cast <const addressList&>(other);

	removeAllAddresses();

	for (std::vector <address*>::const_iterator it = addrList.m_list.begin() ;
	     it != addrList.m_list.end() ; ++it)
	{
		m_list.push_back(static_cast <address*>((*it)->clone()));
	}
}


addressList& addressList::operator=(const addressList& other)
{
	copyFrom(other);
	return (*this);
}


addressList& addressList::operator=(const mailboxList& other)
{
	copyFrom(other);
	return (*this);
}


addressList* addressList::clone() const
{
	return new addressList(*this);
}


void addressList::appendAddress(address* addr)
{
	m_list.push_back(addr);
}


void addressList::insertAddressBefore(address* beforeAddress, address* addr)
{
	const std::vector <address*>::iterator it = std::find
		(m_list.begin(), m_list.end(), beforeAddress);

	if (it == m_list.end())
		throw exceptions::no_such_address();

	m_list.insert(it, addr);
}


void addressList::insertAddressBefore(const int pos, address* addr)
{
	m_list.insert(m_list.begin() + pos, addr);
}


void addressList::insertAddressAfter(address* afterAddress, address* addr)
{
	const std::vector <address*>::iterator it = std::find
		(m_list.begin(), m_list.end(), afterAddress);

	if (it == m_list.end())
		throw exceptions::no_such_address();

	m_list.insert(it + 1, addr);
}


void addressList::insertAddressAfter(const int pos, address* addr)
{
	m_list.insert(m_list.begin() + pos + 1, addr);
}


void addressList::removeAddress(address* addr)
{
	const std::vector <address*>::iterator it = std::find
		(m_list.begin(), m_list.end(), addr);

	if (it == m_list.end())
		throw exceptions::no_such_address();

	delete (*it);

	m_list.erase(it);
}


void addressList::removeAddress(const int pos)
{
	const std::vector <address*>::iterator it = m_list.begin() + pos;

	delete (*it);

	m_list.erase(it);
}


void addressList::removeAllAddresses()
{
	free_container(m_list);
}


const int addressList::getAddressCount() const
{
	return (m_list.size());
}


const bool addressList::isEmpty() const
{
	return (m_list.empty());
}


address* addressList::getAddressAt(const int pos)
{
	return (m_list[pos]);
}


const address* const addressList::getAddressAt(const int pos) const
{
	return (m_list[pos]);
}


const std::vector <const address*> addressList::getAddressList() const
{
	std::vector <const address*> list;

	list.reserve(m_list.size());

	for (std::vector <address*>::const_iterator it = m_list.begin() ;
	     it != m_list.end() ; ++it)
	{
		list.push_back(*it);
	}

	return (list);
}


const std::vector <address*> addressList::getAddressList()
{
	return (m_list);
}


const std::vector <const component*> addressList::getChildComponents() const
{
	std::vector <const component*> list;

	copy_vector(m_list, list);

	return (list);
}


} // vmime
