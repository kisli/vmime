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

#ifndef VMIME_MESSAGING_IMAPUTILS_HPP_INCLUDED
#define VMIME_MESSAGING_IMAPUTILS_HPP_INCLUDED


#include "folder.hpp"
#include "../types.hpp"
#include "IMAPParser.hpp"
#include "../dateTime.hpp"

#include <vector>


namespace vmime {
namespace messaging {


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


} // messaging
} // vmime


#endif // VMIME_MESSAGING_IMAPUTILS_HPP_INCLUDED
