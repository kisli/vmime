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

#ifndef VMIME_HEADER_HPP_INCLUDED
#define VMIME_HEADER_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"
#include "vmime/exception.hpp"

#include "vmime/headerField.hpp"
#include "vmime/headerFieldFactory.hpp"

#include "vmime/mailboxField.hpp"
#include "vmime/contentTypeField.hpp"
#include "vmime/contentDispositionField.hpp"

#include "vmime/standardFields.hpp"
#include "vmime/standardParams.hpp"


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

#define FIELD_ACCESS(methodName, fieldName, type) \
	type& methodName() { return dynamic_cast <type&> \
		(*getField(fields::fieldName)); } \
	const type& methodName() const { return dynamic_cast <const type&> \
		(*findField(fields::fieldName)); }

	FIELD_ACCESS(From,                    FROM,                      mailboxField)
	FIELD_ACCESS(Sender,                  SENDER,                    mailboxField)
	FIELD_ACCESS(ReplyTo,                 REPLY_TO,                  mailboxField)
	FIELD_ACCESS(DeliveredTo,             DELIVERED_TO,              mailboxField)

	FIELD_ACCESS(To,                      TO,                        addressListField)
	FIELD_ACCESS(Cc,                      CC,                        addressListField)
	FIELD_ACCESS(Bcc,                     BCC,                       addressListField)
	FIELD_ACCESS(Date,                    DATE,                      dateField)
	FIELD_ACCESS(Subject,                 SUBJECT,                   textField)
	FIELD_ACCESS(Organization,            ORGANIZATION,              textField)
	FIELD_ACCESS(UserAgent,               USER_AGENT,                textField)

	FIELD_ACCESS(ContentType,             CONTENT_TYPE,              contentTypeField)
	FIELD_ACCESS(ContentDescription,      CONTENT_DESCRIPTION,       textField)
	FIELD_ACCESS(ContentTransferEncoding, CONTENT_TRANSFER_ENCODING, contentEncodingField)
	FIELD_ACCESS(MimeVersion,             MIME_VERSION,              defaultField)
	FIELD_ACCESS(ContentDisposition,      CONTENT_DISPOSITION,       contentDispositionField)
	FIELD_ACCESS(ContentId,               CONTENT_ID,                messageIdField)
	FIELD_ACCESS(MessageId,               MESSAGE_ID,                messageIdField)
	FIELD_ACCESS(ContentLocation,         CONTENT_LOCATION,          defaultField)

#undef FIELD_ACCESS

	/** Checks whether (at least) one field with this name exists.
	  *
	  * @return true if at least one field with the specified name
	  * exists, or false otherwise
	  */
	const bool hasField(const string& fieldName) const;

	/** Find the first field that matches the specified name.
	  * If no field is found, an exception is thrown.
	  *
	  * @throw exceptions::no_such_field if no field with this name exists
	  * @return first field with the specified name
	  */
	headerField* findField(const string& fieldName) const;

	/** Find all fields that match the specified name.
	  * If no field is found, an empty vector is returned.
	  *
	  * @return list of fields with the specified name
	  */
	std::vector <headerField*> findAllFields(const string& fieldName);

	/** Find the first field that matches the specified name.
	  * If no field is found, one will be created and inserted into
	  * the header.
	  *
	  * @return first field with the specified name or a new field
	  * if no field is found
	  */
	headerField* getField(const string& fieldName);

	/** Add a field at the end of the list.
	  *
	  * @param field field to append
	  */
	void appendField(headerField* field);

	/** Insert a new field before the specified field.
	  *
	  * @param beforeField field before which the new field will be inserted
	  * @param field field to insert
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void insertFieldBefore(headerField* beforeField, headerField* field);

	/** Insert a new field before the specified position.
	  *
	  * @param pos position at which to insert the new field (0 to insert at
	  * the beginning of the list)
	  * @param field field to insert
	  */
	void insertFieldBefore(const int pos, headerField* field);

	/** Insert a new field after the specified field.
	  *
	  * @param afterField field after which the new field will be inserted
	  * @param field field to insert
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void insertFieldAfter(headerField* afterField, headerField* field);

	/** Insert a new field after the specified position.
	  *
	  * @param pos position of the field before the new field
	  * @param field field to insert
	  */
	void insertFieldAfter(const int pos, headerField* field);

	/** Remove the specified field from the list.
	  *
	  * @param field field to remove
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void removeField(headerField* field);

	/** Remove the field at the specified position.
	  *
	  * @param pos position of the field to remove
	  */
	void removeField(const int pos);

	/** Remove all fields from the list.
	  */
	void removeAllFields();

	/** Return the number of fields in the list.
	  *
	  * @return number of fields
	  */
	const int getFieldCount() const;

	/** Tests whether the list of fields is empty.
	  *
	  * @return true if there is no field, false otherwise
	  */
	const bool isEmpty() const;

	/** Return the field at the specified position.
	  *
	  * @param pos position
	  * @return field at position 'pos'
	  */
	headerField* getFieldAt(const int pos);

	/** Return the field at the specified position.
	  *
	  * @param pos position
	  * @return field at position 'pos'
	  */
	const headerField* const getFieldAt(const int pos) const;

	/** Return the field list.
	  *
	  * @return list of fields
	  */
	const std::vector <const headerField*> getFieldList() const;

	/** Return the field list.
	  *
	  * @return list of fields
	  */
	const std::vector <headerField*> getFieldList();

	header* clone() const;
	void copyFrom(const component& other);
	header& operator=(const header& other);

	const std::vector <const component*> getChildComponents() const;

private:

	std::vector <headerField*> m_fields;

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_HEADER_HPP_INCLUDED
