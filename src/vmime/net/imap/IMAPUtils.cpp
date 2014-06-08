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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "vmime/net/imap/IMAPUtils.hpp"
#include "vmime/net/imap/IMAPStore.hpp"

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

		for (string::const_iterator it = text.begin() ; it != text.end() ; ++it)
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

	for (size_t i = 0 ; i < path.getSize() ; ++i)
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

	// iconv() is buggy with UTF-8 to UTF-7 conversion, so we do it "by hand".
	// This code is largely inspired from "imap/utf7.c", in mutt 1.4.
	// Copyright (C) 2000 Edmund Grimley Evans <edmundo@rano.org>

	// WARNING: This may throw "exceptions::charset_conv_error"
	const string cvt = text.getConvertedText(charset(charsets::UTF_8));

	// In the worst case we convert 2 chars to 7 chars.
	// For example: "\x10&\x10&..." -> "&ABA-&-&ABA-&-...".
	string out;
	out.reserve((cvt.length() / 2) * 7 + 6);

	int b = 0, k = 0;
	bool base64 = false;

	size_t remaining = cvt.length();

	for (size_t i = 0, len = cvt.length() ; i < len ; )
	{
		const unsigned char c = cvt[i];

		// Replace hierarchy separator with an equivalent UTF-7 Base64 sequence
		if (!base64 && c == hierarchySeparator)
		{
			out += "&" + hsUTF7 + "-";

			++i;
			--remaining;
			continue;
		}

		size_t n = 0;
		int ch = 0;

		if (c < 0x80)
			ch = c, n = 0;
		else if (c < 0xc2)
			return "";
		else if (c < 0xe0)
			ch = c & 0x1f, n = 1;
		else if (c < 0xf0)
			ch = c & 0x0f, n = 2;
		else if (c < 0xf8)
			ch = c & 0x07, n = 3;
		else if (c < 0xfc)
			ch = c & 0x03, n = 4;
		else if (c < 0xfe)
			ch = c & 0x01, n = 5;
		else
			return "";

		if (n > remaining)
			return "";  // error

		++i;
		--remaining;

		for (size_t j = 0 ; j < n ; j++)
		{
			if ((cvt[i + j] & 0xc0) != 0x80)
				return "";  // error

			ch = (ch << 6) | (cvt[i + j] & 0x3f);
		}

		if (n > 1 && !(ch >> (n * 5 + 1)))
			return "";  // error

		i += n;
		remaining -= n;

		if (ch < 0x20 || ch >= 0x7f)
		{
			if (!base64)
			{
				out += '&';
				base64 = true;
				b = 0;
				k = 10;
			}

			if (ch & ~0xffff)
				ch = 0xfffe;

			out += base64alphabet[b | ch >> k];

			k -= 6;

			for ( ; k >= 0 ; k -= 6)
				out += base64alphabet[(ch >> k) & 0x3f];

			b = (ch << (-k)) & 0x3f;
			k += 16;
		}
		else
		{
			if (base64)
			{
				if (k > 10)
					out += base64alphabet[b];

				out += '-';
				base64 = false;
			}

			out += static_cast <char>(ch);

			if (ch == '&')
				out += '-';
		}
	}

	if (base64)
	{
		if (k > 10)
			out += base64alphabet[b];

		out += '-';
	}

	return (out);
}


const folder::path::component IMAPUtils::fromModifiedUTF7(const string& text)
{
	// Transcode from modified UTF-7 (RFC-2060).
	string out;
	out.reserve(text.length());

	bool inB64sequence = false;
	bool plusOutput = false;
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
				plusOutput = false;
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
			if (inB64sequence && prev == '&')  // special case "&-" --> "&"
				out += '&';
			else
				out += '-';

			inB64sequence = false;
			break;
		}
		// ',' is used instead of '/' in modified Base64
		case ',':
		{
			if (inB64sequence && !plusOutput)
			{
				out += '+';
				plusOutput = true;
			}

			out += (inB64sequence ? '/' : ',');
			break;
		}
		default:
		{
			if (inB64sequence && !plusOutput)
			{
				out += '+';
				plusOutput = true;
			}

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


// static
void IMAPUtils::mailboxFlagsToFolderAttributes
	(shared_ptr <const IMAPConnection> cnt, const IMAPParser::mailbox_flag_list* list,
	 folderAttributes& attribs)
{
	int specialUse = folderAttributes::SPECIALUSE_NONE;
	int type = folderAttributes::TYPE_CONTAINS_MESSAGES | folderAttributes::TYPE_CONTAINS_FOLDERS;
	int flags = 0;

	// If CHILDREN extension (RFC-3348) is not supported, assume folder has children
	// as we have no hint about it
	if (!cnt->hasCapability("CHILDREN"))
		flags |= folderAttributes::FLAG_HAS_CHILDREN;

	const std::vector <IMAPParser::mailbox_flag*>& mailboxFlags = list->flags();

	for (std::vector <IMAPParser::mailbox_flag*>::const_iterator it = mailboxFlags.begin() ;
	     it != mailboxFlags.end() ; ++it)
	{
		switch ((*it)->type())
		{
		case IMAPParser::mailbox_flag::NOSELECT:

			type &= ~folderAttributes::TYPE_CONTAINS_MESSAGES;
			flags |= folderAttributes::FLAG_NO_OPEN;
			break;

		case IMAPParser::mailbox_flag::NOINFERIORS:
		case IMAPParser::mailbox_flag::HASNOCHILDREN:

			flags &= ~folderAttributes::FLAG_HAS_CHILDREN;
			break;

		case IMAPParser::mailbox_flag::HASCHILDREN:

			flags |= folderAttributes::FLAG_HAS_CHILDREN;
			break;

		case IMAPParser::mailbox_flag::SPECIALUSE_ALL:

			specialUse = folderAttributes::SPECIALUSE_ALL;
			break;

		case IMAPParser::mailbox_flag::SPECIALUSE_ARCHIVE:

			specialUse = folderAttributes::SPECIALUSE_ARCHIVE;
			break;

		case IMAPParser::mailbox_flag::SPECIALUSE_DRAFTS:

			specialUse = folderAttributes::SPECIALUSE_DRAFTS;
			break;

		case IMAPParser::mailbox_flag::SPECIALUSE_FLAGGED:

			specialUse = folderAttributes::SPECIALUSE_FLAGGED;
			break;

		case IMAPParser::mailbox_flag::SPECIALUSE_JUNK:

			specialUse = folderAttributes::SPECIALUSE_JUNK;
			break;

		case IMAPParser::mailbox_flag::SPECIALUSE_SENT:

			specialUse = folderAttributes::SPECIALUSE_SENT;
			break;

		case IMAPParser::mailbox_flag::SPECIALUSE_TRASH:

			specialUse = folderAttributes::SPECIALUSE_TRASH;
			break;

		case IMAPParser::mailbox_flag::SPECIALUSE_IMPORTANT:

			specialUse = folderAttributes::SPECIALUSE_IMPORTANT;
			break;
		}
	}

	attribs.setSpecialUse(specialUse);
	attribs.setType(type);
	attribs.setFlags(flags);
}


int IMAPUtils::messageFlagsFromFlags(const IMAPParser::flag_list* list)
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
		case IMAPParser::flag::DRAFT:
			flags |= message::FLAG_DRAFT;
			break;

		default:
		//case IMAPParser::flag::UNKNOWN:
			break;
		}
	}

	return (flags);
}


// static
const std::vector <string> IMAPUtils::messageFlagList(const int flags)
{
	std::vector <string> flagList;

	if (flags == -1)
		return flagList;  // default flags

	if (flags & message::FLAG_REPLIED) flagList.push_back("\\Answered");
	if (flags & message::FLAG_MARKED) flagList.push_back("\\Flagged");
	if (flags & message::FLAG_DELETED) flagList.push_back("\\Deleted");
	if (flags & message::FLAG_SEEN) flagList.push_back("\\Seen");
	if (flags & message::FLAG_DRAFT) flagList.push_back("\\Draft");

	return flagList;
}


// static
const string IMAPUtils::dateTime(const vmime::datetime& date)
{
	std::ostringstream res;
	res.imbue(std::locale::classic());

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
shared_ptr <IMAPCommand> IMAPUtils::buildFetchCommand
	(shared_ptr <IMAPConnection> cnt, const messageSet& msgs, const fetchAttributes& options)
{
	// Example:
	//   C: A654 FETCH 2:4 (FLAGS BODY[HEADER.FIELDS (DATE FROM)])
	//   S: * 2 FETCH ....
	//   S: * 3 FETCH ....
	//   S: * 4 FETCH ....
	//   S: A654 OK FETCH completed

	std::vector <string> items;

	if (options.has(fetchAttributes::SIZE))
		items.push_back("RFC822.SIZE");

	if (options.has(fetchAttributes::FLAGS))
		items.push_back("FLAGS");

	if (options.has(fetchAttributes::STRUCTURE))
		items.push_back("BODYSTRUCTURE");

	if (options.has(fetchAttributes::UID))
	{
		items.push_back("UID");

		// Also fetch MODSEQ if CONDSTORE is supported
		if (cnt && cnt->hasCapability("CONDSTORE") && !cnt->isMODSEQDisabled())
			items.push_back("MODSEQ");
	}

	if (options.has(fetchAttributes::FULL_HEADER))
		items.push_back("RFC822.HEADER");
	else
	{
		if (options.has(fetchAttributes::ENVELOPE))
			items.push_back("ENVELOPE");

		std::vector <string> headerFields;

		if (options.has(fetchAttributes::CONTENT_INFO))
			headerFields.push_back("CONTENT_TYPE");

		if (options.has(fetchAttributes::IMPORTANCE))
		{
			headerFields.push_back("IMPORTANCE");
			headerFields.push_back("X-PRIORITY");
		}

		// Also add custom header fields to fetch, if any
		const std::vector <string> customHeaderFields = options.getHeaderFields();
		std::copy(customHeaderFields.begin(), customHeaderFields.end(), std::back_inserter(headerFields));

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

	return IMAPCommand::FETCH(msgs, items);
}


// static
void IMAPUtils::convertAddressList
	(const IMAPParser::address_list& src, mailboxList& dest)
{
	for (std::vector <IMAPParser::address*>::const_iterator
	     it = src.addresses().begin() ; it != src.addresses().end() ; ++it)
	{
		const IMAPParser::address& addr = **it;

		text name;
		text::decodeAndUnfold(addr.addr_name()->value(), &name);

		string email = addr.addr_mailbox()->value()
			+ "@" + addr.addr_host()->value();

		dest.appendMailbox(make_shared <mailbox>(name, email));
	}
}



class IMAPUIDMessageSetEnumerator : public messageSetEnumerator
{
public:

	IMAPUIDMessageSetEnumerator()
		: m_first(true)
	{
	}

	void enumerateNumberMessageRange(const vmime::net::numberMessageRange& range)
	{
		if (!m_first)
			m_oss << ",";

		if (range.getFirst() == range.getLast())
			m_oss << range.getFirst();
		else
			m_oss << range.getFirst() << ":" << range.getLast();

		m_first = false;
	}

	void enumerateUIDMessageRange(const vmime::net::UIDMessageRange& range)
	{
		if (!m_first)
			m_oss << ",";

		if (range.getFirst() == range.getLast())
			m_oss << range.getFirst();
		else
			m_oss << range.getFirst() << ":" << range.getLast();

		m_first = false;
	}

	const std::string str() const
	{
		return m_oss.str();
	}

private:

	std::ostringstream m_oss;
	bool m_first;
};


class IMAPMessageSetEnumerator : public messageSetEnumerator
{
public:

	void enumerateNumberMessageRange(const vmime::net::numberMessageRange& range)
	{
		for (int i = range.getFirst(), last = range.getLast() ; i <= last ; ++i)
			m_list.push_back(i);
	}

	void enumerateUIDMessageRange(const vmime::net::UIDMessageRange& /* range */)
	{
		// Not used
	}

	const std::vector <int>& list() const
	{
		return m_list;
	}

public:

	std::vector <int> m_list;
};



// static
const string IMAPUtils::messageSetToSequenceSet(const messageSet& msgs)
{
	IMAPUIDMessageSetEnumerator en;
	msgs.enumerate(en);

	return en.str();
}


// static
const std::vector <int> IMAPUtils::messageSetToNumberList(const messageSet& msgs)
{
	IMAPMessageSetEnumerator en;
	msgs.enumerate(en);

	return en.list();
}


// static
messageSet IMAPUtils::buildMessageSet(const IMAPParser::uid_set* uidSet)
{
	messageSet set = messageSet::empty();

	for ( ; uidSet ; uidSet = uidSet->next_uid_set())
	{
		if (uidSet->uid_range())
		{
			set.addRange(UIDMessageRange
				(message::uid(uidSet->uid_range()->uniqueid1()->value()),
				 message::uid(uidSet->uid_range()->uniqueid2()->value())));
		}
		else
		{
			set.addRange(UIDMessageRange
				(message::uid(uidSet->uniqueid()->value())));
		}
	}

	return set;
}


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

