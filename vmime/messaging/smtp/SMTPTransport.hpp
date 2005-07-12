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

#ifndef VMIME_MESSAGING_SMTP_SMTPTRANSPORT_HPP_INCLUDED
#define VMIME_MESSAGING_SMTP_SMTPTRANSPORT_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/messaging/transport.hpp"
#include "vmime/messaging/socket.hpp"
#include "vmime/messaging/timeoutHandler.hpp"


namespace vmime {
namespace messaging {
namespace smtp {


/** SMTP transport service.
  */

class SMTPTransport : public transport
{
public:

	SMTPTransport(ref <session> sess, ref <authenticator> auth);
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

	static const int responseCode(const string& response);
	static const string responseText(const string& response);

	void sendRequest(const string& buffer, const bool end = true);

	void readResponse(string& buffer);

	void internalDisconnect();

	ref <socket> m_socket;
	bool m_authentified;
	bool m_extendedSMTP;

	ref <timeoutHandler> m_timeoutHandler;


	// Service infos
	class _infos : public serviceInfos
	{
	public:

		struct props
		{
			// SMTP-specific options
			serviceInfos::property PROPERTY_OPTIONS_NEEDAUTH;

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
} // messaging
} // vmime


#endif // VMIME_MESSAGING_SMTP_SMTPTRANSPORT_HPP_INCLUDED
