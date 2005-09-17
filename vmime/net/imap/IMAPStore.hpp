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

#ifndef VMIME_NET_IMAP_IMAPSTORE_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPSTORE_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/net/store.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/folder.hpp"

#include <ostream>


namespace vmime {
namespace net {
namespace imap {


class IMAPParser;
class IMAPTag;
class IMAPConnection;
class IMAPFolder;


/** IMAP store service.
  */

class IMAPStore : public store
{
	friend class IMAPFolder;
	friend class IMAPMessage;
	friend class IMAPConnection;

public:

	IMAPStore(ref <session> sess, ref <security::authenticator> auth);
	~IMAPStore();

	const string getProtocolName() const;

	ref <folder> getDefaultFolder();
	ref <folder> getRootFolder();
	ref <folder> getFolder(const folder::path& path);

	const bool isValidFolderName(const folder::path::component& name) const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	const bool isConnected() const;
	void disconnect();

	void noop();

	const int getCapabilities() const;

private:

	// Connection
	ref <IMAPConnection> m_connection;



	ref <IMAPConnection> connection();


	void registerFolder(IMAPFolder* folder);
	void unregisterFolder(IMAPFolder* folder);

	std::list <IMAPFolder*> m_folders;



	// Service infos
	class _infos : public serviceInfos
	{
	public:

		struct props
		{
			// IMAP-specific options
#if VMIME_HAVE_SASL_SUPPORT
			serviceInfos::property PROPERTY_OPTIONS_SASL;
			serviceInfos::property PROPERTY_OPTIONS_SASL_FALLBACK;
#endif // VMIME_HAVE_SASL_SUPPORT

			// Common properties
			serviceInfos::property PROPERTY_AUTH_USERNAME;
			serviceInfos::property PROPERTY_AUTH_PASSWORD;

			serviceInfos::property PROPERTY_SERVER_ADDRESS;
			serviceInfos::property PROPERTY_SERVER_PORT;
			serviceInfos::property PROPERTY_SERVER_SOCKETFACTORY;

			serviceInfos::property PROPERTY_TIMEOUT_FACTORY;
		};

		const props& getProperties() const;

		const string getPropertyPrefix() const;
		const std::vector <serviceInfos::property> getAvailableProperties() const;
	};

	static _infos sm_infos;
};


} // imap
} // net
} // vmime


#endif // VMIME_NET_IMAP_IMAPSTORE_HPP_INCLUDED
