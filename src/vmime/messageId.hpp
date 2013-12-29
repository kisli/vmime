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

#ifndef VMIME_MESSAGEID_HPP_INCLUDED
#define VMIME_MESSAGEID_HPP_INCLUDED


#include "vmime/base.hpp"
#include "vmime/headerFieldValue.hpp"


namespace vmime
{


/** Message identifier (basic type).
  */

class VMIME_EXPORT messageId : public headerFieldValue
{
	friend class messageIdSequence;

public:

	messageId();
	messageId(const string& id);
	messageId(const messageId& mid);
	messageId(const string& left, const string& right);

public:

	/** Return the left part of the message identifier.
	  *
	  * @return left part of message identifier
	  */
	const string& getLeft() const;

	/** Set the left part of the message identifier.
	  *
	  * @param left left part of message identifier
	  */
	void setLeft(const string& left);

	/** Return the right part of the message identifier.
	  *
	  * @return right part of message identifier
	  */
	const string& getRight() const;

	/** Set the right part of the message identifier.
	  *
	  * @param right right part of message identifier
	  */
	void setRight(const string& right);


	messageId& operator=(const string& id);

	bool operator==(const messageId& mid) const;
	bool operator!=(const messageId& mid) const;

	/** Generate a random message identifier.
	  *
	  * @return randomly created message identifier
	  */
	static messageId generateId();

	/** Return the message identifier constructed by using
	  * the right part and the left part, separated by
	  * a '@' character.
	  *
	  * @return full message identifier
	  */
	const string getId() const;

	shared_ptr <component> clone() const;
	void copyFrom(const component& other);
	messageId& operator=(const messageId& other);

	const std::vector <shared_ptr <component> > getChildComponents();

private:

	string m_left;
	string m_right;

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

	/** Parse a message-id from an input buffer.
	  *
	  * @param buffer input buffer
	  * @param position position in the input buffer
	  * @param end end position in the input buffer
	  * @param newPosition will receive the new position in the input buffer
	  * @return a new message-id object, or null if no more message-id can be parsed from the input buffer
	  */
	static shared_ptr <messageId> parseNext
		(const parsingContext& ctx,
		 const string& buffer,
		 const size_t position,
		 const size_t end,
		 size_t* newPosition);
};


} // vmime


#endif // VMIME_MESSAGEID_HPP_INCLUDED
