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
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/net/imap/IMAPUtils.hpp"
#include "vmime/net/message.hpp"
#include "vmime/net/folder.hpp"

#include <sstream>
#include <iterator>
#include <algorithm>


namespace vmime {
namespace net {
namespace imap {


// static
const string IMAPUtils::quoteString(const string& text)
{
	//
	// ATOM_CHAR       ::= <any CHAR except atom_specials>
	//
	// atom_specials   ::= "(" / ")" / "{" / SPACE / CTL /
	//                     list_wildcards / quoted_specials
	//
	// list_wildcards  ::= "%" / "*"
	//
	// quoted_specials ::= <"> / "\"
	//
	// CHAR            ::= <any 7-bit US-ASCII character except NUL,
	//                      0x01 - 0x7f>
	//
	// CTL             ::= <any ASCII control character and DEL,
	//                      0x00 - 0x1f, 0x7f>
	//

	bool needQuoting = text.empty();

	for (string::const_iterator it = text.begin() ;
	     !needQuoting && it != text.end() ; ++it)
	{
		const unsigned char c = *it;

		switch (c)
		{
		case '(':
		case ')':
		case '{':
		case 0x20:   // SPACE
		case '%':
		case '*':
		case '"':
		case '\\':

			needQuoting = true;
			break;

		default:

			if (c <= 0x1f || c >= 0x7f)
				needQuoting = true;
		}
	}

	if (needQuoting)
	{
		string quoted;
		quoted.reserve((text.length() * 3) / 2 + 2);

		quoted += '"';

		for (string::const_iterator it = text.begin() ;
		     !needQuoting && it != text.end() ; ++it)
		{
			const unsigned char c = *it;

			if (c == '\\' || c == '"')
				quoted += '\\';

			quoted += c;
		}

		quoted += '"';

		return (quoted);
	}
	else
	{
		return (text);
	}
}


const string IMAPUtils::pathToString
	(const char hierarchySeparator, const folder::path& path)
{
	string result;

	for (int i = 0 ; i < path.getSize() ; ++i)
	{
		if (i > 0) result += hierarchySeparator;
		result += toModifiedUTF7(hierarchySeparator, path[i]);
	}

	return (result);
}


const folder::path IMAPUtils::stringToPath
	(const char hierarchySeparator, const string& str)
{
	folder::path result;
	string::const_iterator begin = str.begin();

	for (string::const_iterator it = str.begin() ; it != str.end() ; ++it)
	{
		if (*it == hierarchySeparator)
		{
			result /= fromModifiedUTF7(string(begin, it));
			begin = it + 1;
		}
	}

	if (begin != str.end())
	{
		result /= fromModifiedUTF7(string(begin, str.end()));
	}

	return (result);
}


const string IMAPUtils::toModifiedUTF7
	(const char hierarchySeparator, const folder::path::component& text)
{
	// We will replace the hierarchy separator with an equivalent
	// UTF-7 sequence, so we compute it here...
	const char base64alphabet[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,=";

	const unsigned int hs = static_cast <unsigned int>(static_cast <unsigned char>(hierarchySeparator));

	string hsUTF7;
	hsUTF7.resize(3);

	hsUTF7[0] = base64alphabet[0];
	hsUTF7[1] = base64alphabet[(hs & 0xF0) >> 4];
	hsUTF7[2] = base64alphabet[(hs & 0x0F) << 2];

	// Transcode path component to UTF-7 charset.
	// WARNING: This may throw "exceptions::charset_conv_error"
	const string cvt = text.getConvertedText(charset(charsets::UTF_7));

	// Transcode to modified UTF-7 (RFC-2060).
	string out;
	out.reserve((cvt.length() * 3) / 2);

	bool inB64sequence = false;

	for (string::const_iterator it = cvt.begin() ; it != cvt.end() ; ++it)
	{
		const unsigned char c = *it;

		// Replace hierarchy separator with an equivalent UTF-7 Base64 sequence
		if (!inB64sequence && c == hierarchySeparator)
		{
			out += "&" + hsUTF7 + "-";
			continue;
		}

		switch (c)
		{
		// Beginning of Base64 sequence: replace '+' with '&'
		case '+':
		{
			if (!inB64sequence)
			{
				inB64sequence = true;
				out += '&';
			}
			else
			{
				out += '+';
			}

			break;
		}
		// End of Base64 sequence
		case '-':
		{
			inB64sequence = false;
			out += '-';
			break;
		}
		// ',' is used instead of '/' in modified Base64
		case '/':
		{
			out += inB64sequence ? ',' : '/';
			break;
		}
		// '&' (0x26) is represented by the two-octet sequence "&-"
		case '&':
		{
			if (!inB64sequence)
				out += "&-";
			else
				out += '&';

			break;
		}
		default:
		{
			out += c;
			break;
		}

		}
	}

	return (out);
}


const folder::path::component IMAPUtils::fromModifiedUTF7(const string& text)
{
	// Transcode from modified UTF-7 (RFC-2060).
	string out;
	out.reserve(text.length());

	bool inB64sequence = false;
	unsigned char prev = 0;

	for (string::const_iterator it = text.begin() ; it != text.end() ; ++it)
	{
		const unsigned char c = *it;

		switch (c)
		{
		// Start of Base64 sequence
		case '&':
		{
			if (!inB64sequence)
			{
				inB64sequence = true;
				out += '+';
			}
			else
			{
				out += '&';
			}

			break;
		}
		// End of Base64 sequence (or "&-" --> "&")
		case '-':
		{
			if (inB64sequence && prev == '&')
				out += '&';
			else
				out += '-';

			inB64sequence = false;
			break;
		}
		// ',' is used instead of '/' in modified Base64
		case ',':
		{
			out += (inB64sequence ? '/' : ',');
			break;
		}
		default:
		{
			out += c;
			break;
		}

		}

		prev = c;
	}

	// Store it as UTF-8 by default
	string cvt;
	charset::convert(out, cvt,
		charset(charsets::UTF_7), charset(charsets::UTF_8));

	return (folder::path::component(cvt, charset(charsets::UTF_8)));
}


const int IMAPUtils::folderTypeFromFlags(const IMAPParser::mailbox_flag_list* list)
{
	// Get folder type
	int type = folder::TYPE_CONTAINS_MESSAGES | folder::TYPE_CONTAINS_FOLDERS;
	const std::vector <IMAPParser::mailbox_flag*>& flags = list->flags();

	for (std::vector <IMAPParser::mailbox_flag*>::const_iterator it = flags.begin() ;
	     it != flags.end() ; ++it)
	{
		if ((*it)->type() == IMAPParser::mailbox_flag::NOSELECT)
			type &= ~folder::TYPE_CONTAINS_MESSAGES;
	}

	if (type & folder::TYPE_CONTAINS_MESSAGES)
		type &= ~folder::TYPE_CONTAINS_FOLDERS;

	return (type);
}


const int IMAPUtils::folderFlagsFromFlags(const IMAPParser::mailbox_flag_list* list)
{
	// Get folder flags
	int folderFlags = folder::FLAG_CHILDREN;
	const std::vector <IMAPParser::mailbox_flag*>& flags = list->flags();

	for (std::vector <IMAPParser::mailbox_flag*>::const_iterator it = flags.begin() ;
	     it != flags.end() ; ++it)
	{
		if ((*it)->type() == IMAPParser::mailbox_flag::NOSELECT)
			folderFlags |= folder::FLAG_NO_OPEN;
		else if ((*it)->type() == IMAPParser::mailbox_flag::NOINFERIORS)
			folderFlags &= ~folder::FLAG_CHILDREN;
	}

	return (folderFlags);
}


const int IMAPUtils::messageFlagsFromFlags(const IMAPParser::flag_list* list)
{
	const std::vector <IMAPParser::flag*>& flagList = list->flags();
	int flags = 0;

	for (std::vector <IMAPParser::flag*>::const_iterator
	     it = flagList.begin() ; it != flagList.end() ; ++it)
	{
		switch ((*it)->type())
		{
		case IMAPParser::flag::ANSWERED:
			flags |= message::FLAG_REPLIED;
			break;
		case IMAPParser::flag::FLAGGED:
			flags |= message::FLAG_MARKED;
			break;
		case IMAPParser::flag::DELETED:
			flags |= message::FLAG_DELETED;
			break;
		case IMAPParser::flag::SEEN:
			flags |= message::FLAG_SEEN;
			break;

		default:
		//case IMAPParser::flag::UNKNOWN:
		//case IMAPParser::flag::DRAFT:
			break;
		}
	}

	return (flags);
}


const string IMAPUtils::messageFlagList(const int flags)
{
	std::vector <string> flagList;

	if (flags & message::FLAG_REPLIED) flagList.push_back("\\Answered");
	if (flags & message::FLAG_MARKED) flagList.push_back("\\Flagged");
	if (flags & message::FLAG_DELETED) flagList.push_back("\\Deleted");
	if (flags & message::FLAG_SEEN) flagList.push_back("\\Seen");

	if (!flagList.empty())
	{
		std::ostringstream res;
		res << "(";

		if (flagList.size() >= 2)
		{
			std::copy(flagList.begin(), flagList.end() - 1,
			          std::ostream_iterator <string>(res, " "));
		}

		res << *(flagList.end() - 1) << ")";

		return (res.str());
	}

	return "";
}


// static
const string IMAPUtils::listToSet(const std::vector <int>& list, const int max,
                                  const bool alreadySorted)
{
	// Sort a copy of the list (if not already sorted)
	std::vector <int> temp;

	if (!alreadySorted)
	{
		temp.resize(list.size());
		std::copy(list.begin(), list.end(), temp.begin());

		std::sort(temp.begin(), temp.end());
	}

	const std::vector <int>& theList = (alreadySorted ? list : temp);

	// Build the set
	std::ostringstream res;
	int previous = -1, setBegin = -1;

	for (std::vector <int>::const_iterator it = theList.begin() ;
	     it != theList.end() ; ++it)
	{
		const int current = *it;

		if (previous == -1)
		{
			res << current;

			previous = current;
			setBegin = current;
		}
		else
		{
			if (current == previous + 1)
			{
				previous = current;
			}
			else
			{
				if (setBegin != previous)
				{
					res << ":" << previous << "," << current;

					previous = current;
					setBegin = current;
				}
				else
				{
					if (setBegin != current)  // skip duplicates
						res << "," << current;

					previous = current;
					setBegin = current;
				}
			}
		}
	}

	if (previous != setBegin)
	{
		if (previous == max)
			res << ":*";
		else
			res << ":" << previous;
	}

	return (res.str());
}


// static
const string IMAPUtils::dateTime(const vmime::datetime& date)
{
	std::ostringstream res;

	// date_time ::= <"> date_day_fixed "-" date_month "-" date_year
	//               SPACE time SPACE zone <">
	//
	// time      ::= 2digit ":" 2digit ":" 2digit
	//               ;; Hours minutes seconds
	// zone      ::= ("+" / "-") 4digit
	//               ;; Signed four-digit value of hhmm representing
	//               ;; hours and minutes west of Greenwich
	res << '"';

	// Date
	if (date.getDay() < 10) res << ' ';
	res << date.getDay();

	res << '-';

	static const char* monthNames[12] =
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	res << monthNames[std::min(std::max(date.getMonth() - 1, 0), 11)];

	res << '-';

	if (date.getYear() < 10) res << '0';
	if (date.getYear() < 100) res << '0';
	if (date.getYear() < 1000) res << '0';
	res << date.getYear();

	res << ' ';

	// Time
	if (date.getHour() < 10) res << '0';
	res << date.getHour() << ':';

	if (date.getMinute() < 10) res << '0';
	res << date.getMinute() << ':';

	if (date.getSecond() < 10) res << '0';
	res << date.getSecond();

	res << ' ';

	// Zone
	const int zs = (date.getZone() < 0 ? -1 : 1);
	const int zh = (date.getZone() * zs) / 60;
	const int zm = (date.getZone() * zs) % 60;

	res << (zs < 0 ? '-' : '+');

	if (zh < 10) res << '0';
	res << zh;

	if (zm < 10) res << '0';
	res << zm;

	res << '"';


	return (res.str());
}


// static
const string IMAPUtils::buildFetchRequest(const std::vector <int>& list, const int options)
{
	// Example:
	//   C: A654 FETCH 2:4 (FLAGS BODY[HEADER.FIELDS (DATE FROM)])
	//   S: * 2 FETCH ....
	//   S: * 3 FETCH ....
	//   S: * 4 FETCH ....
	//   S: A654 OK FETCH completed

	std::vector <string> items;

	if (options & folder::FETCH_SIZE)
		items.push_back("RFC822.SIZE");

	if (options & folder::FETCH_FLAGS)
		items.push_back("FLAGS");

	if (options & folder::FETCH_STRUCTURE)
		items.push_back("BODYSTRUCTURE");

	if (options & folder::FETCH_UID)
		items.push_back("UID");

	if (options & folder::FETCH_FULL_HEADER)
		items.push_back("RFC822.HEADER");
	else
	{
		if (options & folder::FETCH_ENVELOPE)
			items.push_back("ENVELOPE");

		std::vector <string> headerFields;

		if (options & folder::FETCH_CONTENT_INFO)
			headerFields.push_back("CONTENT_TYPE");

		if (options & folder::FETCH_IMPORTANCE)
		{
			headerFields.push_back("IMPORTANCE");
			headerFields.push_back("X-PRIORITY");
		}

		if (!headerFields.empty())
		{
			string list;

			for (std::vector <string>::iterator it = headerFields.begin() ;
			     it != headerFields.end() ; ++it)
			{
				if (it != headerFields.begin())
					list += " ";

				list += *it;
			}

			items.push_back("BODY[HEADER.FIELDS (" + list + ")]");
		}
	}

	// Build the request text
	std::ostringstream command;
	command << "FETCH " << listToSet(list, -1, false) << " (";

	for (std::vector <string>::const_iterator it = items.begin() ;
	     it != items.end() ; ++it)
	{
		if (it != items.begin()) command << " ";
		command << *it;
	}

	command << ")";

	return command.str();
}


} // imap
} // net
} // vmime
