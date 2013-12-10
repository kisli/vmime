//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

class VMIME_EXPORT header : public component
{
	friend class bodyPart;
	friend class body;
	friend class message;

public:

	header();
	~header();

#define FIELD_ACCESS(methodName, fieldName) \
	shared_ptr <headerField> methodName() { return getField(fields::fieldName); } \
	shared_ptr <const headerField> methodName() const { return findField(fields::fieldName); }

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
	  * Field name is case-insensitive.
	  *
	  * @return true if at least one field with the specified name
	  * exists, or false otherwise
	  */
	bool hasField(const string& fieldName) const;

	/** Find the first field that matches the specified name.
	  * Field name is case-insensitive.
	  * If no field is found, NULL is returned.
	  *
	  * @param fieldName name of field to return (eg: "X-Mailer" or "From",
	  * common field names are available in the vmime::fields namespace)
	  * @return first field with the specified name, or NULL if no field
	  * with this name was found
	  */
	shared_ptr <headerField> findField(const string& fieldName) const;

	/** Find the first field that matches the specified name,
	  * casted to the specified field type. Field name is case-insensitive.
	  * If no field is found, or the field is not of the specified type,
	  * NULL is returned.
	  *
	  * @param fieldName name of field whose value is to be returned
	  * (eg: "X-Mailer" or "From", common field names are available in
	  * the vmime::fields namespace)
	  * @return first field with the specified name, or NULL if no field
	  * with this name was found
	  */
	template <typename T>
	shared_ptr <T> findField(const string& fieldName) const
	{
		return dynamicCast <T>(findField(fieldName));
	}

	/** Find the value of the first field that matches the specified name,
	  * casted to the specified value type. Field name is case-insensitive.
	  * If no field is found, or the field value is not of the specified
	  * type, NULL is returned.
	  *
	  * @param fieldName name of field to return (eg: "X-Mailer" or "From",
	  * common field names are available in the vmime::fields namespace)
	  * @return value of the first field with the specified name, or NULL
	  * if no field with this name was found, or the value is not of the
	  * specified type
	  */
	template <typename T>
	shared_ptr <T> findFieldValue(const string& fieldName) const
	{
		shared_ptr <headerField> field = findField(fieldName);

		if (field)
			return dynamicCast <T>(field->getValue());
		else
			return null;
	}

	/** Find all fields that match the specified name.
	  * If no field is found, an empty vector is returned.
	  *
	  * @param fieldName name of field to return (eg: "X-Mailer" or "From",
	  * common field names are available in the vmime::fields namespace)
	  * @return list of fields with the specified name
	  */
	std::vector <shared_ptr <headerField> > findAllFields(const string& fieldName);

	/** Find the first field that matches the specified name.
	  * If no field is found, one will be created and inserted into
	  * the header.
	  *
	  * @param fieldName name of field to return (eg: "X-Mailer" or "From",
	  * common field names are available in the vmime::fields namespace)
	  * @return first field with the specified name or a new field
	  * if no field is found
	  */
	shared_ptr <headerField> getField(const string& fieldName);

	/** Find the first field that matches the specified name,
	  * casted to the specified type.
	  * If no field is found, one will be created and inserted into
	  * the header.
	  *
	  * @return first field with the specified name or a new field
	  * if no field is found
	  */
	template <typename T>
	shared_ptr <T> getField(const string& fieldName)
	{
		return dynamicCast <T>(getField(fieldName));
	}

	/** Add a field at the end of the list.
	  *
	  * @param field field to append
	  */
	void appendField(shared_ptr <headerField> field);

	/** Insert a new field before the specified field.
	  *
	  * @param beforeField field before which the new field will be inserted
	  * @param field field to insert
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void insertFieldBefore(shared_ptr <headerField> beforeField, shared_ptr <headerField> field);

	/** Insert a new field before the specified position.
	  *
	  * @param pos position at which to insert the new field (0 to insert at
	  * the beginning of the list)
	  * @param field field to insert
	  */
	void insertFieldBefore(const size_t pos, shared_ptr <headerField> field);

	/** Insert a new field after the specified field.
	  *
	  * @param afterField field after which the new field will be inserted
	  * @param field field to insert
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void insertFieldAfter(shared_ptr <headerField> afterField, shared_ptr <headerField> field);

	/** Insert a new field after the specified position.
	  *
	  * @param pos position of the field before the new field
	  * @param field field to insert
	  */
	void insertFieldAfter(const size_t pos, shared_ptr <headerField> field);

	/** Remove the specified field from the list.
	  *
	  * @param field field to remove
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void removeField(shared_ptr <headerField> field);

	/** Remove the field at the specified position.
	  *
	  * @param pos position of the field to remove
	  */
	void removeField(const size_t pos);

	/** Replaces a field with another field.
	  *
	  * @param field field to be replaced
	  * @param newField field to replace with
	  * @throw exceptions::no_such_field if the field is not in the list
	  */
	void replaceField(shared_ptr <headerField> field, shared_ptr <headerField> newField);

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
	size_t getFieldCount() const;

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
	const shared_ptr <headerField> getFieldAt(const size_t pos);

	/** Return the field at the specified position.
	  *
	  * @param pos position
	  * @return field at position 'pos'
	  */
	const shared_ptr <const headerField> getFieldAt(const size_t pos) const;

	/** Return the field list.
	  *
	  * @return list of fields
	  */
	const std::vector <shared_ptr <const headerField> > getFieldList() const;

	/** Return the field list.
	  *
	  * @return list of fields
	  */
	const std::vector <shared_ptr <headerField> > getFieldList();

	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	header& operator=(const header& other);

	const std::vector <shared_ptr <component> > getChildComponents();

	size_t getGeneratedSize(const generationContext& ctx);

private:

	std::vector <shared_ptr <headerField> > m_fields;


	class fieldHasName
	{
	public:

		fieldHasName(const string& name);
		bool operator() (const shared_ptr <const headerField>& field);

	private:

		string m_name;
	};

	class fieldHasNotName
	{
	public:

		fieldHasNotName(const string& name);
		bool operator() (const shared_ptr <const headerField>& field);

	private:

		string m_name;
	};

protected:

	// Component parsing & assembling
	void parseImpl
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	void generateImpl
		(const generationContext& ctx,
		 utility::outputStream& os,
		 const size_t curLinePos = 0,
		 size_t* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_HEADER_HPP_INCLUDED
