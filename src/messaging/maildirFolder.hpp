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

#ifndef VMIME_MESSAGING_MAILDIRFOLDER_HPP_INCLUDED
#define VMIME_MESSAGING_MAILDIRFOLDER_HPP_INCLUDED


#include <vector>
#include <map>

#include "types.hpp"

#include "messaging/folder.hpp"

#include "utility/file.hpp"


namespace vmime {
namespace messaging {


class maildirStore;


/** maildir folder implementation.
  */

class maildirFolder : public folder
{
private:

	friend class maildirStore;
	friend class maildirMessage;


	maildirFolder(const folder::path& path, maildirStore* store);
	maildirFolder(const maildirFolder&) : folder() { }

	~maildirFolder();

public:

	const int getMode() const;

	const int getType();

	const int getFlags();

	const folder::path::component getName() const;
	const folder::path getFullPath() const;

	void open(const int mode, bool failIfModeIsNotAvailable = false);
	void close(const bool expunge);
	void create(const int type);

	const bool exists();

	const bool isOpen() const;

	message* getMessage(const int num);
	std::vector <message*> getMessages(const int from = 1, const int to = -1);
	std::vector <message*> getMessages(const std::vector <int>& nums);
	const int getMessageCount();

	folder* getFolder(const folder::path::component& name);
	std::vector <folder*> getFolders(const bool recursive = false);

	void rename(const folder::path& newPath);

	void deleteMessage(const int num);
	void deleteMessages(const int from = 1, const int to = -1);
	void deleteMessages(const std::vector <int>& nums);

	void setMessageFlags(const int from, const int to, const int flags, const int mode = message::FLAG_MODE_SET);
	void setMessageFlags(const std::vector <int>& nums, const int flags, const int mode = message::FLAG_MODE_SET);

	void addMessage(vmime::message* msg, const int flags = message::FLAG_UNDEFINED, vmime::datetime* date = NULL, progressionListener* progress = NULL);
	void addMessage(utility::inputStream& is, const int size, const int flags = message::FLAG_UNDEFINED, vmime::datetime* date = NULL, progressionListener* progress = NULL);

	void copyMessage(const folder::path& dest, const int num);
	void copyMessages(const folder::path& dest, const int from = 1, const int to = -1);
	void copyMessages(const folder::path& dest, const std::vector <int>& nums);

	void status(int& count, int& unseen);

	void expunge();

	folder* getParent();

	const store* getStore() const;
	store* getStore();


	void fetchMessages(std::vector <message*>& msg, const int options, progressionListener* progress = NULL);
	void fetchMessage(message* msg, const int options);

	const int getFetchCapabilities() const;

private:

	void scanFolder();

	void listFolders(std::vector <folder*>& list, const bool recursive);

	void registerMessage(maildirMessage* msg);
	void unregisterMessage(maildirMessage* msg);

	const utility::file::path getMessageFSPath(const int number);

	void onStoreDisconnected();

	void onClose();

	void deleteMessagesImpl(const std::vector <int>& nums);
	void setMessageFlagsImpl(const std::vector <int>& nums, const int flags, const int mode);

	void copyMessagesImpl(const folder::path& dest, const std::vector <int>& nums);
	void copyMessageImpl(const utility::file::path& tmpDirPath, const utility::file::path& curDirPath, const utility::file::path::component& filename, utility::inputStream& is, const utility::stream::size_type size, progressionListener* progress);

	void notifyMessagesCopied(const folder::path& dest);


	maildirStore* m_store;

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


} // messaging
} // vmime


#endif // VMIME_MESSAGING_MAILDIRFOLDER_HPP_INCLUDED
