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

#ifndef VMIME_MAILBOXFIELD_HPP_INCLUDED
#define VMIME_MAILBOXFIELD_HPP_INCLUDED


#include "vmime/headerField.hpp"
#include "vmime/mailbox.hpp"


// Hide implementation details from user
#ifndef VMIME_BUILDING_DOC


namespace vmime
{


/** Work-around for malformed header fields that are of type 'mailbox'
  * and contains multiple addresses.
  */

class mailboxField : public headerField
{
	friend class vmime::creator;  // create ref

protected:

	mailboxField();
	mailboxField(const mailboxField&);

public:

	void parse(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition = NULL);
};


#endif // VMIME_BUILDING_DOC


} // vmime


#endif // VMIME_MAILBOXFIELD_HPP_INCLUDED

