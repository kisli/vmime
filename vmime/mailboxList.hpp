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

#ifndef VMIME_MAILBOXLIST_HPP_INCLUDED
#define VMIME_MAILBOXLIST_HPP_INCLUDED


#include "vmime/addressList.hpp"
#include "vmime/mailbox.hpp"


namespace vmime
{


/** A list of mailboxes (basic type).
  */

#ifdef __GNUC__
#	if (__GNUC__ >= 3) && (__GNUC_MINOR__ <= 2)
class mailboxList : public addressList  // BUG with gcc <= 3.2
#	else
class mailboxList : protected addressList
#	endif
#else
class mailboxList : protected addressList
#endif
{
	friend class mailboxGroup;

public:

	// This class works exactly like 'addressList' except it prevents user
	// from inserting mailbox groups where it is not allowed by the RFC.

	mailboxList();
	mailboxList(const mailboxList& mboxList);

	/** Add a mailbox at the end of the list.
	  *
	  * @param mbox mailbox to append
	  */
	void appendMailbox(mailbox* mbox);

	/** Insert a new mailbox before the specified mailbox.
	  *
	  * @param beforeMailbox mailbox before which the new mailbox will be inserted
	  * @param mbox mailbox to insert
	  * @throw exceptions::no_such_mailbox if the mailbox is not in the list
	  */
	void insertMailboxBefore(mailbox* beforeMailbox, mailbox* mbox);

	/** Insert a new mailbox before the specified position.
	  *
	  * @param pos position at which to insert the new mailbox (0 to insert at
	  * the beginning of the list)
	  * @param mbox mailbox to insert
	  */
	void insertMailboxBefore(const int pos, mailbox* mbox);

	/** Insert a new mailbox after the specified mailbox.
	  *
	  * @param afterMailbox mailbox after which the new mailbox will be inserted
	  * @param mbox mailbox to insert
	  * @throw exceptions::no_such_mailbox if the mailbox is not in the list
	  */
	void insertMailboxAfter(mailbox* afterMailbox, mailbox* mbox);

	/** Insert a new mailbox after the specified position.
	  *
	  * @param pos position of the mailbox before the new mailbox
	  * @param mbox mailbox to insert
	  */
	void insertMailboxAfter(const int pos, mailbox* mbox);

	/** Remove the specified mailbox from the list.
	  *
	  * @param mbox mailbox to remove
	  * @throw exceptions::no_such_mailbox if the mailbox is not in the list
	  */
	void removeMailbox(mailbox* mbox);

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
	const int getMailboxCount() const;

	/** Tests whether the list of mailboxes is empty.
	  *
	  * @return true if there is no mailbox, false otherwise
	  */
	const bool isEmpty() const;

	/** Return the mailbox at the specified position.
	  *
	  * @param pos position
	  * @return mailbox at position 'pos'
	  */
	mailbox* getMailboxAt(const int pos);

	/** Return the mailbox at the specified position.
	  *
	  * @param pos position
	  * @return mailbox at position 'pos'
	  */
	const mailbox* const getMailboxAt(const int pos) const;

	/** Return the mailbox list.
	  *
	  * @return list of mailboxes
	  */
	const std::vector <const mailbox*> getMailboxList() const;

	/** Return the mailbox list.
	  *
	  * @return list of mailboxes
	  */
	const std::vector <mailbox*> getMailboxList();
};


} // vmime


#endif // VMIME_MAILBOXLIST_HPP_INCLUDED
