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

#ifndef VMIME_NET_MESSAGESET_HPP_INCLUDED
#define VMIME_NET_MESSAGESET_HPP_INCLUDED


#include "vmime/net/message.hpp"


namespace vmime {
namespace net {


// Forward references
class numberMessageRange;
class UIDMessageRange;


/** Enumerator used to retrieve the message number/UID ranges contained
  * in a messageSet object.
  */

class VMIME_EXPORT messageSetEnumerator
{
public:

	virtual void enumerateNumberMessageRange(const numberMessageRange& range) = 0;
	virtual void enumerateUIDMessageRange(const UIDMessageRange& range) = 0;
};


/** A range of (continuous) messages, designated either by their
  * sequence number, or by their UID.
  */

class VMIME_EXPORT messageRange : public object
{
public:

	virtual ~messageRange();

	/** Enumerates this range with the specified enumerator.
	  *
	  * @param en enumerator that will receive the method calls while
	  * enumerating this range
	  */
	virtual void enumerate(messageSetEnumerator& en) const = 0;

	/** Clones this message range.
	  */
	virtual messageRange* clone() const = 0;

protected:

	messageRange();
	messageRange(const messageRange&);
};


/** A range of (continuous) messages designated by their sequence number.
  */

class VMIME_EXPORT numberMessageRange : public messageRange
{
public:

	/** Constructs a message range containing a single message.
	  *
	  * @param number message number (numbering starts at 1, not 0)
	  */
	numberMessageRange(const int number);

	/** Constructs a message range for multiple messages.
	  *
	  * @param first number of the first message in the range (numbering
	  * starts at 1, not 0)
	  * @param last number of the last message in the range, or use the
	  * special value -1 to designate the last message in the folder
	  */
	numberMessageRange(const int first, const int last);

	/** Constructs a message range by copying from another range.
	  *
	  * @param other range to copy
	  */
	numberMessageRange(const numberMessageRange& other);

	/** Returns the number of the first message in the range.
	  *
	  * @return number of the first message
	  */
	int getFirst() const;

	/** Returns the number of the last message in the range, or -1
	  * to designate the last message in the folder
	  *
	  * @return number of the last message
	  */
	int getLast() const;

	void enumerate(messageSetEnumerator& en) const;

	messageRange* clone() const;

private:

	int m_first, m_last;
};


/** A range of (continuous) messages represented by their UID.
  */

class VMIME_EXPORT UIDMessageRange : public messageRange
{
public:

	/** Constructs a message range containing a single message.
	  *
	  * @param uid message UID
	  */
	UIDMessageRange(const message::uid& uid);

	/** Constructs a message range for multiple messages.
	  *
	  * @param first UID of the first message in the range
	  * @param last UID of the last message in the range, or use the
	  * special value '*' to designate the last message in the folder
	  */
	UIDMessageRange(const message::uid& first, const message::uid& last);

	/** Constructs a message range by copying from another range.
	  *
	  * @param other range to copy
	  */
	UIDMessageRange(const UIDMessageRange& other);

	/** Returns the UID of the first message in the range.
	  *
	  * @return UID of the first message
	  */
	const message::uid getFirst() const;

	/** Returns the UID of the last message in the range, or '*'
	  * to designate the last message in the folder
	  *
	  * @return UID of the last message
	  */
	const message::uid getLast() const;

	void enumerate(messageSetEnumerator& en) const;

	messageRange* clone() const;

private:

	message::uid m_first, m_last;
};


/** Represents a set of messages, designated either by their sequence
  * number, or by their UID (but not both).
  *
  * Following is example code to designate messages by their number:
  * \code{.cpp}
  *    // Designate a single message with sequence number 42
  *    vmime::net::messageSet::byNumber(42)
  *
  *    // Designate messages from sequence number 5 to sequence number 8 (including)
  *    vmime::net::messageSet::byNumber(5, 8)
  *
  *    // Designate all messages in the folder, starting from number 42
  *    vmime::net::messageSet::byNumber(42, -1)
  * \endcode
  * Or, to designate messages by their UID, use:
  * \code{.cpp}
  *    // Designate a single message with UID 1042
  *    vmime::net::messageSet::byUID(1042)
  *
  *    // Designate messages from UID 1000 to UID 1042 (including)
  *    vmime::net::messageSet::byUID(1000, 1042)
  *
  *    // Designate all messages in the folder, starting from UID 1000
  *    vmime::net::messageSet::byUID(1000, "*")
  * \endcode
  */

class VMIME_EXPORT messageSet : public object
{
public:

	~messageSet();

	messageSet(const messageSet& other);

	/** Constructs an empty set.
	  *
	  * @return new empty message set
	  */
	static messageSet empty();

	/** Constructs a new message set and initializes it with a single
	  * message represented by its sequence number.
	  *
	  * @param number message number (numbering starts at 1, not 0)
	  * @return new message set
	  */
	static messageSet byNumber(const int number);

	/** Constructs a new message set and initializes it with a range
	  * of messages represented by their sequence number.
	  *
	  * @param first number of the first message in the range (numbering
	  * starts at 1, not 0)
	  * @param last number of the last message in the range, or use the
	  * special value -1 to designate the last message in the folder
	  * @return new message set
	  */
	static messageSet byNumber(const int first, const int last);

	/** Constructs a new message set and initializes it with a possibly
	  * unsorted list of messages represented by their sequence number.
	  * Please note that numbering starts at 1, not 0.
	  *
	  * The function tries to group consecutive message numbers into
	  * ranges to reduce the size of the resulting set.
	  *
	  * For example, given the list "1,2,3,4,5,7,8,13,15,16,17" it will
	  * result in the following ranges: "1:5,7:8,13,15:17".
	  *
	  * @param numbers a vector containing numbers of the messages
	  * @return new message set
	  */
	static messageSet byNumber(const std::vector <int>& numbers);

	/** Constructs a new message set and initializes it with a single
	  * message represented by its UID.
	  *
	  * @param uid message UID
	  * @return new message set
	  */
	static messageSet byUID(const message::uid& uid);

	/** Constructs a new message set and initializes it with a range
	  * of messages represented by their sequence number.
	  *
	  * @param first UID of the first message in the range
	  * @param last UID of the last message in the range, or use the
	  * special value '*' to designate the last message in the folder
	  * @return new message set
	  */
	static messageSet byUID(const message::uid& first, const message::uid& last);

	/** Constructs a new message set and initializes it with a possibly
	  * unsorted list of messages represented by their UID.
	  *
	  * For UIDs that actually are numbers (this is the case for IMAP), the
	  * function tries to group consecutive UIDs into ranges to reduce the
	  * size of the resulting set.
	  *
	  * For example, given the list "1,2,3,4,5,7,8,13,15,16,17" it will
	  * result in the following ranges: "1:5,7:8,13,15:17".
	  *
	  * @param uids a vector containing UIDs of the messages
	  * @return new message set
	  */
	static messageSet byUID(const std::vector <message::uid>& uids);

	/** Adds the specified range to this set. The type of message range
	  * (either number or UID) must match the type of the ranges already
	  * contained in this set (ie. it's not possible to have a message
	  * set which contains both number ranges and UID ranges).
	  *
	  * @param range range to add
	  * @throw std::invalid_argument exception if the range type does
	  * not match the type of the ranges in this set
	  */
	void addRange(const messageRange& range);

	/** Enumerates this set with the specified enumerator.
	  *
	  * @param en enumerator that will receive the method calls while
	  * enumerating the ranges in this set
	  */
	void enumerate(messageSetEnumerator& en) const;

	/** Returns whether this set is empty (contains no range).
	  *
	  * @return true if this set is empty, or false otherwise
	  */
	bool isEmpty() const;

	/** Returns whether this set references messages by their sequence
	  * number.
	  *
	  * @return true if this set references messages by their sequence
	  * number, or false otherwise
	  */
	bool isNumberSet() const;

	/** Returns whether this set references messages by their UID.
	  *
	  * @return true if this set references messages by their UID,
	  * or false otherwise
	  */
	bool isUIDSet() const;

	/** Returns the number of ranges contained in this set.
	  *
	  * @return range count
	  */
	size_t getRangeCount() const;

	/** Returns the message range at the specified index.
	  *
	  * @param i range index (from 0 to getRangeCount())
	  * @return a reference to the message range at the specified index
	  */
	const messageRange& getRangeAt(const size_t i) const;

private:

	messageSet();

	std::vector <messageRange*> m_ranges;
};


} // net
} // vmime


#endif // VMIME_NET_MESSAGESET_HPP_INCLUDED
