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

#ifndef VMIME_MEDIATYPE_HPP_INCLUDED
#define VMIME_MEDIATYPE_HPP_INCLUDED


#include "base.hpp"
#include "component.hpp"


namespace vmime
{


/** Content media type (basic type).
  */

class mediaType : public component
{
public:

	mediaType();
	mediaType(const string& type);
	mediaType(const string& type, const string& subType);

public:

	const bool operator==(const mediaType& type) const;
	const bool operator!=(const mediaType& type) const;

	mediaType& operator=(const string& type);

	mediaType* clone() const;
	void copyFrom(const component& other);
	mediaType& operator=(const mediaType& other);

	const std::vector <const component*> getChildComponents() const;

	/** Return the media type.
	  * See the constants in vmime::mediaTypes.
	  *
	  * @return media type
	  */
	const string& getType() const;

	/** Set the media type.
	  * See the constants in vmime::mediaTypes.
	  *
	  * @param type media type
	  */
	void setType(const string& type);

	/** Return the media subtype.
	  * See the constants in vmime::mediaTypes.
	  *
	  * @return media subtype
	  */
	const string& getSubType() const;

	/** Set the media subtype.
	  * See the constants in vmime::mediaTypes.
	  *
	  * @param subType media subtype
	  */
	void setSubType(const string& subType);

	/** Set the media type and subtype from a string
	  * in the form "type/subtype" (eg: "image/jpeg").
	  *
	  * @param type media type and subtype
	  */
	void setFromString(const string& type);

protected:

	string m_type;
	string m_subType;

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_MEDIATYPE_HPP_INCLUDED
