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

#ifndef VMIME_TEXT_HPP_INCLUDED
#define VMIME_TEXT_HPP_INCLUDED


#include "base.hpp"
#include "word.hpp"


namespace vmime
{


/** A class representing a list of encoded-words, as defined
  * in RFC-2047 (basic type).
  */

class text
{
public:

	text();
	text(const text& t);
	text(const string& t, const charset& ch);
	explicit text(const string& t);
	explicit text(const word& w);
	~text();

public:

	text& operator=(const text& t);

	const bool operator==(const text& t) const;
	const bool operator!=(const text& t) const;

	// Words iterator
	class const_iterator;

	class iterator
	{
		friend class text::const_iterator;
		friend class text;

	public:

		typedef std::vector <word*>::iterator::difference_type difference_type;

		iterator(std::vector <word*>::iterator it) : m_iterator(it) { }
		iterator(const iterator& it) : m_iterator(it.m_iterator) { }

		iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

		word& operator*() const { return (**m_iterator); }
		word* operator->() const { return (*m_iterator); }

		iterator& operator++() { ++m_iterator; return (*this); }
		iterator operator++(int) { iterator i(*this); ++m_iterator; return (i); }

		iterator& operator--() { --m_iterator; return (*this); }
		iterator operator--(int) { iterator i(*this); --m_iterator; return (i); }

		iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
		iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

		iterator operator+(difference_type x) const { return iterator(m_iterator + x); }
		iterator operator-(difference_type x) const { return iterator(m_iterator - x); }

		word& operator[](difference_type n) const { return *(m_iterator[n]); }

		const bool operator==(const iterator& it) const { return (it.m_iterator == m_iterator); }
		const bool operator!=(const iterator& it) const { return (!(*this == it)); }

	protected:

		std::vector <word*>::iterator m_iterator;
	};

	class const_iterator
	{
	public:

		typedef std::vector <word*>::const_iterator::difference_type difference_type;

		const_iterator(std::vector <word*>::const_iterator it) : m_iterator(it) { }
		const_iterator(const iterator& it) : m_iterator(it.m_iterator) { }
		const_iterator(const const_iterator& it) : m_iterator(it.m_iterator) { }

		const_iterator& operator=(const const_iterator& it) { m_iterator = it.m_iterator; return (*this); }
		const_iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

		const word& operator*() const { return (**m_iterator); }
		const word* operator->() const { return (*m_iterator); }

		const_iterator& operator++() { ++m_iterator; return (*this); }
		const_iterator operator++(int) { const_iterator i(*this); ++m_iterator; return (i); }

		const_iterator& operator--() { --m_iterator; return (*this); }
		const_iterator operator--(int) { const_iterator i(*this); --m_iterator; return (i); }

		const_iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
		const_iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

		const_iterator operator+(difference_type x) const { return const_iterator(m_iterator + x); }
		const_iterator operator-(difference_type x) const { return const_iterator(m_iterator - x); }

		const word& operator[](difference_type n) const { return *(m_iterator[n]); }

		const bool operator==(const const_iterator& it) const { return (it.m_iterator == m_iterator); }
		const bool operator!=(const const_iterator& it) const { return (!(*this == it)); }

	protected:

		std::vector <word*>::const_iterator m_iterator;
	};


	iterator begin() { return (m_words.begin()); }
	iterator end() { return (m_words.end()); }

	const_iterator begin() const { return (const_iterator(m_words.begin())); }
	const_iterator end() const { return (const_iterator(m_words.end())); }

	const word& operator[](const std::vector <word*>::size_type x) const { return (*m_words[x]); }
	word& operator[](const std::vector <word*>::size_type x) { return (*m_words[x]); }

	// Word manipulation
	void append(const word& w);
	void insert(const iterator it, const word& w);

	void clear();
	void remove(const iterator it);

	// Word count
	const bool empty() const;
	const size_t count() const;
	const size_t size() const;

	word& front();
	const word& front() const;
	word& back();
	const word& back() const;

	// Decoding
#if VMIME_WIDE_CHAR_SUPPORT
	const wstring getDecodedText() const;
#endif
	const string getConvertedText(const charset& dest) const;

protected:

	std::vector <word*> m_words;
};


} // vmime


#endif // VMIME_TEXT_HPP_INCLUDED
