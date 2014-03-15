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

#ifndef VMIME_NET_IMAP_IMAPUTILS_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPUTILS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/types.hpp"
#include "vmime/dateTime.hpp"

#include "vmime/net/folder.hpp"
#include "vmime/net/message.hpp"
#include "vmime/net/imap/IMAPParser.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"
#include "vmime/net/imap/IMAPCommand.hpp"

#include "vmime/mailboxList.hpp"

#include <vector>


namespace vmime {
namespace net {
namespace imap {


class VMIME_EXPORT IMAPUtils
{
public:

	static const string pathToString(const char hierarchySeparator, const folder::path& path);
	static const folder::path stringToPath(const char hierarchySeparator, const string& str);

	static const string toModifiedUTF7(const char hierarchySeparator, const folder::path::component& text);
	static const folder::path::component fromModifiedUTF7(const string& text);

	/** Quote string if it contains IMAP-special characters.
	  *
	  * @param text string to quote
	  * @return quoted string
	  */
	static const string quoteString(const string& text);

	/** Parse mailbox flags and fill in folder attributes.
	  *
	  * @param cnt reference to current IMAP connection (for testing capabilities)
	  * @param list list of mailbox flags
	  * @param attribs reference to an object holding folder attributes
	  */
	static void mailboxFlagsToFolderAttributes
		(shared_ptr <const IMAPConnection> cnt,
		 const IMAPParser::mailbox_flag_list* list,
		 folderAttributes& attribs);

	static int messageFlagsFromFlags(const IMAPParser::flag_list* list);

	static const std::vector <string> messageFlagList(const int flags);

	/** Format a date/time to IMAP date/time format.
	  *
	  * @param date date/time to format
	  * @return IMAP-formatted date/time
	  */
	static const string dateTime(const vmime::datetime& date);

	/** Construct a fetch request for the specified messages, designated
	  * either by their sequence numbers or their UIDs.
	  *
	  * @param cnt connection
	  * @param msgs message set
	  * @param options fetch options
	  * @return fetch request
	  */
	static shared_ptr <IMAPCommand> buildFetchCommand
		(shared_ptr <IMAPConnection> cnt, const messageSet& msgs, const fetchAttributes& options);

	/** Convert a parser-style address list to a mailbox list.
	  *
	  * @param src input address list
	  * @param dest output mailbox list
	  */
	static void convertAddressList(const IMAPParser::address_list& src, mailboxList& dest);

	/** Returns an IMAP-formatted sequence set given a message set.
	  *
	  * @param msgs message set
	  * @return IMAP sequence set (eg. "1:5,7,15:*")
	  */
	static const string messageSetToSequenceSet(const messageSet& msgs);

	/** Returns a list of message sequence numbers given a message set.
	  *
	  * @param msgs message set
	  * @return list of message numbers
	  */
	static const std::vector <int> messageSetToNumberList(const messageSet& msgs);

	/** Constructs a message set from a parser 'uid_set' structure.
	  *
	  * @param uidSet UID set, as returned by the parser
	  * @return message set
	  */
	static messageSet buildMessageSet(const IMAPParser::uid_set* uidSet);

private:

	static const string buildFetchRequestImpl
		(shared_ptr <IMAPConnection> cnt, const string& mode, const string& set, const int options);
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPUTILS_HPP_INCLUDED
