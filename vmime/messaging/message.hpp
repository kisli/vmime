//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_MESSAGING_MESSAGE_HPP_INCLUDED
#define VMIME_MESSAGING_MESSAGE_HPP_INCLUDED


#include "vmime/header.hpp"
#include "vmime/messaging/progressionListener.hpp"
#include "vmime/utility/stream.hpp"


namespace vmime {
namespace messaging {


class structure;


/** A MIME part in a message.
  */

class part
{
protected:

	part() { }
	part(const part&) { }

	virtual ~part() { }

public:

	/** Return the structure of this part.
	  *
	  * @return structure of the part
	  */
	virtual const structure& getStructure() const = 0;

	/** Return the structure of this part.
	  *
	  * @return structure of the part
	  */
	virtual structure& getStructure() = 0;

	/** Return the header section for this part (you must fetch header
	  * before using this function: see message::fetchPartHeader).
	  *
	  * @return header section
	  */
	virtual const header& getHeader() const = 0;

	/** Return the media-type of the content in this part.
	  *
	  * @return content media type
	  */
	virtual const mediaType& getType() const = 0;

	/** Return the size of this part.
	  *
	  * @return size of the part (in bytes)
	  */
	virtual const int getSize() const = 0;

	/** Return the part sequence number (index)
	  *
	  * @return part number
	  */
	virtual const int getNumber() const = 0;   // begin at 1

	/** Return the sub-part at the specified position.
	  * This provide easy access to parts:
	  * Eg: "message->extract(message->structure()[3][1][2])".
	  *
	  * @param x index of the sub-part
	  * @return sub-part at position 'x'
	  */
	const part& operator[](const int x) const;

	/** Return the sub-part at the specified position.
	  * This provide easy access to parts:
	  * Eg: "message->extract(message->structure()[3][1][2])".
	  *
	  * @param x index of the sub-part
	  * @return sub-part at position 'x'
	  */
	part& operator[](const int x);

	/** Return the number of sub-parts in this part.
	  *
	  * @return number of sub-parts
	  */
	const int getCount() const;
};


/** Structure of a MIME part/message.
  */

class structure
{
protected:

	structure() { }
	structure(const structure&) { }

public:

	virtual ~structure() { }

	/** Return the part at the specified position (first
	  * part is at position 1).
	  *
	  * @param x position
	  * @return part at position 'x'
	  */
	virtual const part& operator[](const int x) const = 0;

	/** Return the part at the specified position (first
	  * part is at position 1).
	  *
	  * @param x position
	  * @return part at position 'x'
	  */
	virtual part& operator[](const int x) = 0;

	/** Return the number of parts in this part.
	  *
	  * @return number of parts
	  */
	virtual const int getCount() const = 0;
};


/** Abstract representation of a message in a store/transport service.
  */

class message
{
protected:

	message() { }
	message(const message&) { }

public:

	virtual ~message() { }

	/** The type for an unique message identifier.
	  */
	typedef string uid;

	/** Return the MIME structure of the message (must fetch before).
	  *
	  * @return MIME structure of the message
	  */
	virtual const structure& getStructure() const = 0;

	/** Return the MIME structure of the message (must fetch before).
	  *
	  * @return MIME structure of the message
	  */
	virtual structure& getStructure() = 0;

	/** Return a reference to the header fields of the message (must fetch before).
	  *
	  * @return header section of the message
	  */
	virtual const header& getHeader() const = 0;

	/** Return the sequence number of this message. This number is
	  * used to reference the message in the folder.
	  *
	  * @return sequence number of the message
	  */
	virtual const int getNumber() const = 0;

	/** Return the unique identified of this message (must fetch before).
	  *
	  * @return UID of the message
	  */
	virtual const uid getUniqueId() const = 0;

	/** Return the size of the message (must fetch before).
	  *
	  * @return size of the message (in bytes)
	  */
	virtual const int getSize() const = 0;

	/** Check whether this message has been expunged
	  * (ie: definitively deleted).
	  *
	  * @return true if the message is expunged, false otherwise
	  */
	virtual const bool isExpunged() const = 0;

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

		FLAG_UNDEFINED = 9999      /**< Used internally (this should not be returned
		                                by the flags() function). */
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
	virtual const int getFlags() const = 0;

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
	  * @param progress progression listener, or NULL if not used
	  * @param start index of the first byte to retrieve (used for partial fetch)
	  * @param length number of bytes to retrieve (used for partial fetch)
	  */

	virtual void extract(utility::outputStream& os, progressionListener* progress = NULL, const int start = 0, const int length = -1) const = 0;

	/** Extract the specified (MIME) part of the message (header + contents).
	  *
	  * \warning Partial fetch might not be supported by the underlying protocol.
	  *
	  * @param p part to extract
	  * @param os output stream in which to write part data
	  * @param progress progression listener, or NULL if not used
	  * @param start index of the first byte to retrieve (used for partial fetch)
	  * @param length number of bytes to retrieve (used for partial fetch)
	  */
	virtual void extractPart(const part& p, utility::outputStream& os, progressionListener* progress = NULL, const int start = 0, const int length = -1) const = 0;

	/** Fetch the MIME header for the specified part.
	  *
	  * @param p the part for which to fetch the header
	  */
	virtual void fetchPartHeader(part& p) = 0;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_MESSAGE_HPP_INCLUDED
