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


#include "vmime/net/maildir/maildirStore.hpp"

#include "vmime/net/maildir/maildirFolder.hpp"
#include "vmime/net/maildir/maildirFormat.hpp"

#include "vmime/exception.hpp"
#include "vmime/platform.hpp"

#include "vmime/net/defaultConnectionInfos.hpp"


// Helpers for service properties
#define GET_PROPERTY(type, prop) \
	(getInfos().getPropertyValue <type>(getSession(), \
		dynamic_cast <const maildirServiceInfos&>(getInfos()).getProperties().prop))
#define HAS_PROPERTY(prop) \
	(getInfos().hasProperty(getSession(), \
		dynamic_cast <const maildirServiceInfos&>(getInfos()).getProperties().prop))


namespace vmime {
namespace net {
namespace maildir {


maildirStore::maildirStore(shared_ptr <session> sess, shared_ptr <security::authenticator> auth)
	: store(sess, getInfosInstance(), auth), m_connected(false)
{
}


maildirStore::~maildirStore()
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


const string maildirStore::getProtocolName() const
{
	return "maildir";
}


shared_ptr <folder> maildirStore::getRootFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return make_shared <maildirFolder>
		(folder::path(),
		 dynamicCast <maildirStore>(shared_from_this()));
}


shared_ptr <folder> maildirStore::getDefaultFolder()
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return make_shared <maildirFolder>
		(folder::path::component("inbox"),
		 dynamicCast <maildirStore>(shared_from_this()));
}


shared_ptr <folder> maildirStore::getFolder(const folder::path& path)
{
	if (!isConnected())
		throw exceptions::illegal_state("Not connected");

	return make_shared <maildirFolder>
		(path, dynamicCast <maildirStore>(shared_from_this()));
}


bool maildirStore::isValidFolderName(const folder::path::component& name) const
{
	if (!platform::getHandler()->getFileSystemFactory()->isValidPathComponent(name))
		return false;

	const string& buf = name.getBuffer();

	// Name cannot start/end with spaces
	if (utility::stringUtils::trim(buf) != buf)
		return false;

	// Name cannot start with '.'
	const size_t length = buf.length();
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
	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	m_fsPath = fsf->stringToPath(GET_PROPERTY(string, PROPERTY_SERVER_ROOTPATH));

	shared_ptr <utility::file> rootDir = fsf->create(m_fsPath);

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

	m_format = maildirFormat::detect(dynamicCast <maildirStore>(shared_from_this()));

	m_connected = true;
}


bool maildirStore::isConnected() const
{
	return (m_connected);
}


bool maildirStore::isSecuredConnection() const
{
	return false;
}


shared_ptr <connectionInfos> maildirStore::getConnectionInfos() const
{
	return make_shared <defaultConnectionInfos>("localhost", static_cast <port_t>(0));
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


shared_ptr <maildirFormat> maildirStore::getFormat()
{
	return m_format;
}


shared_ptr <const maildirFormat> maildirStore::getFormat() const
{
	return m_format;
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


int maildirStore::getCapabilities() const
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

maildirServiceInfos maildirStore::sm_infos;


const serviceInfos& maildirStore::getInfosInstance()
{
	return sm_infos;
}


const serviceInfos& maildirStore::getInfos() const
{
	return sm_infos;
}


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

