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

#ifndef VMIME_NET_MESSAGE_HPP_INCLUDED
#define VMIME_NET_MESSAGE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/header.hpp"
#include "vmime/mediaType.hpp"

#include "vmime/utility/progressListener.hpp"
#include "vmime/utility/stream.hpp"

#include "vmime/message.hpp"


namespace vmime {
namespace net {


class messageStructure;


/** A MIME part in a message.
  */

class VMIME_EXPORT messagePart : public object
{
protected:

	messagePart() { }
	messagePart(const messagePart&) : object() { }

	virtual ~messagePart() { }

public:

	/** Return the structure of this part.
	  *
	  * @return structure of the part
	  */
	virtual shared_ptr <const messageStructure> getStructure() const = 0;

	/** Return the structure of this part.
	  *
	  * @return structure of the part
	  */
	virtual shared_ptr <messageStructure> getStructure() = 0;

	/** Return the header section for this part (you must fetch header
	  * before using this function: see message::fetchPartHeader).
	  *
	  * @return header section
	  */
	virtual shared_ptr <const header> getHeader() const = 0;

	/** Return the media-type of the content in this part.
	  *
	  * @return content media type
	  */
	virtual const mediaType& getType() const = 0;

	/** Return the size of this part.
	  *
	  * @return size of the part (in bytes)
	  */
	virtual size_t getSize() const = 0;

	/** Return the part sequence number (index).
	  * The first part is at index zero.
	  *
	  * @return part number
	  */
	virtual int getNumber() const = 0;

	/** Return the sub-part at the specified position (zero is the
	  * first part).
	  *
	  * @param pos index of the sub-part
	  * @return sub-part at position 'pos'
	  */
	shared_ptr <const messagePart> getPartAt(const size_t pos) const;

	/** Return the sub-part at the specified position (zero is the
	  * first part).
	  *
	  * @param pos index of the sub-part
	  * @return sub-part at position 'pos'
	  */
	shared_ptr <messagePart> getPartAt(const size_t pos);

	/** Return the number of sub-parts in this part.
	  *
	  * @return number of sub-parts
	  */
	size_t getPartCount() const;
};


/** Structure of a MIME part/message.
  */

class VMIME_EXPORT messageStructure : public object
{
protected:

	messageStructure() { }
	messageStructure(const messageStructure&) : object() { }

public:

	virtual ~messageStructure() { }

	/** Return the part at the specified position (first
	  * part is at position 0).
	  *
	  * @param pos position
	  * @return part at position 'pos'
	  */
	virtual shared_ptr <const messagePart> getPartAt(const size_t pos) const = 0;

	/** Return the part at the specified position (first
	  * part is at position 0).
	  *
	  * @param pos position
	  * @return part at position 'pos'
	  */
	virtual shared_ptr <messagePart> getPartAt(const size_t pos) = 0;

	/** Return the number of parts in this part.
	  *
	  * @return number of parts
	  */
	virtual size_t getPartCount() const = 0;
};


/** Abstract representation of a message in a store/transport service.
  */

class VMIME_EXPORT message : public object
{
protected:

	message() { }
	message(const message&) : object() { }

	enum PrivateConstants
	{
		FLAG_UNDEFINED = 9999       /**< Used internally to indicate flags have not
		                                 been initialized yet. */
	};

public:

	virtual ~message() { }

	/** The type for an unique message identifier.
	  */
	class VMIME_EXPORT uid
	{
	public:

		uid();
		uid(const string& uid);
		uid(const unsigned long uid);
		uid(const char* uid);
		uid(const uid& other);

		uid& operator=(const uid& other);
		uid& operator=(const string& uid);
		uid& operator=(const unsigned long uid);

		operator string() const;

		bool empty() const;

		bool operator==(const uid& other) const;

	private:

		string m_str;
	};

	/** Return the MIME structure of the message (must fetch before).
	  *
	  * @return MIME structure of the message
	  */
	virtual shared_ptr <const messageStructure> getStructure() const = 0;

	/** Return the MIME structure of the message (must fetch before).
	  *
	  * @return MIME structure of the message
	  */
	virtual shared_ptr <messageStructure> getStructure() = 0;

	/** Return a reference to the header fields of the message (must fetch before).
	  *
	  * @return header section of the message
	  */
	virtual shared_ptr <const header> getHeader() const = 0;

	/** Return the sequence number of this message. This number is
	  * used to reference the message in the folder.
	  *
	  * @return sequence number of the message
	  */
	virtual int getNumber() const = 0;

	/** Return the unique identifier (UID) of this message in its
	  * folder (must fetch before).
	  *
	  * @return UID of the message
	  */
	virtual const uid getUID() const = 0;

	/** Return the size of the message (must fetch before).
	  *
	  * @return size of the message (in bytes)
	  */
	virtual size_t getSize() const = 0;

	/** Check whether this message has been expunged (ie: definitively
	  * deleted) and does not exist in the folder anymore.
	  *
	  * @return true if the message is expunged, false otherwise
	  */
	virtual bool isExpunged() const = 0;

	/** Possible flags for a message.
	  */
	enum Flags
	{
		FLAG_SEEN    = (1 << 0),   /**< Message has been seen. */
		FLAG_RECENT  = (1 << 1),   /**< Message has been recently received. */
		FLAG_DELETED = (1 << 2),   /**< Message is marked for deletion. */
		FLAG_REPLIED = (1 << 3),   /**< User replied to this message. */
		FLAG_MARKED  = (1 << 4),   /**< Used-defined flag. */
		FLAG_PASSED  = (1 << 5),   /**< Message has been resent/forwarded/bounced. */
		FLAG_DRAFT   = (1 << 6)    /**< Message is marked as a 'draft'. */
	};

	/** Methods for setting the flags.
	  */
	enum FlagsModes
	{
		FLAG_MODE_SET,     /**< Set (replace) the flags. */
		FLAG_MODE_ADD,     /**< Add the flags. */
		FLAG_MODE_REMOVE   /**< Remove the flags. */
	};

	/** Return the flags of this message.
	  *
	  * @return flags of the message
	  */
	virtual int getFlags() const = 0;

	/** Set the flags of this message.
	  *
	  * @param flags set of flags (see Flags)
	  * @param mode indicate how to treat old and new flags (see FlagsModes)
	  */
	virtual void setFlags(const int flags, const int mode = FLAG_MODE_SET) = 0;

	/** Extract the whole message data (header + contents).
	  *
	  * \warning Partial fetch might not be supported by the underlying protocol.
	  *
	  * @param os output stream in which to write message data
	  * @param progress progress listener, or NULL if not used
	  * @param start index of the first byte to retrieve (used for partial fetch)
	  * @param length number of bytes to retrieve (used for partial fetch)
	  * @param peek if true, try not to mark the message as read. This may not
	  * be supported by the protocol (IMAP supports this), but it will NOT throw
	  * an exception if not supported.
	  */
	virtual void extract
		(utility::outputStream& os,
		 utility::progressListener* progress = NULL,
		 const size_t start = 0,
		 const size_t length = -1,
		 const bool peek = false) const = 0;

	/** Extract the specified MIME part of the message (header + contents).
	  *
	  * \warning Partial fetch might not be supported by the underlying protocol.
	  *
	  * @param p part to extract
	  * @param os output stream in which to write part data
	  * @param progress progress listener, or NULL if not used
	  * @param start index of the first byte to retrieve (used for partial fetch)
	  * @param length number of bytes to retrieve (used for partial fetch)
	  * @param peek if true, try not to mark the message as read. This may not
	  * be supported by the protocol (IMAP supports this), but it will NOT throw
	  * an exception if not supported.
	  */
	virtual void extractPart
		(shared_ptr <const messagePart> p,
		 utility::outputStream& os,
		 utility::progressListener* progress = NULL,
		 const size_t start = 0,
		 const size_t length = -1,
		 const bool peek = false) const = 0;

	/** Fetch the MIME header for the specified part.
	  *
	  * @param p the part for which to fetch the header
	  */
	virtual void fetchPartHeader(shared_ptr <messagePart> p) = 0;

	/** Get the RFC-822 message for this abstract message.
	  * Warning: This may require getting some data (ie: structure and headers) from
	  * the server, which is done automatically. Actual message contents (ie: body)
	  * will not be fetched if possible (IMAP allows it, whereas POP3 will require
	  * to fetch the whole message).
	  *
	  * @return a RFC-822-parsed message
	  */
	virtual shared_ptr <vmime::message> getParsedMessage() = 0;
};


VMIME_EXPORT std::ostream& operator<<(std::ostream& os, const message::uid& uid);


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_NET_MESSAGE_HPP_INCLUDED
