//
// VMime library (http://vmime.sourceforge.net)
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

#ifndef VMIME_ADDRESS_HPP_INCLUDED
#define VMIME_ADDRESS_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/component.hpp"


namespace vmime
{


/** Abstract class representing a mailbox or a group of mailboxes.
  *
  * This class define a common behaviour for the mailbox
  * and mailboxGroup classes.
  */

class address : public component
{
	friend class addressList;

protected:

	address();

public:

	/** Check whether this address is empty (no mailboxes specified
	  * if this is a mailboxGroup -or- no email specified if this is
	  * a mailbox).
	  *
	  * @return true if this address is empty
	  */
	virtual const bool isEmpty() const = 0;

	/** Test whether this is object is a mailboxGroup.
	  *
	  * @return true if this is a mailboxGroup, false otherwise
	  */
	virtual const bool isGroup() const = 0;

	virtual address* clone() const = 0;

protected:

	/** Parse an address from an input buffer.
	  *
	  * @param buffer input buffer
	  * @param position position in the input buffer
	  * @param end end position in the input buffer
	  * @param newPosition will receive the new position in the input buffer
	  * @return a new address object, or null if no more address is available in the input buffer
	  */
	static address* parseNext(const string& buffer, const string::size_type position, const string::size_type end, string::size_type* newPosition);
};


} // vmime


#endif // VMIME_ADDRESS_HPP_INCLUDED
