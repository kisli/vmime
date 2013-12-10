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

#ifndef VMIME_NET_POP3_POP3MESSAGE_HPP_INCLUDED
#define VMIME_NET_POP3_POP3MESSAGE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/message.hpp"
#include "vmime/net/folder.hpp"


namespace vmime {
namespace net {
namespace pop3 {


class POP3Folder;


/** POP3 message implementation.
  */

class VMIME_EXPORT POP3Message : public message
{
private:

	friend class POP3Folder;

	POP3Message(const POP3Message&);

public:

	POP3Message(shared_ptr <POP3Folder> folder, const int num);

	~POP3Message();


	int getNumber() const;

	const uid getUID() const;

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

	void fetch(shared_ptr <POP3Folder> folder, const fetchAttributes& options);

	void onFolderClosed();

	weak_ptr <POP3Folder> m_folder;
	int m_num;
	uid m_uid;
	size_t m_size;

	bool m_deleted;

	shared_ptr <header> m_header;
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_POP3_POP3MESSAGE_HPP_INCLUDED
