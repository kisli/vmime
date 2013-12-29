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

#ifndef VMIME_NET_IMAP_IMAPFOLDERSTATUS_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPFOLDERSTATUS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/folderStatus.hpp"

#include "vmime/net/imap/IMAPParser.hpp"


namespace vmime {
namespace net {
namespace imap {


/** Holds the status of an IMAP folder.
  */

class VMIME_EXPORT IMAPFolderStatus : public folderStatus
{
public:

	IMAPFolderStatus();
	IMAPFolderStatus(const IMAPFolderStatus& other);

	// Inherited from folderStatus
	unsigned int getMessageCount() const;
	unsigned int getUnseenCount() const;

	shared_ptr <folderStatus> clone() const;

	/** Returns the the number of messages with the Recent flag set.
	  *
	  * @return number of messages flagged Recent
	  */
	unsigned int getRecentCount() const;

	/** Returns the UID validity of the folder for the current session.
	  * If the server is capable of persisting UIDs accross sessions,
	  * this value should never change for a folder.
	  *
	  * @return UID validity of the folder
	  */
	vmime_uint32 getUIDValidity() const;

	/** Returns the UID value that will be assigned to a new message
	  * in the folder. If the server does not support the UIDPLUS
	  * extension, it will return 0.
	  *
	  * @return UID of the next message
	  */
	vmime_uint32 getUIDNext() const;

	/** Returns the highest modification sequence of all messages
	  * in the folder, or 0 if not available for this folder, or not
	  * supported by the server. The server must support the CONDSTORE
	  * extension for this to be available.
	  *
	  * @return highest modification sequence
	  */
	vmime_uint64 getHighestModSeq() const;


	/** Reads the folder status from the specified IMAP response.
	  *
	  * @param resp parsed IMAP response
	  * @return true if the status changed, or false otherwise
	  */
	bool updateFromResponse(const IMAPParser::mailbox_data* resp);

	/** Reads the folder status from the specified IMAP response.
	  *
	  * @param resp parsed IMAP response
	  * @return true if the status changed, or false otherwise
	  */
	bool updateFromResponse(const IMAPParser::resp_text_code* resp);

private:

	unsigned int m_count;
	unsigned int m_unseen;
	unsigned int m_recent;
	vmime_uint32 m_uidValidity;
	vmime_uint32 m_uidNext;
	vmime_uint64 m_highestModSeq;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPFOLDERSTATUS_HPP_INCLUDED
