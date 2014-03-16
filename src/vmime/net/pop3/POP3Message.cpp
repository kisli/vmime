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


#include "vmime/net/pop3/POP3Message.hpp"
#include "vmime/net/pop3/POP3Command.hpp"
#include "vmime/net/pop3/POP3Response.hpp"
#include "vmime/net/pop3/POP3Folder.hpp"
#include "vmime/net/pop3/POP3Store.hpp"

#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"

#include <sstream>


namespace vmime {
namespace net {
namespace pop3 {


POP3Message::POP3Message(shared_ptr <POP3Folder> folder, const int num)
	: m_folder(folder), m_num(num), m_size(-1), m_deleted(false)
{
	folder->registerMessage(this);
}


POP3Message::~POP3Message()
{
	shared_ptr <POP3Folder> folder = m_folder.lock();

	if (folder)
		folder->unregisterMessage(this);
}


void POP3Message::onFolderClosed()
{
	m_folder.reset();
}


int POP3Message::getNumber() const
{
	return (m_num);
}


const message::uid POP3Message::getUID() const
{
	return (m_uid);
}


size_t POP3Message::getSize() const
{
	if (m_size == static_cast <size_t>(-1))
		throw exceptions::unfetched_object();

	return (m_size);
}


bool POP3Message::isExpunged() const
{
	return (false);
}


int POP3Message::getFlags() const
{
	int flags = 0;

	if (m_deleted)
		flags |= FLAG_DELETED;

	return (flags);
}


shared_ptr <const messageStructure> POP3Message::getStructure() const
{
	throw exceptions::operation_not_supported();
}


shared_ptr <messageStructure> POP3Message::getStructure()
{
	throw exceptions::operation_not_supported();
}


shared_ptr <const header> POP3Message::getHeader() const
{
	if (m_header == NULL)
		throw exceptions::unfetched_object();

	return (m_header);
}


void POP3Message::extract
	(utility::outputStream& os,
	 utility::progressListener* progress,
	 const size_t start, const size_t length,
	 const bool /* peek */) const
{
	shared_ptr <const POP3Folder> folder = m_folder.lock();

	if (!folder)
		throw exceptions::illegal_state("Folder closed");
	else if (!folder->getStore())
		throw exceptions::illegal_state("Store disconnected");

	if (start != 0 && length != static_cast <size_t>(-1))
		throw exceptions::partial_fetch_not_supported();

	// Emit the "RETR" command
	shared_ptr <POP3Store> store = constCast <POP3Folder>(folder)->m_store.lock();

	POP3Command::RETR(m_num)->send(store->getConnection());

	try
	{
		POP3Response::readLargeResponse
			(store->getConnection(), os, progress, m_size == -1 ? 0 : m_size);
	}
	catch (exceptions::command_error& e)
	{
		throw exceptions::command_error("RETR", e.response());
	}
}


void POP3Message::extractPart
	(shared_ptr <const messagePart> /* p */,
	 utility::outputStream& /* os */,
	 utility::progressListener* /* progress */,
	 const size_t /* start */, const size_t /* length */,
	 const bool /* peek */) const
{
	throw exceptions::operation_not_supported();
}


void POP3Message::fetchPartHeader(shared_ptr <messagePart> /* p */)
{
	throw exceptions::operation_not_supported();
}


void POP3Message::fetch(shared_ptr <POP3Folder> msgFolder, const fetchAttributes& options)
{
	shared_ptr <POP3Folder> folder = m_folder.lock();

	if (folder != msgFolder)
		throw exceptions::folder_not_found();

	// STRUCTURE and FLAGS attributes are not supported by POP3
	if (options.has(fetchAttributes::STRUCTURE | fetchAttributes::FLAGS))
		throw exceptions::operation_not_supported();

	// Check for the real need to fetch the full header
	static const int optionsRequiringHeader =
		fetchAttributes::ENVELOPE | fetchAttributes::CONTENT_INFO |
		fetchAttributes::FULL_HEADER | fetchAttributes::IMPORTANCE;

	if (!options.has(optionsRequiringHeader))
		return;

	// No need to differenciate between ENVELOPE, CONTENT_INFO, ...
	// since POP3 only permits to retrieve the whole header and not
	// fields in particular.

	// Emit the "TOP" command
	shared_ptr <POP3Store> store = folder->m_store.lock();

	POP3Command::TOP(m_num, 0)->send(store->getConnection());

	try
	{
		string buffer;
		utility::outputStreamStringAdapter bufferStream(buffer);

		POP3Response::readLargeResponse(store->getConnection(),
			bufferStream, /* progress */ NULL, /* predictedSize */ 0);

		m_header = make_shared <header>();
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


shared_ptr <vmime::message> POP3Message::getParsedMessage()
{
	std::ostringstream oss;
	utility::outputStreamAdapter os(oss);

	extract(os);

	shared_ptr <vmime::message> msg = make_shared <vmime::message>();
	msg->parse(oss.str());

	return msg;
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

