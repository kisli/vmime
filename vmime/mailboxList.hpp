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

#ifndef VMIME_MAILBOXLIST_HPP_INCLUDED
#define VMIME_MAILBOXLIST_HPP_INCLUDED


#include "vmime/addressList.hpp"
#include "vmime/mailbox.hpp"


namespace vmime
{


/** A list of mailboxes (basic type).
  *
  * This class works exactly like 'addressList' except it prevents user
  * from inserting mailbox groups where it is not allowed by the RFC.
  */

class mailboxList : public headerFieldValue
{
public:

	mailboxList();
	mailboxList(const mailboxList& mboxList);


	ref <component> clone() const;
	void copyFrom(const component& other);
	mailboxList& operator=(const mailboxList& other);

	const std::vector <ref <component> > getChildComponents();

	/** Add a mailbox at the end of the list.
	  *
	  * @param mbox mailbox to append
	  */
	void appendMailbox(ref <mailbox> mbox);

	/** Insert a new mailbox before the specified mailbox.
	  *
	  * @param beforeMailbox mailbox before which the new mailbox will be inserted
	  * @param mbox mailbox to insert
	  * @throw exceptions::no_such_mailbox if the mailbox is not in the list
	  */
	void insertMailboxBefore(ref <mailbox> beforeMailbox, ref <mailbox> mbox);

	/** Insert a new mailbox before the specified position.
	  *
	  * @param pos position at which to insert the new mailbox (0 to insert at
	  * the beginning of the list)
	  * @param mbox mailbox to insert
	  */
	void insertMailboxBefore(const int pos, ref <mailbox> mbox);

	/** Insert a new mailbox after the specified mailbox.
	  *
	  * @param afterMailbox mailbox after which the new mailbox will be inserted
	  * @param mbox mailbox to insert
	  * @throw exceptions::no_such_mailbox if the mailbox is not in the list
	  */
	void insertMailboxAfter(ref <mailbox> afterMailbox, ref <mailbox> mbox);

	/** Insert a new mailbox after the specified position.
	  *
	  * @param pos position of the mailbox before the new mailbox
	  * @param mbox mailbox to insert
	  */
	void insertMailboxAfter(const int pos, ref <mailbox> mbox);

	/** Remove the specified mailbox from the list.
	  *
	  * @param mbox mailbox to remove
	  * @throw exceptions::no_such_mailbox if the mailbox is not in the list
	  */
	void removeMailbox(ref <mailbox> mbox);

	/** Remove the mailbox at the specified position.
	  *
	  * @param pos position of the mailbox to remove
	  */
	void removeMailbox(const int pos);

	/** Remove all mailboxes from the list.
	  */
	void removeAllMailboxes();

	/** Return the number of mailboxes in the list.
	  *
	  * @return number of mailboxes
	  */
	int getMailboxCount() const;

	/** Tests whether the list of mailboxes is empty.
	  *
	  * @return true if there is no mailbox, false otherwise
	  */
	bool isEmpty() const;

	/** Return the mailbox at the specified position.
	  *
	  * @param pos position
	  * @return mailbox at position 'pos'
	  */
	ref <mailbox> getMailboxAt(const int pos);

	/** Return the mailbox at the specified position.
	  *
	  * @param pos position
	  * @return mailbox at position 'pos'
	  */
	const ref <const mailbox> getMailboxAt(const int pos) const;

	/** Return the mailbox list.
	  *
	  * @return list of mailboxes
	  */
	const std::vector <ref <const mailbox> > getMailboxList() const;

	/** Return the mailbox list.
	  *
	  * @return list of mailboxes
	  */
	const std::vector <ref <mailbox> > getMailboxList();

	/** Return a list of addresses.
	  *
	  * @return list of addresses
	  */
	ref <addressList> toAddressList() const;

private:

	addressList m_list;

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


#endif // VMIME_MAILBOXLIST_HPP_INCLUDED
