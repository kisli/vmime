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
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/net/imap/IMAPStore.hpp"
#include "vmime/net/imap/IMAPFolder.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"

#include <map>


namespace vmime {
namespace net {
namespace imap {


IMAPStore::IMAPStore(ref <session> sess, ref <security::authenticator> auth, const bool secured)
	: store(sess, getInfosInstance(), auth), m_connection(NULL), m_secured(secured)
{
}


IMAPStore::~IMAPStore()
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


const string IMAPStore::getProtocolName() const
{
	return "imap";
}


ref <folder> IMAPStore::getRootFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <IMAPFolder>(folder::path(), this);
}


ref <folder> IMAPStore::getDefaultFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <IMAPFolder>(folder::path::component("INBOX"), this);
}


ref <folder> IMAPStore::getFolder(const folder::path& path)
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <IMAPFolder>(path, this);
}


const bool IMAPStore::isValidFolderName(const folder::path::component& /* name */) const
{
	return true;
}


const bool IMAPStore::isSecuredConnection() const
{
	return m_secured;
}


void IMAPStore::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	m_connection = vmime::create <IMAPConnection>
		(thisWeakRef().dynamicCast <IMAPStore>(), getAuthenticator());

	try
	{
		m_connection->connect();
	}
	catch (std::exception&)
	{
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


ref <IMAPConnection> IMAPStore::connection()
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

IMAPServiceInfos IMAPStore::sm_infos(false);


const serviceInfos& IMAPStore::getInfosInstance()
{
	return sm_infos;
}


const serviceInfos& IMAPStore::getInfos() const
{
	return sm_infos;
}


} // imap
} // net
} // vmime
