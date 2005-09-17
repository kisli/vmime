//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

	static const string quoteString(const string& text);

	static const int folderTypeFromFlags(const IMAPParser::mailbox_flag_list* list);
	static const int folderFlagsFromFlags(const IMAPParser::mailbox_flag_list* list);

	static const int messageFlagsFromFlags(const IMAPParser::flag_list* list);

	static const string messageFlagList(const int flags);

	static const string listToSet(const std::vector <int>& list, const int max = -1, const bool alreadySorted  = false);

	static const string dateTime(const vmime::datetime& date);
};


} // imap
} // net
} // vmime


#endif // VMIME_NET_IMAP_IMAPUTILS_HPP_INCLUDED
