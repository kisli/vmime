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

#ifndef VMIME_MAILBOXLIST_HPP_INCLUDED
#define VMIME_MAILBOXLIST_HPP_INCLUDED


#include "addressList.hpp"
#include "mailbox.hpp"


namespace vmime
{


/** A list of mailboxes (basic type).
  */

class mailboxList : public addressList
{
	friend class mailboxGroup;

public:

	//
	// The following functions have the same name and work *exactly* like
	// the ones in "addressList", except we don't accept anything other
	// than objects of type "mailbox" (instead of a generic "address").
	//
	// This prevents user from inserting mailbox groups where it is not
	// allowed by the RFC.
	//

	// Address iterator
	class const_iterator;

	class iterator
	{
		friend class mailboxList;
		friend class const_iterator;

	protected:

		iterator(std::vector <address*>::iterator it) : m_iterator(it) { }

	public:

		iterator(const iterator& it) : m_iterator(it.m_iterator) { }

		iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

		mailbox& operator*() const { return static_cast<mailbox&>(**m_iterator); }
		mailbox* operator->() const { return static_cast<mailbox*>(*m_iterator); }

		iterator& operator++() { ++m_iterator; return (*this); }
		iterator& operator++(int) { ++m_iterator; return (*this); }

		const bool operator==(const iterator& it) const { return (it.m_iterator == m_iterator); }
		const bool operator!=(const iterator& it) const { return (!(*this == it)); }

	private:

		std::vector <address*>::iterator m_iterator;
	};

	class const_iterator
	{
		friend class mailboxList;

	protected:

		const_iterator(std::vector <address*>::const_iterator it) : m_iterator(it) { }

	public:

		const_iterator(const iterator& it) : m_iterator(it.m_iterator) { }
		const_iterator(const const_iterator& it) : m_iterator(it.m_iterator) { }

		const_iterator& operator=(const const_iterator& it) { m_iterator = it.m_iterator; return (*this); }
		const_iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

		const mailbox& operator*() const { return static_cast<const mailbox&>(**m_iterator); }
		const mailbox* operator->() const { return static_cast<const mailbox*>(*m_iterator); }

		const_iterator& operator++() { ++m_iterator; return (*this); }
		const_iterator& operator++(int) { ++m_iterator; return (*this); }

		const bool operator==(const const_iterator& it) const { return (it.m_iterator == m_iterator); }
		const bool operator!=(const const_iterator& it) const { return (!(*this == it)); }

	private:

		std::vector <address*>::const_iterator m_iterator;
	};

	iterator begin() { return (m_list.begin()); }
	iterator end() { return (m_list.end()); }

	const_iterator begin() const { return (const_iterator(m_list.begin())); }
	const_iterator end() const { return (const_iterator(m_list.end())); }

	// Address insertion
	void append(const address& addr);
	void insert(const iterator it, const address& addr);
};


} // vmime


#endif // VMIME_MAILBOXLIST_HPP_INCLUDED
