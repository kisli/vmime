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

#ifndef VMIME_NET_POP3_POP3CONNECTION_HPP_INCLUDED
#define VMIME_NET_POP3_POP3CONNECTION_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/messageId.hpp"

#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"
#include "vmime/net/session.hpp"
#include "vmime/net/connectionInfos.hpp"

#include "vmime/net/pop3/POP3Command.hpp"
#include "vmime/net/pop3/POP3Response.hpp"

#include "vmime/security/authenticator.hpp"


namespace vmime {
namespace net {


class socket;
class timeoutHandler;


namespace pop3 {


class POP3Store;


/** Manage connection to a POP3 server.
  */
class VMIME_EXPORT POP3Connection : public object
{
	friend class vmime::creator;

public:

	POP3Connection(ref <POP3Store> store, ref <security::authenticator> auth);
	virtual ~POP3Connection();


	virtual void connect();
	virtual bool isConnected() const;
	virtual void disconnect();

	bool isSecuredConnection() const;
	ref <connectionInfos> getConnectionInfos() const;

	virtual ref <POP3Store> getStore();
	virtual ref <socket> getSocket();
	virtual ref <timeoutHandler> getTimeoutHandler();
	virtual ref <security::authenticator> getAuthenticator();
	virtual ref <session> getSession();

private:

	void authenticate(const messageId& randomMID);
#if VMIME_HAVE_SASL_SUPPORT
	void authenticateSASL();
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	void startTLS();
#endif // VMIME_HAVE_TLS_SUPPORT

	const std::vector <string> getCapabilities();

	void internalDisconnect();


	weak_ref <POP3Store> m_store;

	ref <security::authenticator> m_auth;
	ref <socket> m_socket;
	ref <timeoutHandler> m_timeoutHandler;

	bool m_authenticated;
	bool m_secured;

	ref <connectionInfos> m_cntInfos;
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_POP3_POP3CONNECTION_HPP_INCLUDED
