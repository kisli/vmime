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

#include "addressList.hpp"
#include "parserHelpers.hpp"


namespace vmime
{


addressList::addressList()
{
}


addressList::addressList(const class addressList& addressList)
	: component()
{
	copyFrom(addressList);
}


addressList::~addressList()
{
	clear();
}


void addressList::parse(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	clear();

	string::size_type pos = position;

	while (pos < end)
	{
		address* parsedAddress = address::parseNext(buffer, pos, end, &pos);

		if (parsedAddress != NULL)
			m_list.push_back(parsedAddress);
	}

	if (newPosition)
		*newPosition = end;
}


void addressList::generate(utility::outputStream& os, const string::size_type maxLineLength,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	if (!m_list.empty())
	{
		string::size_type pos = curLinePos;
		const_iterator i = m_list.begin();

		for ( ; ; )
		{
			(*i).generate(os, maxLineLength - 2, pos, &pos);

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


/** Return the number of addresses in the list.
  *
  * @return number of addresses in the list
  */

const std::vector <address*>::size_type addressList::size() const
{
	return (m_list.size());
}


/** Return the number of addresses in the list.
  *
  * @return number of addresses in the list
  */

const std::vector <address*>::size_type addressList::count() const
{
	return (m_list.size());
}


/** Test whether the list is empty.
  *
  * @return true if the list is empty, false otherwise
  */

const bool addressList::empty() const
{
	return (m_list.empty());
}


/** Append an address to the list.
  *
  * @param addr the address to add
  */

void addressList::append(const address& addr)
{
	m_list.push_back(addr.clone());
}


/** Insert an address at the specified position in the list.
  *
  * @param it position of the new address
  * @param addr the address to insert
  */

void addressList::insert(const iterator it, const address& addr)
{
	m_list.insert(it.m_iterator, addr.clone());
}


/** Remove the address at the specified position.
  *
  * @param it position of the address to remove
  */

void addressList::erase(const iterator it)
{
	delete (*it.m_iterator);
	m_list.erase(it.m_iterator);
}


/** Remove all the addresses from the list.
  */

void addressList::clear()
{
	free_container(m_list);
}


void addressList::copyFrom(const addressList& source)
{
	clear();

	for (std::vector <address*>::const_iterator i = source.m_list.begin() ; i != source.m_list.end() ; ++i)
		m_list.push_back((*i)->clone());
}


addressList& addressList::operator=(const addressList& source)
{
	copyFrom(source);
	return (*this);
}


} // vmime
