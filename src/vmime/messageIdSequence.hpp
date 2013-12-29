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

#ifndef VMIME_MESSAGEIDSEQUENCE_HPP_INCLUDED
#define VMIME_MESSAGEIDSEQUENCE_HPP_INCLUDED


#include "vmime/messageId.hpp"


namespace vmime
{


/** A list of message identifiers (basic type).
  */

class VMIME_EXPORT messageIdSequence : public headerFieldValue
{
public:

	messageIdSequence();
	messageIdSequence(const messageIdSequence& midSeq);

	~messageIdSequence();


	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	messageIdSequence& operator=(const messageIdSequence& other);

	const std::vector <shared_ptr <component> > getChildComponents();


	/** Add a message-id at the end of the list.
	  *
	  * @param mid message-id to append
	  */
	void appendMessageId(shared_ptr <messageId> mid);

	/** Insert a new message-id before the specified message-id.
	  *
	  * @param beforeMid message-id before which the new message-id will be inserted
	  * @param mid message-id to insert
	  * @throw exceptions::no_such_messageid if the message-id is not in the list
	  */
	void insertMessageIdBefore(shared_ptr <messageId> beforeMid, shared_ptr <messageId> mid);

	/** Insert a new message-id before the specified position.
	  *
	  * @param pos position at which to insert the new message-id (0 to insert at
	  * the beginning of the list)
	  * @param mid message-id to insert
	  */
	void insertMessageIdBefore(const size_t pos, shared_ptr <messageId> mid);

	/** Insert a new message-id after the specified message-id.
	  *
	  * @param afterMid message-id after which the new message-id will be inserted
	  * @param mid message-id to insert
	  * @throw exceptions::no_such_message_id if the message-id is not in the list
	  */
	void insertMessageIdAfter(shared_ptr <messageId> afterMid, shared_ptr <messageId> mid);

	/** Insert a new message-id after the specified position.
	  *
	  * @param pos position of the message-id before the new message-id
	  * @param mid message-id to insert
	  */
	void insertMessageIdAfter(const size_t pos, shared_ptr <messageId> mid);

	/** Remove the specified message-id from the list.
	  *
	  * @param mid message-id to remove
	  * @throw exceptions::no_such_message_id if the message-id is not in the list
	  */
	void removeMessageId(shared_ptr <messageId> mid);

	/** Remove the message-id at the specified position.
	  *
	  * @param pos position of the message-id to remove
	  */
	void removeMessageId(const size_t pos);

	/** Remove all message-ids from the list.
	  */
	void removeAllMessageIds();

	/** Return the number of message-ides in the list.
	  *
	  * @return number of message-ides
	  */
	size_t getMessageIdCount() const;

	/** Tests whether the list of message-ides is empty.
	  *
	  * @return true if there is no message-id, false otherwise
	  */
	bool isEmpty() const;

	/** Return the message-id at the specified position.
	  *
	  * @param pos position
	  * @return message-id at position 'pos'
	  */
	const shared_ptr <messageId> getMessageIdAt(const size_t pos);

	/** Return the message-id at the specified position.
	  *
	  * @param pos position
	  * @return message-id at position 'pos'
	  */
	const shared_ptr <const messageId> getMessageIdAt(const size_t pos) const;

	/** Return the message-id list.
	  *
	  * @return list of message-ids
	  */
	const std::vector <shared_ptr <const messageId> > getMessageIdList() const;

	/** Return the message-id list.
	  *
	  * @return list of message-ids
	  */
	const std::vector <shared_ptr <messageId> > getMessageIdList();

private:

	std::vector <shared_ptr <messageId> > m_list;

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


#endif // VMIME_MESSAGEIDSEQUENCE_HPP_INCLUDED
