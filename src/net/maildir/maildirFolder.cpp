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

#include "vmime/net/maildir/maildirFolder.hpp"

#include "vmime/net/maildir/maildirStore.hpp"
#include "vmime/net/maildir/maildirMessage.hpp"
#include "vmime/net/maildir/maildirUtils.hpp"
#include "vmime/net/maildir/maildirFormat.hpp"

#include "vmime/utility/smartPtr.hpp"

#include "vmime/message.hpp"

#include "vmime/exception.hpp"
#include "vmime/platform.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"


namespace vmime {
namespace net {
namespace maildir {


maildirFolder::maildirFolder(const folder::path& path, ref <maildirStore> store)
	: m_store(store), m_path(path),
	  m_name(path.isEmpty() ? folder::path::component("") : path.getLastComponent()),
	  m_mode(-1), m_open(false), m_unreadMessageCount(0), m_messageCount(0)
{
	store->registerFolder(this);
}


maildirFolder::~maildirFolder()
{
	ref <maildirStore> store = m_store.acquire();

	if (store)
	{
		if (m_open)
			close(false);

		store->unregisterFolder(this);
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


int maildirFolder::getMode() const
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return (m_mode);
}


int maildirFolder::getType()
{
	if (m_path.isEmpty())
		return (TYPE_CONTAINS_FOLDERS);
	else
		return (TYPE_CONTAINS_FOLDERS | TYPE_CONTAINS_MESSAGES);
}


int maildirFolder::getFlags()
{
	int flags = 0;

	if (m_store.acquire()->getFormat()->folderHasSubfolders(m_path))
		flags |= FLAG_CHILDREN; // Contains at least one sub-folder

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
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (isOpen())
		throw exceptions::illegal_state("Folder is already open");
	else if (!exists())
		throw exceptions::illegal_state("Folder does not exist");

	scanFolder();

	m_open = true;
	m_mode = mode;
}


void maildirFolder::close(const bool expunge)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (expunge)
		this->expunge();

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


void maildirFolder::create(const int /* type */)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (isOpen())
		throw exceptions::illegal_state("Folder is open");
	else if (exists())
		throw exceptions::illegal_state("Folder already exists");
	else if (!store->isValidFolderName(m_name))
		throw exceptions::invalid_folder_name();

	// Create directory on file system
	try
	{
		store->getFormat()->createFolder(m_path);
	}
	catch (exceptions::filesystem_exception& e)
	{
		throw exceptions::command_error("CREATE", "", "File system exception", e);
	}

	// Notify folder created
	events::folderEvent event
		(thisRef().dynamicCast <folder>(),
		 events::folderEvent::TYPE_CREATED, m_path, m_path);

	notifyFolder(event);
}


void maildirFolder::destroy()
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (isOpen())
		throw exceptions::illegal_state("Folder is open");

	// Delete folder
	try
	{
		store->getFormat()->destroyFolder(m_path);
	}
	catch (std::exception&)
	{
		// Ignore exception: anyway, we can't recover from this...
	}

	// Notify folder deleted
	events::folderEvent event
		(thisRef().dynamicCast <folder>(),
		 events::folderEvent::TYPE_DELETED, m_path, m_path);

	notifyFolder(event);
}


bool maildirFolder::exists()
{
	ref <maildirStore> store = m_store.acquire();

	return store->getFormat()->folderExists(m_path);
}


bool maildirFolder::isOpen() const
{
	return (m_open);
}


void maildirFolder::scanFolder()
{
	ref <maildirStore> store = m_store.acquire();

	try
	{
		m_messageCount = 0;
		m_unreadMessageCount = 0;

		ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

		utility::file::path newDirPath = store->getFormat()->folderPathToFileSystemPath
			(m_path, maildirFormat::NEW_DIRECTORY);
		ref <utility::file> newDir = fsf->create(newDirPath);

		utility::file::path curDirPath = store->getFormat()->folderPathToFileSystemPath
			(m_path, maildirFormat::CUR_DIRECTORY);
		ref <utility::file> curDir = fsf->create(curDirPath);

		// New received messages (new/)
		ref <utility::fileIterator> nit = newDir->getFiles();
		std::vector <utility::file::path::component> newMessageFilenames;

		while (nit->hasMoreElements())
		{
			ref <utility::file> file = nit->nextElement();

			if (maildirUtils::isMessageFile(*file))
				newMessageFilenames.push_back(file->getFullPath().getLastComponent());
		}

		// Current messages (cur/)
		ref <utility::fileIterator> cit = curDir->getFiles();
		std::vector <utility::file::path::component> curMessageFilenames;

		while (cit->hasMoreElements())
		{
			ref <utility::file> file = cit->nextElement();

			if (maildirUtils::isMessageFile(*file))
				curMessageFilenames.push_back(file->getFullPath().getLastComponent());
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

		m_messageInfos.reserve(m_messageInfos.size()
			+ newMessageFilenames.size() + curMessageFilenames.size());

		// Add new messages from 'new': we are responsible to move the files
		// from the 'new' directory to the 'cur' directory, and append them
		// to our message list.
		for (std::vector <utility::file::path::component>::const_iterator
		     it = newMessageFilenames.begin() ; it != newMessageFilenames.end() ; ++it)
		{
			const utility::file::path::component newFilename =
				maildirUtils::buildFilename(maildirUtils::extractId(*it), 0);

			// Move messages from 'new' to 'cur'
			ref <utility::file> file = fsf->create(newDirPath / *it);
			file->rename(curDirPath / newFilename);

			// Append to message list
			messageInfos msgInfos;
			msgInfos.path = newFilename;

			if (maildirUtils::extractFlags(msgInfos.path) & message::FLAG_DELETED)
				msgInfos.type = messageInfos::TYPE_DELETED;
			else
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

			if (maildirUtils::extractFlags(msgInfos.path) & message::FLAG_DELETED)
				msgInfos.type = messageInfos::TYPE_DELETED;
			else
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


ref <message> maildirFolder::getMessage(const int num)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (num < 1 || num > m_messageCount)
		throw exceptions::message_not_found();

	return vmime::create <maildirMessage>
		(thisRef().dynamicCast <maildirFolder>(), num);
}


std::vector <ref <message> > maildirFolder::getMessages(const int from, const int to)
{
	const int to2 = (to == -1 ? m_messageCount : to);

	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (to2 < from || from < 1 || to2 < 1 || from > m_messageCount || to2 > m_messageCount)
		throw exceptions::message_not_found();

	std::vector <ref <message> > v;
	ref <maildirFolder> thisFolder = thisRef().dynamicCast <maildirFolder>();

	for (int i = from ; i <= to2 ; ++i)
		v.push_back(vmime::create <maildirMessage>(thisFolder, i));

	return (v);
}


std::vector <ref <message> > maildirFolder::getMessages(const std::vector <int>& nums)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	std::vector <ref <message> > v;
	ref <maildirFolder> thisFolder = thisRef().dynamicCast <maildirFolder>();

	for (std::vector <int>::const_iterator it = nums.begin() ; it != nums.end() ; ++it)
		v.push_back(vmime::create <maildirMessage>(thisFolder, *it));

	return (v);
}


int maildirFolder::getMessageCount()
{
	return (m_messageCount);
}


ref <folder> maildirFolder::getFolder(const folder::path::component& name)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	return vmime::create <maildirFolder>(m_path / name, store);
}


std::vector <ref <folder> > maildirFolder::getFolders(const bool recursive)
{
	ref <maildirStore> store = m_store.acquire();

	if (!isOpen() && !store)
		throw exceptions::illegal_state("Store disconnected");

	std::vector <ref <folder> > list;

	listFolders(list, recursive);

	return (list);
}


void maildirFolder::listFolders(std::vector <ref <folder> >& list, const bool recursive)
{
	ref <maildirStore> store = m_store.acquire();

	try
	{
		std::vector <folder::path> pathList =
			store->getFormat()->listFolders(m_path, recursive);

		list.reserve(pathList.size());

		for (unsigned int i = 0, n = pathList.size() ; i < n ; ++i)
		{
			ref <maildirFolder> subFolder =
				vmime::create <maildirFolder>(pathList[i], store);

			list.push_back(subFolder);
		}
	}
	catch (exceptions::filesystem_exception& e)
	{
		throw exceptions::command_error("LIST", "", "", e);
	}
}


void maildirFolder::rename(const folder::path& newPath)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (m_path.isEmpty() || newPath.isEmpty())
		throw exceptions::illegal_operation("Cannot rename root folder");
	else if (!store->isValidFolderName(newPath.getLastComponent()))
		throw exceptions::invalid_folder_name();

	// Rename the directory on the file system
	try
	{
		store->getFormat()->renameFolder(m_path, newPath);
	}
	catch (vmime::exception& e)
	{
		throw exceptions::command_error("RENAME", "", "", e);
	}

	// Notify folder renamed
	folder::path oldPath(m_path);

	m_path = newPath;
	m_name = newPath.getLastComponent();

	events::folderEvent event
		(thisRef().dynamicCast <folder>(),
		 events::folderEvent::TYPE_RENAMED, oldPath, newPath);

	notifyFolder(event);

	// Notify folders with the same path
	for (std::list <maildirFolder*>::iterator it = store->m_folders.begin() ;
	     it != store->m_folders.end() ; ++it)
	{
		if ((*it) != this && (*it)->getFullPath() == oldPath)
		{
			(*it)->m_path = newPath;
			(*it)->m_name = newPath.getLastComponent();

			events::folderEvent event
				((*it)->thisRef().dynamicCast <folder>(),
				 events::folderEvent::TYPE_RENAMED, oldPath, newPath);

			(*it)->notifyFolder(event);
		}
		else if ((*it) != this && oldPath.isParentOf((*it)->getFullPath()))
		{
			folder::path oldPath((*it)->m_path);

			(*it)->m_path.renameParent(oldPath, newPath);

			events::folderEvent event
				((*it)->thisRef().dynamicCast <folder>(),
				 events::folderEvent::TYPE_RENAMED, oldPath, (*it)->m_path);

			(*it)->notifyFolder(event);
		}
	}
}


void maildirFolder::deleteMessage(const int num)
{
	// Mark messages as deleted
	setMessageFlags(num, num, message::FLAG_DELETED, message::FLAG_MODE_ADD);
}


void maildirFolder::deleteMessages(const int from, const int to)
{
	// Mark messages as deleted
	setMessageFlags(from, to, message::FLAG_DELETED, message::FLAG_MODE_ADD);
}


void maildirFolder::deleteMessages(const std::vector <int>& nums)
{
	// Mark messages as deleted
	setMessageFlags(nums, message::FLAG_DELETED, message::FLAG_MODE_ADD);
}


void maildirFolder::setMessageFlags
	(const int from, const int to, const int flags, const int mode)
{
	ref <maildirStore> store = m_store.acquire();

	if (from < 1 || (to < from && to != -1))
		throw exceptions::invalid_argument();

	if (!store)
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
	events::messageChangedEvent event
		(thisRef().dynamicCast <folder>(),
		 events::messageChangedEvent::TYPE_FLAGS, nums);

	notifyMessageChanged(event);

	// TODO: notify other folders with the same path
}


void maildirFolder::setMessageFlags
	(const std::vector <int>& nums, const int flags, const int mode)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
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
	events::messageChangedEvent event
		(thisRef().dynamicCast <folder>(),
		 events::messageChangedEvent::TYPE_FLAGS, nums);

	notifyMessageChanged(event);

	// TODO: notify other folders with the same path
}


void maildirFolder::setMessageFlagsImpl
	(const std::vector <int>& nums, const int flags, const int mode)
{
	ref <maildirStore> store = m_store.acquire();

	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	utility::file::path curDirPath = store->getFormat()->
		folderPathToFileSystemPath(m_path, maildirFormat::CUR_DIRECTORY);

	for (std::vector <int>::const_iterator it =
	     nums.begin() ; it != nums.end() ; ++it)
	{
		const int num = *it - 1;

		try
		{
			const utility::file::path::component path = m_messageInfos[num].path;
			ref <utility::file> file = fsf->create(curDirPath / path);

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


void maildirFolder::addMessage(ref <vmime::message> msg, const int flags,
	vmime::datetime* date, utility::progressListener* progress)
{
	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	msg->generate(ossAdapter);

	const std::string& str = oss.str();
	utility::inputStreamStringAdapter strAdapter(str);

	addMessage(strAdapter, str.length(), flags, date, progress);
}


void maildirFolder::addMessage(utility::inputStream& is, const int size,
	const int flags, vmime::datetime* /* date */, utility::progressListener* progress)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	utility::file::path tmpDirPath = store->getFormat()->
		folderPathToFileSystemPath(m_path,maildirFormat::TMP_DIRECTORY);
	utility::file::path dstDirPath = store->getFormat()->
		folderPathToFileSystemPath(m_path,
			flags == message::FLAG_RECENT ?
				maildirFormat::NEW_DIRECTORY :
				maildirFormat::CUR_DIRECTORY);

	const utility::file::path::component filename =
		maildirUtils::buildFilename(maildirUtils::generateId(),
			((flags == message::FLAG_UNDEFINED) ? 0 : flags));

	try
	{
		ref <utility::file> tmpDir = fsf->create(tmpDirPath);
		tmpDir->createDirectory(true);
	}
	catch (exceptions::filesystem_exception&)
	{
		// Don't throw now, it will fail later...
	}

	try
	{
		ref <utility::file> curDir = fsf->create(dstDirPath);
		curDir->createDirectory(true);
	}
	catch (exceptions::filesystem_exception&)
	{
		// Don't throw now, it will fail later...
	}

	// Actually add the message
	copyMessageImpl(tmpDirPath, dstDirPath, filename, is, size, progress);

	// Append the message to the cache list
	messageInfos msgInfos;
	msgInfos.path = filename;
	msgInfos.type = messageInfos::TYPE_CUR;

	m_messageInfos.push_back(msgInfos);
	m_messageCount++;

	if ((flags == message::FLAG_UNDEFINED) || !(flags & message::FLAG_SEEN))
		m_unreadMessageCount++;

	// Notification
	std::vector <int> nums;
	nums.push_back(m_messageCount);

	events::messageCountEvent event
		(thisRef().dynamicCast <folder>(),
		 events::messageCountEvent::TYPE_ADDED, nums);

	notifyMessageCount(event);

	// Notify folders with the same path
	for (std::list <maildirFolder*>::iterator it = store->m_folders.begin() ;
	     it != store->m_folders.end() ; ++it)
	{
		if ((*it) != this && (*it)->getFullPath() == m_path)
		{
			(*it)->m_messageCount = m_messageCount;
			(*it)->m_unreadMessageCount = m_unreadMessageCount;

			(*it)->m_messageInfos.resize(m_messageInfos.size());
			std::copy(m_messageInfos.begin(), m_messageInfos.end(), (*it)->m_messageInfos.begin());

			events::messageCountEvent event
				((*it)->thisRef().dynamicCast <folder>(),
				 events::messageCountEvent::TYPE_ADDED, nums);

			(*it)->notifyMessageCount(event);
		}
	}
}


void maildirFolder::copyMessageImpl(const utility::file::path& tmpDirPath,
	const utility::file::path& dstDirPath,
	const utility::file::path::component& filename,
	utility::inputStream& is, const utility::stream::size_type size,
	utility::progressListener* progress)
{
	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	ref <utility::file> file = fsf->create(tmpDirPath / filename);

	if (progress)
		progress->start(size);

	// First, write the message into 'tmp'...
	try
	{
		file->createFile();

		ref <utility::fileWriter> fw = file->getFileWriter();
		ref <utility::outputStream> os = fw->getOutputStream();

		utility::stream::value_type buffer[65536];
		utility::stream::size_type total = 0;

		while (!is.eof())
		{
			const utility::stream::size_type read = is.read(buffer, sizeof(buffer));

			if (read != 0)
			{
				os->write(buffer, read);
				total += read;
			}

			if (progress)
				progress->progress(total, size);
		}

		os->flush();
	}
	catch (exception& e)
	{
		if (progress)
			progress->stop(size);

		// Delete temporary file
		try
		{
			ref <utility::file> file = fsf->create(tmpDirPath / filename);
			file->remove();
		}
		catch (exceptions::filesystem_exception&)
		{
			// Ignore
		}

		throw exceptions::command_error("ADD", "", "", e);
	}

	// ...then, move it to 'cur'
	try
	{
		file->rename(dstDirPath / filename);
	}
	catch (exception& e)
	{
		if (progress)
			progress->stop(size);

		// Delete temporary file
		try
		{
			file->remove();
			ref <utility::file> file = fsf->create(dstDirPath / filename);
			file->remove();
		}
		catch (exceptions::filesystem_exception&)
		{
			// Ignore
		}

		throw exceptions::command_error("ADD", "", "", e);
	}

	if (progress)
		progress->stop(size);
}


void maildirFolder::copyMessage(const folder::path& dest, const int num)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	copyMessages(dest, num, num);
}


void maildirFolder::copyMessages(const folder::path& dest, const int from, const int to)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (from < 1 || (to < from && to != -1))
		throw exceptions::invalid_argument();

	// Construct the list of message numbers
	const int to2 = (to == -1) ? m_messageCount : to;
	const int count = to - from + 1;

	std::vector <int> nums;
	nums.resize(count);

	for (int i = from, j = 0 ; i <= to2 ; ++i, ++j)
		nums[j] = i;

	// Copy messages
	copyMessagesImpl(dest, nums);
}


void maildirFolder::copyMessages(const folder::path& dest, const std::vector <int>& nums)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	// Copy messages
	copyMessagesImpl(dest, nums);
}


void maildirFolder::copyMessagesImpl(const folder::path& dest, const std::vector <int>& nums)
{
	ref <maildirStore> store = m_store.acquire();

	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	utility::file::path curDirPath = store->getFormat()->folderPathToFileSystemPath
		(m_path, maildirFormat::CUR_DIRECTORY);

	utility::file::path destCurDirPath = store->getFormat()->
		folderPathToFileSystemPath(dest, maildirFormat::CUR_DIRECTORY);
	utility::file::path destTmpDirPath = store->getFormat()->
		folderPathToFileSystemPath(dest, maildirFormat::TMP_DIRECTORY);

	// Create destination directories
	try
	{
		ref <utility::file> destTmpDir = fsf->create(destTmpDirPath);
		destTmpDir->createDirectory(true);
	}
	catch (exceptions::filesystem_exception&)
	{
		// Don't throw now, it will fail later...
	}

	try
	{
		ref <utility::file> destCurDir = fsf->create(destCurDirPath);
		destCurDir->createDirectory(true);
	}
	catch (exceptions::filesystem_exception&)
	{
		// Don't throw now, it will fail later...
	}

	// Copy messages
	try
	{
		for (std::vector <int>::const_iterator it =
		     nums.begin() ; it != nums.end() ; ++it)
		{
			const int num = *it;
			const messageInfos& msg = m_messageInfos[num - 1];
			const int flags = maildirUtils::extractFlags(msg.path);

			const utility::file::path::component filename =
				maildirUtils::buildFilename(maildirUtils::generateId(), flags);

			ref <utility::file> file = fsf->create(curDirPath / msg.path);
			ref <utility::fileReader> fr = file->getFileReader();
			ref <utility::inputStream> is = fr->getInputStream();

			copyMessageImpl(destTmpDirPath, destCurDirPath,
				filename, *is, file->getLength(), NULL);
		}
	}
	catch (exception& e)
	{
		notifyMessagesCopied(dest);
		throw exceptions::command_error("COPY", "", "", e);
	}

	notifyMessagesCopied(dest);
}


void maildirFolder::notifyMessagesCopied(const folder::path& dest)
{
	ref <maildirStore> store = m_store.acquire();

	for (std::list <maildirFolder*>::iterator it = store->m_folders.begin() ;
	     it != store->m_folders.end() ; ++it)
	{
		if ((*it) != this && (*it)->getFullPath() == dest)
		{
			// We only need to update the first folder we found as calling
			// status() will notify all the folders with the same path.
			int count, unseen;
			(*it)->status(count, unseen);

			return;
		}
	}
}


void maildirFolder::status(int& count, int& unseen)
{
	ref <maildirStore> store = m_store.acquire();

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

		events::messageCountEvent event
			(thisRef().dynamicCast <folder>(),
			 events::messageCountEvent::TYPE_ADDED, nums);

		notifyMessageCount(event);

		// Notify folders with the same path
		for (std::list <maildirFolder*>::iterator it = store->m_folders.begin() ;
		     it != store->m_folders.end() ; ++it)
		{
			if ((*it) != this && (*it)->getFullPath() == m_path)
			{
				(*it)->m_messageCount = m_messageCount;
				(*it)->m_unreadMessageCount = m_unreadMessageCount;

				(*it)->m_messageInfos.resize(m_messageInfos.size());
				std::copy(m_messageInfos.begin(), m_messageInfos.end(), (*it)->m_messageInfos.begin());

				events::messageCountEvent event
					((*it)->thisRef().dynamicCast <folder>(),
					 events::messageCountEvent::TYPE_ADDED, nums);

				(*it)->notifyMessageCount(event);
			}
		}
	}
}


void maildirFolder::expunge()
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	utility::file::path curDirPath = store->getFormat()->
		folderPathToFileSystemPath(m_path, maildirFormat::CUR_DIRECTORY);

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
				ref <utility::file> file = fsf->create(curDirPath / infos.path);
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
	events::messageCountEvent event
		(thisRef().dynamicCast <folder>(),
		 events::messageCountEvent::TYPE_REMOVED, nums);

	notifyMessageCount(event);

	// Notify folders with the same path
	for (std::list <maildirFolder*>::iterator it = store->m_folders.begin() ;
	     it != store->m_folders.end() ; ++it)
	{
		if ((*it) != this && (*it)->getFullPath() == m_path)
		{
			(*it)->m_messageCount = m_messageCount;
			(*it)->m_unreadMessageCount = m_unreadMessageCount;

			(*it)->m_messageInfos.resize(m_messageInfos.size());
			std::copy(m_messageInfos.begin(), m_messageInfos.end(), (*it)->m_messageInfos.begin());

			events::messageCountEvent event
				((*it)->thisRef().dynamicCast <folder>(),
				 events::messageCountEvent::TYPE_REMOVED, nums);

			(*it)->notifyMessageCount(event);
		}
	}
}


ref <folder> maildirFolder::getParent()
{
	if (m_path.isEmpty())
		return NULL;
	else
		return vmime::create <maildirFolder>(m_path.getParent(), m_store.acquire());
}


ref <const store> maildirFolder::getStore() const
{
	return m_store.acquire();
}


ref <store> maildirFolder::getStore()
{
	return m_store.acquire();
}


void maildirFolder::fetchMessages(std::vector <ref <message> >& msg,
	const int options, utility::progressListener* progress)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	const int total = msg.size();
	int current = 0;

	if (progress)
		progress->start(total);

	ref <maildirFolder> thisFolder = thisRef().dynamicCast <maildirFolder>();

	for (std::vector <ref <message> >::iterator it = msg.begin() ;
	     it != msg.end() ; ++it)
	{
		(*it).dynamicCast <maildirMessage>()->fetch(thisFolder, options);

		if (progress)
			progress->progress(++current, total);
	}

	if (progress)
		progress->stop(total);
}


void maildirFolder::fetchMessage(ref <message> msg, const int options)
{
	ref <maildirStore> store = m_store.acquire();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	msg.dynamicCast <maildirMessage>()->fetch
		(thisRef().dynamicCast <maildirFolder>(), options);
}


int maildirFolder::getFetchCapabilities() const
{
	return (FETCH_ENVELOPE | FETCH_STRUCTURE | FETCH_CONTENT_INFO |
	        FETCH_FLAGS | FETCH_SIZE | FETCH_FULL_HEADER | FETCH_UID |
	        FETCH_IMPORTANCE);
}


const utility::file::path maildirFolder::getMessageFSPath(const int number) const
{
	utility::file::path curDirPath = m_store.acquire()->getFormat()->
		folderPathToFileSystemPath(m_path, maildirFormat::CUR_DIRECTORY);

	return (curDirPath / m_messageInfos[number - 1].path);
}


std::vector <int> maildirFolder::getMessageNumbersStartingOnUID(const message::uid& /* uid */)
{
	throw exceptions::operation_not_supported();
}


} // maildir
} // net
} // vmime
