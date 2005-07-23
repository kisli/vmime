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

#include "vmime/messaging/pop3/POP3Folder.hpp"

#include "vmime/messaging/pop3/POP3Store.hpp"
#include "vmime/messaging/pop3/POP3Message.hpp"

#include "vmime/exception.hpp"


namespace vmime {
namespace messaging {
namespace pop3 {


POP3Folder::POP3Folder(const folder::path& path, POP3Store* store)
	: m_store(store), m_path(path),
	  m_name(path.isEmpty() ? folder::path::component("") : path.getLastComponent()),
	  m_mode(-1), m_open(false)
{
	m_store->registerFolder(this);
}


POP3Folder::~POP3Folder()
{
	if (m_store)
	{
		if (m_open)
			close(false);

		m_store->unregisterFolder(this);
	}
	else if (m_open)
	{
		onClose();
	}
}


const int POP3Folder::getMode() const
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return (m_mode);
}


const int POP3Folder::getType()
{
	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (m_path.isEmpty())
		return (TYPE_CONTAINS_FOLDERS);
	else if (m_path.getSize() == 1 && m_path[0].getBuffer() == "INBOX")
		return (TYPE_CONTAINS_MESSAGES);
	else
		throw exceptions::folder_not_found();
}


const int POP3Folder::getFlags()
{
	return (0);
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
	if (!m_store)
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
		m_store->sendRequest("STAT");

		string response;
		m_store->readResponse(response, false);

		if (!m_store->isSuccessResponse(response))
			throw exceptions::command_error("STAT", response);

		m_store->stripResponseCode(response, response);

		std::istringstream iss(response);
		iss >> m_messageCount;

		if (iss.fail())
			throw exceptions::invalid_response("STAT", response);

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
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");

	if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	if (!expunge)
	{
		m_store->sendRequest("RSET");

		string response;
		m_store->readResponse(response, false);
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


void POP3Folder::create(const int /* type */)
{
	throw exceptions::operation_not_supported();
}


const bool POP3Folder::exists()
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");

	return (m_path.isEmpty() || (m_path.getSize() == 1 && m_path[0].getBuffer() == "INBOX"));
}


const bool POP3Folder::isOpen() const
{
	return (m_open);
}


ref <message> POP3Folder::getMessage(const int num)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (num < 1 || num > m_messageCount)
		throw exceptions::message_not_found();

	return vmime::create <POP3Message>(this, num);
}


std::vector <ref <message> > POP3Folder::getMessages(const int from, const int to)
{
	const int to2 = (to == -1 ? m_messageCount : to);

	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");
	else if (to2 < from || from < 1 || to2 < 1 || from > m_messageCount || to2 > m_messageCount)
		throw exceptions::message_not_found();

	std::vector <ref <message> > v;

	for (int i = from ; i <= to2 ; ++i)
		v.push_back(vmime::create <POP3Message>(this, i));

	return (v);
}


std::vector <ref <message> > POP3Folder::getMessages(const std::vector <int>& nums)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	std::vector <ref <message> > v;

	for (std::vector <int>::const_iterator it = nums.begin() ; it != nums.end() ; ++it)
	{
		if (*it < 1|| *it > m_messageCount)
			throw exceptions::message_not_found();

		v.push_back(vmime::create <POP3Message>(this, *it));
	}

	return (v);
}


const int POP3Folder::getMessageCount()
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	return (m_messageCount);
}


ref <folder> POP3Folder::getFolder(const folder::path::component& name)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");

	return vmime::create <POP3Folder>(m_path / name, m_store);
}


std::vector <ref <folder> > POP3Folder::getFolders(const bool /* recursive */)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");

	if (m_path.isEmpty())
	{
		std::vector <ref <folder> > v;
		v.push_back(vmime::create <POP3Folder>(folder::path::component("INBOX"), m_store));
		return (v);
	}
	else
	{
		std::vector <ref <folder> > v;
		return (v);
	}
}


void POP3Folder::fetchMessages(std::vector <ref <message> >& msg, const int options,
                               utility::progressionListener* progress)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	const int total = msg.size();
	int current = 0;

	if (progress)
		progress->start(total);

	for (std::vector <ref <message> >::iterator it = msg.begin() ;
	     it != msg.end() ; ++it)
	{
		(*it).dynamicCast <POP3Message>()->fetch(this, options);

		if (progress)
			progress->progress(++current, total);
	}

	if (options & FETCH_SIZE)
	{
		// Send the "LIST" command
		std::ostringstream command;
		command << "LIST";

		m_store->sendRequest(command.str());

		// Get the response
		string response;
		m_store->readResponse(response, true, NULL);

		if (m_store->isSuccessResponse(response))
		{
			m_store->stripFirstLine(response, response, NULL);

			// C: LIST
			// S: +OK
			// S: 1 47548
			// S: 2 12653
			// S: .
			std::map <int, string> result;
			parseMultiListOrUidlResponse(response, result);

			for (std::vector <ref <message> >::iterator it = msg.begin() ;
			     it != msg.end() ; ++it)
			{
				ref <POP3Message> m = (*it).dynamicCast <POP3Message>();

				std::map <int, string>::const_iterator x = result.find(m->m_num);

				if (x != result.end())
				{
					int size = 0;

					std::istringstream iss((*x).second);
					iss >> size;

					m->m_size = size;
				}
			}
		}

	}

	if (options & FETCH_UID)
	{
		// Send the "UIDL" command
		std::ostringstream command;
		command << "UIDL";

		m_store->sendRequest(command.str());

		// Get the response
		string response;
		m_store->readResponse(response, true, NULL);

		if (m_store->isSuccessResponse(response))
		{
			m_store->stripFirstLine(response, response, NULL);

			// C: UIDL
			// S: +OK
			// S: 1 whqtswO00WBw418f9t5JxYwZ
			// S: 2 QhdPYR:00WBw1Ph7x7
			// S: .
			std::map <int, string> result;
			parseMultiListOrUidlResponse(response, result);

			for (std::vector <ref <message> >::iterator it = msg.begin() ;
			     it != msg.end() ; ++it)
			{
				ref <POP3Message> m = (*it).dynamicCast <POP3Message>();

				std::map <int, string>::const_iterator x = result.find(m->m_num);

				if (x != result.end())
					m->m_uid = (*x).second;
			}
		}
	}

	if (progress)
		progress->stop(total);
}


void POP3Folder::fetchMessage(ref <message> msg, const int options)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	msg.dynamicCast <POP3Message>()->fetch(this, options);

	if (options & FETCH_SIZE)
	{
		// Send the "LIST" command
		std::ostringstream command;
		command << "LIST " << msg->getNumber();

		m_store->sendRequest(command.str());

		// Get the response
		string response;
		m_store->readResponse(response, false, NULL);

		if (m_store->isSuccessResponse(response))
		{
			m_store->stripResponseCode(response, response);

			// C: LIST 2
			// S: +OK 2 4242
			string::iterator it = response.begin();

			while (it != response.end() && (*it == ' ' || *it == '\t')) ++it;
			while (it != response.end() && !(*it == ' ' || *it == '\t')) ++it;
			while (it != response.end() && (*it == ' ' || *it == '\t')) ++it;

			if (it != response.end())
			{
				int size = 0;

				std::istringstream iss(string(it, response.end()));
				iss >> size;

				msg.dynamicCast <POP3Message>()->m_size = size;
			}
		}
	}

	if (options & FETCH_UID)
	{
		// Send the "UIDL" command
		std::ostringstream command;
		command << "UIDL " << msg->getNumber();

		m_store->sendRequest(command.str());

		// Get the response
		string response;
		m_store->readResponse(response, false, NULL);

		if (m_store->isSuccessResponse(response))
		{
			m_store->stripResponseCode(response, response);

			// C: UIDL 2
			// S: +OK 2 QhdPYR:00WBw1Ph7x7
			string::iterator it = response.begin();

			while (it != response.end() && (*it == ' ' || *it == '\t')) ++it;
			while (it != response.end() && !(*it == ' ' || *it == '\t')) ++it;
			while (it != response.end() && (*it == ' ' || *it == '\t')) ++it;

			if (it != response.end())
			{
				msg.dynamicCast <POP3Message>()->m_uid =
					string(it, response.end());
			}
		}
	}
}


const int POP3Folder::getFetchCapabilities() const
{
	return (FETCH_ENVELOPE | FETCH_CONTENT_INFO |
	        FETCH_SIZE | FETCH_FULL_HEADER | FETCH_UID);
}


ref <folder> POP3Folder::getParent()
{
	if (m_path.isEmpty())
		return NULL;
	else
		return vmime::create <POP3Folder>(m_path.getParent(), m_store);
}


weak_ref <const store> POP3Folder::getStore() const
{
	return (m_store);
}


weak_ref <store> POP3Folder::getStore()
{
	return (m_store);
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
	m_store = NULL;
}


void POP3Folder::deleteMessage(const int num)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	std::ostringstream command;
	command << "DELE " << num;

	m_store->sendRequest(command.str());

	string response;
	m_store->readResponse(response, false);

	if (!m_store->isSuccessResponse(response))
		throw exceptions::command_error("DELE", response);

	// Update local flags
	for (std::map <POP3Message*, int>::iterator it =
	     m_messages.begin() ; it != m_messages.end() ; ++it)
	{
		POP3Message* msg = (*it).first;

		if (msg->getNumber() == num)
			msg->m_deleted = true;
	}

	// Notify message flags changed
	std::vector <int> nums;
	nums.push_back(num);

	events::messageChangedEvent event
		(thisRef().dynamicCast <folder>(),
		 events::messageChangedEvent::TYPE_FLAGS, nums);

	notifyMessageChanged(event);
}


void POP3Folder::deleteMessages(const int from, const int to)
{
	if (from < 1 || (to < from && to != -1))
		throw exceptions::invalid_argument();

	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	const int to2 = (to == -1 ? m_messageCount : to);

	for (int i = from ; i <= to2 ; ++i)
	{
		std::ostringstream command;
		command << "DELE " << i;

		m_store->sendRequest(command.str());

		string response;
		m_store->readResponse(response, false);

		if (!m_store->isSuccessResponse(response))
			throw exceptions::command_error("DELE", response);
	}

	// Update local flags
	for (std::map <POP3Message*, int>::iterator it =
	     m_messages.begin() ; it != m_messages.end() ; ++it)
	{
		POP3Message* msg = (*it).first;

		if (msg->getNumber() >= from && msg->getNumber() <= to2)
			msg->m_deleted = true;
	}

	// Notify message flags changed
	std::vector <int> nums;

	for (int i = from ; i <= to2 ; ++i)
		nums.push_back(i);

	events::messageChangedEvent event
		(thisRef().dynamicCast <folder>(),
		 events::messageChangedEvent::TYPE_FLAGS, nums);

	notifyMessageChanged(event);
}


void POP3Folder::deleteMessages(const std::vector <int>& nums)
{
	if (nums.empty())
		throw exceptions::invalid_argument();

	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	for (std::vector <int>::const_iterator
	     it = nums.begin() ; it != nums.end() ; ++it)
	{
		std::ostringstream command;
		command << "DELE " << (*it);

		m_store->sendRequest(command.str());

		string response;
		m_store->readResponse(response, false);

		if (!m_store->isSuccessResponse(response))
			throw exceptions::command_error("DELE", response);
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
	events::messageChangedEvent event
		(thisRef().dynamicCast <folder>(),
		 events::messageChangedEvent::TYPE_FLAGS, list);

	notifyMessageChanged(event);
}


void POP3Folder::setMessageFlags(const int /* from */, const int /* to */,
	const int /* flags */, const int /* mode */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::setMessageFlags(const std::vector <int>& /* nums */,
	const int /* flags */, const int /* mode */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::rename(const folder::path& /* newPath */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::addMessage(ref <vmime::message> /* msg */, const int /* flags */,
	vmime::datetime* /* date */, utility::progressionListener* /* progress */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::addMessage(utility::inputStream& /* is */, const int /* size */, const int /* flags */,
	vmime::datetime* /* date */, utility::progressionListener* /* progress */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::copyMessage(const folder::path& /* dest */, const int /* num */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::copyMessages(const folder::path& /* dest */, const int /* from */, const int /* to */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::copyMessages(const folder::path& /* dest */, const std::vector <int>& /* nums */)
{
	throw exceptions::operation_not_supported();
}


void POP3Folder::status(int& count, int& unseen)
{
	if (!m_store)
		throw exceptions::illegal_state("Store disconnected");
	else if (!isOpen())
		throw exceptions::illegal_state("Folder not open");

	m_store->sendRequest("STAT");

	string response;
	m_store->readResponse(response, false);

	if (!m_store->isSuccessResponse(response))
		throw exceptions::command_error("STAT", response);

	m_store->stripResponseCode(response, response);

	std::istringstream iss(response);
	iss >> count;

	unseen = count;

	// Update local message count
	if (m_messageCount != count)
	{
		const int oldCount = m_messageCount;

		m_messageCount = count;

		if (count > oldCount)
		{
			std::vector <int> nums;
			nums.reserve(count - oldCount);

			for (int i = oldCount + 1, j = 0 ; i <= count ; ++i, ++j)
				nums[j] = i;

			// Notify message count changed
			events::messageCountEvent event
				(thisRef().dynamicCast <folder>(),
				 events::messageCountEvent::TYPE_ADDED, nums);

			notifyMessageCount(event);

			// Notify folders with the same path
			for (std::list <POP3Folder*>::iterator it = m_store->m_folders.begin() ;
			     it != m_store->m_folders.end() ; ++it)
			{
				if ((*it) != this && (*it)->getFullPath() == m_path)
				{
					(*it)->m_messageCount = count;

					events::messageCountEvent event
						((*it)->thisRef().dynamicCast <folder>(),
						 events::messageCountEvent::TYPE_ADDED, nums);

					(*it)->notifyMessageCount(event);
				}
			}
		}
	}
}


void POP3Folder::expunge()
{
	// Not supported by POP3 protocol (deleted messages are automatically
	// expunged at the end of the session...).
}


void POP3Folder::parseMultiListOrUidlResponse(const string& response, std::map <int, string>& result)
{
	std::istringstream iss(response);
	std::map <int, string> ids;

	string line;

	while (std::getline(iss, line))
	{
		string::iterator it = line.begin();

		while (it != line.end() && (*it == ' ' || *it == '\t'))
			++it;

		if (it != line.end())
		{
			int number = 0;

			while (it != line.end() && (*it >= '0' && *it <= '9'))
			{
				number = (number * 10) + (*it - '0');
				++it;
			}

			while (it != line.end() && !(*it == ' ' || *it == '\t')) ++it;
			while (it != line.end() && (*it == ' ' || *it == '\t')) ++it;

			if (it != line.end())
			{
				result.insert(std::map <int, string>::value_type(number, string(it, line.end())));
			}
		}
	}
}


} // pop3
} // messaging
} // vmime
