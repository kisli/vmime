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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/net/maildir/maildirFolder.hpp"

#include "vmime/net/maildir/maildirStore.hpp"
#include "vmime/net/maildir/maildirMessage.hpp"
#include "vmime/net/maildir/maildirUtils.hpp"
#include "vmime/net/maildir/maildirFormat.hpp"
#include "vmime/net/maildir/maildirFolderStatus.hpp"

#include "vmime/message.hpp"

#include "vmime/exception.hpp"
#include "vmime/platform.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"


namespace vmime {
namespace net {
namespace maildir {


maildirFolder::maildirFolder(const folder::path& path, shared_ptr <maildirStore> store)
	: m_store(store), m_path(path),
	  m_name(path.isEmpty() ? folder::path::component("") : path.getLastComponent()),
	  m_mode(-1), m_open(false), m_unreadMessageCount(0), m_messageCount(0)
{
	store->registerFolder(this);
}


maildirFolder::~maildirFolder()
{
	shared_ptr <maildirStore> store = m_store.lock();

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
	m_store.reset();
}


int maildirFolder::getMode() const
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return (m_mode);
}


const folderAttributes maildirFolder::getAttributes()
{
	folderAttributes attribs;

	if (m_path.isEmpty())
		attribs.setType(folderAttributes::TYPE_CONTAINS_FOLDERS);
	else
		attribs.setType(folderAttributes::TYPE_CONTAINS_FOLDERS | folderAttributes::TYPE_CONTAINS_MESSAGES);

	if (m_store.lock()->getFormat()->folderHasSubfolders(m_path))
		attribs.setFlags(folderAttributes::FLAG_HAS_CHILDREN);  // contains at least one sub-folder

	return attribs;
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
	shared_ptr <maildirStore> store = m_store.lock();

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
	shared_ptr <maildirStore> store = m_store.lock();

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


void maildirFolder::create(const folderAttributes& /* attribs */)
{
	shared_ptr <maildirStore> store = m_store.lock();

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
	shared_ptr <events::folderEvent> event =
		make_shared <events::folderEvent>
			(dynamicCast <folder>(shared_from_this()),
			 events::folderEvent::TYPE_CREATED, m_path, m_path);

	notifyFolder(event);
}


void maildirFolder::destroy()
{
	shared_ptr <maildirStore> store = m_store.lock();

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
	shared_ptr <events::folderEvent> event =
		make_shared <events::folderEvent>
			(dynamicCast <folder>(shared_from_this()),
			 events::folderEvent::TYPE_DELETED, m_path, m_path);

	notifyFolder(event);
}


bool maildirFolder::exists()
{
	shared_ptr <maildirStore> store = m_store.lock();

	return store->getFormat()->folderExists(m_path);
}


bool maildirFolder::isOpen() const
{
	return (m_open);
}


void maildirFolder::scanFolder()
{
	shared_ptr <maildirStore> store = m_store.lock();

	try
	{
		m_messageCount = 0;
		m_unreadMessageCount = 0;

		shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

		utility::file::path newDirPath = store->getFormat()->folderPathToFileSystemPath
			(m_path, maildirFormat::NEW_DIRECTORY);
		shared_ptr <utility::file> newDir = fsf->create(newDirPath);

		utility::file::path curDirPath = store->getFormat()->folderPathToFileSystemPath
			(m_path, maildirFormat::CUR_DIRECTORY);
		shared_ptr <utility::file> curDir = fsf->create(curDirPath);

		// New received messages (new/)
		shared_ptr <utility::fileIterator> nit = newDir->getFiles();
		std::vector <utility::file::path::component> newMessageFilenames;

		while (nit->hasMoreElements())
		{
			shared_ptr <utility::file> file = nit->nextElement();

			if (maildirUtils::isMessageFile(*file))
				newMessageFilenames.push_back(file->getFullPath().getLastComponent());
		}

		// Current messages (cur/)
		shared_ptr <utility::fileIterator> cit = curDir->getFiles();
		std::vector <utility::file::path::component> curMessageFilenames;

		while (cit->hasMoreElements())
		{
			shared_ptr <utility::file> file = cit->nextElement();

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
			shared_ptr <utility::file> file = fsf->create(newDirPath / *it);
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


shared_ptr <message> maildirFolder::getMessage(const int num)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (num < 1 || num > m_messageCount)
		throw exceptions::message_not_found();

	return make_shared <maildirMessage>
		(dynamicCast <maildirFolder>(shared_from_this()), num);
}


std::vector <shared_ptr <message> > maildirFolder::getMessages(const messageSet& msgs)
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (msgs.isNumberSet())
	{
		const std::vector <int> numbers = maildirUtils::messageSetToNumberList(msgs);

		std::vector <shared_ptr <message> > messages;
		shared_ptr <maildirFolder> thisFolder = dynamicCast <maildirFolder>(shared_from_this());

		for (std::vector <int>::const_iterator it = numbers.begin() ; it != numbers.end() ; ++it)
		{
			if (*it < 1|| *it > m_messageCount)
				throw exceptions::message_not_found();

			messages.push_back(make_shared <maildirMessage>(thisFolder, *it));
		}

		return messages;
	}
	else
	{
		throw exceptions::operation_not_supported();
	}
}


int maildirFolder::getMessageCount()
{
	return (m_messageCount);
}


shared_ptr <folder> maildirFolder::getFolder(const folder::path::component& name)
{
	shared_ptr <maildirStore> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	return make_shared <maildirFolder>(m_path / name, store);
}


std::vector <shared_ptr <folder> > maildirFolder::getFolders(const bool recursive)
{
	shared_ptr <maildirStore> store = m_store.lock();

	if (!isOpen() && !store)
		throw exceptions::illegal_state("Store disconnected");

	std::vector <shared_ptr <folder> > list;

	listFolders(list, recursive);

	return (list);
}


void maildirFolder::listFolders(std::vector <shared_ptr <folder> >& list, const bool recursive)
{
	shared_ptr <maildirStore> store = m_store.lock();

	try
	{
		std::vector <folder::path> pathList =
			store->getFormat()->listFolders(m_path, recursive);

		list.reserve(pathList.size());

		for (std::vector <folder::path>::size_type i = 0, n = pathList.size() ; i < n ; ++i)
		{
			shared_ptr <maildirFolder> subFolder =
				make_shared <maildirFolder>(pathList[i], store);

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
	shared_ptr <maildirStore> store = m_store.lock();

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

	shared_ptr <events::folderEvent> event =
		make_shared <events::folderEvent>
			(dynamicCast <folder>(shared_from_this()),
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

			shared_ptr <events::folderEvent> event =
				make_shared <events::folderEvent>
					(dynamicCast <folder>((*it)->shared_from_this()),
					 events::folderEvent::TYPE_RENAMED, oldPath, newPath);

			(*it)->notifyFolder(event);
		}
		else if ((*it) != this && oldPath.isParentOf((*it)->getFullPath()))
		{
			folder::path oldPath((*it)->m_path);

			(*it)->m_path.renameParent(oldPath, newPath);

			shared_ptr <events::folderEvent> event =
				make_shared <events::folderEvent>
					(dynamicCast <folder>((*it)->shared_from_this()),
					 events::folderEvent::TYPE_RENAMED, oldPath, (*it)->m_path);

			(*it)->notifyFolder(event);
		}
	}
}


void maildirFolder::deleteMessages(const messageSet& msgs)
{
	// Mark messages as deleted
	setMessageFlags(msgs, message::FLAG_DELETED, message::FLAG_MODE_ADD);
}


void maildirFolder::setMessageFlags
	(const messageSet& msgs, const int flags, const int mode)
{
	shared_ptr <maildirStore> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	if (msgs.isNumberSet())
	{
		const std::vector <int> nums = maildirUtils::messageSetToNumberList(msgs);

		// Change message flags
		shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

		utility::file::path curDirPath = store->getFormat()->
			folderPathToFileSystemPath(m_path, maildirFormat::CUR_DIRECTORY);

		for (std::vector <int>::const_iterator it =
			 nums.begin() ; it != nums.end() ; ++it)
		{
			const int num = *it - 1;

			try
			{
				const utility::file::path::component path = m_messageInfos[num].path;
				shared_ptr <utility::file> file = fsf->create(curDirPath / path);

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

		// Update local flags
		switch (mode)
		{
		case message::FLAG_MODE_ADD:
		{
			for (std::vector <maildirMessage*>::iterator it =
				 m_messages.begin() ; it != m_messages.end() ; ++it)
			{
				if (std::binary_search(nums.begin(), nums.end(), (*it)->getNumber()) &&
					(*it)->m_flags != maildirMessage::FLAG_UNDEFINED)
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
				if (std::binary_search(nums.begin(), nums.end(), (*it)->getNumber()) &&
					(*it)->m_flags != maildirMessage::FLAG_UNDEFINED)
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
				if (std::binary_search(nums.begin(), nums.end(), (*it)->getNumber()) &&
					(*it)->m_flags != maildirMessage::FLAG_UNDEFINED)
				{
					(*it)->m_flags = flags;
				}
			}

			break;
		}

		}

		// Notify message flags changed
		shared_ptr <events::messageChangedEvent> event =
			make_shared <events::messageChangedEvent>
				(dynamicCast <folder>(shared_from_this()),
				 events::messageChangedEvent::TYPE_FLAGS, nums);

		notifyMessageChanged(event);

		// TODO: notify other folders with the same path
	}
	else
	{
		throw exceptions::operation_not_supported();
	}
}


messageSet maildirFolder::addMessage
	(shared_ptr <vmime::message> msg, const int flags,
	 vmime::datetime* date, utility::progressListener* progress)
{
	std::ostringstream oss;
	utility::outputStreamAdapter ossAdapter(oss);

	msg->generate(ossAdapter);

	const string& str = oss.str();
	utility::inputStreamStringAdapter strAdapter(str);

	return addMessage(strAdapter, str.length(), flags, date, progress);
}


messageSet maildirFolder::addMessage
	(utility::inputStream& is, const size_t size,
	 const int flags, vmime::datetime* /* date */, utility::progressListener* progress)
{
	shared_ptr <maildirStore> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	utility::file::path tmpDirPath = store->getFormat()->
		folderPathToFileSystemPath(m_path,maildirFormat::TMP_DIRECTORY);
	utility::file::path dstDirPath = store->getFormat()->
		folderPathToFileSystemPath(m_path,
			flags == message::FLAG_RECENT ?
				maildirFormat::NEW_DIRECTORY :
				maildirFormat::CUR_DIRECTORY);

	const utility::file::path::component filename =
		maildirUtils::buildFilename(maildirUtils::generateId(),
			((flags == -1) ? 0 : flags));

	try
	{
		shared_ptr <utility::file> tmpDir = fsf->create(tmpDirPath);
		tmpDir->createDirectory(true);
	}
	catch (exceptions::filesystem_exception&)
	{
		// Don't throw now, it will fail later...
	}

	try
	{
		shared_ptr <utility::file> curDir = fsf->create(dstDirPath);
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

	if ((flags == -1) || !(flags & message::FLAG_SEEN))
		m_unreadMessageCount++;

	// Notification
	std::vector <int> nums;
	nums.push_back(m_messageCount);

	shared_ptr <events::messageCountEvent> event =
		make_shared <events::messageCountEvent>
			(dynamicCast <folder>(shared_from_this()),
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

			shared_ptr <events::messageCountEvent> event =
				make_shared <events::messageCountEvent>
					(dynamicCast <folder>((*it)->shared_from_this()),
					 events::messageCountEvent::TYPE_ADDED, nums);

			(*it)->notifyMessageCount(event);
		}
	}

	return messageSet::empty();
}


void maildirFolder::copyMessageImpl(const utility::file::path& tmpDirPath,
	const utility::file::path& dstDirPath,
	const utility::file::path::component& filename,
	utility::inputStream& is, const size_t size,
	utility::progressListener* progress)
{
	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	shared_ptr <utility::file> file = fsf->create(tmpDirPath / filename);

	if (progress)
		progress->start(size);

	// First, write the message into 'tmp'...
	try
	{
		file->createFile();

		shared_ptr <utility::fileWriter> fw = file->getFileWriter();
		shared_ptr <utility::outputStream> os = fw->getOutputStream();

		byte_t buffer[65536];
		size_t total = 0;

		while (!is.eof())
		{
			const size_t read = is.read(buffer, sizeof(buffer));

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
			shared_ptr <utility::file> file = fsf->create(tmpDirPath / filename);
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
			shared_ptr <utility::file> file = fsf->create(dstDirPath / filename);
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


messageSet maildirFolder::copyMessages(const folder::path& dest, const messageSet& msgs)
{
	shared_ptr <maildirStore> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	utility::file::path curDirPath = store->getFormat()->folderPathToFileSystemPath
		(m_path, maildirFormat::CUR_DIRECTORY);

	utility::file::path destCurDirPath = store->getFormat()->
		folderPathToFileSystemPath(dest, maildirFormat::CUR_DIRECTORY);
	utility::file::path destTmpDirPath = store->getFormat()->
		folderPathToFileSystemPath(dest, maildirFormat::TMP_DIRECTORY);

	// Create destination directories
	try
	{
		shared_ptr <utility::file> destTmpDir = fsf->create(destTmpDirPath);
		destTmpDir->createDirectory(true);
	}
	catch (exceptions::filesystem_exception&)
	{
		// Don't throw now, it will fail later...
	}

	try
	{
		shared_ptr <utility::file> destCurDir = fsf->create(destCurDirPath);
		destCurDir->createDirectory(true);
	}
	catch (exceptions::filesystem_exception&)
	{
		// Don't throw now, it will fail later...
	}

	// Copy messages
	const std::vector <int> nums = maildirUtils::messageSetToNumberList(msgs);

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

			shared_ptr <utility::file> file = fsf->create(curDirPath / msg.path);
			shared_ptr <utility::fileReader> fr = file->getFileReader();
			shared_ptr <utility::inputStream> is = fr->getInputStream();

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

	return messageSet::empty();
}


void maildirFolder::notifyMessagesCopied(const folder::path& dest)
{
	shared_ptr <maildirStore> store = m_store.lock();

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
	count = 0;
	unseen = 0;

	shared_ptr <folderStatus> status = getStatus();

	count = status->getMessageCount();
	unseen = status->getUnseenCount();
}


shared_ptr <folderStatus> maildirFolder::getStatus()
{
	shared_ptr <maildirStore> store = m_store.lock();

	const int oldCount = m_messageCount;

	scanFolder();

	shared_ptr <maildirFolderStatus> status = make_shared <maildirFolderStatus>();

	status->setMessageCount(m_messageCount);
	status->setUnseenCount(m_unreadMessageCount);

	// Notify message count changed (new messages)
	if (m_messageCount > oldCount)
	{
		std::vector <int> nums;
		nums.reserve(m_messageCount - oldCount);

		for (int i = oldCount + 1, j = 0 ; i <= m_messageCount ; ++i, ++j)
			nums[j] = i;

		shared_ptr <events::messageCountEvent> event =
			make_shared <events::messageCountEvent>
				(dynamicCast <folder>(shared_from_this()),
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

				shared_ptr <events::messageCountEvent> event =
					make_shared <events::messageCountEvent>
						(dynamicCast <folder>((*it)->shared_from_this()),
						 events::messageCountEvent::TYPE_ADDED, nums);

				(*it)->notifyMessageCount(event);
			}
		}
	}

	return status;
}


void maildirFolder::expunge()
{
	shared_ptr <maildirStore> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (m_mode == MODE_READ_ONLY)
		throw exceptions::illegal_state("Folder is read-only");

	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

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
				shared_ptr <utility::file> file = fsf->create(curDirPath / infos.path);
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
		for (std::vector <int>::size_type i = nums.size() ; i != 0 ; --i)
			m_messageInfos.erase(m_messageInfos.begin() + (i - 1));
	}

	m_messageCount -= nums.size();
	m_unreadMessageCount -= unreadCount;

	// Notify message expunged
	shared_ptr <events::messageCountEvent> event =
		make_shared <events::messageCountEvent>
			(dynamicCast <folder>(shared_from_this()),
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

			shared_ptr <events::messageCountEvent> event =
				make_shared <events::messageCountEvent>
					(dynamicCast <folder>((*it)->shared_from_this()),
					 events::messageCountEvent::TYPE_REMOVED, nums);

			(*it)->notifyMessageCount(event);
		}
	}
}


shared_ptr <folder> maildirFolder::getParent()
{
	if (m_path.isEmpty())
		return null;
	else
		return make_shared <maildirFolder>(m_path.getParent(), m_store.lock());
}


shared_ptr <const store> maildirFolder::getStore() const
{
	return m_store.lock();
}


shared_ptr <store> maildirFolder::getStore()
{
	return m_store.lock();
}


void maildirFolder::fetchMessages(std::vector <shared_ptr <message> >& msg,
	const fetchAttributes& options, utility::progressListener* progress)
{
	shared_ptr <maildirStore> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (msg.empty())
		return;

	const size_t total = msg.size();
	size_t current = 0;

	if (progress)
		progress->start(total);

	shared_ptr <maildirFolder> thisFolder = dynamicCast <maildirFolder>(shared_from_this());

	for (std::vector <shared_ptr <message> >::iterator it = msg.begin() ;
	     it != msg.end() ; ++it)
	{
		dynamicCast <maildirMessage>(*it)->fetch(thisFolder, options);

		if (progress)
			progress->progress(++current, total);
	}

	if (progress)
		progress->stop(total);
}


void maildirFolder::fetchMessage(shared_ptr <message> msg, const fetchAttributes& options)
{
	shared_ptr <maildirStore> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	dynamicCast <maildirMessage>(msg)->fetch
		(dynamicCast <maildirFolder>(shared_from_this()), options);
}


std::vector <shared_ptr <message> > maildirFolder::getAndFetchMessages
	(const messageSet& msgs, const fetchAttributes& attribs)
{
	if (msgs.isEmpty())
		return std::vector <shared_ptr <message> >();

	std::vector <shared_ptr <message> > messages = getMessages(msgs);
	fetchMessages(messages, attribs);

	return messages;
}


int maildirFolder::getFetchCapabilities() const
{
	return fetchAttributes::ENVELOPE | fetchAttributes::STRUCTURE |
	       fetchAttributes::CONTENT_INFO | fetchAttributes::FLAGS |
	       fetchAttributes::SIZE | fetchAttributes::FULL_HEADER |
	       fetchAttributes::UID | fetchAttributes::IMPORTANCE;
}


const utility::file::path maildirFolder::getMessageFSPath(const int number) const
{
	utility::file::path curDirPath = m_store.lock()->getFormat()->
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


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

