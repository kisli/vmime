//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_MESSAGING_SMTPTRANSPORT_HPP_INCLUDED
#define VMIME_MESSAGING_SMTPTRANSPORT_HPP_INCLUDED


#include "transport.hpp"
#include "socket.hpp"
#include "../config.hpp"
#include "timeoutHandler.hpp"


namespace vmime {
namespace messaging {


/** SMTP transport service.
  */

class SMTPTransport : public transport
{
public:

	SMTPTransport(class session& sess, class authenticator* auth);
	~SMTPTransport();

	const string protocolName() const;

	static const serviceInfos& infosInstance() { return (sm_infos); }
	const serviceInfos& infos() const { return (sm_infos); }

	void connect();
	const bool isConnected() const;
	void disconnect();

	void noop();

	void send(vmime::message* msg, progressionListener* progress = NULL);
	void send(const mailbox& expeditor, const mailboxList& recipients, utility::inputStream& is, const utility::stream::size_type size, progressionListener* progress = NULL);

private:

	static const int responseCode(const string& response);
	static const string responseText(const string& response);

	void sendRequest(const string& buffer, const bool end = true);

	void readResponse(string& buffer);

	void internalDisconnect();

	socket* m_socket;
	bool m_authentified;
	bool m_extendedSMTP;

	timeoutHandler* m_timeoutHandler;


	// Service infos
	class _infos : public serviceInfos
	{
	public:

		const port_t defaultPort() const;

		const string propertyPrefix() const;
		const std::vector <string> availableProperties() const;
	};

	static _infos sm_infos;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_SMTPTRANSPORT_HPP_INCLUDED
