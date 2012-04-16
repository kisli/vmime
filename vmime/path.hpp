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

#ifndef VMIME_PATH_HPP_INCLUDED
#define VMIME_PATH_HPP_INCLUDED


#include "vmime/headerFieldValue.hpp"


namespace vmime
{


/** A path: a local part + '@' + a domain.
  */

class path : public headerFieldValue
{
public:

	path();
	path(const string& localPart, const string& domain);
	path(const path& p);

	/** Return the local part of the address.
	  *
	  * @return local part of the address
	  */
	const string& getLocalPart() const;

	/** Set the local part of the address.
	  *
	  * @param localPart local part of the address
	  */
	void setLocalPart(const string& localPart);

	/** Return the domain of the address.
	  *
	  * @return domain of the address
	  */
	const string& getDomain() const;

	/** Set the domain of the address.
	  *
	  * @param domain domain of the address
	  */
	void setDomain(const string& domain);

	// Comparison
	bool operator==(const path& p) const;
	bool operator!=(const path& p) const;

	// Assignment
	void copyFrom(const component& other);
	ref <component> clone() const;
	path& operator=(const path& other);

	const std::vector <ref <component> > getChildComponents();

protected:

	string m_localPart;
	string m_domain;


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


#endif // VMIME_PATH_HPP_INCLUDED
