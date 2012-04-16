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

#ifndef VMIME_CONTENTDISPOSITION_HPP_INCLUDED
#define VMIME_CONTENTDISPOSITION_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/headerFieldValue.hpp"


namespace vmime
{


/** Content disposition (basic type).
  */

class contentDisposition : public headerFieldValue
{
public:

	contentDisposition();
	contentDisposition(const string& name);
	contentDisposition(const contentDisposition& disp);


	/** Return the content disposition type.
	  * See the constants in vmime::dispositionTypes.
	  *
	  * @return name of the disposition type (eg. "inline")
	  */
	const string& getName() const;

	/** Set the content disposition type.
	  * See the constants in vmime::dispositionTypes.
	  *
	  * @param name name of the disposition type
	  */
	void setName(const string& name);

	ref <component> clone() const;
	void copyFrom(const component& other);
	contentDisposition& operator=(const contentDisposition& other);

	const std::vector <ref <component> > getChildComponents();


	contentDisposition& operator=(const string& name);

	bool operator==(const contentDisposition& value) const;
	bool operator!=(const contentDisposition& value) const;

private:

	string m_name;

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


#endif // VMIME_CONTENTDISPOSITION_HPP_INCLUDED
