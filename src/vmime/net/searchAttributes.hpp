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

#ifndef VMIME_NET_SEARCHATTRIBUTES_HPP_INCLUDED
#define VMIME_NET_SEARCHATTRIBUTES_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include <vector>

#include "vmime/types.hpp"
#include "vmime/utility/stringUtils.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/net/message.hpp"
#include "vmime/net/messageSet.hpp"

namespace vmime {
namespace net {

class searchToken : public object {

public:
	searchToken(const char* token)
		: m_token(token) {

		if (nullptr == m_token) {
			throw exceptions::invalid_argument();
		}
	}

	virtual void generate(std::ostringstream& out) const = 0;

protected:
	const char* m_token;
};

typedef vmime::shared_ptr< const searchToken> searchTokenPtr;

class searchTokenFactory : public object {

public:
	static searchTokenPtr AND(const std::vector< searchTokenPtr >&);
	static searchTokenPtr AND(const std::vector< searchTokenPtr >&&);
	static searchTokenPtr ANSWERED();
	static searchTokenPtr BCC(const string&);
	static searchTokenPtr BEFORE(const datetime&);
	static searchTokenPtr BODY(const string&);
	static searchTokenPtr CC(const string&);
	static searchTokenPtr DELETED();
	static searchTokenPtr DRAFT();
	static searchTokenPtr FLAGGED();
	static searchTokenPtr FROM(const string&);
	static searchTokenPtr HEADER(const char* fieldName);
	static searchTokenPtr HEADER(const char* filedName, const string& fieldContents);
	static searchTokenPtr KEYWORD(vmime::net::message::Flags);
	static searchTokenPtr LARGER(uint32_t);
	static searchTokenPtr MESSAGESET(const vmime::net::messageSet&);
	static searchTokenPtr MESSAGESET(const vmime::net::messageSet&&);
	static searchTokenPtr NEW();
	static searchTokenPtr NOT(const searchTokenPtr&);
	static searchTokenPtr OLD();
	static searchTokenPtr ON(const datetime&);
	static searchTokenPtr OR(const searchTokenPtr&, const searchTokenPtr&);
	static searchTokenPtr RECENT();
	static searchTokenPtr SEEN();
	static searchTokenPtr SENTBEFORE(const datetime&);
	static searchTokenPtr SENTON(const datetime&);
	static searchTokenPtr SENTSINCE(const datetime&);
	static searchTokenPtr SINCE(const datetime&);
	static searchTokenPtr SMALLER(uint32_t);
	static searchTokenPtr SUBJECT(const string&);
	static searchTokenPtr TEXT(const string&);
	static searchTokenPtr TO(const string&);
	static searchTokenPtr UID(const vmime::net::messageSet&);
	static searchTokenPtr UID(const vmime::net::messageSet&&);
	static searchTokenPtr UNANSWERED();
	static searchTokenPtr UNDELETED();
	static searchTokenPtr UNDRAFT();
	static searchTokenPtr UNFLAGGED();
	static searchTokenPtr UNKEYWORD(vmime::net::message::Flags);
	static searchTokenPtr UNSEEN();
};

/** Holds a set of attributes to match messages against when searching folder contents.
  */
class VMIME_EXPORT searchAttributes : public object {

public:
	searchAttributes() = default;
	searchAttributes(std::vector< vmime::shared_ptr< const searchToken > >&&);

	/** Adds a new search token that will be used to match messages against. Multiple tokens are
	  * treated as an AND operation.
	  *
	  * @param token the search token to add
	  */
	void add(const vmime::shared_ptr< const searchToken >& token);

	std::vector< string > generate() const;

protected:

	std::vector< vmime::shared_ptr< const searchToken > > m_andTokens;
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES


#endif // VMIME_NET_SEARCHATTRIBUTES_HPP_INCLUDED
