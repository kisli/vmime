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

#ifndef VMIME_NET_SMTP_SMTPCONNECTION_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPCONNECTION_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/messageId.hpp"

#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"
#include "vmime/net/session.hpp"
#include "vmime/net/connectionInfos.hpp"

#include "vmime/net/smtp/SMTPCommand.hpp"
#include "vmime/net/smtp/SMTPResponse.hpp"

#include "vmime/security/authenticator.hpp"


namespace vmime {
namespace net {


class socket;
class timeoutHandler;


namespace smtp {


class SMTPTransport;


/** Manage connection to a SMTP server.
  */
class VMIME_EXPORT SMTPConnection : public object
{
	friend class vmime::creator;

public:

	SMTPConnection(ref <SMTPTransport> transport, ref <security::authenticator> auth);
	virtual ~SMTPConnection();


	virtual void connect();
	virtual bool isConnected() const;
	virtual void disconnect();

	bool isSecuredConnection() const;
	ref <connectionInfos> getConnectionInfos() const;

	virtual ref <SMTPTransport> getTransport();
	virtual ref <socket> getSocket();
	virtual ref <timeoutHandler> getTimeoutHandler();
	virtual ref <security::authenticator> getAuthenticator();
	virtual ref <session> getSession();

	void sendRequest(ref <SMTPCommand> cmd);
	ref <SMTPResponse> readResponse();

	bool hasExtension(const std::string& extName, std::vector <string>* params = NULL) const;

private:

	void internalDisconnect();

	void helo();
	void authenticate();
#if VMIME_HAVE_SASL_SUPPORT
	void authenticateSASL();
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	void startTLS();
#endif // VMIME_HAVE_TLS_SUPPORT


	weak_ref <SMTPTransport> m_transport;

	ref <security::authenticator> m_auth;
	ref <socket> m_socket;
	ref <timeoutHandler> m_timeoutHandler;

	SMTPResponse::state m_responseState;

	bool m_authenticated;
	bool m_secured;

	ref <connectionInfos> m_cntInfos;

	bool m_extendedSMTP;
	std::map <string, std::vector <string> > m_extensions;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPCONNECTION_HPP_INCLUDED
