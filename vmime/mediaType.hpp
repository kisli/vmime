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

#ifndef VMIME_MEDIATYPE_HPP_INCLUDED
#define VMIME_MEDIATYPE_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/headerFieldValue.hpp"


namespace vmime
{


/** Content media type (basic type).
  */

class VMIME_EXPORT mediaType : public headerFieldValue
{
public:

	mediaType();
	mediaType(const string& type);
	mediaType(const string& type, const string& subType);

public:

	bool operator==(const mediaType& type) const;
	bool operator!=(const mediaType& type) const;

	mediaType& operator=(const string& type);

	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	mediaType& operator=(const mediaType& other);

	const std::vector <shared_ptr <component> > getChildComponents();

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


#endif // VMIME_MEDIATYPE_HPP_INCLUDED
