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

#include "vmime/messaging/maildirStore.hpp"

#include "vmime/messaging/maildirFolder.hpp"

#include "vmime/utility/smartPtr.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"


namespace vmime {
namespace messaging {


maildirStore::maildirStore(session* sess, authenticator* auth)
	: store(sess, getInfosInstance(), auth), m_connected(false)
{
}


maildirStore::~maildirStore()
{
	if (isConnected())
		disconnect();
}


const string maildirStore::getProtocolName() const
{
	return "maildir";
}


folder* maildirStore::getRootFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return new maildirFolder(folder::path(), this);
}


folder* maildirStore::getDefaultFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return new maildirFolder(folder::path::component("inbox"), this);
}


folder* maildirStore::getFolder(const folder::path& path)
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return new maildirFolder(path, this);
}


const bool maildirStore::isValidFolderName(const folder::path::component& name) const
{
	if (!platformDependant::getHandler()->getFileSystemFactory()->isValidPathComponent(name))
		return false;

	const string& buf = name.getBuffer();

	// Name cannot start/end with spaces
	if (stringUtils::trim(buf) != name.getBuffer())
		return false;

	// Name cannot start with '.'
	const int length = buf.length();
	int pos = 0;

	while ((pos < length) && (buf[pos] == '.'))
		++pos;

	return (pos == 0);
}


void maildirStore::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	// Get root directory
	utility::fileSystemFactory* fsf = platformDependant::getHandler()->getFileSystemFactory();

	m_fsPath = fsf->stringToPath
		(getSession()->getProperties()[getInfos().getPropertyPrefix() + "server.rootpath"]);

	utility::auto_ptr <utility::file> rootDir = fsf->create(m_fsPath);

	// Try to create the root directory if it does not exist
	if (!(rootDir->exists() && rootDir->isDirectory()))
	{
		try
		{
			rootDir->createDirectory();
		}
		catch (exceptions::filesystem_exception& e)
		{
			throw exceptions::connection_error(e);
		}
	}

	m_connected = true;
}


const bool maildirStore::isConnected() const
{
	return (m_connected);
}


void maildirStore::disconnect()
{
	for (std::list <maildirFolder*>::iterator it = m_folders.begin() ;
	     it != m_folders.end() ; ++it)
	{
		(*it)->onStoreDisconnected();
	}

	m_folders.clear();

	m_connected = false;
}


void maildirStore::noop()
{
	// Nothing to do.
}


void maildirStore::registerFolder(maildirFolder* folder)
{
	m_folders.push_back(folder);
}


void maildirStore::unregisterFolder(maildirFolder* folder)
{
	std::list <maildirFolder*>::iterator it = std::find(m_folders.begin(), m_folders.end(), folder);
	if (it != m_folders.end()) m_folders.erase(it);
}


const utility::path& maildirStore::getFileSystemPath() const
{
	return (m_fsPath);
}


const int maildirStore::getCapabilities() const
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

maildirStore::_infos maildirStore::sm_infos;


const serviceInfos& maildirStore::getInfosInstance()
{
	return (sm_infos);
}


const serviceInfos& maildirStore::getInfos() const
{
	return (sm_infos);
}


const port_t maildirStore::_infos::getDefaultPort() const
{
	return (0);
}


const string maildirStore::_infos::getPropertyPrefix() const
{
	return "store.maildir.";
}


const std::vector <string> maildirStore::_infos::getAvailableProperties() const
{
	std::vector <string> list;

	list.push_back("server.rootpath");

	return (list);
}


} // messaging
} // vmime
