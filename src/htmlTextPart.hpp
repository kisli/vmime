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

#ifndef VMIME_HTMLTEXTPART_HPP_INCLUDED
#define VMIME_HTMLTEXTPART_HPP_INCLUDED


#include "textPart.hpp"
#include "messageId.hpp"
#include "encoding.hpp"

#include "contentHandler.hpp"


namespace vmime
{


class htmlTextPart : public textPart
{
protected:

	~htmlTextPart();

public:

	const mediaType type() const;

	const vmime::charset& charset() const { return (m_charset); }
	vmime::charset& charset() { return (m_charset); }

	const contentHandler& plainText() const { return (m_plainText); }
	contentHandler& plainText() { return (m_plainText); }

	const contentHandler& text() const { return (m_text); }
	contentHandler& text() { return (m_text); }

	// Embedded object (eg. image for <IMG> tag)
	class embeddedObject
	{
	public:

		embeddedObject(const contentHandler& data, const vmime::encoding& enc,
		               const string& id, const mediaType& type)
			: m_data(data), m_encoding(enc), m_id(id), m_type(type)
		{
		}

	public:

		const contentHandler& data() const { return (m_data); }
		const vmime::encoding& encoding() const { return (m_encoding); }
		const string& id() const { return (m_id); }
		const mediaType& type() const { return (m_type); }

	private:

		contentHandler m_data;
		vmime::encoding m_encoding;
		string m_id;
		mediaType m_type;
	};

	// Embedded objects container
	class embeddedObjectsContainer
	{
		friend class htmlTextPart;

	protected:

		~embeddedObjectsContainer();

	public:

		// Test the existence/get an embedded object given its identifier.
		const bool has(const string& id) const;
		const embeddedObject& find(const string& id) const;

		// Embed an object and returns a string which identifies it.
		const string add(const string& data, const mediaType& type);
		const string add(const contentHandler& data, const mediaType& type);
		const string add(const contentHandler& data, const encoding& enc, const mediaType& type);

		// Embedded objects enumerator
		class const_iterator
		{
		public:

			typedef std::vector <embeddedObject*>::const_iterator::difference_type difference_type;

			const_iterator(std::vector <embeddedObject*>::const_iterator it) : m_iterator(it) { }
			const_iterator(const const_iterator& it) : m_iterator(it.m_iterator) { }

			const_iterator& operator=(const const_iterator& it) { m_iterator = it.m_iterator; return (*this); }

			const embeddedObject& operator*() const { return (**m_iterator); }
			const embeddedObject* operator->() const { return (*m_iterator); }

			const_iterator& operator++() { ++m_iterator; return (*this); }
			const_iterator operator++(int) { const_iterator i(*this); ++m_iterator; return (i); }

			const_iterator& operator--() { --m_iterator; return (*this); }
			const_iterator operator--(int) { const_iterator i(*this); --m_iterator; return (i); }

			const_iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
			const_iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

			const_iterator operator-(difference_type x) const { return const_iterator(m_iterator - x); }

			const embeddedObject& operator[](difference_type n) const { return *(m_iterator[n]); }

			const bool operator==(const const_iterator& it) const { return (it.m_iterator == m_iterator); }
			const bool operator!=(const const_iterator& it) const { return (!(*this == it)); }

		protected:

			std::vector <embeddedObject*>::const_iterator m_iterator;
		};

	public:

		const_iterator begin() const { return (const_iterator(m_list.begin())); }
		const_iterator end() const { return (const_iterator(m_list.end())); }

		// Object count
		const std::vector <embeddedObject*>::size_type count() const { return (m_list.size()); }
		const std::vector <embeddedObject*>::size_type size() const { return (m_list.size()); }
		const bool empty() const { return (m_list.empty()); }

		embeddedObject& front() { return (*m_list.front()); }
		const embeddedObject& front() const { return (*m_list.front()); }
		embeddedObject& back() { return (*m_list.back()); }
		const embeddedObject& back() const { return (*m_list.back()); }

	protected:

		std::vector <embeddedObject*> m_list;

	} embeddedObjects;

	typedef embeddedObjectsContainer::const_iterator const_iterator;

protected:

	contentHandler m_plainText;
	contentHandler m_text;
	vmime::charset m_charset;

	void findEmbeddedParts(const bodyPart& part, std::vector <const bodyPart*>& cidParts, std::vector <const bodyPart*>& locParts);
	void addEmbeddedObject(const bodyPart& part, const string& id);

	bool findPlainTextPart(const bodyPart& part, const bodyPart& parent, const bodyPart& textPart);

	const int getPartCount() const;

	void generateIn(bodyPart& message, bodyPart& parent) const;
	virtual void parse(const bodyPart& message, const bodyPart& parent, const bodyPart& textPart);
};


} // vmime


#endif // VMIME_HTMLTEXTPART_HPP_INCLUDED
