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

#include "maildirFolder.hpp"

#include "maildirStore.hpp"
#include "maildirMessage.hpp"
#include "maildirUtils.hpp"

#include "../exception.hpp"
#include "../platformDependant.hpp"


namespace vmime {
namespace messaging {


maildirFolder::maildirFolder(const folder::path& path, maildirStore* store)
	: m_store(store), m_path(path), m_name(path.getLastComponent()), m_mode(-1), m_open(false)
{
	m_store->registerFolder(this);
}


maildirFolder::~maildirFolder()
{
	if (m_store)
	{
		if (m_open)
			close(false);

		m_store->unregisterFolder(this);
	}
	else if (m_open)
	{
		close(false);
	}
}


void maildirFolder::onStoreDisconnected()
{
	m_store = NULL;
}


const int maildirFolder::getMode() const
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return (m_mode);
}


const int maildirFolder::getType()
{
	if (m_path.isEmpty())
		return (TYPE_CONTAINS_FOLDERS);
	else
		return (TYPE_CONTAINS_FOLDERS | TYPE_CONTAINS_MESSAGES);
}


const int maildirFolder::getFlags()
{
	int flags = 0;

	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	utility::auto_ptr <utility::file> rootDir = fsf->create
		(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_CONTAINER));

	utility::auto_ptr <utility::fileIterator> it = rootDir->getFiles();

	while (it->hasMoreElements())
	{
		utility::auto_ptr <utility::file> file = it->nextElement();

		if (maildirUtils::isSubfolderDirectory(*file))
		{
			flags |= FLAG_CHILDREN; // Contains at least one sub-folder
			break;
		}
	}

	return (flags);
}


const folder::path::component maildirFolder::getName() const
{
	return (m_name);
}


const folder::path maildirFolder::getFullPath() const
{
	return (m_path);
}


void maildirFolder::open(const int mode, bool /* failIfModeIsNotAvailable */)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (isOpen())
		throw exceptions::illegal_state("Folder is already open");
	else if (!exists())
		throw exceptions::illegal_state("Folder already exists");

	m_open = true;
	m_mode = mode;
}


void maildirFolder::close(const bool expunge)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");

	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	m_open = false;
	m_mode = -1;

	onClose();
}


void maildirFolder::onClose()
{
	for (std::vector <maildirMessage*>::iterator it = m_messages.begin() ;
	     it != m_messages.end() ; ++it)
	{
		(*it)->onFolderClosed();
	}

	m_messages.clear();
}


void maildirFolder::registerMessage(maildirMessage* msg)
{
	m_messages.push_back(msg);
}


void maildirFolder::unregisterMessage(maildirMessage* msg)
{
	std::remove(m_messages.begin(), m_messages.end(), msg);
}


void maildirFolder::create(const int type)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (isOpen())
		throw exceptions::illegal_state("Folder is open");
	else if (exists())
		throw exceptions::illegal_state("Folder already exists");

	// Folder name cannot start with '.'
	if (!m_path.isEmpty())
	{
		const path::component& comp = m_path.getLastComponent();

		const int length = comp.getBuffer().length();
		int pos = 0;

		while ((pos < length) && (comp.getBuffer()[pos] == '.'))
			++pos;

		if (pos != 0)
			throw exceptions::invalid_folder_name("Name cannot start with '.'");
	}

	// Create directory on file system
	try
	{
		utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

		utility::auto_ptr <utility::file> rootDir = fsf->create
			(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_ROOT));

		utility::auto_ptr <utility::file> newDir = fsf->create
			(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_NEW));
		utility::auto_ptr <utility::file> tmpDir = fsf->create
			(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_TMP));
		utility::auto_ptr <utility::file> curDir = fsf->create
			(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_CUR));

		rootDir->createDirectory(true);

		newDir->createDirectory(false);
		tmpDir->createDirectory(false);
		curDir->createDirectory(false);
	}
	catch (exceptions::filesystem_exception& e)
	{
		throw exceptions::command_error("CREATE", e.what(), "File system exception");
	}

	// Notify folder created
	events::folderEvent event(this, events::folderEvent::TYPE_CREATED, m_path, m_path);
	notifyFolder(event);
}


const bool maildirFolder::exists()
{
	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	utility::auto_ptr <utility::file> rootDir = fsf->create
		(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_ROOT));

	utility::auto_ptr <utility::file> newDir = fsf->create
		(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_NEW));
	utility::auto_ptr <utility::file> tmpDir = fsf->create
		(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_TMP));
	utility::auto_ptr <utility::file> curDir = fsf->create
		(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_CUR));

	return (rootDir->exists() && rootDir->isDirectory() &&
	        newDir->exists() && newDir->isDirectory() &&
	        tmpDir->exists() && tmpDir->isDirectory() &&
	        curDir->exists() && curDir->isDirectory());
}


const bool maildirFolder::isOpen() const
{
	return (m_open);
}


void maildirFolder::scanFolder()
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	try
	{
		utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

		utility::file::path newDirPath = maildirUtils::getFolderFSPath
			(m_store, m_path, maildirUtils::FOLDER_PATH_NEW);
		utility::auto_ptr <utility::file> newDir = fsf->create(newDirPath);

		utility::file::path curDirPath = maildirUtils::getFolderFSPath
			(m_store, m_path, maildirUtils::FOLDER_PATH_CUR);
		utility::auto_ptr <utility::file> curDir = fsf->create(curDirPath);

		// New received messages (new/)
		utility::auto_ptr <utility::fileIterator> nit = newDir->getFiles();
		std::vector <utility::file::path::component> newMessageFilenames;

		while (nit->hasMoreElements())
		{
			utility::auto_ptr <utility::file> file = nit->nextElement();
			newMessageFilenames.push_back(file->fullPath().getLastComponent());
		}

		// Current messages (cur/)
		utility::auto_ptr <utility::fileIterator> cit = curDir->getFiles();
		std::vector <utility::file::path::component> curMessageFilenames;

		while (cit->hasMoreElements())
		{
			utility::auto_ptr <utility::file> file = cit->nextElement();
			curMessageFilenames.push_back(file->fullPath().getLastComponent());
		}

		// Update/delete existing messages (found in previous scan)
		for (unsigned int i = 0 ; i < m_messageInfos.size() ; ++i)
		{
			messageInfos& msgInfos = m_messageInfos[i];

			// NOTE: the flags may have changed (eg. moving from 'new' to 'cur'
			// may imply the 'S' flag) and so the filename. That's why we use
			// "maildirUtils::messageIdComparator" to compare only the 'unique'
			// portion of the filename...

			if (msgInfos.type == messageInfos::TYPE_CUR)
			{
				const std::vector <utility::file::path::component>::iterator pos =
					std::find_if(curMessageFilenames.begin(), curMessageFilenames.end(),
						maildirUtils::messageIdComparator(msgInfos.path));

				// If we cannot find this message in the 'cur' directory,
				// it means it has been deleted (and expunged).
				if (pos == curMessageFilenames.end())
				{
					msgInfos.type = messageInfos::TYPE_DELETED;
				}
				// Otherwise, update its information.
				else
				{
					msgInfos.path = *pos;
					curMessageFilenames.erase(pos);
				}
			}
		}

		// Add new messages from 'new': we are responsible to move the files
		// from the 'new' directory to the 'cur' directory, and append them
		// to our message list.
		for (std::vector <utility::file::path::component>::const_iterator
		     it = newMessageFilenames.begin() ; it != newMessageFilenames.end() ; ++it)
		{
			// Move messages from 'new' to 'cur'
			utility::auto_ptr <utility::file> file = fsf->create(newDirPath / *it);
			file->rename(curDirPath / *it);

			// Append to message list
			messageInfos msgInfos;
			msgInfos.path = *it;
			msgInfos.type = messageInfos::TYPE_CUR;

			m_messageInfos.push_back(msgInfos);
		}

		// Add new messages from 'cur': the files have already been moved
		// from 'new' to 'cur'. Just append them to our message list.
		for (std::vector <utility::file::path::component>::const_iterator
		     it = curMessageFilenames.begin() ; it != curMessageFilenames.end() ; ++it)
		{
			// Append to message list
			messageInfos msgInfos;
			msgInfos.path = *it;
			msgInfos.type = messageInfos::TYPE_CUR;

			m_messageInfos.push_back(msgInfos);
		}

		// Update message count
		int unreadMessageCount = 0;

		for (std::vector <messageInfos>::const_iterator
		     it = m_messageInfos.begin() ; it != m_messageInfos.end() ; ++it)
		{
			if ((maildirUtils::extractFlags((*it).path) & message::FLAG_SEEN) == 0)
				++unreadMessageCount;
		}

		m_unreadMessageCount = unreadMessageCount;
		m_messageCount = m_messageInfos.size();
	}
	catch (exceptions::filesystem_exception&)
	{
		// Should not happen...
	}
}


message* maildirFolder::getMessage(const int num)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (num < 1 || num > m_messageCount)
		throw exceptions::message_not_found();

	return new maildirMessage(this, num);
}


std::vector <message*> maildirFolder::getMessages(const int from, const int to)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	std::vector <message*> v;

	for (int i = from ; i <= to ; ++i)
		v.push_back(new maildirMessage(this, i));

	return (v);
}


std::vector <message*> maildirFolder::getMessages(const std::vector <int>& nums)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	std::vector <message*> v;

	for (std::vector <int>::const_iterator it = nums.begin() ; it != nums.end() ; ++it)
		v.push_back(new maildirMessage(this, *it));

	return (v);
}


const int maildirFolder::getMessageCount()
{
	return (m_messageCount);
}


folder* maildirFolder::getFolder(const folder::path::component& name)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");

	return new maildirFolder(m_path / name, m_store);
}


std::vector <folder*> maildirFolder::getFolders(const bool recursive)
{
	if (!isOpen() && !m_store)
		throw exceptions::illegal_state("Store disconnected");

	std::vector <folder*> list;

	try
	{
		listFolders(list, recursive);
	}
	catch (std::exception&)
	{
		for (std::vector <folder*>::iterator it = list.begin() ; it != list.end() ; ++it)
			delete (*it);

		throw;
	}

	return (list);
}


void maildirFolder::listFolders(std::vector <folder*>& list, const bool recursive)
{
	try
	{
		utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

		utility::auto_ptr <utility::file> rootDir = fsf->create
			(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_CONTAINER));
		utility::auto_ptr <utility::fileIterator> it = rootDir->getFiles();

		while (it->hasMoreElements())
		{
			utility::auto_ptr <utility::file> file = it->nextElement();

			if (maildirUtils::isSubfolderDirectory(*file))
			{
				const utility::path subPath = m_path / file->fullPath().getLastComponent();
				maildirFolder* subFolder = new maildirFolder(subPath, m_store);

				list.push_back(subFolder);

				if (recursive)
					subFolder->listFolders(list, true);
			}
		}
	}
	catch (exceptions::filesystem_exception& e)
	{
		throw exceptions::command_error("LIST", e.what());
	}
}


void maildirFolder::rename(const folder::path& newPath)
{
	// TODO
}


void maildirFolder::deleteMessage(const int num)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	if (m_messageInfos[num].type == messageInfos::TYPE_DELETED)
		return;

	m_messageInfos[num].type = messageInfos::TYPE_DELETED;

	// Delete file from file system
	try
	{
		utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

		utility::file::path curDirPath = maildirUtils::getFolderFSPath
			(m_store, m_path, maildirUtils::FOLDER_PATH_CUR);

		utility::auto_ptr <utility::file> file = fsf->create
			(curDirPath / m_messageInfos[num].path);

		file->remove();
	}
	catch (exceptions::filesystem_exception& e)
	{
		// Ignore (not important)
	}

	// Update local flags
	for (std::vector <maildirMessage*>::iterator it =
	     m_messages.begin() ; it != m_messages.end() ; ++it)
	{
		if ((*it)->getNumber() == num &&
			(*it)->m_flags != message::FLAG_UNDEFINED)
		{
			(*it)->m_flags |= message::FLAG_DELETED;
		}
	}

	// Notify message flags changed
	std::vector <int> nums;
	nums.push_back(num);

	events::messageChangedEvent event(this, events::messageChangedEvent::TYPE_FLAGS, nums);

	notifyMessageChanged(event);
}


void maildirFolder::deleteMessages(const int from, const int to)
{
	if (from < 1 || (to < from && to != -1))
		throw exceptions::invalid_argument();

	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	const int to2 = (to == -1) ? m_messageCount : to;
	const int count = to - from + 1;

	// Delete files from file system
	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	utility::file::path curDirPath = maildirUtils::getFolderFSPath
		(m_store, m_path, maildirUtils::FOLDER_PATH_CUR);

	for (int i = from ; i <= to2 ; ++i)
	{
		if (m_messageInfos[i].type != messageInfos::TYPE_DELETED)
		{
			m_messageInfos[i].type = messageInfos::TYPE_DELETED;

			try
			{
				utility::auto_ptr <utility::file> file = fsf->create
					(curDirPath / m_messageInfos[i].path);

				file->remove();
			}
			catch (exceptions::filesystem_exception& e)
			{
				// Ignore (not important)
			}
		}
	}

	// Update local flags
	for (std::vector <maildirMessage*>::iterator it =
	     m_messages.begin() ; it != m_messages.end() ; ++it)
	{
		if ((*it)->getNumber() >= from && (*it)->getNumber() <= to2 &&
		    (*it)->m_flags != message::FLAG_UNDEFINED)
		{
			(*it)->m_flags |= message::FLAG_DELETED;
		}
	}

	// Notify message flags changed
	std::vector <int> nums;
	nums.resize(count);

	for (int i = from, j = 0 ; i <= to2 ; ++i, ++j)
		nums[j] = i;

	events::messageChangedEvent event(this, events::messageChangedEvent::TYPE_FLAGS, nums);

	notifyMessageChanged(event);
}


void maildirFolder::deleteMessages(const std::vector <int>& nums)
{
	if (nums.empty())
		throw exceptions::invalid_argument();

	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	// Sort the list of message numbers
	std::vector <int> list;

	list.resize(nums.size());
	std::copy(nums.begin(), nums.end(), list.begin());

	std::sort(list.begin(), list.end());

	// Delete files from file system
	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	utility::file::path curDirPath = maildirUtils::getFolderFSPath
		(m_store, m_path, maildirUtils::FOLDER_PATH_CUR);

	for (std::vector <int>::const_iterator it =
	     list.begin() ; it != list.end() ; ++it)
	{
		const int num = *it;

		if (m_messageInfos[num].type != messageInfos::TYPE_DELETED)
		{
			m_messageInfos[num].type = messageInfos::TYPE_DELETED;

			try
			{
				utility::auto_ptr <utility::file> file = fsf->create
					(curDirPath / m_messageInfos[num].path);

				file->remove();
			}
			catch (exceptions::filesystem_exception& e)
			{
				// Ignore (not important)
			}
		}
	}


	// Update local flags
	for (std::vector <maildirMessage*>::iterator it =
	     m_messages.begin() ; it != m_messages.end() ; ++it)
	{
		if (std::binary_search(list.begin(), list.end(), (*it)->getNumber()))
		{
			if ((*it)->m_flags != message::FLAG_UNDEFINED)
				(*it)->m_flags |= message::FLAG_DELETED;
		}
	}

	// Notify message flags changed
	events::messageChangedEvent event(this, events::messageChangedEvent::TYPE_FLAGS, list);

	notifyMessageChanged(event);
}


void maildirFolder::setMessageFlags
	(const int from, const int to, const int flags, const int mode)
{
	// TODO
}


void maildirFolder::setMessageFlags
	(const std::vector <int>& nums, const int flags, const int mode)
{
	// TODO
}


void maildirFolder::addMessage(vmime::message* msg, const int flags,
	vmime::datetime* date, progressionListener* progress)
{
	// TODO
}


void maildirFolder::addMessage(utility::inputStream& is, const int size,
	const int flags, vmime::datetime* date, progressionListener* progress)
{
	// TODO
}


void maildirFolder::copyMessage(const folder::path& dest, const int num)
{
	// TODO
}


void maildirFolder::copyMessages(const folder::path& dest, const int from, const int to)
{
	// TODO
}


void maildirFolder::copyMessages(const folder::path& dest, const std::vector <int>& nums)
{
	// TODO
}


void maildirFolder::status(int& count, int& unseen)
{
	const int oldCount = m_messageCount;

	scanFolder();

	count = m_messageCount;
	unseen = m_unreadMessageCount;

	// Notify message count changed (new messages)
	if (count > oldCount)
	{
		std::vector <int> nums;
		nums.reserve(count - oldCount);

		for (int i = oldCount + 1, j = 0 ; i <= count ; ++i, ++j)
			nums[j] = i;

		events::messageCountEvent event(this, events::messageCountEvent::TYPE_ADDED, nums);

		for (std::list <maildirFolder*>::iterator it = m_store->m_folders.begin() ;
		     it != m_store->m_folders.end() ; ++it)
		{
			if ((*it)->getFullPath() == m_path)
			{
				(*it)->m_messageCount = count;
				(*it)->notifyMessageCount(event);

				(*it)->scanFolder();
			}
		}
	}
}


void maildirFolder::expunge()
{
	// TODO
}


folder* maildirFolder::getParent()
{
	return (m_path.isEmpty() ? NULL : new maildirFolder(m_path.getParent(), m_store));
}


const store* maildirFolder::getStore() const
{
	return (m_store);
}


store* maildirFolder::getStore()
{
	return (m_store);
}


void maildirFolder::fetchMessages(std::vector <message*>& msg,
	const int options, progressionListener* progress)
{
	// TODO
}


void maildirFolder::fetchMessage(message* msg, const int options)
{
	// TODO
}


const int maildirFolder::getFetchCapabilities() const
{
	return (FETCH_ENVELOPE | FETCH_STRUCTURE | FETCH_CONTENT_INFO |
	        FETCH_FLAGS | FETCH_SIZE | FETCH_FULL_HEADER | FETCH_UID);
}


} // messaging
} // vmime
