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

#include "maildirStore.hpp"

#include "maildirFolder.hpp"

#include "../exception.hpp"
#include "../platformDependant.hpp"


namespace vmime {
namespace messaging {


maildirStore::maildirStore(class session& sess, class authenticator* auth)
	: store(sess, infosInstance(), auth), m_connected(false)
{
}


maildirStore::~maildirStore()
{
	if (isConnected())
		disconnect();
}


const string maildirStore::protocolName() const
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


void maildirStore::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	m_fsPath = platformDependant::getHandler()->getFileSystemFactory()->
		stringToPath(session().properties()[infos().propertyPrefix() + "server.rootpath"]);

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




// Service infos

maildirStore::_infos maildirStore::sm_infos;


const port_t maildirStore::_infos::defaultPort() const
{
	return (0);
}


const string maildirStore::_infos::propertyPrefix() const
{
	return "store.maildir.";
}


const std::vector <string> maildirStore::_infos::availableProperties() const
{
	std::vector <string> list;

	list.push_back("server.rootpath");

	return (list);
}


} // messaging
} // vmime
