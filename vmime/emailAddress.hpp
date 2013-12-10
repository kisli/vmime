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

#ifndef VMIME_EMAILADDRESS_HPP_INCLUDED
#define VMIME_EMAILADDRESS_HPP_INCLUDED


#include "vmime/component.hpp"
#include "vmime/text.hpp"


namespace vmime
{


/** An email address: local name and domain name (basic type).
  */

class VMIME_EXPORT emailAddress : public component
{
public:

	emailAddress();
	emailAddress(const emailAddress& eml);
	emailAddress(const string& email);
	emailAddress(const char* email);
	emailAddress(const string& localName, const string& domainName);
	emailAddress(const word& localName, const word& domainName);

	/** Return the local name of the address.
	  *
	  * @return local name of the address
	  */
	const word& getLocalName() const;

	/** Set the local name of the address.
	  *
	  * @param localName local name of the address
	  */
	void setLocalName(const word& localName);

	/** Return the domain name of the address.
	  *
	  * @return domain name of the address
	  */
	const word& getDomainName() const;

	/** Set the domain name of the address.
	  *
	  * @param domainName domain name of the address
	  */
	void setDomainName(const word& domainName);

	/** Returns whether this email address is empty.
	  * Address is considered as empty if the local part is not specified.
	  *
	  * @return true if the address is empty, false otherwise
	  */
	bool isEmpty() const;

	/** Returns the email address as a string, by joining components.
	  * (ie. the local name, followed by a @ then the domain name.)
	  *
	  * @return email address as a string
	  */
	const string toString() const;

	/** Returns the email address as multibyte text, by joining components.
	  * (ie. the local name, followed by a @ then the domain name.)
	  *
	  * @return email address as multibyte text
	  */
	const text toText() const;

	// Comparison
	bool operator==(const class emailAddress& eml) const;
	bool operator!=(const class emailAddress& eml) const;

	// Assignment
	void copyFrom(const component& other);
	shared_ptr <component> clone() const;
	emailAddress& operator=(const emailAddress& other);

	const std::vector <shared_ptr <component> > getChildComponents();

protected:

	word m_localName;
	word m_domainName;

public:

	using component::parse;
	using component::generate;

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


#endif // VMIME_EMAILADDRESS_HPP_INCLUDED
