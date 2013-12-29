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

#ifndef VMIME_MAILBOX_HPP_INCLUDED
#define VMIME_MAILBOX_HPP_INCLUDED


#include "vmime/emailAddress.hpp"
#include "vmime/address.hpp"
#include "vmime/text.hpp"


namespace vmime
{


/** A mailbox: full name + email (basic type).
  */

class VMIME_EXPORT mailbox : public address
{
	friend class mailboxGroup;
	friend class mailboxField;

public:

	mailbox();
	mailbox(const mailbox& mbox);
	mailbox(const emailAddress& email);
	mailbox(const text& name, const emailAddress& email);

	/** Return the full name of the mailbox (empty if not specified).
	  *
	  * @return full name of the mailbox
	  */
	const text& getName() const;

	/** Set the full name of the mailbox.
	  *
	  * @param name full name of the mailbox
	  */
	void setName(const text& name);

	/** Return the email of the mailbox.
	  *
	  * @return email of the mailbox
	  */
	const emailAddress& getEmail() const;

	/** Set the email of the mailbox.
	  *
	  * @param email email of the mailbox
	  */
	void setEmail(const emailAddress& email);

	// Comparison
	bool operator==(const class mailbox& mailbox) const;
	bool operator!=(const class mailbox& mailbox) const;

	// Assignment
	void copyFrom(const component& other);
	shared_ptr <component> clone() const;
	mailbox& operator=(const mailbox& other);

	bool isEmpty() const;

	void clear();

	const std::vector <shared_ptr <component> > getChildComponents();


	bool isGroup() const;

protected:

	text m_name;
	emailAddress m_email;

public:

	using address::parse;
	using address::generate;

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


#endif // VMIME_MAILBOX_HPP_INCLUDED
