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

#include "vmime/net/maildir/maildirStore.hpp"

#include "vmime/net/maildir/maildirFolder.hpp"

#include "vmime/utility/smartPtr.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"


// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(sm_infos.getPropertyValue <type>(getSession(), sm_infos.getProperties().prop))
#define HAS_PROPERTY(prop) \
	(sm_infos.hasProperty(getSession(), sm_infos.getProperties().prop))


namespace vmime {
namespace net {
namespace maildir {


maildirStore::maildirStore(ref <session> sess, ref <authenticator> auth)
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


ref <folder> maildirStore::getRootFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <maildirFolder>(folder::path(),
		thisWeakRef().dynamicCast <maildirStore>());
}


ref <folder> maildirStore::getDefaultFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <maildirFolder>(folder::path::component("inbox"),
		thisWeakRef().dynamicCast <maildirStore>());
}


ref <folder> maildirStore::getFolder(const folder::path& path)
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return vmime::create <maildirFolder>(path,
		thisWeakRef().dynamicCast <maildirStore>());
}


const bool maildirStore::isValidFolderName(const folder::path::component& name) const
{
	if (!platformDependant::getHandler()->getFileSystemFactory()->isValidPathComponent(name))
		return false;

	const string& buf = name.getBuffer();

	// Name cannot start/end with spaces
	if (utility::stringUtils::trim(buf) != name.getBuffer())
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

	m_fsPath = fsf->stringToPath(GET_PROPERTY(string, PROPERTY_SERVER_ROOTPATH));

	ref <utility::file> rootDir = fsf->create(m_fsPath);

	// Try to create the root directory if it does not exist
	if (!(rootDir->exists() && rootDir->isDirectory()))
	{
		try
		{
			rootDir->createDirectory();
		}
		catch (exceptions::filesystem_exception& e)
		{
			throw exceptions::connection_error("Cannot create root directory.", e);
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


const string maildirStore::_infos::getPropertyPrefix() const
{
	return "store.maildir.";
}


const maildirStore::_infos::props& maildirStore::_infos::getProperties() const
{
	static props p =
	{
		property(serviceInfos::property::SERVER_ROOTPATH, serviceInfos::property::FLAG_REQUIRED)
	};

	return p;
}


const std::vector <serviceInfos::property> maildirStore::_infos::getAvailableProperties() const
{
	std::vector <property> list;
	const props& p = getProperties();

	// Maildir-specific properties
	list.push_back(p.PROPERTY_SERVER_ROOTPATH);

	return (list);
}


} // maildir
} // net
} // vmime
