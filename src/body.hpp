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

#ifndef VMIME_BODY_HPP_INCLUDED
#define VMIME_BODY_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"

#include "header.hpp"

#include "mediaType.hpp"
#include "charset.hpp"
#include "encoding.hpp"

#include "contentHandler.hpp"


namespace vmime
{


class bodyPart;


/** Body section of a MIME part.
  */

class body : public component
{
	friend class bodyPart;

protected:

	body(bodyPart& part);

public:

	// A sub-class for part manipulation
	class partsContainer
	{
		friend class body;

	protected:

		partsContainer(class body& body);
		~partsContainer();

	public:

		// Part iterator
		class const_iterator;

		class iterator
		{
			friend class body::partsContainer::const_iterator;
			friend class body::partsContainer;

		public:

			typedef std::vector <bodyPart*>::iterator::difference_type difference_type;

			iterator(std::vector <bodyPart*>::iterator it) : m_iterator(it) { }
			iterator(const iterator& it) : m_iterator(it.m_iterator) { }

			iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

			bodyPart& operator*() const { return (**m_iterator); }
			bodyPart* operator->() const { return (*m_iterator); }

			iterator& operator++() { ++m_iterator; return (*this); }
			iterator operator++(int) { iterator i(*this); ++m_iterator; return (i); }

			iterator& operator--() { --m_iterator; return (*this); }
			iterator operator--(int) { iterator i(*this); --m_iterator; return (i); }

			iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
			iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

			iterator operator-(difference_type x) const { return iterator(m_iterator - x); }

			bodyPart& operator[](difference_type n) const { return *(m_iterator[n]); }

			const bool operator==(const iterator& it) const { return (it.m_iterator == m_iterator); }
			const bool operator!=(const iterator& it) const { return (!(*this == it)); }

		protected:

			std::vector <bodyPart*>::iterator m_iterator;
		};

		class const_iterator
		{
		public:

			typedef std::vector <bodyPart*>::const_iterator::difference_type difference_type;

			const_iterator(std::vector <bodyPart*>::const_iterator it) : m_iterator(it) { }
			const_iterator(const iterator& it) : m_iterator(it.m_iterator) { }
			const_iterator(const const_iterator& it) : m_iterator(it.m_iterator) { }

			const_iterator& operator=(const const_iterator& it) { m_iterator = it.m_iterator; return (*this); }
			const_iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

			const bodyPart& operator*() const { return (**m_iterator); }
			const bodyPart* operator->() const { return (*m_iterator); }

			const_iterator& operator++() { ++m_iterator; return (*this); }
			const_iterator operator++(int) { const_iterator i(*this); ++m_iterator; return (i); }

			const_iterator& operator--() { --m_iterator; return (*this); }
			const_iterator operator--(int) { const_iterator i(*this); --m_iterator; return (i); }

			const_iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
			const_iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

			const_iterator operator-(difference_type x) const { return const_iterator(m_iterator - x); }

			const bodyPart& operator[](difference_type n) const { return *(m_iterator[n]); }

			const bool operator==(const const_iterator& it) const { return (it.m_iterator == m_iterator); }
			const bool operator!=(const const_iterator& it) const { return (!(*this == it)); }

		protected:

			std::vector <bodyPart*>::const_iterator m_iterator;
		};

	public:

		iterator begin() { return (m_parts.begin()); }
		iterator end() { return (m_parts.end()); }

		const_iterator begin() const { return (const_iterator(m_parts.begin())); }
		const_iterator end() const { return (const_iterator(m_parts.end())); }

		const bodyPart& operator[](const std::vector <bodyPart*>::size_type x) const { return (*m_parts[x]); }
		bodyPart& operator[](const std::vector <bodyPart*>::size_type x) { return (*m_parts[x]); }

		// Part insertion
		void append(bodyPart* part);
		void insert(const iterator it, bodyPart* part);

		// Part removing
		void remove(const iterator it);
		void clear();

		// Part count
		const size_t count() const { return (m_parts.size()); }
		const size_t size() const { return (m_parts.size()); }

		bodyPart& front() { return (*m_parts.front()); }
		const bodyPart& front() const { return (*m_parts.front()); }
		bodyPart& back() { return (*m_parts.back()); }
		const bodyPart& back() const { return (*m_parts.back()); }

		partsContainer& operator=(const partsContainer& c);

	protected:

		body& m_body;

		std::vector <bodyPart*> m_parts;

	} parts;

	typedef partsContainer::iterator iterator;
	typedef partsContainer::const_iterator const_iterator;


	const string& prologText() const { return (m_prologText); }
	string& prologText() { return (m_prologText); }

	const string& epilogText() const { return (m_epilogText); }
	string& epilogText() { return (m_epilogText); }

	const contentHandler& contents() const { return (m_contents); }
	contentHandler& contents() { return (m_contents); }

	// Quick-access functions
	const mediaType contentType() const;
	const class charset charset() const;
	const class encoding encoding() const;

	// Boundary string functions
	static const string generateRandomBoundaryString();
	static const bool isValidBoundary(const string& boundary);

	body& operator=(const body& b);

protected:

	string m_prologText;
	string m_epilogText;

	contentHandler m_contents;

	bodyPart& m_part;
	header& m_header;

	const bool isRootPart() const;

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_BODY_HPP_INCLUDED
