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

#include "messaging/POP3Message.hpp"
#include "messaging/POP3Folder.hpp"
#include "messaging/POP3Store.hpp"

#include <sstream>


namespace vmime {
namespace messaging {


POP3Message::POP3Message(POP3Folder* folder, const int num)
	: m_folder(folder), m_num(num), m_header(NULL)
{
	m_folder->registerMessage(this);
}


POP3Message::~POP3Message()
{
	if (m_folder)
		m_folder->unregisterMessage(this);

	delete (m_header);
}


void POP3Message::onFolderClosed()
{
	m_folder = NULL;
}


const int POP3Message::getNumber() const
{
	return (m_num);
}


const message::uid POP3Message::getUniqueId() const
{
	return (m_uid);
}


const int POP3Message::getSize() const
{
	if (!m_folder)
		throw exceptions::illegal_state("Folder closed");

	POP3Folder::MessageMap::const_iterator it =
		m_folder->m_messages.find(const_cast <POP3Message*>(this));

	return ((it != m_folder->m_messages.end()) ? (*it).second : 0);
}


const bool POP3Message::isExpunged() const
{
	return (false);
}


const int POP3Message::getFlags() const
{
	return (FLAG_RECENT);
}


const structure& POP3Message::getStructure() const
{
	throw exceptions::operation_not_supported();
}


structure& POP3Message::getStructure()
{
	throw exceptions::operation_not_supported();
}


const header& POP3Message::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();

	return (*m_header);
}


void POP3Message::extract(utility::outputStream& os, progressionListener* progress,
                          const int start, const int length) const
{
	if (!m_folder)
		throw exceptions::illegal_state("Folder closed");
	else if (!m_folder->m_store)
		throw exceptions::illegal_state("Store disconnected");

	if (start != 0 && length != -1)
		throw exceptions::partial_fetch_not_supported();

	// Emit the "RETR" command
	std::ostringstream oss;
	oss << "RETR " << m_num;

	const_cast <POP3Folder*>(m_folder)->m_store->sendRequest(oss.str());

	try
	{
		POP3Folder::MessageMap::const_iterator it =
			m_folder->m_messages.find(const_cast <POP3Message*>(this));

		const int totalSize = (it != m_folder->m_messages.end())
			? (*it).second : 0;

		const_cast <POP3Folder*>(m_folder)->m_store->
			readResponse(os, progress, totalSize);
	}
	catch (exceptions::command_error& e)
	{
		throw exceptions::command_error("RETR", e.response());
	}
}


void POP3Message::extractPart
	(const part& /* p */, utility::outputStream& /* os */, progressionListener* /* progress */,
	 const int /* start */, const int /* length */) const
{
	throw exceptions::operation_not_supported();
}


void POP3Message::fetchPartHeader(part& /* p */)
{
	throw exceptions::operation_not_supported();
}


void POP3Message::fetch(POP3Folder* folder, const int options)
{
	if (m_folder != folder)
		throw exceptions::folder_not_found();

	// FETCH_STRUCTURE and FETCH_FLAGS are not supported by POP3.
	if (options & (folder::FETCH_STRUCTURE | folder::FETCH_FLAGS))
		throw exceptions::operation_not_supported();

	// Check for the real need to fetch the full header
	if (!((options & folder::FETCH_ENVELOPE) ||
	      (options & folder::FETCH_CONTENT_INFO) ||
	      (options & folder::FETCH_FULL_HEADER)))
	{
		return;
	}

	// No need to differenciate between FETCH_ENVELOPE,
	// FETCH_CONTENT_INFO, ... since POP3 only permits to
	// retrieve the whole header and not fields in particular.

	// Emit the "TOP" command
	std::ostringstream oss;
	oss << "TOP " << m_num << " 0";

	m_folder->m_store->sendRequest(oss.str());

	try
	{
		string buffer;
		m_folder->m_store->readResponse(buffer, true);

		if (m_header != NULL)
		{
			delete (m_header);
			m_header = NULL;
		}

		m_header = new header();
		m_header->parse(buffer);
	}
	catch (exceptions::command_error& e)
	{
		throw exceptions::command_error("TOP", e.response());
	}
}


void POP3Message::setFlags(const int /* flags */, const int /* mode */)
{
	throw exceptions::operation_not_supported();
}


} // messaging
} // vmime
