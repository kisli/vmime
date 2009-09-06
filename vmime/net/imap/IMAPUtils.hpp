//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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


#include "vmime/types.hpp"
#include "vmime/dateTime.hpp"

#include "vmime/net/folder.hpp"
#include "vmime/net/imap/IMAPParser.hpp"

#include "vmime/mailboxList.hpp"

#include <vector>


namespace vmime {
namespace net {
namespace imap {


class IMAPUtils
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

	static int folderTypeFromFlags(const IMAPParser::mailbox_flag_list* list);
	static int folderFlagsFromFlags(const IMAPParser::mailbox_flag_list* list);

	static int messageFlagsFromFlags(const IMAPParser::flag_list* list);

	static const string messageFlagList(const int flags);

	/** Build an "IMAP set" given a list. The function tries to group
	  * consecutive message numbers to reduce the list.
	  *
	  * Example:
	  *    IN  = "1,2,3,4,5,7,8,13,15,16,17"
	  *    OUT = "1:5,7:8,13,15:*" for a mailbox with a total of 17 messages (max = 17)
	  *
	  * @param list list of message numbers
	  * @param max number of messages in the mailbox (or -1 if not known)
	  * @param alreadySorted set to true if the list of message numbers is
	  * already sorted in ascending order
	  * @return a set corresponding to the message list
	  */
	static const string listToSet(const std::vector <int>& list,
		const int max = -1, const bool alreadySorted = false);

	/** Format a date/time to IMAP date/time format.
	  *
	  * @param date date/time to format
	  * @return IMAP-formatted date/time
	  */
	static const string dateTime(const vmime::datetime& date);

	/** Construct a fetch request for the specified messages.
	  *
	  * @param list list of message numbers
	  * @param options fetch options
	  * @return fetch request
	  */
	static const string buildFetchRequest(const std::vector <int>& list, const int options);

	/** Convert a parser-style address list to a mailbox list.
	  *
	  * @param src input address list
	  * @param dest output mailbox list
	  */
	static void convertAddressList(const IMAPParser::address_list& src, mailboxList& dest);
};


} // imap
} // net
} // vmime


#endif // VMIME_NET_IMAP_IMAPUTILS_HPP_INCLUDED
