//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_NET_MAILDIR_MAILDIRSTORE_HPP_INCLUDED
#define VMIME_NET_MAILDIR_MAILDIRSTORE_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/net/store.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/folder.hpp"

#include "vmime/net/maildir/maildirFormat.hpp"
#include "vmime/net/maildir/maildirServiceInfos.hpp"

#include "vmime/utility/file.hpp"

#include <ostream>


namespace vmime {
namespace net {
namespace maildir {


class maildirFolder;


/** maildir store service.
  */

class maildirStore : public store
{
	friend class maildirFolder;

public:

	maildirStore(ref <session> sess, ref <security::authenticator> auth);
	~maildirStore();

	const string getProtocolName() const;

	ref <folder> getDefaultFolder();
	ref <folder> getRootFolder();
	ref <folder> getFolder(const folder::path& path);

	bool isValidFolderName(const folder::path::component& name) const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	bool isConnected() const;
	void disconnect();

	void noop();

	const utility::path& getFileSystemPath() const;

	int getCapabilities() const;

	bool isSecuredConnection() const;
	ref <connectionInfos> getConnectionInfos() const;

	ref <maildirFormat> getFormat();
	ref <const maildirFormat> getFormat() const;

private:

	void registerFolder(maildirFolder* folder);
	void unregisterFolder(maildirFolder* folder);


	std::list <maildirFolder*> m_folders;

	ref <maildirFormat> m_format;

	bool m_connected;

	utility::path m_fsPath;


	// Service infos
	static maildirServiceInfos sm_infos;
};


} // maildir
} // net
} // vmime


#endif // VMIME_NET_MAILDIR_MAILDIRSTORE_HPP_INCLUDED
