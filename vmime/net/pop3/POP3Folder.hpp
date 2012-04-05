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

#ifndef VMIME_NET_POP3_POP3FOLDER_HPP_INCLUDED
#define VMIME_NET_POP3_POP3FOLDER_HPP_INCLUDED


#include <vector>
#include <map>

#include "vmime/config.hpp"
#include "vmime/types.hpp"

#include "vmime/net/folder.hpp"


namespace vmime {
namespace net {
namespace pop3 {


class POP3Store;
class POP3Message;


/** POP3 folder implementation.
  */

class POP3Folder : public folder
{
private:

	friend class POP3Store;
	friend class POP3Message;
	friend class vmime::creator;  // vmime::create <POP3Folder>

	POP3Folder(const folder::path& path, ref <POP3Store> store);
	POP3Folder(const POP3Folder&) : folder() { }

	~POP3Folder();

public:

	int getMode() const;

	int getType();

	int getFlags();

	const folder::path::component getName() const;
	const folder::path getFullPath() const;

	void open(const int mode, bool failIfModeIsNotAvailable = false);
	void close(const bool expunge);
	void create(const int type);

	bool exists();

	void destroy();

	bool isOpen() const;

	ref <message> getMessage(const int num);
	std::vector <ref <message> > getMessages(const int from = 1, const int to = -1);
	std::vector <ref <message> > getMessages(const std::vector <int>& nums);
	int getMessageCount();

	ref <folder> getFolder(const folder::path::component& name);
	std::vector <ref <folder> > getFolders(const bool recursive = false);

	void rename(const folder::path& newPath);

	void deleteMessage(const int num);
	void deleteMessages(const int from = 1, const int to = -1);
	void deleteMessages(const std::vector <int>& nums);

	void setMessageFlags(const int from, const int to, const int flags, const int mode = message::FLAG_MODE_SET);
	void setMessageFlags(const std::vector <int>& nums, const int flags, const int mode = message::FLAG_MODE_SET);

	void addMessage(ref <vmime::message> msg, const int flags = message::FLAG_UNDEFINED, vmime::datetime* date = NULL, utility::progressListener* progress = NULL);
	void addMessage(utility::inputStream& is, const int size, const int flags = message::FLAG_UNDEFINED, vmime::datetime* date = NULL, utility::progressListener* progress = NULL);

	void copyMessage(const folder::path& dest, const int num);
	void copyMessages(const folder::path& dest, const int from = 1, const int to = -1);
	void copyMessages(const folder::path& dest, const std::vector <int>& nums);

	void status(int& count, int& unseen);

	void expunge();

	ref <folder> getParent();

	ref <const store> getStore() const;
	ref <store> getStore();


	void fetchMessages(std::vector <ref <message> >& msg, const int options, utility::progressListener* progress = NULL);
	void fetchMessage(ref <message> msg, const int options);

	int getFetchCapabilities() const;

	std::vector <int> getMessageNumbersStartingOnUID(const message::uid& uid);

private:

	void registerMessage(POP3Message* msg);
	void unregisterMessage(POP3Message* msg);

	void onStoreDisconnected();

	void onClose();


	weak_ref <POP3Store> m_store;

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


#endif // VMIME_NET_POP3_POP3FOLDER_HPP_INCLUDED
