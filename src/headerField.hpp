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

#ifndef VMIME_HEADERFIELD_HPP_INCLUDED
#define VMIME_HEADERFIELD_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"


namespace vmime
{


/** Base class for header fields.
  */

class headerField : public component
{
	friend class headerFieldFactory;

protected:

	headerField();
	headerField(const string& fieldName);

public:

	~headerField();

	headerField* clone() const;
	void copyFrom(const component& other);
	headerField& operator=(const headerField& other);

	const std::vector <const component*> getChildComponents() const;

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
	const bool isCustom() const;

	/** Return the read-only value object attached to this field.
	  *
	  * @return read-only value object
	  */
	virtual const component& getValue() const = 0;

	/** Return the value object attached to this field.
	  *
	  * @return value object
	  */
	virtual component& getValue() = 0;

	/** Set the value of this field.
	  *
	  * @throw std::bad_cast_exception if the value type is
	  * incompatible with the header field type
	  * @param value value object
	  */
	virtual void setValue(const component& value) = 0;


	using component::parse;
	using component::generate;

	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;

private:

	string m_name;
};


} // vmime


#endif // VMIME_HEADERFIELD_HPP_INCLUDED
