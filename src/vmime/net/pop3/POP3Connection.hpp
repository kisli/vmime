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
#include "vmime/net/tracer.hpp"

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
public:

	POP3Connection(shared_ptr <POP3Store> store, shared_ptr <security::authenticator> auth);
	virtual ~POP3Connection();


	virtual void connect();
	virtual bool isConnected() const;
	virtual void disconnect();

	bool isSecuredConnection() const;
	shared_ptr <connectionInfos> getConnectionInfos() const;

	virtual shared_ptr <POP3Store> getStore();
	virtual shared_ptr <socket> getSocket();
	virtual shared_ptr <timeoutHandler> getTimeoutHandler();
	virtual shared_ptr <security::authenticator> getAuthenticator();
	virtual shared_ptr <session> getSession();
	virtual shared_ptr <tracer> getTracer();

private:

	void authenticate(const messageId& randomMID);
#if VMIME_HAVE_SASL_SUPPORT
	void authenticateSASL();
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	void startTLS();
#endif // VMIME_HAVE_TLS_SUPPORT

	void fetchCapabilities();
	void invalidateCapabilities();
	const std::vector <string> getCapabilities();

	void internalDisconnect();


	weak_ptr <POP3Store> m_store;

	shared_ptr <security::authenticator> m_auth;
	shared_ptr <socket> m_socket;
	shared_ptr <timeoutHandler> m_timeoutHandler;
	shared_ptr <tracer> m_tracer;

	bool m_authenticated;
	bool m_secured;

	shared_ptr <connectionInfos> m_cntInfos;

	std::vector <string> m_capabilities;
	bool m_capabilitiesFetched;
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_POP3_POP3CONNECTION_HPP_INCLUDED
