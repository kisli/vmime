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

#ifndef VMIME_MAILBOX_HPP_INCLUDED
#define VMIME_MAILBOX_HPP_INCLUDED


#include "address.hpp"
#include "text.hpp"


namespace vmime
{


/** A mailbox: full name + email (basic type).
  */

class mailbox : public address
{
	friend class mailboxGroup;
	friend class mailboxField;

public:

	mailbox();
	mailbox(const mailbox& mbox);
	mailbox(const string& email);
	mailbox(const text& name, const string& email);

	/** Return the full name of the mailbox (empty if not specified).
	  *
	  * @return full name of the mailbox
	  */
	const text& getName() const;

	/** Set the full name of the mailbox.
	  *
	  * @return full name of the mailbox
	  */
	void setName(const text& name);

	/** Return the email of the mailbox.
	  *
	  * @return email of the mailbox
	  */
	const string& getEmail() const;

	/** Set the email of the mailbox.
	  *
	  * @return email of the mailbox
	  */
	void setEmail(const string& email);

	// Comparison
	const bool operator==(const class mailbox& mailbox) const;
	const bool operator!=(const class mailbox& mailbox) const;

	// Assignment
	void copyFrom(const component& other);
	mailbox* clone() const;
	mailbox& operator=(const mailbox& other);

	const bool isEmpty() const;

	void clear();

	const std::vector <const component*> getChildComponents() const;


	const bool isGroup() const;

protected:

	text m_name;
	string m_email;

public:

	using address::parse;
	using address::generate;

	// Component parsing & assembling
	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
	void generate(utility::outputStream& os, const string::size_type maxLineLength = lineLengthLimits::infinite, const string::size_type curLinePos = 0, string::size_type* newLinePos = NULL) const;
};


} // vmime


#endif // VMIME_MAILBOX_HPP_INCLUDED
