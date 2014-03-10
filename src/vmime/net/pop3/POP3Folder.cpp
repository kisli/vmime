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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/pop3/POP3Folder.hpp"

#include "vmime/net/pop3/POP3Store.hpp"
#include "vmime/net/pop3/POP3Message.hpp"
#include "vmime/net/pop3/POP3Command.hpp"
#include "vmime/net/pop3/POP3Response.hpp"
#include "vmime/net/pop3/POP3FolderStatus.hpp"

#include "vmime/net/pop3/POP3Utils.hpp"

#include "vmime/exception.hpp"


namespace vmime {
namespace net {
namespace pop3 {


POP3Folder::POP3Folder(const folder::path& path, shared_ptr <POP3Store> store)
	: m_store(store), m_path(path),
	  m_name(path.isEmpty() ? folder::path::component("") : path.getLastComponent()),
	  m_mode(-1), m_open(false)
{
	store->registerFolder(this);
}


POP3Folder::~POP3Folder()
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (store)
	{
		if (m_open)
			close(false);

		store->unregisterFolder(this);
	}
	else if (m_open)
	{
		onClose();
	}
}


int POP3Folder::getMode() const
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return (m_mode);
}


const folderAttributes POP3Folder::getAttributes()
{
	folderAttributes attribs;

	if (m_path.isEmpty())
		attribs.setType(folderAttributes::TYPE_CONTAINS_FOLDERS);
	else if (m_path.getSize() == 1 && m_path[0].getBuffer() == "INBOX")
		attribs.setType(folderAttributes::TYPE_CONTAINS_MESSAGES);
	else
		throw exceptions::folder_not_found();

	attribs.setFlags(0);

	return attribs;
}


const folder::path::component POP3Folder::getName() const
{
	return (m_name);
}


const folder::path POP3Folder::getFullPath() const
{
	return (m_path);
}


void POP3Folder::open(const int mode, bool failIfModeIsNotAvailable)
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	if (m_path.isEmpty())
	{
		if (mode != MODE_READ_ONLY && failIfModeIsNotAvailable)
			throw exceptions::operation_not_supported();

		m_open = true;
		m_mode = mode;

		m_messageCount = 0;
	}
	else if (m_path.getSize() == 1 && m_path[0].getBuffer() == "INBOX")
	{
		POP3Command::STAT()->send(store->getConnection());

		shared_ptr <POP3Response> response = POP3Response::readResponse(store->getConnection());

		if (!response->isSuccess())
			throw exceptions::command_error("STAT", response->getFirstLine());

		std::istringstream iss(response->getText());
		iss >> m_messageCount;

		if (iss.fail())
			throw exceptions::invalid_response("STAT", response->getFirstLine());

		m_open = true;
		m_mode = mode;
	}
	else
	{
		throw exceptions::folder_not_found();
	}
}

void POP3Folder::close(const bool expunge)
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (!expunge)
	{
		POP3Command::RSET()->send(store->getConnection());
		POP3Response::readResponse(store->getConnection());
	}

	m_open = false;
	m_mode = -1;

	onClose();
}


void POP3Folder::onClose()
{
	for (MessageMap::iterator it = m_messages.begin() ; it != m_messages.end() ; ++it)
		(*it).first->onFolderClosed();

	m_messages.clear();
}


void POP3Folder::create(const folderAttributes& /* attribs */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::destroy()
{
	throw exceptions::operation_not_supported();
}


bool POP3Folder::exists()
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	return (m_path.isEmpty() || (m_path.getSize() == 1 && m_path[0].getBuffer() == "INBOX"));
}


bool POP3Folder::isOpen() const
{
	return (m_open);
}


shared_ptr <message> POP3Folder::getMessage(const int num)
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (num < 1 || num > m_messageCount)
		throw exceptions::message_not_found();

	return make_shared <POP3Message>(dynamicCast <POP3Folder>(shared_from_this()), num);
}


std::vector <shared_ptr <message> > POP3Folder::getMessages(const messageSet& msgs)
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (msgs.isNumberSet())
	{
		const std::vector <int> numbers = POP3Utils::messageSetToNumberList(msgs);

		std::vector <shared_ptr <message> > messages;
		shared_ptr <POP3Folder> thisFolder(dynamicCast <POP3Folder>(shared_from_this()));

		for (std::vector <int>::const_iterator it = numbers.begin() ; it != numbers.end() ; ++it)
		{
			if (*it < 1|| *it > m_messageCount)
				throw exceptions::message_not_found();

			messages.push_back(make_shared <POP3Message>(thisFolder, *it));
		}

		return messages;
	}
	else
	{
		throw exceptions::operation_not_supported();
	}
}


int POP3Folder::getMessageCount()
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return (m_messageCount);
}


shared_ptr <folder> POP3Folder::getFolder(const folder::path::component& name)
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	return make_shared <POP3Folder>(m_path / name, store);
}


std::vector <shared_ptr <folder> > POP3Folder::getFolders(const bool /* recursive */)
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	if (m_path.isEmpty())
	{
		std::vector <shared_ptr <folder> > v;
		v.push_back(make_shared <POP3Folder>(folder::path::component("INBOX"), store));
		return (v);
	}
	else
	{
		std::vector <shared_ptr <folder> > v;
		return (v);
	}
}


void POP3Folder::fetchMessages(std::vector <shared_ptr <message> >& msg, const fetchAttributes& options,
                               utility::progressListener* progress)
{
	shared_ptr <POP3Store> store = m_store.lock();

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

	for (std::vector <shared_ptr <message> >::iterator it = msg.begin() ;
	     it != msg.end() ; ++it)
	{
		dynamicCast <POP3Message>(*it)->fetch
			(dynamicCast <POP3Folder>(shared_from_this()), options);

		if (progress)
			progress->progress(++current, total);
	}

	if (options.has(fetchAttributes::SIZE))
	{
		// Send the "LIST" command
		POP3Command::LIST()->send(store->getConnection());

		// Get the response
		shared_ptr <POP3Response> response =
			POP3Response::readMultilineResponse(store->getConnection());

		if (response->isSuccess())
		{
			// C: LIST
			// S: +OK
			// S: 1 47548
			// S: 2 12653
			// S: .
			std::map <int, string> result;
			POP3Utils::parseMultiListOrUidlResponse(response, result);

			for (std::vector <shared_ptr <message> >::iterator it = msg.begin() ;
			     it != msg.end() ; ++it)
			{
				shared_ptr <POP3Message> m = dynamicCast <POP3Message>(*it);

				std::map <int, string>::const_iterator x = result.find(m->m_num);

				if (x != result.end())
				{
					size_t size = 0;

					std::istringstream iss((*x).second);
					iss >> size;

					m->m_size = size;
				}
			}
		}

	}

	if (options.has(fetchAttributes::UID))
	{
		// Send the "UIDL" command
		POP3Command::UIDL()->send(store->getConnection());

		// Get the response
		shared_ptr <POP3Response> response =
			POP3Response::readMultilineResponse(store->getConnection());

		if (response->isSuccess())
		{
			// C: UIDL
			// S: +OK
			// S: 1 whqtswO00WBw418f9t5JxYwZ
			// S: 2 QhdPYR:00WBw1Ph7x7
			// S: .
			std::map <int, string> result;
			POP3Utils::parseMultiListOrUidlResponse(response, result);

			for (std::vector <shared_ptr <message> >::iterator it = msg.begin() ;
			     it != msg.end() ; ++it)
			{
				shared_ptr <POP3Message> m = dynamicCast <POP3Message>(*it);

				std::map <int, string>::const_iterator x = result.find(m->m_num);

				if (x != result.end())
					m->m_uid = (*x).second;
			}
		}
	}

	if (progress)
		progress->stop(total);
}


void POP3Folder::fetchMessage(shared_ptr <message> msg, const fetchAttributes& options)
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	dynamicCast <POP3Message>(msg)->fetch
		(dynamicCast <POP3Folder>(shared_from_this()), options);

	if (options.has(fetchAttributes::SIZE))
	{
		// Send the "LIST" command
		POP3Command::LIST(msg->getNumber())->send(store->getConnection());

		// Get the response
		shared_ptr <POP3Response> response =
			POP3Response::readResponse(store->getConnection());

		if (response->isSuccess())
		{
			string responseText = response->getText();

			// C: LIST 2
			// S: +OK 2 4242
			string::iterator it = responseText.begin();

			while (it != responseText.end() && (*it == ' ' || *it == '\t')) ++it;
			while (it != responseText.end() && !(*it == ' ' || *it == '\t')) ++it;
			while (it != responseText.end() && (*it == ' ' || *it == '\t')) ++it;

			if (it != responseText.end())
			{
				size_t size = 0;

				std::istringstream iss(string(it, responseText.end()));
				iss >> size;

				dynamicCast <POP3Message>(msg)->m_size = size;
			}
		}
	}

	if (options.has(fetchAttributes::UID))
	{
		// Send the "UIDL" command
		POP3Command::UIDL(msg->getNumber())->send(store->getConnection());

		// Get the response
		shared_ptr <POP3Response> response =
			POP3Response::readResponse(store->getConnection());

		if (response->isSuccess())
		{
			string responseText = response->getText();

			// C: UIDL 2
			// S: +OK 2 QhdPYR:00WBw1Ph7x7
			string::iterator it = responseText.begin();

			while (it != responseText.end() && (*it == ' ' || *it == '\t')) ++it;
			while (it != responseText.end() && !(*it == ' ' || *it == '\t')) ++it;
			while (it != responseText.end() && (*it == ' ' || *it == '\t')) ++it;

			if (it != responseText.end())
			{
				dynamicCast <POP3Message>(msg)->m_uid =
					string(it, responseText.end());
			}
		}
	}
}


std::vector <shared_ptr <message> > POP3Folder::getAndFetchMessages
	(const messageSet& msgs, const fetchAttributes& attribs)
{
	if (msgs.isEmpty())
		return std::vector <shared_ptr <message> >();

	std::vector <shared_ptr <message> > messages = getMessages(msgs);
	fetchMessages(messages, attribs);

	return messages;
}


int POP3Folder::getFetchCapabilities() const
{
	return fetchAttributes::ENVELOPE | fetchAttributes::CONTENT_INFO |
	       fetchAttributes::SIZE | fetchAttributes::FULL_HEADER |
	       fetchAttributes::UID | fetchAttributes::IMPORTANCE;
}


shared_ptr <folder> POP3Folder::getParent()
{
	if (m_path.isEmpty())
		return null;
	else
		return make_shared <POP3Folder>(m_path.getParent(), m_store.lock());
}


shared_ptr <const store> POP3Folder::getStore() const
{
	return m_store.lock();
}


shared_ptr <store> POP3Folder::getStore()
{
	return m_store.lock();
}


void POP3Folder::registerMessage(POP3Message* msg)
{
	m_messages.insert(MessageMap::value_type(msg, msg->getNumber()));
}


void POP3Folder::unregisterMessage(POP3Message* msg)
{
	m_messages.erase(msg);
}


void POP3Folder::onStoreDisconnected()
{
	m_store.reset();
}


void POP3Folder::deleteMessages(const messageSet& msgs)
{
	shared_ptr <POP3Store> store = m_store.lock();

	const std::vector <int> nums = POP3Utils::messageSetToNumberList(msgs);

	if (nums.empty())
		throw exceptions::invalid_argument();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	for (std::vector <int>::const_iterator
	     it = nums.begin() ; it != nums.end() ; ++it)
	{
		POP3Command::DELE(*it)->send(store->getConnection());

		shared_ptr <POP3Response> response =
			POP3Response::readResponse(store->getConnection());

		if (!response->isSuccess())
			throw exceptions::command_error("DELE", response->getFirstLine());
	}

	// Sort message list
	std::vector <int> list;

	list.resize(nums.size());
	std::copy(nums.begin(), nums.end(), list.begin());

	std::sort(list.begin(), list.end());

	// Update local flags
	for (std::map <POP3Message*, int>::iterator it =
	     m_messages.begin() ; it != m_messages.end() ; ++it)
	{
		POP3Message* msg = (*it).first;

		if (std::binary_search(list.begin(), list.end(), msg->getNumber()))
			msg->m_deleted = true;
	}

	// Notify message flags changed
	shared_ptr <events::messageChangedEvent> event =
		make_shared <events::messageChangedEvent>
			(dynamicCast <folder>(shared_from_this()),
			 events::messageChangedEvent::TYPE_FLAGS, list);

	notifyMessageChanged(event);
}


void POP3Folder::setMessageFlags(const messageSet& /* msgs */,
	const int /* flags */, const int /* mode */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::rename(const folder::path& /* newPath */)
{
	throw exceptions::operation_not_supported();
}


messageSet POP3Folder::addMessage
	(shared_ptr <vmime::message> /* msg */, const int /* flags */,
	 vmime::datetime* /* date */, utility::progressListener* /* progress */)
{
	throw exceptions::operation_not_supported();
}


messageSet POP3Folder::addMessage
	(utility::inputStream& /* is */, const size_t /* size */, const int /* flags */,
	 vmime::datetime* /* date */, utility::progressListener* /* progress */)
{
	throw exceptions::operation_not_supported();
}


messageSet POP3Folder::copyMessages
	(const folder::path& /* dest */, const messageSet& /* msgs */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::status(int& count, int& unseen)
{
	count = 0;
	unseen = 0;

	shared_ptr <folderStatus> status = getStatus();

	count = status->getMessageCount();
	unseen = status->getUnseenCount();
}


shared_ptr <folderStatus> POP3Folder::getStatus()
{
	shared_ptr <POP3Store> store = m_store.lock();

	if (!store)
		throw exceptions::illegal_state("Store disconnected");

	POP3Command::STAT()->send(store->getConnection());

	shared_ptr <POP3Response> response =
		POP3Response::readResponse(store->getConnection());

	if (!response->isSuccess())
		throw exceptions::command_error("STAT", response->getFirstLine());


	unsigned int count = 0;

	std::istringstream iss(response->getText());
	iss >> count;

	shared_ptr <POP3FolderStatus> status = make_shared <POP3FolderStatus>();

	status->setMessageCount(count);
	status->setUnseenCount(count);

	// Update local message count
	if (m_messageCount != count)
	{
		const int oldCount = m_messageCount;

		m_messageCount = count;

		if (count > oldCount)
		{
			std::vector <int> nums;
			nums.resize(count - oldCount);

			for (int i = oldCount + 1, j = 0 ; i <= count ; ++i, ++j)
				nums[j] = i;

			// Notify message count changed
			shared_ptr <events::messageCountEvent> event =
				make_shared <events::messageCountEvent>
					(dynamicCast <folder>(shared_from_this()),
					 events::messageCountEvent::TYPE_ADDED, nums);

			notifyMessageCount(event);

			// Notify folders with the same path
			for (std::list <POP3Folder*>::iterator it = store->m_folders.begin() ;
			     it != store->m_folders.end() ; ++it)
			{
				if ((*it) != this && (*it)->getFullPath() == m_path)
				{
					(*it)->m_messageCount = count;

					shared_ptr <events::messageCountEvent> event =
						make_shared <events::messageCountEvent>
							(dynamicCast <folder>((*it)->shared_from_this()),
							 events::messageCountEvent::TYPE_ADDED, nums);

					(*it)->notifyMessageCount(event);
				}
			}
		}
	}

	return status;
}


void POP3Folder::expunge()
{
	// Not supported by POP3 protocol (deleted messages are automatically
	// expunged at the end of the session...).
}


std::vector <int> POP3Folder::getMessageNumbersStartingOnUID(const message::uid& /* uid */)
{
	throw exceptions::operation_not_supported();
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

