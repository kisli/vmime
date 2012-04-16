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

#ifndef VMIME_HEADERFIELD_HPP_INCLUDED
#define VMIME_HEADERFIELD_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"
#include "vmime/headerFieldValue.hpp"


namespace vmime
{


/** Base class for header fields.
  */

class headerField : public component
{
	friend class headerFieldFactory;
	friend class header;

	friend class vmime::creator;  // create ref

protected:

	// Protected constructor to prevent the user from creating
	// new objects without using 'headerFieldFactory'
	headerField();
	headerField(const string& fieldName);

public:

	~headerField();

	ref <component> clone() const;
	void copyFrom(const component& other);
	headerField& operator=(const headerField& other);

	const std::vector <ref <component> > getChildComponents();

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
	virtual ref <const headerFieldValue> getValue() const;

	/** Return the value object attached to this field.
	  *
	  * @return value object
	  */
	virtual ref <headerFieldValue> getValue();

	/** Set the value of this field.
	  *
	  * @param value new value
	  */
	virtual void setValue(ref <headerFieldValue> value);

	/** Set the value of this field by cloning the specified value.
	  *
	  * @param value new value
	  */
	virtual void setValueConst(ref <const headerFieldValue> value);

	/** Set the value of this field (reference version).
	  * The value will be cloned.
	  *
	  * @param value new value
	  */
	virtual void setValue(const headerFieldValue& value);

	/** Set the value of this field given a character string.
	  *
	  * @param value value string to parse
	  */
	void setValue(const string& value);


protected:

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


	static ref <headerField> parseNext
		(const string& buffer,
		 const string::size_type position,
		 const string::size_type end,
		 string::size_type* newPosition = NULL);


	string m_name;
	ref <headerFieldValue> m_value;
};


} // vmime


#endif // VMIME_HEADERFIELD_HPP_INCLUDED
