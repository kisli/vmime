//
// VMime library (http://www.vmime.org)
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

#ifndef VMIME_CONTENTDISPOSITION_HPP_INCLUDED
#define VMIME_CONTENTDISPOSITION_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"


namespace vmime
{


/** Content disposition (basic type).
  */

class contentDisposition : public component
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

	contentDisposition* clone() const;
	void copyFrom(const component& other);
	contentDisposition& operator=(const contentDisposition& other);

	const std::vector <const component*> getChildComponents() const;


	contentDisposition& operator=(const string& name);

	const bool operator==(const contentDisposition& value) const;
	const bool operator!=(const contentDisposition& value) const;

private:

	string m_name;

public:

	using component::parse;
	using component::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_CONTENTDISPOSITION_HPP_INCLUDED
