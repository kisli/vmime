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
	: m_store(store), m_path(path), m_name(path.getLastComponent()), m_mode(-1), m_open(false),
	  m_unreadMessageCount(0), m_messageCount(0)
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

	scanFolder();

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
	std::vector <maildirMessage*>::iterator it =
		std::find(m_messages.begin(), m_messages.end(), msg);

	if (it != m_messages.end())
		m_messages.erase(it);
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

		if (!fsf->isValidPath(maildirUtils::getFolderFSPath(m_store, m_path, maildirUtils::FOLDER_PATH_ROOT)))
			throw exceptions::invalid_folder_name();

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
	try
	{
		m_messageCount = 0;
		m_unreadMessageCount = 0;

		utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

		utility::file::path newDirPath = maildirUtils::getFolderFSPath
			(m_store, m_path, maildirUtils::FOLDER_PATH_NEW);
		utility::auto_ptr <utility::file> newDir = fsf->create(newDirPath);

		utility::file::path curDirPath = maildirUtils::getFolderFSPath
			(m_store, m_path, maildirUtils::FOLDER_PATH_CUR);
		utility::auto_ptr <utility::file> curDir = fsf->create(curDirPath);

		// New received messages (new/)
		utility::fileIterator* nit = newDir->getFiles();
		std::vector <utility::file::path::component> newMessageFilenames;

		while (nit->hasMoreElements())
		{
			utility::auto_ptr <utility::file> file = nit->nextElement();
			newMessageFilenames.push_back(file->getFullPath().getLastComponent());
		}

		delete (nit);  // Free directory

		// Current messages (cur/)
		utility::fileIterator* cit = curDir->getFiles();
		std::vector <utility::file::path::component> curMessageFilenames;

		while (cit->hasMoreElements())
		{
			utility::auto_ptr <utility::file> file = cit->nextElement();
			curMessageFilenames.push_back(file->getFullPath().getLastComponent());
		}

		delete (cit);  // Free directory

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

		m_messageInfos.reserve(m_messageInfos.size()
			+ newMessageFilenames.size() + curMessageFilenames.size());

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
			(maildirUtils::getFolderFSPath(m_store, m_path,
				m_path.isEmpty() ? maildirUtils::FOLDER_PATH_ROOT
				                 : maildirUtils::FOLDER_PATH_CONTAINER));

		if (rootDir->exists())
		{
			utility::auto_ptr <utility::fileIterator> it = rootDir->getFiles();

			while (it->hasMoreElements())
			{
				utility::auto_ptr <utility::file> file = it->nextElement();

				if (maildirUtils::isSubfolderDirectory(*file))
				{
					const utility::path subPath = m_path / file->getFullPath().getLastComponent();
					maildirFolder* subFolder = new maildirFolder(subPath, m_store);

					list.push_back(subFolder);

					if (recursive)
						subFolder->listFolders(list, true);
				}
			}
		}
		else
		{
			// No sub-folder
		}
	}
	catch (exceptions::filesystem_exception& e)
	{
		throw exceptions::command_error("LIST", "", "", e);
	}
}


void maildirFolder::rename(const folder::path& newPath)
{
	// TODO
}


void maildirFolder::deleteMessage(const int num)
{
	// Mark messages as deleted
	setMessageFlags(num, num, message::FLAG_MODE_ADD, message::FLAG_DELETED);
}


void maildirFolder::deleteMessages(const int from, const int to)
{
	// Mark messages as deleted
	setMessageFlags(from, to, message::FLAG_MODE_ADD, message::FLAG_DELETED);
}


void maildirFolder::deleteMessages(const std::vector <int>& nums)
{
	// Mark messages as deleted
	setMessageFlags(nums, message::FLAG_MODE_ADD, message::FLAG_DELETED);
}


void maildirFolder::setMessageFlags
	(const int from, const int to, const int flags, const int mode)
{
	if (from < 1 || (to < from && to != -1))
		throw exceptions::invalid_argument();

	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	// Construct the list of message numbers
	const int to2 = (to == -1) ? m_messageCount : to;
	const int count = to - from + 1;

	std::vector <int> nums;
	nums.resize(count);

	for (int i = from, j = 0 ; i <= to2 ; ++i, ++j)
		nums[j] = i;

	// Change message flags
	setMessageFlagsImpl(nums, flags, mode);

	// Update local flags
	switch (mode)
	{
	case message::FLAG_MODE_ADD:
	{
		for (std::vector <maildirMessage*>::iterator it =
		     m_messages.begin() ; it != m_messages.end() ; ++it)
		{
			if ((*it)->getNumber() >= from && (*it)->getNumber() <= to2 &&
			    (*it)->m_flags != message::FLAG_UNDEFINED)
			{
				(*it)->m_flags |= flags;
			}
		}

		break;
	}
	case message::FLAG_MODE_REMOVE:
	{
		for (std::vector <maildirMessage*>::iterator it =
		     m_messages.begin() ; it != m_messages.end() ; ++it)
		{
			if ((*it)->getNumber() >= from && (*it)->getNumber() <= to2 &&
			    (*it)->m_flags != message::FLAG_UNDEFINED)
			{
				(*it)->m_flags &= ~flags;
			}
		}

		break;
	}
	default:
	case message::FLAG_MODE_SET:
	{
		for (std::vector <maildirMessage*>::iterator it =
		     m_messages.begin() ; it != m_messages.end() ; ++it)
		{
			if ((*it)->getNumber() >= from && (*it)->getNumber() <= to2 &&
			    (*it)->m_flags != message::FLAG_UNDEFINED)
			{
				(*it)->m_flags = flags;
			}
		}

		break;
	}

	}

	// Notify message flags changed
	events::messageChangedEvent event(this, events::messageChangedEvent::TYPE_FLAGS, nums);

	notifyMessageChanged(event);
}


void maildirFolder::setMessageFlags
	(const std::vector <int>& nums, const int flags, const int mode)
{
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

	// Change message flags
	setMessageFlagsImpl(list, flags, mode);

	// Update local flags
	switch (mode)
	{
	case message::FLAG_MODE_ADD:
	{
		for (std::vector <maildirMessage*>::iterator it =
		     m_messages.begin() ; it != m_messages.end() ; ++it)
		{
			if (std::binary_search(list.begin(), list.end(), (*it)->getNumber()) &&
			    (*it)->m_flags != message::FLAG_UNDEFINED)
			{
				(*it)->m_flags |= flags;
			}
		}

		break;
	}
	case message::FLAG_MODE_REMOVE:
	{
		for (std::vector <maildirMessage*>::iterator it =
		     m_messages.begin() ; it != m_messages.end() ; ++it)
		{
			if (std::binary_search(list.begin(), list.end(), (*it)->getNumber()) &&
			    (*it)->m_flags != message::FLAG_UNDEFINED)
			{
				(*it)->m_flags &= ~flags;
			}
		}

		break;
	}
	default:
	case message::FLAG_MODE_SET:
	{
		for (std::vector <maildirMessage*>::iterator it =
		     m_messages.begin() ; it != m_messages.end() ; ++it)
		{
			if (std::binary_search(list.begin(), list.end(), (*it)->getNumber()) &&
			    (*it)->m_flags != message::FLAG_UNDEFINED)
			{
				(*it)->m_flags = flags;
			}
		}

		break;
	}

	}

	// Notify message flags changed
	events::messageChangedEvent event(this, events::messageChangedEvent::TYPE_FLAGS, nums);

	notifyMessageChanged(event);
}


void maildirFolder::setMessageFlagsImpl
	(const std::vector <int>& nums, const int flags, const int mode)
{
	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	utility::file::path curDirPath = maildirUtils::getFolderFSPath
		(m_store, m_path, maildirUtils::FOLDER_PATH_CUR);

	for (std::vector <int>::const_iterator it =
	     nums.begin() ; it != nums.end() ; ++it)
	{
		const int num = *it - 1;

		try
		{
			const utility::file::path::component path = m_messageInfos[num].path;
			utility::auto_ptr <utility::file> file = fsf->create(curDirPath / path);

			int newFlags = maildirUtils::extractFlags(path);

			switch (mode)
			{
			case message::FLAG_MODE_ADD:    newFlags |= flags; break;
			case message::FLAG_MODE_REMOVE: newFlags &= ~flags; break;
			default:
			case message::FLAG_MODE_SET:    newFlags = flags; break;
			}

			const utility::file::path::component newPath = maildirUtils::buildFilename
				(maildirUtils::extractId(path), newFlags);

			file->rename(curDirPath / newPath);

			if (flags & message::FLAG_DELETED)
				m_messageInfos[num].type = messageInfos::TYPE_DELETED;
			else
				m_messageInfos[num].type = messageInfos::TYPE_CUR;

			m_messageInfos[num].path = newPath;
		}
		catch (exceptions::filesystem_exception& e)
		{
			// Ignore (not important)
		}
	}
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
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	// TODO
}


void maildirFolder::copyMessages(const folder::path& dest, const int from, const int to)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (from < 1 || (to < from && to != -1))
		throw exceptions::invalid_argument();

	// TODO
}


void maildirFolder::copyMessages(const folder::path& dest, const std::vector <int>& nums)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

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

		notifyMessageCount(event);

		// Notify folders with the same path
		for (std::list <maildirFolder*>::iterator it = m_store->m_folders.begin() ;
		     it != m_store->m_folders.end() ; ++it)
		{
			if ((*it) != this && (*it)->getFullPath() == m_path)
			{
				(*it)->m_messageCount = m_messageCount;
				(*it)->m_unreadMessageCount = m_unreadMessageCount;

				events::messageCountEvent event(*it, events::messageCountEvent::TYPE_ADDED, nums);

				(*it)->notifyMessageCount(event);

				(*it)->m_messageInfos.resize(m_messageInfos.size());
				std::copy(m_messageInfos.begin(), m_messageInfos.end(), (*it)->m_messageInfos.begin());
			}
		}
	}
}


void maildirFolder::expunge()
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	utility::file::path curDirPath = maildirUtils::getFolderFSPath
		(m_store, m_path, maildirUtils::FOLDER_PATH_CUR);

	std::vector <int> nums;
	int unreadCount = 0;

	for (int num = 1 ; num <= m_messageCount ; ++num)
	{
		messageInfos& infos = m_messageInfos[num - 1];

		if (infos.type == messageInfos::TYPE_DELETED)
		{
			nums.push_back(num);

			for (std::vector <maildirMessage*>::iterator it =
			     m_messages.begin() ; it != m_messages.end() ; ++it)
			{
				if ((*it)->m_num == num)
					(*it)->m_expunged = true;
				else if ((*it)->m_num > num)
					(*it)->m_num--;
			}

			if (maildirUtils::extractFlags(infos.path) & message::FLAG_SEEN)
				++unreadCount;

			// Delete file from file system
			try
			{
				utility::auto_ptr <utility::file> file = fsf->create(curDirPath / infos.path);
				file->remove();
			}
			catch (exceptions::filesystem_exception& e)
			{
				// Ignore (not important)
			}
		}
	}

	if (!nums.empty())
	{
		for (int i = nums.size() - 1 ; i >= 0 ; --i)
			m_messageInfos.erase(m_messageInfos.begin() + i);
	}

	m_messageCount -= nums.size();
	m_unreadMessageCount -= unreadCount;

	// Notify message expunged
	events::messageCountEvent event(this, events::messageCountEvent::TYPE_REMOVED, nums);

	notifyMessageCount(event);

	// Notify folders with the same path
	for (std::list <maildirFolder*>::iterator it = m_store->m_folders.begin() ;
	     it != m_store->m_folders.end() ; ++it)
	{
		if ((*it) != this && (*it)->getFullPath() == m_path)
		{
			(*it)->m_messageCount = m_messageCount;
			(*it)->m_unreadMessageCount = m_unreadMessageCount;

			events::messageCountEvent event(*it, events::messageCountEvent::TYPE_REMOVED, nums);

			(*it)->notifyMessageCount(event);

			(*it)->m_messageInfos.resize(m_messageInfos.size());
			std::copy(m_messageInfos.begin(), m_messageInfos.end(), (*it)->m_messageInfos.begin());
		}
	}
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
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	const int total = msg.size();
	int current = 0;

	if (progress)
		progress->start(total);

	for (std::vector <message*>::iterator it = msg.begin() ;
	     it != msg.end() ; ++it)
	{
		dynamic_cast <maildirMessage*>(*it)->fetch(this, options);

		if (progress)
			progress->progress(++current, total);
	}

	if (progress)
		progress->stop(total);
}


void maildirFolder::fetchMessage(message* msg, const int options)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	dynamic_cast <maildirMessage*>(msg)->fetch(this, options);
}


const int maildirFolder::getFetchCapabilities() const
{
	return (FETCH_ENVELOPE | FETCH_STRUCTURE | FETCH_CONTENT_INFO |
	        FETCH_FLAGS | FETCH_SIZE | FETCH_FULL_HEADER | FETCH_UID);
}


const utility::file::path maildirFolder::getMessageFSPath(const int number)
{
	utility::file::path curDirPath = maildirUtils::getFolderFSPath
		(m_store, m_path, maildirUtils::FOLDER_PATH_CUR);

	return (curDirPath / m_messageInfos[number - 1].path);
}


} // messaging
} // vmime
