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

#ifndef VMIME_MAILBOXGROUP_HPP_INCLUDED
#define VMIME_MAILBOXGROUP_HPP_INCLUDED


#include "vmime/address.hpp"
#include "vmime/mailbox.hpp"
#include "vmime/text.hpp"


namespace vmime
{


/** A group of mailboxes (basic type).
  */

class VMIME_EXPORT mailboxGroup : public address
{
public:

	mailboxGroup();
	mailboxGroup(const mailboxGroup& mboxGroup);
	mailboxGroup(const text& name);

	~mailboxGroup();


	void copyFrom(const component& other);
	shared_ptr <component> clone() const;
	mailboxGroup& operator=(const component& other);

	const std::vector <shared_ptr <component> > getChildComponents();

	/** Return the name of the group.
	  *
	  * @return group name
	  */
	const text& getName() const;

	/** Set the name of the group.
	  *
	  * @param name group name
	  */
	void setName(const text& name);

	/** Add a mailbox at the end of the list.
	  *
	  * @param mbox mailbox to append
	  */
	void appendMailbox(shared_ptr <mailbox> mbox);

	/** Insert a new mailbox before the specified mailbox.
	  *
	  * @param beforeMailbox mailbox before which the new mailbox will be inserted
	  * @param mbox mailbox to insert
	  * @throw std::out_of_range if the mailbox is not in the list
	  */
	void insertMailboxBefore(shared_ptr <mailbox> beforeMailbox, shared_ptr <mailbox> mbox);

	/** Insert a new mailbox before the specified position.
	  *
	  * @param pos position at which to insert the new mailbox (0 to insert at
	  * the beginning of the list)
	  * @param mbox mailbox to insert
	  * @throw std::out_of_range if the position is out of range
	  */
	void insertMailboxBefore(const size_t pos, shared_ptr <mailbox> mbox);

	/** Insert a new mailbox after the specified mailbox.
	  *
	  * @param afterMailbox mailbox after which the new mailbox will be inserted
	  * @param mbox mailbox to insert
	  * @throw std::out_of_range if the mailbox is not in the list
	  */
	void insertMailboxAfter(shared_ptr <mailbox> afterMailbox, shared_ptr <mailbox> mbox);

	/** Insert a new mailbox after the specified position.
	  *
	  * @param pos position of the mailbox before the new mailbox
	  * @param mbox mailbox to insert
	  * @throw std::out_of_range if the position is out of range
	  */
	void insertMailboxAfter(const size_t pos, shared_ptr <mailbox> mbox);

	/** Remove the specified mailbox from the list.
	  *
	  * @param mbox mailbox to remove
	  * @throw std::out_of_range if the mailbox is not in the list
	  */
	void removeMailbox(shared_ptr <mailbox> mbox);

	/** Remove the mailbox at the specified position.
	  *
	  * @param pos position of the mailbox to remove
	  * @throw std::out_of_range if the position is out of range
	  */
	void removeMailbox(const size_t pos);

	/** Remove all mailboxes from the list.
	  */
	void removeAllMailboxes();

	/** Return the number of mailboxes in the list.
	  *
	  * @return number of mailboxes
	  */
	size_t getMailboxCount() const;

	/** Tests whether the list of mailboxes is empty.
	  *
	  * @return true if there is no mailbox, false otherwise
	  */
	bool isEmpty() const;

	/** Return the mailbox at the specified position.
	  *
	  * @param pos position
	  * @return mailbox at position 'pos'
	  * @throw std::out_of_range if the position is out of range
	  */
	shared_ptr <mailbox> getMailboxAt(const size_t pos);

	/** Return the mailbox at the specified position.
	  *
	  * @param pos position
	  * @return mailbox at position 'pos'
	  * @throw std::out_of_range if the position is out of range
	  */
	const shared_ptr <const mailbox> getMailboxAt(const size_t pos) const;

	/** Return the mailbox list.
	  *
	  * @return list of mailboxes
	  */
	const std::vector <shared_ptr <const mailbox> > getMailboxList() const;

	/** Return the mailbox list.
	  *
	  * @return list of mailboxes
	  */
	const std::vector <shared_ptr <mailbox> > getMailboxList();

	bool isGroup() const;

private:

	text m_name;
	std::vector <shared_ptr <mailbox> > m_list;

protected:

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


#endif // VMIME_MAILBOXGROUP_HPP_INCLUDED
