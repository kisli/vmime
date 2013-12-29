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


#include "vmime/net/pop3/POP3Store.hpp"
#include "vmime/net/pop3/POP3Folder.hpp"
#include "vmime/net/pop3/POP3Command.hpp"
#include "vmime/net/pop3/POP3Response.hpp"

#include "vmime/exception.hpp"

#include <algorithm>


namespace vmime {
namespace net {
namespace pop3 {


POP3Store::POP3Store(shared_ptr <session> sess, shared_ptr <security::authenticator> auth, const bool secured)
	: store(sess, getInfosInstance(), auth), m_isPOP3S(secured)
{
}


POP3Store::~POP3Store()
{
	try
	{
		if (isConnected())
			disconnect();
	}
	catch (vmime::exception&)
	{
		// Ignore
	}
}


const string POP3Store::getProtocolName() const
{
	return "pop3";
}


shared_ptr <folder> POP3Store::getDefaultFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return make_shared <POP3Folder>
		(folder::path(folder::path::component("INBOX")),
		 dynamicCast <POP3Store>(shared_from_this()));
}


shared_ptr <folder> POP3Store::getRootFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return make_shared <POP3Folder>
		(folder::path(), dynamicCast <POP3Store>(shared_from_this()));
}


shared_ptr <folder> POP3Store::getFolder(const folder::path& path)
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return make_shared <POP3Folder>
		(path, dynamicCast <POP3Store>(shared_from_this()));
}


bool POP3Store::isValidFolderName(const folder::path::component& /* name */) const
{
	return true;
}


void POP3Store::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	m_connection = make_shared <POP3Connection>
		(dynamicCast <POP3Store>(shared_from_this()), getAuthenticator());

	try
	{
		m_connection->connect();
	}
	catch (std::exception&)
	{
		m_connection = null;
		throw;
	}
}


bool POP3Store::isPOP3S() const
{
	return m_isPOP3S;
}


bool POP3Store::isConnected() const
{
	return m_connection && m_connection->isConnected();
}


bool POP3Store::isSecuredConnection() const
{
	if (m_connection == NULL)
		return false;

	return m_connection->isSecuredConnection();
}


shared_ptr <connectionInfos> POP3Store::getConnectionInfos() const
{
	if (m_connection == NULL)
		return null;

	return m_connection->getConnectionInfos();
}


shared_ptr <POP3Connection> POP3Store::getConnection()
{
	return m_connection;
}


void POP3Store::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	for (std::list <POP3Folder*>::iterator it = m_folders.begin() ;
	     it != m_folders.end() ; ++it)
	{
		(*it)->onStoreDisconnected();
	}

	m_folders.clear();


	m_connection->disconnect();
	m_connection = null;
}


void POP3Store::noop()
{
	if (!m_connection)
		throw exceptions::not_connected();

	POP3Command::NOOP()->send(m_connection);

	shared_ptr <POP3Response> response = POP3Response::readResponse(m_connection);

	if (!response->isSuccess())
		throw exceptions::command_error("NOOP", response->getFirstLine());
}


void POP3Store::registerFolder(POP3Folder* folder)
{
	m_folders.push_back(folder);
}


void POP3Store::unregisterFolder(POP3Folder* folder)
{
	std::list <POP3Folder*>::iterator it = std::find(m_folders.begin(), m_folders.end(), folder);
	if (it != m_folders.end()) m_folders.erase(it);
}


int POP3Store::getCapabilities() const
{
	return (CAPABILITY_DELETE_MESSAGE);
}



// Service infos

POP3ServiceInfos POP3Store::sm_infos(false);


const serviceInfos& POP3Store::getInfosInstance()
{
	return sm_infos;
}


const serviceInfos& POP3Store::getInfos() const
{
	return sm_infos;
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

