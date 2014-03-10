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

#ifndef VMIME_NET_MAILDIR_MAILDIRFOLDER_HPP_INCLUDED
#define VMIME_NET_MAILDIR_MAILDIRFOLDER_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include <vector>
#include <map>

#include "vmime/types.hpp"

#include "vmime/net/folder.hpp"

#include "vmime/utility/file.hpp"


namespace vmime {
namespace net {
namespace maildir {


class maildirStore;
class maildirMessage;


/** maildir folder implementation.
  */

class VMIME_EXPORT maildirFolder : public folder
{
private:

	friend class maildirStore;
	friend class maildirMessage;

	maildirFolder(const maildirFolder&) : folder() { }

public:

	maildirFolder(const folder::path& path, shared_ptr <maildirStore> store);

	~maildirFolder();


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

	void scanFolder();

	void listFolders(std::vector <shared_ptr <folder> >& list, const bool recursive);

	void registerMessage(maildirMessage* msg);
	void unregisterMessage(maildirMessage* msg);

	const utility::file::path getMessageFSPath(const int number) const;

	void onStoreDisconnected();

	void onClose();

	void deleteMessagesImpl(const std::vector <int>& nums);
	void setMessageFlagsImpl(const std::vector <int>& nums, const int flags, const int mode);

	void copyMessagesImpl(const folder::path& dest, const std::vector <int>& nums);
	void copyMessageImpl(const utility::file::path& tmpDirPath, const utility::file::path& curDirPath, const utility::file::path::component& filename, utility::inputStream& is, const size_t size, utility::progressListener* progress);

	void notifyMessagesCopied(const folder::path& dest);


	weak_ptr <maildirStore> m_store;

	folder::path m_path;
	folder::path::component m_name;

	int m_mode;
	bool m_open;

	int m_unreadMessageCount;
	int m_messageCount;

	// Store information about scanned messages
	struct messageInfos
	{
		enum Type
		{
			TYPE_CUR,
			TYPE_DELETED
		};

		utility::file::path::component path;    // filename
		Type type;                              // current location
	};

	std::vector <messageInfos> m_messageInfos;

	// Instanciated message objects
	std::vector <maildirMessage*> m_messages;
};


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

#endif // VMIME_NET_MAILDIR_MAILDIRFOLDER_HPP_INCLUDED
