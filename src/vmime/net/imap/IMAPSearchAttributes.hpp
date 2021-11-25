//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#ifndef VMIME_NET_IMAP_IMAPSEARCHATTRIBUTES_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPSEARCHATTRIBUTES_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include <vector>

#include "vmime/types.hpp"
#include "vmime/utility/stringUtils.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/net/message.hpp"
#include "vmime/net/messageSet.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPSearchToken : public object {

public:

	IMAPSearchToken(const char* token)
		: m_token(token) {

		if (nullptr == m_token) {
			throw exceptions::invalid_argument();
		}
	}

	virtual void generate(std::ostringstream& out) const = 0;

protected:

	const char* m_token;
};


typedef vmime::shared_ptr <const IMAPSearchToken> IMAPSearchTokenPtr;


class IMAPSearchTokenFactory : public object {

public:

	static IMAPSearchTokenPtr AND(const std::vector <IMAPSearchTokenPtr>&);
	static IMAPSearchTokenPtr AND(const std::vector <IMAPSearchTokenPtr>&&);
	static IMAPSearchTokenPtr ANSWERED();
	static IMAPSearchTokenPtr BCC(const string&);
	static IMAPSearchTokenPtr BEFORE(const datetime&);
	static IMAPSearchTokenPtr BODY(const string&);
	static IMAPSearchTokenPtr CC(const string&);
	static IMAPSearchTokenPtr DELETED();
	static IMAPSearchTokenPtr DRAFT();
	static IMAPSearchTokenPtr FLAGGED();
	static IMAPSearchTokenPtr FROM(const string&);
	static IMAPSearchTokenPtr HEADER(const char* fieldName);
	static IMAPSearchTokenPtr HEADER(const char* filedName, const string& fieldContents);
	static IMAPSearchTokenPtr KEYWORD(vmime::net::message::Flags);
	static IMAPSearchTokenPtr LARGER(uint32_t);
	static IMAPSearchTokenPtr MESSAGESET(const vmime::net::messageSet&);
	static IMAPSearchTokenPtr MESSAGESET(const vmime::net::messageSet&&);
	static IMAPSearchTokenPtr NEW();
	static IMAPSearchTokenPtr NOT(const IMAPSearchTokenPtr&);
	static IMAPSearchTokenPtr OLD();
	static IMAPSearchTokenPtr ON(const datetime&);
	static IMAPSearchTokenPtr OR(const IMAPSearchTokenPtr&, const IMAPSearchTokenPtr&);
	static IMAPSearchTokenPtr RECENT();
	static IMAPSearchTokenPtr SEEN();
	static IMAPSearchTokenPtr SENTBEFORE(const datetime&);
	static IMAPSearchTokenPtr SENTON(const datetime&);
	static IMAPSearchTokenPtr SENTSINCE(const datetime&);
	static IMAPSearchTokenPtr SINCE(const datetime&);
	static IMAPSearchTokenPtr SMALLER(uint32_t);
	static IMAPSearchTokenPtr SUBJECT(const string&);
	static IMAPSearchTokenPtr TEXT(const string&);
	static IMAPSearchTokenPtr TO(const string&);
	static IMAPSearchTokenPtr UID(const vmime::net::messageSet&);
	static IMAPSearchTokenPtr UID(const vmime::net::messageSet&&);
	static IMAPSearchTokenPtr UNANSWERED();
	static IMAPSearchTokenPtr UNDELETED();
	static IMAPSearchTokenPtr UNDRAFT();
	static IMAPSearchTokenPtr UNFLAGGED();
	static IMAPSearchTokenPtr UNKEYWORD(vmime::net::message::Flags);
	static IMAPSearchTokenPtr UNSEEN();
};


/** Holds a set of attributes to match messages against when searching folder contents.
  */
class VMIME_EXPORT IMAPSearchAttributes : public object {

public:

	IMAPSearchAttributes() = default;
	IMAPSearchAttributes(std::vector <vmime::shared_ptr <const IMAPSearchToken>>&&);

	/** Adds a new search token that will be used to match messages against. Multiple tokens are
	  * treated as an AND operation.
	  *
	  * @param token the search token to add
	  */
	void add(const vmime::shared_ptr <const IMAPSearchToken>& token);

	std::vector <string> generate() const;

protected:

	std::vector <vmime::shared_ptr <const IMAPSearchToken>> m_andTokens;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#endif // VMIME_NET_IMAP_IMAPSEARCHATTRIBUTES_HPP_INCLUDED
