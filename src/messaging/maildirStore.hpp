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

#ifndef VMIME_MESSAGING_MAILDIRSTORE_HPP_INCLUDED
#define VMIME_MESSAGING_MAILDIRSTORE_HPP_INCLUDED


#include "config.hpp"

#include "messaging/store.hpp"
#include "messaging/socket.hpp"
#include "messaging/folder.hpp"

#include "utility/file.hpp"

#include <ostream>


namespace vmime {
namespace messaging {


class maildirFolder;


/** maildir store service.
  */

class maildirStore : public store
{
	friend class maildirFolder;

public:

	maildirStore(session* sess, authenticator* auth);
	~maildirStore();

	const string getProtocolName() const;

	folder* getDefaultFolder();
	folder* getRootFolder();
	folder* getFolder(const folder::path& path);

	const bool isValidFolderName(const folder::path::component& name) const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	const bool isConnected() const;
	void disconnect();

	void noop();

	const utility::path& getFileSystemPath() const;

	const int getCapabilities() const;

private:

	void registerFolder(maildirFolder* folder);
	void unregisterFolder(maildirFolder* folder);


	std::list <maildirFolder*> m_folders;

	bool m_connected;

	utility::path m_fsPath;


	// Service infos
	class _infos : public serviceInfos
	{
	public:

		const port_t getDefaultPort() const;

		const string getPropertyPrefix() const;
		const std::vector <string> getAvailableProperties() const;
	};

	static _infos sm_infos;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_MAILDIRSTORE_HPP_INCLUDED
