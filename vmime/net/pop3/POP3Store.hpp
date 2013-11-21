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

#ifndef VMIME_NET_POP3_POP3STORE_HPP_INCLUDED
#define VMIME_NET_POP3_POP3STORE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/store.hpp"

#include "vmime/net/pop3/POP3ServiceInfos.hpp"
#include "vmime/net/pop3/POP3Connection.hpp"

#include "vmime/utility/stream.hpp"


namespace vmime {
namespace net {
namespace pop3 {


class POP3Folder;
class POP3Command;
class POP3Response;


/** POP3 store service.
  */

class VMIME_EXPORT POP3Store : public store
{
	friend class POP3Folder;
	friend class POP3Message;

public:

	POP3Store(shared_ptr <session> sess, shared_ptr <security::authenticator> auth, const bool secured = false);
	~POP3Store();

	const string getProtocolName() const;

	shared_ptr <folder> getDefaultFolder();
	shared_ptr <folder> getRootFolder();
	shared_ptr <folder> getFolder(const folder::path& path);

	bool isValidFolderName(const folder::path::component& name) const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	bool isConnected() const;
	void disconnect();

	void noop();

	int getCapabilities() const;

	bool isSecuredConnection() const;
	shared_ptr <connectionInfos> getConnectionInfos() const;
	shared_ptr <POP3Connection> getConnection();

	bool isPOP3S() const;

private:

	shared_ptr <POP3Connection> m_connection;


	void registerFolder(POP3Folder* folder);
	void unregisterFolder(POP3Folder* folder);

	std::list <POP3Folder*> m_folders;


	const bool m_isPOP3S;


	// Service infos
	static POP3ServiceInfos sm_infos;
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_POP3_POP3STORE_HPP_INCLUDED
