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

#ifndef VMIME_HEADER_HPP_INCLUDED
#define VMIME_HEADER_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"
#include "exception.hpp"

#include "headerField.hpp"
#include "headerFieldFactory.hpp"

#include "addressListField.hpp"
#include "mailboxListField.hpp"
#include "mailboxField.hpp"
#include "textField.hpp"
#include "dateField.hpp"
#include "contentTypeField.hpp"
#include "contentEncodingField.hpp"
#include "defaultField.hpp"
#include "contentDispositionField.hpp"
#include "messageIdField.hpp"


namespace vmime
{


class bodyPart;


/** Header section of a MIME part.
  */

class header : public component
{
	friend class bodyPart;
	friend class body;
	friend class message;

public:

	header();
	~header();

	// A sub-class for field manipulation
	class fieldsContainer
	{
		friend class header;

	protected:

		fieldsContainer();
		~fieldsContainer();

	public:

		// Field access
		mailboxField& From() { return (dynamic_cast<mailboxField&>(get(headerField::From))); }
		mailboxField& Sender() { return (dynamic_cast<mailboxField&>(get(headerField::Sender))); }
		mailboxField& ReplyTo() { return (dynamic_cast<mailboxField&>(get(headerField::ReplyTo))); }
		mailboxField& DeliveredTo() { return (dynamic_cast<mailboxField&>(get(headerField::DeliveredTo))); }
		addressListField& To() { return (dynamic_cast<addressListField&>(get(headerField::To))); }
		addressListField& Cc() { return (dynamic_cast<addressListField&>(get(headerField::Cc))); }
		addressListField& Bcc() { return (dynamic_cast<addressListField&>(get(headerField::Bcc))); }
		dateField& Date() { return (dynamic_cast<dateField&>(get(headerField::Date))); }
		textField& Subject() { return (dynamic_cast<textField&>(get(headerField::Subject))); }
		textField& Organization() { return (dynamic_cast<textField&>(get(headerField::Organization))); }
		textField& UserAgent() { return (dynamic_cast<textField&>(get(headerField::UserAgent))); }
		contentTypeField& ContentType() { return (dynamic_cast<contentTypeField&>(get(headerField::ContentType))); }
		textField& ContentDescription() { return (dynamic_cast<textField&>(get(headerField::ContentDescription))); }
		contentEncodingField& ContentTransferEncoding() { return (dynamic_cast<contentEncodingField&>(get(headerField::ContentTransferEncoding))); }
		defaultField& MimeVersion() { return (dynamic_cast<defaultField&>(get(headerField::MimeVersion))); }
		contentDispositionField& ContentDisposition() { return (dynamic_cast<contentDispositionField&>(get(headerField::ContentDisposition))); }
		messageIdField& ContentId() { return (dynamic_cast<messageIdField&>(get(headerField::ContentId))); }
		messageIdField& MessageId() { return (dynamic_cast<messageIdField&>(get(headerField::MessageId))); }
		defaultField& ContentLocation() { return (dynamic_cast<defaultField&>(get(headerField::ContentLocation))); }

		const mailboxField& From() const { return (dynamic_cast<mailboxField&>(find(headerField::From))); }
		const mailboxField& Sender() const { return (dynamic_cast<mailboxField&>(find(headerField::Sender))); }
		const mailboxField& ReplyTo() const { return (dynamic_cast<mailboxField&>(find(headerField::ReplyTo))); }
		const mailboxField& DeliveredTo() const { return (dynamic_cast<mailboxField&>(find(headerField::DeliveredTo))); }
		const addressListField& To() const { return (dynamic_cast<addressListField&>(find(headerField::To))); }
		const addressListField& Cc() const { return (dynamic_cast<addressListField&>(find(headerField::Cc))); }
		const addressListField& Bcc() const { return (dynamic_cast<addressListField&>(find(headerField::Bcc))); }
		const dateField& Date() const { return (dynamic_cast<dateField&>(find(headerField::Date))); }
		const textField& Subject() const { return (dynamic_cast<textField&>(find(headerField::Subject))); }
		const textField& Organization() const { return (dynamic_cast<textField&>(find(headerField::Organization))); }
		const textField& UserAgent() const { return (dynamic_cast<textField&>(find(headerField::UserAgent))); }
		const contentTypeField& ContentType() const { return (dynamic_cast<contentTypeField&>(find(headerField::ContentType))); }
		const textField& ContentDescription() const { return (dynamic_cast<textField&>(find(headerField::ContentDescription))); }
		const contentEncodingField& ContentTransferEncoding() const { return (dynamic_cast<contentEncodingField&>(find(headerField::ContentTransferEncoding))); }
		const defaultField& MimeVersion() const { return (dynamic_cast<defaultField&>(find(headerField::MimeVersion))); }
		const contentDispositionField& ContentDisposition() const { return (dynamic_cast<contentDispositionField&>(find(headerField::ContentDisposition))); }
		const messageIdField& ContentId() const { return (dynamic_cast<messageIdField&>(find(headerField::ContentId))); }
		const messageIdField& MessageId() const { return (dynamic_cast<messageIdField&>(find(headerField::MessageId))); }
		const defaultField& ContentLocation() const { return (dynamic_cast<defaultField&>(find(headerField::ContentLocation))); }

		// Checks whether (at least) one field with this type/name exists
		const bool has(const headerField::Types fieldType) const;
		const bool has(const string& fieldName) const;

		// Find the first field that matches the specified type/name.
		// If no field is found, an exception is thrown.
		headerField& find(const headerField::Types fieldType) const;
		headerField& find(const string& fieldName) const;

		// Find all fields that matche the specified type/name.
		// If no field is found, an empty vector is returned.
		std::vector <headerField*> findAllByType(const headerField::Types fieldType);
		std::vector <headerField*> findAllByName(const string& fieldName);

		// Find the first field that matches the specified type/name.
		// If no field is found, one will be created.
		headerField& get(const headerField::Types fieldType);
		headerField& get(const string& fieldName);

		// Field iterator
		class const_iterator;

		class iterator
		{
			friend class header::fieldsContainer::const_iterator;
			friend class header::fieldsContainer;

		public:

			typedef std::vector <headerField*>::iterator::difference_type difference_type;

			iterator(std::vector <headerField*>::iterator it) : m_iterator(it) { }
			iterator(const iterator& it) : m_iterator(it.m_iterator) { }

			iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

			headerField& operator*() const { return (**m_iterator); }
			headerField* operator->() const { return (*m_iterator); }

			iterator& operator++() { ++m_iterator; return (*this); }
			iterator operator++(int) { iterator i(*this); ++m_iterator; return (i); }

			iterator& operator--() { --m_iterator; return (*this); }
			iterator operator--(int) { iterator i(*this); --m_iterator; return (i); }

			iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
			iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

			iterator operator-(difference_type x) const { return iterator(m_iterator - x); }

			headerField& operator[](difference_type n) const { return *(m_iterator[n]); }

			const bool operator==(const iterator& it) const { return (it.m_iterator == m_iterator); }
			const bool operator!=(const iterator& it) const { return (!(*this == it)); }

		protected:

			std::vector <headerField*>::iterator m_iterator;
		};

		class const_iterator
		{
		public:

			typedef std::vector <headerField*>::const_iterator::difference_type difference_type;

			const_iterator(std::vector <headerField*>::const_iterator it) : m_iterator(it) { }
			const_iterator(const iterator& it) : m_iterator(it.m_iterator) { }
			const_iterator(const const_iterator& it) : m_iterator(it.m_iterator) { }

			const_iterator& operator=(const const_iterator& it) { m_iterator = it.m_iterator; return (*this); }
			const_iterator& operator=(const iterator& it) { m_iterator = it.m_iterator; return (*this); }

			const headerField& operator*() const { return (**m_iterator); }
			const headerField* operator->() const { return (*m_iterator); }

			const_iterator& operator++() { ++m_iterator; return (*this); }
			const_iterator operator++(int) { const_iterator i(*this); ++m_iterator; return (i); }

			const_iterator& operator--() { --m_iterator; return (*this); }
			const_iterator operator--(int) { const_iterator i(*this); --m_iterator; return (i); }

			const_iterator& operator+=(difference_type n) { m_iterator += n; return (*this); }
			const_iterator& operator-=(difference_type n) { m_iterator -= n; return (*this); }

			const_iterator operator-(difference_type x) const { return const_iterator(m_iterator - x); }

			const headerField& operator[](difference_type n) const { return *(m_iterator[n]); }

			const bool operator==(const const_iterator& it) const { return (it.m_iterator == m_iterator); }
			const bool operator!=(const const_iterator& it) const { return (!(*this == it)); }

		protected:

			std::vector <headerField*>::const_iterator m_iterator;
		};

	public:

		iterator begin() { return (m_fields.begin()); }
		iterator end() { return (m_fields.end()); }

		const_iterator begin() const { return (const_iterator(m_fields.begin())); }
		const_iterator end() const { return (const_iterator(m_fields.end())); }

		// Field insertion
		void append(const headerField& field);
		void insert(const iterator it, const headerField& field);

		// Field removing
		void remove(const iterator it);
		void clear();

		// Field count
		const std::vector <headerField*>::size_type count() const { return (m_fields.size()); }
		const std::vector <headerField*>::size_type size() const { return (m_fields.size()); }
		const bool empty() const { return (m_fields.empty()); }

		headerField& front() { return (*m_fields.front()); }
		const headerField& front() const { return (*m_fields.front()); }
		headerField& back() { return (*m_fields.back()); }
		const headerField& back() const { return (*m_fields.back()); }

		fieldsContainer& operator=(const fieldsContainer& c);

	protected:

		void insertSorted(headerField* field);

		std::vector <headerField*> m_fields;

	} fields;

	typedef fieldsContainer::iterator iterator;
	typedef fieldsContainer::const_iterator const_iterator;

	header& operator=(const header& h);

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_HEADER_HPP_INCLUDED
