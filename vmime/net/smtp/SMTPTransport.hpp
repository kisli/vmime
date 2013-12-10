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

#ifndef VMIME_NET_SMTP_SMTPTRANSPORT_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPTRANSPORT_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/transport.hpp"
#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"

#include "vmime/net/smtp/SMTPServiceInfos.hpp"
#include "vmime/net/smtp/SMTPConnection.hpp"


namespace vmime {
namespace net {
namespace smtp {


class SMTPCommand;


/** SMTP transport service.
  */

class VMIME_EXPORT SMTPTransport : public transport
{
public:

	SMTPTransport(shared_ptr <session> sess, shared_ptr <security::authenticator> auth, const bool secured = false);
	~SMTPTransport();

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

	void send
		(shared_ptr <vmime::message> msg,
		 const mailbox& expeditor,
		 const mailboxList& recipients,
		 utility::progressListener* progress = NULL,
		 const mailbox& sender = mailbox());

	bool isSecuredConnection() const;
	shared_ptr <connectionInfos> getConnectionInfos() const;
	shared_ptr <SMTPConnection> getConnection();

	bool isSMTPS() const;

private:

	/** Send the MAIL and RCPT commands to the server, checking the
	  * response, and using pipelining if supported by the server.
	  * Optionally, the DATA command can also be sent.
	  *
	  * @param expeditor expeditor mailbox
	  * @param recipients list of recipient mailboxes
	  * @param sender envelope sender (if empty, expeditor will be used)
	  * @param sendDATACommand if true, the DATA command will be sent
	  * @param size message size, in bytes (or 0, if not known)
	  */
	void sendEnvelope
		(const mailbox& expeditor,
		 const mailboxList& recipients,
		 const mailbox& sender,
		 bool sendDATACommand,
		 const size_t size);


	shared_ptr <SMTPConnection> m_connection;


	const bool m_isSMTPS;

	bool m_needReset;

	// Service infos
	static SMTPServiceInfos sm_infos;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPTRANSPORT_HPP_INCLUDED
