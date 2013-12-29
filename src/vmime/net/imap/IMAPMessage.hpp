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

#ifndef VMIME_NET_IMAP_IMAPMESSAGE_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPMESSAGE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/message.hpp"
#include "vmime/net/folder.hpp"

#include "vmime/net/imap/IMAPParser.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPFolder;


/** IMAP message implementation.
  */

class VMIME_EXPORT IMAPMessage : public message
{
private:

	friend class IMAPFolder;
	friend class IMAPMessagePartContentHandler;

	IMAPMessage(const IMAPMessage&) : message() { }

public:

	IMAPMessage(shared_ptr <IMAPFolder> folder, const int num);
	IMAPMessage(shared_ptr <IMAPFolder> folder, const int num, const uid& uid);

	~IMAPMessage();

	int getNumber() const;

	const uid getUID() const;

	/** Returns the modification sequence for this message.
	  *
	  * Every time metadata for this message changes, the modification
	  * sequence is updated, and is greater than the previous one. The
	  * server must support the CONDSTORE extension for this to be
	  * available.
	  *
	  * @return modification sequence, or zero if not supported by
	  * the underlying protocol
	  */
	vmime_uint64 getModSequence() const;

	size_t getSize() const;

	bool isExpunged() const;

	shared_ptr <const messageStructure> getStructure() const;
	shared_ptr <messageStructure> getStructure();

	shared_ptr <const header> getHeader() const;

	int getFlags() const;
	void setFlags(const int flags, const int mode = FLAG_MODE_SET);

	void extract
		(utility::outputStream& os,
		 utility::progressListener* progress = NULL,
		 const size_t start = 0, const size_t length = -1,
		 const bool peek = false) const;

	void extractPart
		(shared_ptr <const messagePart> p,
		 utility::outputStream& os,
		 utility::progressListener* progress = NULL,
		 const size_t start = 0, const size_t length = -1,
		 const bool peek = false) const;

	void fetchPartHeader(shared_ptr <messagePart> p);

	shared_ptr <vmime::message> getParsedMessage();

private:

	/** Renumbers the message.
	  *
	  * @param number new sequence number
	  */
	void renumber(const int number);

	/** Marks the message as expunged.
	  */
	void setExpunged();

	/** Processes the parsed response to fill in the attributes
	  * and metadata of this message.
	  *
	  * @param options one or more fetch options (see folder::fetchAttributes)
	  * @param msgData pointer to message_data component of the parsed response
	  * @return a combination of flags that specify what changed exactly on
	  * this message (see events::messageChangedEvent::Types)
	  */
	int processFetchResponse(const fetchAttributes& options, const IMAPParser::message_data* msgData);

	/** Recursively fetch part header for all parts in the structure.
	  *
	  * @param str structure for which to fetch parts headers
	  */
	void fetchPartHeaderForStructure(shared_ptr <messageStructure> str);

	/** Recursively contruct parsed message from structure.
	  * Called by getParsedMessage().
	  *
	  * @param parentPart root body part (the message)
	  * @param str structure for which to construct part
	  * @param level current nesting level (0 is root)
	  */
	void constructParsedMessage(shared_ptr <bodyPart> parentPart, shared_ptr <messageStructure> str, int level = 0);


	enum ExtractFlags
	{
		EXTRACT_HEADER = 0x1,
		EXTRACT_BODY = 0x2,
		EXTRACT_PEEK = 0x10
	};

	void extractImpl
		(shared_ptr <const messagePart> p,
		 utility::outputStream& os,
		 utility::progressListener* progress,
		 const size_t start, const size_t length,
		 const int extractFlags) const;


	shared_ptr <header> getOrCreateHeader();


	void onFolderClosed();

	weak_ptr <IMAPFolder> m_folder;

	int m_num;
	size_t m_size;
	int m_flags;
	bool m_expunged;
	uid m_uid;
	vmime_uint64 m_modseq;

	shared_ptr <header> m_header;
	shared_ptr <messageStructure> m_structure;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPMESSAGE_HPP_INCLUDED
