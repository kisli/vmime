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


namespace vmime {
namespace net {
namespace smtp {


/** SMTP transport service.
  */

class SMTPTransport : public transport
{
public:

	SMTPTransport(ref <session> sess, ref <security::authenticator> auth);
	~SMTPTransport();

	const string getProtocolName() const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	const bool isConnected() const;
	void disconnect();

	void noop();

	void send(const mailbox& expeditor, const mailboxList& recipients, utility::inputStream& is, const utility::stream::size_type size, utility::progressionListener* progress = NULL);

private:

	static const int getResponseCode(const string& response);

	void sendRequest(const string& buffer, const bool end = true);

	const string readResponseLine();
	const int readResponse(string& text);
	const int readAllResponses(string& text, const bool allText = false);

	void internalDisconnect();

	void authenticate();
#if VMIME_HAVE_SASL_SUPPORT
	void authenticateSASL();
#endif // VMIME_HAVE_SASL_SUPPORT


	ref <socket> m_socket;
	bool m_authentified;

	bool m_extendedSMTP;
	string m_extendedSMTPResponse;

	string m_responseBuffer;
	bool m_responseContinues;

	ref <timeoutHandler> m_timeoutHandler;


	// Service infos
	class _infos : public serviceInfos
	{
	public:

		struct props
		{
			// SMTP-specific options
			serviceInfos::property PROPERTY_OPTIONS_NEEDAUTH;
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


} // smtp
} // net
} // vmime


#endif // VMIME_NET_SMTP_SMTPTRANSPORT_HPP_INCLUDED
