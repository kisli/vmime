//
// VMime library (http://vmime.sourceforge.net)
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

#include "vmime/messaging/IMAPStore.hpp"
#include "vmime/messaging/IMAPFolder.hpp"
#include "vmime/messaging/IMAPConnection.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"

#include <map>


namespace vmime {
namespace messaging {


#ifndef VMIME_BUILDING_DOC

//
// IMAPauthenticator: private class used internally
//
// Used to request user credentials only in the first authentication
// and reuse this information the next times
//

class IMAPauthenticator : public authenticator
{
public:

	IMAPauthenticator(authenticator* auth)
		: m_auth(auth), m_infos(NULL)
	{
	}

	~IMAPauthenticator()
	{
		delete (m_infos);
	}

	const authenticationInfos requestAuthInfos() const
	{
		if (m_infos == NULL)
			m_infos = new authenticationInfos(m_auth->requestAuthInfos());

		return (*m_infos);
	}

private:

	authenticator* m_auth;
	mutable authenticationInfos* m_infos;
};

#endif // VMIME_BUILDING_DOC



//
// IMAPStore
//

IMAPStore::IMAPStore(session* sess, authenticator* auth)
	: store(sess, getInfosInstance(), auth),
	  m_connection(NULL), m_oneTimeAuth(NULL)
{
}


IMAPStore::~IMAPStore()
{
	if (isConnected())
		disconnect();
}


authenticator* IMAPStore::oneTimeAuthenticator()
{
	return (m_oneTimeAuth);
}


const string IMAPStore::getProtocolName() const
{
	return "imap";
}


folder* IMAPStore::getRootFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return new IMAPFolder(folder::path(), this);
}


folder* IMAPStore::getDefaultFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return new IMAPFolder(folder::path::component("INBOX"), this);
}


folder* IMAPStore::getFolder(const folder::path& path)
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return new IMAPFolder(path, this);
}


const bool IMAPStore::isValidFolderName(const folder::path::component& /* name */) const
{
	return true;
}


void IMAPStore::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	m_oneTimeAuth = new IMAPauthenticator(getAuthenticator());

	m_connection = new IMAPConnection(this, m_oneTimeAuth);

	try
	{
		m_connection->connect();
	}
	catch (std::exception&)
	{
		delete (m_connection);
		m_connection = NULL;
		throw;
	}
}


const bool IMAPStore::isConnected() const
{
	return (m_connection && m_connection->isConnected());
}


void IMAPStore::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	for (std::list <IMAPFolder*>::iterator it = m_folders.begin() ;
	     it != m_folders.end() ; ++it)
	{
		(*it)->onStoreDisconnected();
	}

	m_folders.clear();


	m_connection->disconnect();

	delete (m_oneTimeAuth);
	m_oneTimeAuth = NULL;

	delete (m_connection);
	m_connection = NULL;
}


void IMAPStore::noop()
{
	if (!isConnected())
		throw exceptions::not_connected();

	m_connection->send(true, "NOOP", true);

	utility::auto_ptr <IMAPParser::response> resp(m_connection->readResponse());

	if (resp->isBad() || resp->response_done()->response_tagged()->
			resp_cond_state()->status() != IMAPParser::resp_cond_state::OK)
	{
		throw exceptions::command_error("NOOP", m_connection->getParser()->lastLine());
	}
}


IMAPConnection* IMAPStore::connection()
{
	return (m_connection);
}


void IMAPStore::registerFolder(IMAPFolder* folder)
{
	m_folders.push_back(folder);
}


void IMAPStore::unregisterFolder(IMAPFolder* folder)
{
	std::list <IMAPFolder*>::iterator it = std::find(m_folders.begin(), m_folders.end(), folder);
	if (it != m_folders.end()) m_folders.erase(it);
}


const int IMAPStore::getCapabilities() const
{
	return (CAPABILITY_CREATE_FOLDER |
	        CAPABILITY_RENAME_FOLDER |
	        CAPABILITY_ADD_MESSAGE |
	        CAPABILITY_COPY_MESSAGE |
		   CAPABILITY_DELETE_MESSAGE |
	        CAPABILITY_PARTIAL_FETCH |
	        CAPABILITY_MESSAGE_FLAGS |
		   CAPABILITY_EXTRACT_PART);
}




// Service infos

IMAPStore::_infos IMAPStore::sm_infos;


const serviceInfos& IMAPStore::getInfosInstance()
{
	return (sm_infos);
}


const serviceInfos& IMAPStore::getInfos() const
{
	return (sm_infos);
}


const port_t IMAPStore::_infos::getDefaultPort() const
{
	return (143);
}


const string IMAPStore::_infos::getPropertyPrefix() const
{
	return "store.imap.";
}


const std::vector <string> IMAPStore::_infos::getAvailableProperties() const
{
	std::vector <string> list;

	// IMAP-specific options
	//list.push_back("auth.mechanism");

	// Common properties
	list.push_back("auth.username");
	list.push_back("auth.password");

	list.push_back("server.address");
	list.push_back("server.port");
	list.push_back("server.socket-factory");

	list.push_back("timeout.factory");

	return (list);
}


} // messaging
} // vmime
