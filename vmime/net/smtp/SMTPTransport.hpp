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

#ifndef VMIME_NET_SMTP_SMTPTRANSPORT_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPTRANSPORT_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/net/transport.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"

#include "vmime/net/smtp/SMTPServiceInfos.hpp"


namespace vmime {
namespace net {
namespace smtp {


class SMTPResponse;


/** SMTP transport service.
  */

class SMTPTransport : public transport
{
public:

	SMTPTransport(ref <session> sess, ref <security::authenticator> auth, const bool secured = false);
	~SMTPTransport();

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

	void sendRequest(const string& buffer, const bool end = true);
	ref <SMTPResponse> readResponse();

	void internalDisconnect();

	void helo();
	void authenticate();
#if VMIME_HAVE_SASL_SUPPORT
	void authenticateSASL();
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	void startTLS();
#endif // VMIME_HAVE_TLS_SUPPORT

	ref <socket> m_socket;
	bool m_authentified;

	bool m_extendedSMTP;
	std::map <string, std::vector <string> > m_extensions;

	ref <timeoutHandler> m_timeoutHandler;

	const bool m_isSMTPS;

	bool m_secured;
	ref <connectionInfos> m_cntInfos;


	// Service infos
	static SMTPServiceInfos sm_infos;
};


} // smtp
} // net
} // vmime


#endif // VMIME_NET_SMTP_SMTPTRANSPORT_HPP_INCLUDED
