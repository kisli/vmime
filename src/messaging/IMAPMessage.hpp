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

#ifndef VMIME_MESSAGING_IMAPMESSAGE_HPP_INCLUDED
#define VMIME_MESSAGING_IMAPMESSAGE_HPP_INCLUDED


#include "messaging/message.hpp"
#include "messaging/folder.hpp"

#include "mailboxList.hpp"


namespace vmime {
namespace messaging {


/** IMAP message implementation.
  */

class IMAPMessage : public message
{
private:

	friend class IMAPFolder;

	IMAPMessage(IMAPFolder* folder, const int num);
	IMAPMessage(const IMAPMessage&) : message() { }

	~IMAPMessage();

public:

	const int getNumber() const;

	const uid getUniqueId() const;

	const int getSize() const;

	const bool isExpunged() const;

	const structure& getStructure() const;
	structure& getStructure();

	const header& getHeader() const;

	const int getFlags() const;
	void setFlags(const int flags, const int mode = FLAG_MODE_SET);

	void extract(utility::outputStream& os, progressionListener* progress = NULL, const int start = 0, const int length = -1) const;
	void extractPart(const part& p, utility::outputStream& os, progressionListener* progress = NULL, const int start = 0, const int length = -1) const;

	void fetchPartHeader(part& p);

private:

	void fetch(IMAPFolder* folder, const int options);

	void processFetchResponse(const int options, const IMAPParser::msg_att* msgAtt);

	void extract(const part* p, utility::outputStream& os, progressionListener* progress, const int start, const int length, const bool headerOnly) const;


	void convertAddressList(const IMAPParser::address_list& src, mailboxList& dest);


	header& getOrCreateHeader();


	void onFolderClosed();

	IMAPFolder* m_folder;

	int m_num;
	int m_size;
	int m_flags;
	bool m_expunged;
	uid m_uid;

	header* m_header;
	structure* m_structure;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_IMAPMESSAGE_HPP_INCLUDED
