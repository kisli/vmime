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

#ifndef VMIME_ADDRESSLIST_HPP_INCLUDED
#define VMIME_ADDRESSLIST_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"

#include "address.hpp"


namespace vmime
{


/** A list of addresses.
  */

class addressList : public component
{
	friend class addressListField;
	friend class mailboxListField;

public:

	addressList();
	addressList(const class addressList& addressList);

	~addressList();

public:

	addressList& operator=(const addressList& source);

	// Address iterator
	class const_iterator;

	class iterator
	{
		friend class addressList;
		friend class const_iterator;

	public:

		iterator(std::vector <address*>::iterator it) : m_iterator(it) { }
		iterator(const iterator& it) : m_iterator(it.m_iterator) { }

		iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

		address& operator*() const { return (**m_iterator); }
		address* operator->() const { return (*m_iterator); }

		iterator& operator++() { ++m_iterator; return (*this); }
		iterator& operator++(int) { ++m_iterator; return (*this); }

		const bool operator==(const iterator& it) const { return (it.m_iterator == m_iterator); }
		const bool operator!=(const iterator& it) const { return (!(*this == it)); }

	private:

		std::vector <address*>::iterator m_iterator;
	};

	class const_iterator
	{
		friend class addressList;

	public:

		const_iterator(std::vector <address*>::const_iterator it) : m_iterator(it) { }
		const_iterator(const iterator& it) : m_iterator(it.m_iterator) { }
		const_iterator(const const_iterator& it) : m_iterator(it.m_iterator) { }

		const_iterator& operator=(const const_iterator& it) { m_iterator = it.m_iterator; return (*this); }
		const_iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

		const address& operator*() const { return (**m_iterator); }
		const address* operator->() const { return (*m_iterator); }

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

	const std::vector <address*>::size_type size() const;
	const std::vector <address*>::size_type count() const;
	const bool empty() const;

	const address& operator[](const std::vector <address*>::size_type x) const { return (*m_list[x]); }
	address& operator[](const std::vector <address*>::size_type x) { return (*m_list[x]); }

	virtual void append(const address& addr);
	virtual void insert(const iterator it, const address& addr);

	void erase(const iterator it);
	void clear();

protected:

	std::vector <address*> m_list;

	void copyFrom(const addressList& source);

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_ADDRESSLIST_HPP_INCLUDED
