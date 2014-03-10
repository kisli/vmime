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

#ifndef VMIME_NET_POP3_POP3FOLDER_HPP_INCLUDED
#define VMIME_NET_POP3_POP3FOLDER_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include <vector>
#include <map>

#include "vmime/types.hpp"

#include "vmime/net/folder.hpp"


namespace vmime {
namespace net {
namespace pop3 {


class POP3Store;
class POP3Message;


/** POP3 folder implementation.
  */

class VMIME_EXPORT POP3Folder : public folder
{
private:

	friend class POP3Store;
	friend class POP3Message;

	POP3Folder(const POP3Folder&);

public:

	POP3Folder(const folder::path& path, shared_ptr <POP3Store> store);

	~POP3Folder();

	int getMode() const;

	const folderAttributes getAttributes();

	const folder::path::component getName() const;
	const folder::path getFullPath() const;

	void open(const int mode, bool failIfModeIsNotAvailable = false);
	void close(const bool expunge);
	void create(const folderAttributes& attribs);

	bool exists();

	void destroy();

	bool isOpen() const;

	shared_ptr <message> getMessage(const int num);
	std::vector <shared_ptr <message> > getMessages(const messageSet& msgs);

	int getMessageCount();

	shared_ptr <folder> getFolder(const folder::path::component& name);
	std::vector <shared_ptr <folder> > getFolders(const bool recursive = false);

	void rename(const folder::path& newPath);

	void deleteMessages(const messageSet& msgs);

	void setMessageFlags(const messageSet& msgs, const int flags, const int mode = message::FLAG_MODE_SET);

	messageSet addMessage(shared_ptr <vmime::message> msg, const int flags = -1, vmime::datetime* date = NULL, utility::progressListener* progress = NULL);
	messageSet addMessage(utility::inputStream& is, const size_t size, const int flags = -1, vmime::datetime* date = NULL, utility::progressListener* progress = NULL);

	messageSet copyMessages(const folder::path& dest, const messageSet& msgs);

	void status(int& count, int& unseen);
	shared_ptr <folderStatus> getStatus();

	void expunge();

	shared_ptr <folder> getParent();

	shared_ptr <const store> getStore() const;
	shared_ptr <store> getStore();


	void fetchMessages(std::vector <shared_ptr <message> >& msg, const fetchAttributes& options, utility::progressListener* progress = NULL);
	void fetchMessage(shared_ptr <message> msg, const fetchAttributes& options);

	std::vector <shared_ptr <message> > getAndFetchMessages
		(const messageSet& msgs, const fetchAttributes& attribs);

	int getFetchCapabilities() const;

	std::vector <int> getMessageNumbersStartingOnUID(const message::uid& uid);

private:

	void registerMessage(POP3Message* msg);
	void unregisterMessage(POP3Message* msg);

	void onStoreDisconnected();

	void onClose();


	weak_ptr <POP3Store> m_store;

	folder::path m_path;
	folder::path::component m_name;

	int m_mode;
	bool m_open;

	int m_messageCount;

	typedef std::map <POP3Message*, int> MessageMap;
	MessageMap m_messages;
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_POP3_POP3FOLDER_HPP_INCLUDED
