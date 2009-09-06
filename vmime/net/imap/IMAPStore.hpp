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

#ifndef VMIME_NET_IMAP_IMAPSTORE_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPSTORE_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/net/store.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/folder.hpp"

#include "vmime/net/imap/IMAPServiceInfos.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPParser;
class IMAPTag;
class IMAPFolder;


/** IMAP store service.
  */

class IMAPStore : public store
{
	friend class IMAPFolder;
	friend class IMAPMessage;
	friend class IMAPConnection;

public:

	IMAPStore(ref <session> sess, ref <security::authenticator> auth, const bool secured = false);
	~IMAPStore();

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

	int getCapabilities() const;

	bool isIMAPS() const;

	bool isSecuredConnection() const;
	ref <connectionInfos> getConnectionInfos() const;

protected:

	// Connection
	ref <IMAPConnection> m_connection;



	ref <IMAPConnection> connection();


	void registerFolder(IMAPFolder* folder);
	void unregisterFolder(IMAPFolder* folder);

	std::list <IMAPFolder*> m_folders;

	const bool m_isIMAPS;  // Use IMAPS


	static IMAPServiceInfos sm_infos;
};


} // imap
} // net
} // vmime


#endif // VMIME_NET_IMAP_IMAPSTORE_HPP_INCLUDED
