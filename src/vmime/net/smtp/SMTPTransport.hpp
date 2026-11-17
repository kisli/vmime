//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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
class VMIME_EXPORT SMTPTransport : public transport {

public:

	SMTPTransport(
		const shared_ptr <session>& sess,
		const shared_ptr <security::authenticator>& auth,
		const bool secured = false
	);

	~SMTPTransport();

	const string getProtocolName() const;

	static const serviceInfos& getInfosInstance();
	const serviceInfos& getInfos() const;

	void connect();
	bool isConnected() const;
	void disconnect();

	void noop();

	void send(
		const mailbox& expeditor,
		const mailboxList& recipients,
		utility::inputStream& is,
		const size_t size,
		utility::progressListener* progress = NULL,
		const mailbox& sender = mailbox(),
		const sendOptions& options = sendOptions()
	);

	void send(
		const shared_ptr <vmime::message>& msg,
		const mailbox& expeditor,
		const mailboxList& recipients,
		utility::progressListener* progress = NULL,
		const mailbox& sender = mailbox(),
		const sendOptions& options = sendOptions()
	);

	bool isSecuredConnection() const;
	shared_ptr <connectionInfos> getConnectionInfos() const;
	shared_ptr <SMTPConnection> getConnection();

	bool isSMTPS() const;

private:

	static bool mailboxNeedsUTF8(const mailbox& mb);
	static bool componentNeedsUTF8(component& comp);
	static bool componentHas8bitBody(component& comp);

	/** Check whether the SMTPUTF8 extension (RFC 6531) is needed to send
	  * a message, and ensure that it is supported by the server.
	  *
	  * @param expeditor expeditor mailbox
	  * @param recipients list of recipient mailboxes
	  * @param sender envelope sender (if empty, expeditor will be used)
	  * @param msg message to send, or NULL if only the envelope is known
	  * @param options send options
	  * @return true if the SMTPUTF8 extension must be used, false otherwise
	  * @throw SMTPUTF8ExtensionNotSupportedException if the SMTPUTF8
	  * extension is needed but not supported by the server (or if the
	  * server does not announce 8BITMIME along with it)
	  */
	bool needSMTPUTF8(
		const mailbox& expeditor,
		const mailboxList& recipients,
		const mailbox& sender,
		const shared_ptr <vmime::message>& msg,
		const sendOptions& options
	) const;

	/** Check whether a message must be declared as 8-bit with the
	  * BODY=8BITMIME parameter (RFC 6152), and ensure that the server
	  * supports it.
	  *
	  * @param utf8 true if the SMTPUTF8 extension is used
	  * @param has8bitData true if the message is known to contain 8-bit data
	  * @param options send options
	  * @return true if the message must be declared as 8-bit, false otherwise
	  * @throw SMTP8BITMIMEExtensionNotSupportedException if the message must
	  * be declared as 8-bit but the server does not support 8BITMIME
	  */
	bool need8bitMIME(
		const bool utf8,
		const bool has8bitData,
		const sendOptions& options
	) const;

	/** Send the MAIL and RCPT commands to the server, checking the
	  * response, and using pipelining if supported by the server.
	  * Optionally, the DATA command can also be sent.
	  *
	  * @param expeditor expeditor mailbox
	  * @param recipients list of recipient mailboxes
	  * @param sender envelope sender (if empty, expeditor will be used)
	  * @param utf8 if true, the SMTPUTF8 extension will be used
	  * @param eightBit if true, the message is declared as 8-bit
	  * @param sendDATACommand if true, the DATA command will be sent
	  * @param size message size, in bytes (or 0, if not known)
	  * @param dsnAttributes attributes for Delivery Status Notification (if needed)
	  */
	void sendEnvelope(
		const mailbox& expeditor,
		const mailboxList& recipients,
		const mailbox& sender,
		const bool utf8,
		const bool eightBit,
		bool sendDATACommand,
		const size_t size,
		const sendOptions& options = sendOptions()
	);

	/** Send the message data, after the DATA command has been accepted,
	  * and check the response.
	  *
	  * @param is input stream provider (message data)
	  * @param size size of the message data
	  * @param progress progress listener, or NULL if not used
	  * @param eightBit true if the message has been declared as 8-bit;
	  * otherwise, the data must not contain 8-bit data
	  * @param canonicalize if true, line endings are converted to CRLF
	  * (for raw message data, which may use another convention)
	  */
	void sendData(
		utility::inputStream& is,
		const size_t size,
		utility::progressListener* progress,
		const bool eightBit,
		const bool canonicalize
	);


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
