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

#ifndef VMIME_NET_SENDMAIL_SENDMAILTRANSPORT_HPP_INCLUDED
#define VMIME_NET_SENDMAIL_SENDMAILTRANSPORT_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/net/transport.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"

#include "vmime/net/sendmail/sendmailServiceInfos.hpp"


#if VMIME_BUILTIN_PLATFORM_POSIX


namespace vmime {
namespace net {
namespace sendmail {


/** Sendmail local transport service.
  */

class sendmailTransport : public transport
{
public:

	sendmailTransport(ref <session> sess, ref <security::authenticator> auth);
	~sendmailTransport();

	const string getProtocolName() const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	bool isConnected() const;
	void disconnect();

	void noop();

	void send(const mailbox& expeditor, const mailboxList& recipients, utility::inputStream& is, const utility::stream::size_type size, utility::progressListener* progress = NULL);

	bool isSecuredConnection() const;
	ref <connectionInfos> getConnectionInfos() const;

private:

	void internalDisconnect();

	void internalSend(const std::vector <string> args, utility::inputStream& is,
		const utility::stream::size_type size, utility::progressListener* progress);


	string m_sendmailPath;

	bool m_connected;


	// Service infos
	static sendmailServiceInfos sm_infos;
};


} // sendmail
} // net
} // vmime


#endif // VMIME_BUILTIN_PLATFORM_POSIX


#endif // VMIME_NET_SENDMAIL_SENDMAILTRANSPORT_HPP_INCLUDED
