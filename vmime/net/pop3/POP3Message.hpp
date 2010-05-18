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

#ifndef VMIME_NET_POP3_POP3MESSAGE_HPP_INCLUDED
#define VMIME_NET_POP3_POP3MESSAGE_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/net/message.hpp"
#include "vmime/net/folder.hpp"


namespace vmime {
namespace net {
namespace pop3 {


class POP3Folder;


/** POP3 message implementation.
  */

class POP3Message : public message
{
private:

	friend class POP3Folder;
	friend class vmime::creator;  // vmime::create <POP3Message>

	POP3Message(ref <POP3Folder> folder, const int num);
	POP3Message(const POP3Message&) : message() { }

	~POP3Message();

public:

	int getNumber() const;

	const uid getUniqueId() const;

	int getSize() const;

	bool isExpunged() const;

	ref <const structure> getStructure() const;
	ref <structure> getStructure();

	ref <const header> getHeader() const;

	int getFlags() const;
	void setFlags(const int flags, const int mode = FLAG_MODE_SET);

	void extract(utility::outputStream& os, utility::progressListener* progress = NULL, const int start = 0, const int length = -1, const bool peek = false) const;
	void extractPart(ref <const part> p, utility::outputStream& os, utility::progressListener* progress = NULL, const int start = 0, const int length = -1, const bool peek = false) const;

	void fetchPartHeader(ref <part> p);

	ref <vmime::message> getParsedMessage();

private:

	void fetch(ref <POP3Folder> folder, const int options);

	void onFolderClosed();

	weak_ref <POP3Folder> m_folder;
	int m_num;
	uid m_uid;
	int m_size;

	bool m_deleted;

	ref <header> m_header;
};


} // pop3
} // net
} // vmime


#endif // VMIME_NET_POP3_POP3MESSAGE_HPP_INCLUDED
