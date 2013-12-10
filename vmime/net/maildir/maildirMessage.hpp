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

#ifndef VMIME_NET_MAILDIR_MAILDIRMESSAGE_HPP_INCLUDED
#define VMIME_NET_MAILDIR_MAILDIRMESSAGE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/net/message.hpp"
#include "vmime/net/folder.hpp"


namespace vmime {
namespace net {
namespace maildir {


class maildirFolder;


/** maildir message implementation.
  */

class VMIME_EXPORT maildirMessage : public message
{
	friend class maildirFolder;

	maildirMessage(const maildirMessage&) : message() { }

public:

	maildirMessage(shared_ptr <maildirFolder> folder, const int num);

	~maildirMessage();


	int getNumber() const;

	const uid getUID() const;

	size_t getSize() const;

	bool isExpunged() const;

	shared_ptr <const messageStructure> getStructure() const;
	shared_ptr <messageStructure> getStructure();

	shared_ptr <const header> getHeader() const;

	int getFlags() const;
	void setFlags(const int flags, const int mode = FLAG_MODE_SET);

	void extract(utility::outputStream& os, utility::progressListener* progress = NULL, const size_t start = 0, const size_t length = -1, const bool peek = false) const;
	void extractPart(shared_ptr <const messagePart> p, utility::outputStream& os, utility::progressListener* progress = NULL, const size_t start = 0, const size_t length = -1, const bool peek = false) const;

	void fetchPartHeader(shared_ptr <messagePart> p);

	shared_ptr <vmime::message> getParsedMessage();

private:

	void fetch(shared_ptr <maildirFolder> folder, const fetchAttributes& options);

	void onFolderClosed();

	shared_ptr <header> getOrCreateHeader();

	void extractImpl(utility::outputStream& os, utility::progressListener* progress, const size_t start, const size_t length, const size_t partialStart, const size_t partialLength, const bool peek) const;


	weak_ptr <maildirFolder> m_folder;

	int m_num;
	size_t m_size;
	int m_flags;
	bool m_expunged;
	uid m_uid;

	shared_ptr <header> m_header;
	shared_ptr <messageStructure> m_structure;
};


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

#endif // VMIME_NET_MAILDIR_MAILDIRMESSAGE_HPP_INCLUDED
