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

#ifndef VMIME_MAILBOXGROUP_HPP_INCLUDED
#define VMIME_MAILBOXGROUP_HPP_INCLUDED


#include "address.hpp"
#include "mailbox.hpp"
#include "text.hpp"


namespace vmime
{


/** A group of mailboxes (basic type).
  */

class mailboxGroup : public address
{
public:

	mailboxGroup();
	mailboxGroup(const class mailboxGroup& mailboxGroup);
	mailboxGroup(const text& name);

	~mailboxGroup();

	// Properties set/get
	const text& name() const { return (m_name); }
	text& name() { return (m_name); }

	// Assignment
	void copyFrom(const address& addr);
	address* clone() const;

public:

	// Mailbox iterator
	class const_iterator;

	class iterator
	{
		friend class mailboxGroup;
		friend class const_iterator;

	public:

		iterator(std::vector <mailbox*>::iterator it) : m_iterator(it) { }
		iterator(const iterator& it) : m_iterator(it.m_iterator) { }

		iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

		mailbox& operator*() const { return (**m_iterator); }
		mailbox* operator->() const { return (*m_iterator); }

		iterator& operator++() { ++m_iterator; return (*this); }
		iterator& operator++(int) { ++m_iterator; return (*this); }

		const bool operator==(const iterator& it) const { return (it.m_iterator == m_iterator); }
		const bool operator!=(const iterator& it) const { return (!(*this == it)); }

	private:

		std::vector <mailbox*>::iterator m_iterator;
	};

	class const_iterator
	{
		friend class mailboxGroup;

	public:

		const_iterator(std::vector <mailbox*>::const_iterator it) : m_iterator(it) { }
		const_iterator(const iterator& it) : m_iterator(it.m_iterator) { }
		const_iterator(const const_iterator& it) : m_iterator(it.m_iterator) { }

		const_iterator& operator=(const const_iterator& it) { m_iterator = it.m_iterator; return (*this); }
		const_iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

		const mailbox& operator*() const { return (**m_iterator); }
		const mailbox* operator->() const { return (*m_iterator); }

		const_iterator& operator++() { ++m_iterator; return (*this); }
		const_iterator& operator++(int) { ++m_iterator; return (*this); }

		const bool operator==(const const_iterator& it) const { return (it.m_iterator == m_iterator); }
		const bool operator!=(const const_iterator& it) const { return (!(*this == it)); }

	private:

		std::vector <mailbox*>::const_iterator m_iterator;
	};

	iterator begin() { return (m_list.begin()); }
	iterator end() { return (m_list.end()); }

	const_iterator begin() const { return (const_iterator(m_list.begin())); }
	const_iterator end() const { return (const_iterator(m_list.end())); }

	const std::vector <mailbox*>::size_type size() const { return (m_list.size()); }
	const std::vector <mailbox*>::size_type count() const { return (m_list.size()); }
	const bool empty() const { return (m_list.empty()); }

	const mailbox& operator[](const std::vector <mailbox*>::size_type x) const { return (*m_list[x]); }
	mailbox& operator[](const std::vector <mailbox*>::size_type x) { return (*m_list[x]); }

	// Mailbox insertion
	virtual void append(const mailbox& field);
	virtual void insert(const iterator it, const mailbox& field);

	// Mailbox removing
	void erase(const iterator it);
	void clear();


	const bool isGroup() const;

protected:

	text m_name;
	std::vector <mailbox*> m_list;

public:

	using address::parse;
	using address::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_MAILBOXGROUP_HPP_INCLUDED
