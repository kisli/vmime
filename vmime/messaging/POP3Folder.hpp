//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_MESSAGING_POP3FOLDER_HPP_INCLUDED
#define VMIME_MESSAGING_POP3FOLDER_HPP_INCLUDED


#include <vector>
#include <map>

#include "vmime/config.hpp"
#include "vmime/types.hpp"

#include "vmime/messaging/folder.hpp"


namespace vmime {
namespace messaging {


class POP3Store;
class POP3Message;


/** POP3 folder implementation.
  */

class POP3Folder : public folder
{
private:

	friend class POP3Store;
	friend class POP3Message;

	POP3Folder(const folder::path& path, POP3Store* store);
	POP3Folder(const POP3Folder&) : folder() { }

	~POP3Folder();

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

	void registerMessage(POP3Message* msg);
	void unregisterMessage(POP3Message* msg);

	void onStoreDisconnected();

	void onClose();


	POP3Store* m_store;

	folder::path m_path;
	folder::path::component m_name;

	int m_mode;
	bool m_open;

	int m_messageCount;

	typedef std::map <POP3Message*, int> MessageMap;
	MessageMap m_messages;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_POP3FOLDER_HPP_INCLUDED
