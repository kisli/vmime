//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_HEADER_HPP_INCLUDED
#define VMIME_HEADER_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"
#include "vmime/exception.hpp"

#include "vmime/headerField.hpp"
#include "vmime/headerFieldFactory.hpp"


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

#define FIELD_ACCESS(methodName, fieldName) \
	ref <headerField> methodName() { return getField(fields::fieldName); } \
	ref <const headerField> methodName() const { return findField(fields::fieldName); }

	FIELD_ACCESS(From,                         FROM)
	FIELD_ACCESS(Sender,                       SENDER)
	FIELD_ACCESS(ReplyTo,                      REPLY_TO)
	FIELD_ACCESS(DeliveredTo,                  DELIVERED_TO)
	FIELD_ACCESS(InReplyTo,                    IN_REPLY_TO)
	FIELD_ACCESS(ReturnPath,                   RETURN_PATH)
	FIELD_ACCESS(References,                   REFERENCES)

	FIELD_ACCESS(To,                           TO)
	FIELD_ACCESS(Cc,                           CC)
	FIELD_ACCESS(Bcc,                          BCC)
	FIELD_ACCESS(Date,                         DATE)
	FIELD_ACCESS(Subject,                      SUBJECT)
	FIELD_ACCESS(Organization,                 ORGANIZATION)
	FIELD_ACCESS(UserAgent,                    USER_AGENT)

	FIELD_ACCESS(ContentType,                  CONTENT_TYPE)
	FIELD_ACCESS(ContentDescription,           CONTENT_DESCRIPTION)
	FIELD_ACCESS(ContentTransferEncoding,      CONTENT_TRANSFER_ENCODING)
	FIELD_ACCESS(MimeVersion,                  MIME_VERSION)
	FIELD_ACCESS(ContentDisposition,           CONTENT_DISPOSITION)
	FIELD_ACCESS(ContentId,                    CONTENT_ID)
	FIELD_ACCESS(MessageId,                    MESSAGE_ID)
	FIELD_ACCESS(ContentLocation,              CONTENT_LOCATION)

	FIELD_ACCESS(OriginalMessageId,            ORIGINAL_MESSAGE_ID)
	FIELD_ACCESS(Disposition,                  DISPOSITION)
	FIELD_ACCESS(DispositionNotificationTo,    DISPOSITION_NOTIFICATION_TO)

#undef FIELD_ACCESS

	/** Checks whether (at least) one field with this name exists.
	  *
	  * @return true if at least one field with the specified name
	  * exists, or false otherwise
	  */
	bool hasField(const string& fieldName) const;

	/** Find the first field that matches the specified name.
	  * If no field is found, an exception is thrown.
	  *
	  * @throw exceptions::no_such_field if no field with this name exists
	  * @return first field with the specified name
	  */
	ref <headerField> findField(const string& fieldName) const;

	/** Find all fields that match the specified name.
	  * If no field is found, an empty vector is returned.
	  *
	  * @return list of fields with the specified name
	  */
	std::vector <ref <headerField> > findAllFields(const string& fieldName);

	/** Find the first field that matches the specified name.
	  * If no field is found, one will be created and inserted into
	  * the header.
	  *
	  * @return first field with the specified name or a new field
	  * if no field is found
	  */
	ref <headerField> getField(const string& fieldName);

	/** Add a field at the end of the list.
	  *
	  * @param field field to append
	  */
	void appendField(ref <headerField> field);

	/** Insert a new field before the specified field.
	  *
	  * @param beforeField field before which the new field will be inserted
	  * @param field field to insert
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void insertFieldBefore(ref <headerField> beforeField, ref <headerField> field);

	/** Insert a new field before the specified position.
	  *
	  * @param pos position at which to insert the new field (0 to insert at
	  * the beginning of the list)
	  * @param field field to insert
	  */
	void insertFieldBefore(const int pos, ref <headerField> field);

	/** Insert a new field after the specified field.
	  *
	  * @param afterField field after which the new field will be inserted
	  * @param field field to insert
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void insertFieldAfter(ref <headerField> afterField, ref <headerField> field);

	/** Insert a new field after the specified position.
	  *
	  * @param pos position of the field before the new field
	  * @param field field to insert
	  */
	void insertFieldAfter(const int pos, ref <headerField> field);

	/** Remove the specified field from the list.
	  *
	  * @param field field to remove
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void removeField(ref <headerField> field);

	/** Remove the field at the specified position.
	  *
	  * @param pos position of the field to remove
	  */
	void removeField(const int pos);

	/** Remove all fields from the list.
	  */
	void removeAllFields();

	/** Remove all fields with the specified name.
	  */
	void removeAllFields(const string& fieldName);

	/** Return the number of fields in the list.
	  *
	  * @return number of fields
	  */
	int getFieldCount() const;

	/** Tests whether the list of fields is empty.
	  *
	  * @return true if there is no field, false otherwise
	  */
	bool isEmpty() const;

	/** Return the field at the specified position.
	  *
	  * @param pos position
	  * @return field at position 'pos'
	  */
	const ref <headerField> getFieldAt(const int pos);

	/** Return the field at the specified position.
	  *
	  * @param pos position
	  * @return field at position 'pos'
	  */
	const ref <const headerField> getFieldAt(const int pos) const;

	/** Return the field list.
	  *
	  * @return list of fields
	  */
	const std::vector <ref <const headerField> > getFieldList() const;

	/** Return the field list.
	  *
	  * @return list of fields
	  */
	const std::vector <ref <headerField> > getFieldList();

	ref <component> clone() const;
	void copyFrom(const component& other);
	header& operator=(const header& other);

	const std::vector <ref <component> > getChildComponents();

private:

	std::vector <ref <headerField> > m_fields;


	class fieldHasName
	{
	public:

		fieldHasName(const string& name);
		bool operator() (const ref <const headerField>& field);

	private:

		string m_name;
	};

	class fieldHasNotName
	{
	public:

		fieldHasNotName(const string& name);
		bool operator() (const ref <const headerField>& field);

	private:

		string m_name;
	};

protected:

	// Component parsing & assembling
	void parseImpl
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition = NULL);

	void generateImpl
		(utility::outputStream& os,
		 const string::size_type maxLineLength = lineLengthLimits::infinite,
		 const string::size_type curLinePos = 0,
		 string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_HEADER_HPP_INCLUDED
