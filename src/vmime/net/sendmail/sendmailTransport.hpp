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

#ifndef VMIME_NET_SENDMAIL_SENDMAILTRANSPORT_HPP_INCLUDED
#define VMIME_NET_SENDMAIL_SENDMAILTRANSPORT_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SENDMAIL


#include "vmime/net/transport.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"

#include "vmime/net/sendmail/sendmailServiceInfos.hpp"


namespace vmime {
namespace net {
namespace sendmail {


/** Sendmail local transport service.
  */

class VMIME_EXPORT sendmailTransport : public transport
{
public:

	sendmailTransport(shared_ptr <session> sess, shared_ptr <security::authenticator> auth);
	~sendmailTransport();

	const string getProtocolName() const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	bool isConnected() const;
	void disconnect();

	void noop();

	void send
		(const mailbox& expeditor,
		 const mailboxList& recipients,
		 utility::inputStream& is,
		 const size_t size,
		 utility::progressListener* progress = NULL,
		 const mailbox& sender = mailbox());

	bool isSecuredConnection() const;
	shared_ptr <connectionInfos> getConnectionInfos() const;

private:

	void internalDisconnect();

	void internalSend(const std::vector <string> args, utility::inputStream& is,
		const size_t size, utility::progressListener* progress);


	string m_sendmailPath;

	bool m_connected;


	// Service infos
	static sendmailServiceInfos sm_infos;
};


} // sendmail
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SENDMAIL

#endif // VMIME_NET_SENDMAIL_SENDMAILTRANSPORT_HPP_INCLUDED
