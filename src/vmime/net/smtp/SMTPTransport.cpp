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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/smtp/SMTPTransport.hpp"
#include "vmime/net/smtp/SMTPResponse.hpp"
#include "vmime/net/smtp/SMTPCommand.hpp"
#include "vmime/net/smtp/SMTPCommandSet.hpp"
#include "vmime/net/smtp/SMTPChunkingOutputStreamAdapter.hpp"
#include "vmime/net/smtp/SMTPExceptions.hpp"
#include "vmime/net/smtp/SMTPSendOptions.hpp"

#include "vmime/exception.hpp"
#include "vmime/mailboxList.hpp"
#include "vmime/message.hpp"

#include "vmime/utility/filteredStream.hpp"
#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/outputStreamSocketAdapter.hpp"
#include "vmime/utility/streamUtils.hpp"
#include "vmime/utility/outputStreamAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/details/classicOStringStream.hpp"


namespace vmime {
namespace net {
namespace smtp {


namespace {

	// Output stream which only lets 7-bit data through: 8-bit data which
	// has not been declared with BODY=8BITMIME must not be sent (RFC 6152,
	// section 3), so an exception is thrown before writing any of it
	class sevenBitOutputStream : public utility::outputStream {

	public:

		sevenBitOutputStream(utility::outputStream& os, const bool has8bitMIME)
			: m_os(os), m_has8bitMIME(has8bitMIME) {

		}

		void flush() {

			m_os.flush();
		}

	protected:

		void writeImpl(const byte_t* const data, const size_t count) {

			for (size_t i = 0 ; i < count ; ++i) {

				if (data[i] >= 0x80) {

					// The message can be sent again as 8-bit, if the
					// server supports it
					if (m_has8bitMIME) {
						throw SMTPUndeclared8bitDataException();
					} else {
						throw SMTP8BITMIMEExtensionNotSupportedException();
					}
				}
			}

			m_os.write(data, count);
		}

	private:

		utility::outputStream& m_os;
		const bool m_has8bitMIME;
	};

}


SMTPTransport::SMTPTransport(
	const shared_ptr <session>& sess,
	const shared_ptr <security::authenticator>& auth,
	const bool secured
)
	: transport(sess, getInfosInstance(), auth),
	  m_isSMTPS(secured),
	  m_needReset(false) {

}


SMTPTransport::~SMTPTransport() {

	try {

		if (isConnected()) {
			disconnect();
		}

	} catch (...) {

		// Don't throw in destructor
	}
}


const string SMTPTransport::getProtocolName() const {

	return "smtp";
}


bool SMTPTransport::isSMTPS() const {

	return m_isSMTPS;
}


void SMTPTransport::connect() {

	if (isConnected()) {
		throw exceptions::already_connected();
	}

	m_connection = make_shared <SMTPConnection>(
		dynamicCast <SMTPTransport>(shared_from_this()), getAuthenticator()
	);

	m_connection->connect();
}


bool SMTPTransport::isConnected() const {

	return m_connection && m_connection->isConnected();
}


bool SMTPTransport::isSecuredConnection() const {

	if (!m_connection) {
		return false;
	}

	return m_connection->isSecuredConnection();
}


shared_ptr <connectionInfos> SMTPTransport::getConnectionInfos() const {

	if (!m_connection) {
		return null;
	}

	return m_connection->getConnectionInfos();
}


shared_ptr <SMTPConnection> SMTPTransport::getConnection() {

	return m_connection;
}


void SMTPTransport::disconnect() {

	// Connection may have been lost (eg. after a network error):
	// this is not an error, but it still needs to be cleaned up
	const bool wasConnected = !!m_connection;

	if (m_connection) {

		if (m_connection->isConnected()) {
			m_connection->disconnect();
		}

		m_connection = null;
	}

	if (!wasConnected) {
		throw exceptions::not_connected();
	}
}


void SMTPTransport::noop() {

	if (!isConnected()) {
		throw exceptions::not_connected();
	}

	m_connection->sendRequest(SMTPCommand::NOOP());

	shared_ptr <SMTPResponse> resp = m_connection->readResponse();

	if (resp->getCode() != 250) {
		throw SMTPCommandError("NOOP", resp->getText(), resp->getCode(), resp->getEnhancedCode());
	}
}


// static
bool SMTPTransport::mailboxNeedsUTF8(const mailbox& mb) {

	// Only a non-ASCII local part needs SMTPUTF8: the domain part can be
	// written as IDNA A-labels, which are not "internationalized" (RFC 6531,
	// section 3.2), and the display name as RFC 2047 encoded-words
	return !utility::stringUtils::is7bit(mb.getEmail().getLocalName().getBuffer());
}


// static
bool SMTPTransport::componentNeedsUTF8(component& comp) {

	if (const mailbox* mb = dynamic_cast <const mailbox*>(&comp)) {
		return mailboxNeedsUTF8(*mb);
	}

	for (const auto& child : comp.getChildComponents()) {

		if (componentNeedsUTF8(*child)) {
			return true;
		}
	}

	return false;
}


bool SMTPTransport::needSMTPUTF8(
	const mailbox& expeditor,
	const mailboxList& recipients,
	const mailbox& sender,
	const shared_ptr <vmime::message>& msg,
	const sendOptions& options
) const {

	auto opts = dynamic_cast <const SMTPSendOptions*>(&options);

	// The caller may tell that the message has UTF-8 header fields, which
	// VMime cannot know when it sends raw message data
	bool need = (opts && opts->isInternationalizedMessage()) ||
		mailboxNeedsUTF8(sender.isEmpty() ? expeditor : sender);

	for (size_t i = 0 ; !need && i < recipients.getMailboxCount() ; ++i) {
		need = mailboxNeedsUTF8(*recipients.getMailboxAt(i));
	}

	// Addresses in the header fields of the message, at any level within
	// its MIME structure (RFC 6531, sections 3.2 and 3.6)
	if (!need && msg) {
		need = componentNeedsUTF8(*msg);
	}

	// An internationalized message must not be sent to a server which
	// does not support SMTPUTF8 (RFC 6531, section 3.5). A server which
	// supports it must also announce 8BITMIME (RFC 6531, section 3.1),
	// without which 8-bit data must not be sent (RFC 6152, section 3)
	if (need && (!m_connection->hasExtension("SMTPUTF8") ||
	             !m_connection->hasExtension("8BITMIME"))) {

		throw SMTPUTF8ExtensionNotSupportedException();
	}

	return need;
}


// static
bool SMTPTransport::componentHas8bitBody(component& comp) {

	if (const body* bdy = dynamic_cast <const body*>(&comp)) {

		if (bdy->getPartCount() == 0 && bdy->getEncoding() == encoding(encodingTypes::EIGHT_BIT)) {
			return true;
		}
	}

	for (const auto& child : comp.getChildComponents()) {

		if (componentHas8bitBody(*child)) {
			return true;
		}
	}

	return false;
}


bool SMTPTransport::need8bitMIME(
	const bool utf8,
	const bool has8bitData,
	const sendOptions& options
) const {

	// UTF-8 header fields are 8-bit data, and a server which supports
	// SMTPUTF8 also supports 8BITMIME (see needSMTPUTF8())
	if (utf8) {
		return true;
	}

	auto opts = dynamic_cast <const SMTPSendOptions*>(&options);

	const bool need = has8bitData || (opts && opts->is8bitMessage());

	// 8-bit data must not be sent to a server which does not support
	// 8BITMIME (RFC 6152, section 3)
	if (need && !m_connection->hasExtension("8BITMIME")) {
		throw SMTP8BITMIMEExtensionNotSupportedException();
	}

	return need;
}


void SMTPTransport::sendEnvelope(
	const mailbox& expeditor,
	const mailboxList& recipients,
	const mailbox& sender,
	const bool utf8,
	const bool eightBit,
	bool sendDATACommand,
	const size_t size,
	const sendOptions& options
) {

	auto opts = dynamic_cast <const SMTPSendOptions*>(&options);

	// If no recipient/expeditor was found, throw an exception
	if (recipients.isEmpty()) {
		throw exceptions::no_recipient();
	} else if (expeditor.isEmpty()) {
		throw exceptions::no_expeditor();
	}

	// If DSN extension is used, ensure it is supported by the server
	if (opts && opts->getDSNAttributes() && !m_connection->hasExtension("DSN")) {
		throw SMTPDSNExtensionNotSupportedException();
	}


	const bool needReset = m_needReset;
	const bool hasPipelining = m_connection->hasExtension("PIPELINING") &&
		getInfos().getPropertyValue <bool>(getSession(),
			dynamic_cast <const SMTPServiceInfos&>(getInfos()).getProperties().PROPERTY_OPTIONS_PIPELINING);

	shared_ptr <SMTPResponse> resp;
	shared_ptr <SMTPCommandSet> commands = SMTPCommandSet::create(hasPipelining);

	// Emit a "RSET" command if we previously sent a message on this connection
	if (needReset) {
		commands->addCommand(SMTPCommand::RSET());
	}

	// Emit the "MAIL" command
	const bool hasSize = m_connection->hasExtension("SIZE");

	if (!sender.isEmpty()) {

		commands->addCommand(
			SMTPCommand::MAIL(
				sender, utf8, eightBit, hasSize ? size : 0,
				opts ? opts->getDSNAttributes() : nullptr
			)
		);

	} else {

		commands->addCommand(
			SMTPCommand::MAIL(
				expeditor, utf8, eightBit, hasSize ? size : 0,
				opts ? opts->getDSNAttributes() : nullptr
			)
		);
	}

	// Now, we will need to reset next time
	m_needReset = true;

	// Emit a "RCPT TO" command for each recipient
	for (size_t i = 0 ; i < recipients.getMailboxCount() ; ++i) {

		const mailbox& mbox = *recipients.getMailboxAt(i);
		commands->addCommand(
			SMTPCommand::RCPT(
				mbox, utf8,
				opts ? opts->getDSNAttributes() : nullptr
			)
		);
	}

	// Prepare sending of message data
	if (sendDATACommand) {
		commands->addCommand(SMTPCommand::DATA());
	}

	// Read response for "RSET" command
	if (needReset) {

		m_connection->sendRequest(commands);

		resp = m_connection->readResponse();

		if (resp->getCode() != 250 &&
		    resp->getCode() != 200) {   // RFC-876: << In reply to a RSET and/or a NOOP command,
		                                //             some servers reply "200" >>

			disconnect();

			throw SMTPCommandError(
				commands->getLastCommandSent()->getText(), resp->getText(),
				resp->getCode(), resp->getEnhancedCode()
			);
		}
	}

	// Read response for "MAIL" command
	m_connection->sendRequest(commands);

	if ((resp = m_connection->readResponse())->getCode() != 250) {
		auto code = resp->getCode();

		// SIZE extension: insufficient system storage
		if (code == 452) {

			throw SMTPMessageSizeExceedsCurLimitsException(
				SMTPCommandError(
					commands->getLastCommandSent()->getText(), resp->getText(),
					code, resp->getEnhancedCode()
				)
			);

		// SIZE extension: message size exceeds fixed maximum message size
		} else if (code == 552) {

			throw SMTPMessageSizeExceedsMaxLimitsException(
				SMTPCommandError(
					commands->getLastCommandSent()->getText(), resp->getText(),
					code, resp->getEnhancedCode()
				 )
			);

		// Other error
		} else {

			throw SMTPCommandError(
				commands->getLastCommandSent()->getText(), resp->getText(),
				code, resp->getEnhancedCode()
			);
		}
	}

	// Read responses for "RCPT TO" commands
	for (size_t i = 0 ; i < recipients.getMailboxCount() ; ++i) {

		m_connection->sendRequest(commands);

		resp = m_connection->readResponse();
		auto code = resp->getCode();

		if (code != 250 && code != 251) {

			// SIZE extension: insufficient system storage
			if (code == 452) {

				throw SMTPMessageSizeExceedsCurLimitsException(
					SMTPCommandError(
						commands->getLastCommandSent()->getText(), resp->getText(),
						code, resp->getEnhancedCode()
					)
				);

			// SIZE extension: message size exceeds fixed maximum message size
			} else if (code == 552) {

				throw SMTPMessageSizeExceedsMaxLimitsException(
					SMTPCommandError(
						commands->getLastCommandSent()->getText(), resp->getText(),
						code, resp->getEnhancedCode()
					)
				);

			// Other error
			} else {

				throw SMTPCommandError(
					commands->getLastCommandSent()->getText(), resp->getText(),
					code, resp->getEnhancedCode()
				);
			}
		}
	}

	// Read response for "DATA" command
	if (sendDATACommand) {

		m_connection->sendRequest(commands);
		auto resp = m_connection->readResponse();
		auto code = resp->getCode();

		if (code != 354) {

			throw SMTPCommandError(
				commands->getLastCommandSent()->getText(), resp->getText(),
				code, resp->getEnhancedCode()
			);
		}
	}
}


void SMTPTransport::send(
	const mailbox& expeditor,
	const mailboxList& recipients,
	utility::inputStream& is,
	const size_t size,
	utility::progressListener* progress,
	const mailbox& sender,
	const sendOptions& options
) {

	if (!isConnected()) {
		throw exceptions::not_connected();
	}

	// The message data is not parsed: only the envelope and the options
	// tell whether SMTPUTF8 is needed
	const bool utf8 = needSMTPUTF8(expeditor, recipients, sender, nullptr, options);

	// Likewise, only the options tell whether the message is 8-bit;
	// otherwise, the data is checked while it is sent
	const bool eightBit = need8bitMIME(utf8, /* has8bitData */ false, options);

	// Send the data by chunks if possible: unlike with DATA, the transfer
	// can then be cancelled without closing the connection
	if (canUseChunking()) {

		sendEnvelope(expeditor, recipients, sender, utf8, eightBit, /* sendDATACommand */ false, size, options);

		sendChunks(
			[&is, size](utility::outputStream& os) {
				utility::bufferedStreamCopy(is, os, size, NULL);
			},
			size, progress, eightBit, /* canonicalize */ true
		);

	} else {

		sendEnvelope(expeditor, recipients, sender, utf8, eightBit, /* sendDATACommand */ true, size, options);

		sendData(is, size, progress, eightBit, /* canonicalize */ true);
	}
}


void SMTPTransport::sendData(
	utility::inputStream& is,
	const size_t size,
	utility::progressListener* progress,
	const bool eightBit,
	const bool canonicalize
) {

	// If an error occurs while sending the message data (network error, but
	// also eg. error while reading the input stream, or undeclared 8-bit
	// data), the server still expects the end of the data, so the connection
	// is not usable anymore: close it, without sending the end-of-data
	// delimiter (the incomplete message must not be delivered)
	try {

		// Send the message data
		// Stream copy with "\n." to "\n.." transformation
		utility::outputStreamSocketAdapter sos(*m_connection->getSocket());
		utility::dotFilteredOutputStream fos(sos);

		// Lines must end with CRLF (RFC 5321, section 2.3.8)
		utility::LFToCRLFFilteredOutputStream crlfStream(fos);

		utility::outputStream& dataStream =
			canonicalize ? static_cast <utility::outputStream&>(crlfStream) : fos;

		if (eightBit) {

			utility::bufferedStreamCopy(is, dataStream, size, progress);

		} else {

			sevenBitOutputStream sevenBitStream(dataStream, m_connection->hasExtension("8BITMIME"));
			utility::bufferedStreamCopy(is, sevenBitStream, size, progress);
		}

		fos.flush();

		// Send end-of-data delimiter
		m_connection->getSocket()->send("\r\n.\r\n");

	} catch (...) {

		m_connection->closeConnection();
		throw;
	}

	if (m_connection->getTracer()) {
		m_connection->getTracer()->traceSendBytes(size);
		m_connection->getTracer()->traceSend(".");
	}

	shared_ptr <SMTPResponse> resp = m_connection->readResponse();
	auto code = resp->getCode();

	if (code != 250) {
		throw SMTPCommandError("DATA", resp->getText(), code, resp->getEnhancedCode());
	}
}


void SMTPTransport::send(
	const shared_ptr <vmime::message>& msg,
	const mailbox& expeditor,
	const mailboxList& recipients,
	utility::progressListener* progress,
	const mailbox& sender,
	const sendOptions& options
) {

	if (!isConnected()) {
		throw exceptions::not_connected();
	}

	// Generate the message with Internationalized Email support (RFC 6532)
	// only if it is sent with SMTPUTF8, which is required for UTF-8 header
	// fields (RFC 6531, section 3.4); otherwise, non-ASCII text is encoded
	const bool utf8 = needSMTPUTF8(expeditor, recipients, sender, msg, options);

	generationContext ctx(generationContext::getDefaultContext());
	ctx.setInternationalizedEmailSupport(utf8);

	// If CHUNKING is not supported, generate the message to a temporary
	// buffer then send it after the DATA command
	if (!canUseChunking()) {

		utility::details::classicOStringStream oss;
		utility::outputStreamAdapter ossAdapter(oss);

		msg->generate(ctx, ossAdapter);

		const string& str(oss.str());

		// The whole message is known: it is declared as 8-bit only if it
		// actually contains 8-bit data
		const bool eightBit = need8bitMIME(
			utf8,
			utility::stringUtils::findFirstNonASCIIchar(str.begin(), str.end()) != string::npos,
			options
		);

		utility::inputStreamStringAdapter isAdapter(str);

		sendEnvelope(expeditor, recipients, sender, utf8, eightBit, /* sendDATACommand */ true, str.length(), options);

		// The generated message already has canonical line endings, and
		// binary data must not be altered
		sendData(isAdapter, str.length(), progress, eightBit, /* canonicalize */ false);
		return;
	}

	// The message is generated while it is sent: it is declared as 8-bit
	// if one of its parts is declared as such; otherwise, the data is
	// checked while it is sent
	const bool eightBit = need8bitMIME(utf8, componentHas8bitBody(*msg), options);

	// Send message envelope
	const size_t msgSize = msg->getGeneratedSize(ctx);

	sendEnvelope(expeditor, recipients, sender, utf8, eightBit, /* sendDATACommand */ false, msgSize, options);

	// The generated message already has canonical line endings, and
	// binary data must not be altered
	sendChunks(
		[&msg, &ctx](utility::outputStream& os) {
			msg->generate(ctx, os);
		},
		msgSize, progress, eightBit, /* canonicalize */ false
	);
}


bool SMTPTransport::canUseChunking() {

	return m_connection->hasExtension("CHUNKING") &&
		getInfos().getPropertyValue <bool>(getSession(),
			dynamic_cast <const SMTPServiceInfos&>(getInfos()).getProperties().PROPERTY_OPTIONS_CHUNKING);
}


void SMTPTransport::sendChunks(
	const std::function <void (utility::outputStream&)>& writeData,
	const size_t size,
	utility::progressListener* progress,
	const bool eightBit,
	const bool canonicalize
) {

	SMTPChunkingOutputStreamAdapter chunkStream(m_connection, size, progress);

	// If an error occurs while sending the message chunks (network error, but
	// also eg. error while generating the message), the server still expects
	// the next chunks, so the connection is not usable anymore: close it,
	// without sending the last chunk (the incomplete message must not be
	// delivered)
	try {

		// Lines must end with CRLF (RFC 5321, section 2.3.8)
		utility::LFToCRLFFilteredOutputStream crlfStream(chunkStream);

		utility::outputStream& dataStream =
			canonicalize ? static_cast <utility::outputStream&>(crlfStream) : chunkStream;

		// Send the message by chunks
		if (eightBit) {

			writeData(dataStream);

		} else {

			sevenBitOutputStream sevenBitStream(dataStream, m_connection->hasExtension("8BITMIME"));
			writeData(sevenBitStream);
		}

		chunkStream.flush();

	} catch (SMTPUndeclared8bitDataException&) {

		// The chunk containing the 8-bit data has not been sent: cancel the
		// transfer, so that the connection remains usable
		chunkStream.cancel();
		m_needReset = false;  // already reset
		throw;

	} catch (SMTP8BITMIMEExtensionNotSupportedException&) {

		// Likewise, for 8-bit data the server does not support
		chunkStream.cancel();
		m_needReset = false;  // already reset
		throw;

	} catch (...) {

		// Connection may have been closed already, if the server rejected a chunk
		if (m_connection) {
			m_connection->closeConnection();
		}

		throw;
	}
}



// Service infos

SMTPServiceInfos SMTPTransport::sm_infos(false);


const serviceInfos& SMTPTransport::getInfosInstance() {

	return sm_infos;
}


const serviceInfos& SMTPTransport::getInfos() const {

	return sm_infos;
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

