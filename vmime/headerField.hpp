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

#ifndef VMIME_HEADERFIELD_HPP_INCLUDED
#define VMIME_HEADERFIELD_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"
#include "vmime/headerFieldValue.hpp"


namespace vmime
{


/** Base class for header fields.
  */

class VMIME_EXPORT headerField : public component
{
	friend class headerFieldFactory;
	friend class header;

protected:

	// Protected constructor to prevent the user from creating
	// new objects without using 'headerFieldFactory'
	headerField();
	headerField(const string& fieldName);

public:

	~headerField();

	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	headerField& operator=(const headerField& other);

	const std::vector <shared_ptr <component> > getChildComponents();

	/** Sets the name of this field.
	  *
	  * @param name field name (eg: "From" or "X-MyField").
	  */
	void setName(const string& name);

	/** Return the name of this field.
	  *
	  * @return field name
	  */
	const string getName() const;

	/** Check whether this field is a custom (non-standard) field.
	  * Custom fields have a name beginning with "X-".
	  *
	  * @return true if the field is a custom field, false otherwise
	  */
	bool isCustom() const;

	/** Return the read-only value object attached to this field.
	  *
	  * @return read-only value object
	  */
	virtual shared_ptr <const headerFieldValue> getValue() const;

	/** Return the read-only value object attached to this field,
	  * casted to the specified type.
	  *
	  * @return value object
	  */
	template <typename T>
	shared_ptr <const T> getValue() const
	{
		return dynamicCast <const T>(m_value);
	}

	/** Return the value object attached to this field.
	  *
	  * @return value object
	  */
	virtual shared_ptr <headerFieldValue> getValue();

	/** Return the value object attached to this field,
	  * casted to the specified type.
	  *
	  * @return value object
	  */
	template <typename T>
	shared_ptr <T> getValue()
	{
		return dynamicCast <T>(m_value);
	}

	/** Set the value of this field.
	  *
	  * @throw exceptions::bad_field_value_type if the value type is not
	  * valid for this header field
	  * @param value new value
	  */
	virtual void setValue(shared_ptr <headerFieldValue> value);

	/** Set the value of this field by cloning the specified value.
	  *
	  * @throw exceptions::bad_field_value_type if the value type is not
	  * valid for this header field
	  * @param value new value
	  */
	virtual void setValueConst(shared_ptr <const headerFieldValue> value);

	/** Set the value of this field (reference version).
	  * The value will be cloned.
	  *
	  * @throw exceptions::bad_field_value_type if the value type is not
	  * valid for this header field
	  * @param value new value
	  */
	virtual void setValue(const headerFieldValue& value);

	/** Set the value of this field given a character string.
	  *
	  * @param value value string to parse
	  */
	void setValue(const string& value);


	/** Parse a header field from a buffer.
	  *
	  * @param ctx parsing context
	  * @param buffer input buffer
	  * @param position current position in the input buffer
	  * @param end end position in the input buffer
	  * @param newPosition will receive the new position in the input buffer
	  * @return parsed header field, or NULL if no more header field can be parsed
	  * in the input buffer
	  */
	static shared_ptr <headerField> parseNext
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition = NULL);

	size_t getGeneratedSize(const generationContext& ctx);

protected:

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


	string m_name;
	shared_ptr <headerFieldValue> m_value;
};


} // vmime


#endif // VMIME_HEADERFIELD_HPP_INCLUDED
