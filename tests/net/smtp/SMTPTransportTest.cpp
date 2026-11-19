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

#include "tests/testUtils.hpp"

#include "vmime/net/smtp/SMTPTransport.hpp"
#include "vmime/net/smtp/SMTPChunkingOutputStreamAdapter.hpp"
#include "vmime/net/smtp/SMTPExceptions.hpp"
#include "vmime/net/smtp/SMTPSendOptions.hpp"

#include "SMTPTransportTestUtils.hpp"

#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/SASLMechanismFactory.hpp"
#include "vmime/security/sasl/OAuth2SASLAuthenticator.hpp"
#include "vmime/security/sasl/OAuthBearerSASLMechanism.hpp"


/** SMTP test server which simulates a network failure.
  *
  * When the client sends the command set with failOn(), the server
  * stops responding and any further read on the socket throws an
  * operation_timed_out exception, as a real socket would. Depending
  * on the failure mode, the connection may also be lost after the
  * command has been processed (eg. while the client is sending the
  * message data), in which case writing to the socket fails.
  *
  * The number of sockets currently connected is tracked, to check
  * that all connections are properly closed by the client, as well
  * as the number of messages accepted for delivery.
  */
class failingSMTPTestSocket : public lineBasedTestSocket {

public:

	enum FailureMode {
		FAILURE_NO_RESPONSE,        /**< Server stops responding (reads time out). */
		FAILURE_CONNECTION_LOST     /**< Connection is lost after the command (writes fail). */
	};

	using lineBasedTestSocket::send;

	failingSMTPTestSocket()
		: m_failed(false),
		  m_data(false) {

	}

	static void reset() {

		sm_failCommand.clear();
		sm_failureMode = FAILURE_NO_RESPONSE;
		sm_connectedCount = 0;
		sm_deliveredCount = 0;
	}

	static void failOn(const vmime::string& verb, const FailureMode mode = FAILURE_NO_RESPONSE) {

		sm_failCommand = verb;
		sm_failureMode = mode;
	}

	static int getConnectedCount() {

		return sm_connectedCount;
	}

	static int getDeliveredCount() {

		return sm_deliveredCount;
	}

	void connect(const vmime::string& address, const vmime::port_t port) {

		++sm_connectedCount;

		lineBasedTestSocket::connect(address, port);
	}

	void disconnect() {

		if (isConnected()) {
			--sm_connectedCount;
		}

		lineBasedTestSocket::disconnect();
	}

	void send(const vmime::string& buffer) {

		if (m_failed && sm_failureMode == FAILURE_CONNECTION_LOST) {
			throw vmime::exceptions::socket_exception("Connection lost");
		}

		lineBasedTestSocket::send(buffer);
	}

	void receive(vmime::string& buffer) {

		lineBasedTestSocket::receive(buffer);

		if (buffer.empty() && m_failed) {
			throw vmime::exceptions::operation_timed_out();
		}
	}

	void onConnected() {

		localSend("220 test.vmime.org ESMTP ready\r\n");
	}

	void processCommand() {

		if (!haveMoreLines()) {
			return;
		}

		const vmime::string line = getNextLine();

		if (m_failed) {

			// Server does not respond anymore

		} else if (m_data) {

			if (line == ".") {

				localSend("250 Message accepted for delivery\r\n");
				m_data = false;

				++sm_deliveredCount;
			}

		} else {

			const vmime::string verb =
				vmime::utility::stringUtils::toUpper(line.substr(0, line.find(' ')));

			if (verb == sm_failCommand && sm_failureMode == FAILURE_NO_RESPONSE) {

				m_failed = true;

			} else {

				processVerb(verb);

				if (verb == sm_failCommand) {
					m_failed = true;
				}
			}
		}

		processCommand();
	}

private:

	void processVerb(const vmime::string& verb) {

		if (verb == "EHLO") {

			localSend("250-test.vmime.org\r\n");
			localSend("250 CHUNKING\r\n");

		} else if (verb == "MAIL" || verb == "RCPT" || verb == "NOOP" || verb == "RSET") {

			localSend("250 OK\r\n");

		} else if (verb == "DATA") {

			localSend("354 Ready to accept data; end with <CRLF>.<CRLF>\r\n");
			m_data = true;

		} else if (verb == "BDAT") {

			// Message data follows (only used to simulate a failure:
			// data is not handled by this server)

		} else if (verb == "QUIT") {

			localSend("221 test.vmime.org Service closing transmission channel\r\n");

		} else {

			localSend("502 Command not implemented\r\n");
		}
	}


	bool m_failed;
	bool m_data;

	static vmime::string sm_failCommand;
	static FailureMode sm_failureMode;
	static int sm_connectedCount;
	static int sm_deliveredCount;
};


vmime::string failingSMTPTestSocket::sm_failCommand;
failingSMTPTestSocket::FailureMode failingSMTPTestSocket::sm_failureMode =
	failingSMTPTestSocket::FAILURE_NO_RESPONSE;
int failingSMTPTestSocket::sm_connectedCount = 0;
int failingSMTPTestSocket::sm_deliveredCount = 0;


/** Input stream which can only be read once (eg. from a socket).
  */
class readOnceInputStream : public vmime::utility::inputStream {

public:

	readOnceInputStream(const vmime::string& data)
		: m_data(data), m_pos(0) {

	}

	bool eof() const {

		return m_pos >= m_data.length();
	}

	void reset() {

		// Cannot be read again
	}

	size_t read(vmime::byte_t* const data, const size_t count) {

		const size_t n = std::min(count, m_data.length() - m_pos);

		std::copy(m_data.begin() + m_pos, m_data.begin() + m_pos + n, data);
		m_pos += n;

		return n;
	}

	size_t skip(const size_t count) {

		const size_t n = std::min(count, m_data.length() - m_pos);

		m_pos += n;

		return n;
	}

private:

	const vmime::string m_data;
	size_t m_pos;
};


/** Message which fails to be generated (eg. read error on an attachment).
  */
class failingMessage : public vmime::message {

public:

	void generateImpl(
		const vmime::generationContext& /* ctx */,
		vmime::utility::outputStream& outputStream,
		const size_t /* curLinePos */ = 0,
		size_t* /* newLinePos */ = NULL
	) const {

		outputStream.write("Message data\r\n", 14);

		throw vmime::exception("Generation error");
	}
};



/** SMTP test server which supports SASL authentication with the PLAIN
  * mechanism.
  *
  * The lines sent by the client during authentication are recorded.
  */
class SASLSMTPTestSocket : public lineBasedTestSocket {

public:

	SASLSMTPTestSocket()
		: m_authenticating(false) {

	}

	static void reset() {

		sm_authLines.clear();
	}

	static const std::vector <vmime::string>& getAuthLines() {

		return sm_authLines;
	}

	void onConnected() {

		localSend("220 test.vmime.org Service ready\r\n");
	}

	void processCommand() {

		if (!haveMoreLines()) {
			return;
		}

		const vmime::string line = getNextLine();

		if (m_authenticating) {

			// Client response to the (empty) server challenge
			sm_authLines.push_back(line);
			localSend("235 2.7.0 Authentication successful\r\n");

			m_authenticating = false;

			processCommand();
			return;
		}

		const vmime::string verb =
			vmime::utility::stringUtils::toUpper(line.substr(0, line.find(' ')));

		if (verb == "EHLO") {

			localSend("250-test.vmime.org\r\n");
			localSend("250 AUTH PLAIN\r\n");

		} else if (verb == "AUTH") {

			sm_authLines.push_back(line);

			if (line == "AUTH PLAIN") {

				// No initial response
				localSend("334 \r\n");

				m_authenticating = true;

			} else {

				localSend("235 2.7.0 Authentication successful\r\n");
			}

		} else if (verb == "QUIT") {

			localSend("221 test.vmime.org Service closing transmission channel\r\n");

		} else {

			localSend("502 Command not implemented\r\n");
		}

		processCommand();
	}

private:

	bool m_authenticating;

	static std::vector <vmime::string> sm_authLines;
};


std::vector <vmime::string> SASLSMTPTestSocket::sm_authLines;



/** SMTP test server which supports SASL authentication with the LOGIN
  * mechanism, and advertises the specified SASL mechanisms.
  *
  * The lines sent by the client during authentication are recorded.
  */
class LOGINSMTPTestSocket : public lineBasedTestSocket {

public:

	LOGINSMTPTestSocket()
		: m_loginStep(0) {

	}

	static void reset(const vmime::string& mechs) {

		sm_mechs = mechs;
		sm_authLines.clear();
	}

	static const std::vector <vmime::string>& getAuthLines() {

		return sm_authLines;
	}

	void onConnected() {

		localSend("220 test.vmime.org Service ready\r\n");
	}

	void processCommand() {

		if (!haveMoreLines()) {
			return;
		}

		const vmime::string line = getNextLine();

		if (m_loginStep == 1) {

			// Client response to the "Username:" challenge
			sm_authLines.push_back(line);
			localSend("334 UGFzc3dvcmQ6\r\n");

			m_loginStep = 2;

		} else if (m_loginStep == 2) {

			// Client response to the "Password:" challenge
			sm_authLines.push_back(line);
			localSend("235 2.7.0 Authentication successful\r\n");

			m_loginStep = 0;

		} else {

			const vmime::string verb =
				vmime::utility::stringUtils::toUpper(line.substr(0, line.find(' ')));

			if (verb == "EHLO") {

				localSend("250-test.vmime.org\r\n");
				localSend("250 AUTH " + sm_mechs + "\r\n");

			} else if (verb == "AUTH") {

				sm_authLines.push_back(line);

				if (line == "AUTH LOGIN") {

					localSend("334 VXNlcm5hbWU6\r\n");

					m_loginStep = 1;

				} else {

					localSend("235 2.7.0 Authentication successful\r\n");
				}

			} else if (verb == "QUIT") {

				localSend("221 test.vmime.org Service closing transmission channel\r\n");

			} else {

				localSend("502 Command not implemented\r\n");
			}
		}

		processCommand();
	}

private:

	int m_loginStep;

	static vmime::string sm_mechs;
	static std::vector <vmime::string> sm_authLines;
};


vmime::string LOGINSMTPTestSocket::sm_mechs;
std::vector <vmime::string> LOGINSMTPTestSocket::sm_authLines;



/** SMTP test server which advertises its extensions with keywords which
  * are not in upper case, and accepts any authentication.
  *
  * The AUTH command sent by the client is recorded.
  */
class mixedCaseEHLOSMTPTestSocket : public lineBasedTestSocket {

public:

	static void reset() {

		sm_authLine.clear();
	}

	static const vmime::string& getAuthLine() {

		return sm_authLine;
	}

	void onConnected() {

		localSend("220 test.vmime.org Service ready\r\n");
	}

	void processCommand() {

		if (!haveMoreLines()) {
			return;
		}

		const vmime::string line = getNextLine();
		const vmime::string verb =
			vmime::utility::stringUtils::toUpper(line.substr(0, line.find(' ')));

		if (verb == "EHLO") {

			localSend("250-test.vmime.org\r\n");
			localSend("250-auth PLAIN\r\n");
			localSend("250-Pipelining\r\n");
			localSend("250 size 1000000\r\n");

		} else if (verb == "AUTH") {

			sm_authLine = line;
			localSend("235 2.7.0 Authentication successful\r\n");

		} else if (verb == "QUIT") {

			localSend("221 test.vmime.org Service closing transmission channel\r\n");

		} else {

			localSend("502 Command not implemented\r\n");
		}

		processCommand();
	}

private:

	static vmime::string sm_authLine;
};


vmime::string mixedCaseEHLOSMTPTestSocket::sm_authLine;



/** SMTP test server which supports SASL authentication with the OAUTHBEARER
  * mechanism, and accepts only the specified initial client response.
  *
  * The lines sent by the client during authentication are recorded.
  */
class OAUTHBEARERSMTPTestSocket : public lineBasedTestSocket {

public:

	OAUTHBEARERSMTPTestSocket()
		: m_promptSent(false),
		  m_errorSent(false) {

	}

	static void reset(const vmime::string& acceptedResponse) {

		sm_acceptedResponse = acceptedResponse;
		sm_authLines.clear();
	}

	static const std::vector <vmime::string>& getAuthLines() {

		return sm_authLines;
	}

	void onConnected() {

		localSend("220 test.vmime.org Service ready\r\n");
	}

	void processCommand() {

		if (!haveMoreLines()) {
			return;
		}

		const vmime::string line = getNextLine();

		if (m_promptSent) {

			// Initial client response, sent after the prompt
			sm_authLines.push_back(line);
			sendAuthResult(line == sm_acceptedResponse);

			m_promptSent = false;

		} else if (m_errorSent) {

			// Client response to the error challenge
			sm_authLines.push_back(line);
			localSend("535 5.7.8 Authentication credentials invalid\r\n");

			m_errorSent = false;

		} else {

			const vmime::string verb =
				vmime::utility::stringUtils::toUpper(line.substr(0, line.find(' ')));

			if (verb == "EHLO") {

				localSend("250-test.vmime.org\r\n");
				localSend("250 AUTH PLAIN OAUTHBEARER\r\n");

			} else if (verb == "AUTH") {

				sm_authLines.push_back(line);

				if (line == "AUTH OAUTHBEARER") {

					// No initial response: like Yahoo, send a prompt ("OAuthBearer:")
					// instead of an empty challenge
					localSend("334 T0F1dGhCZWFyZXI6\r\n");

					m_promptSent = true;

				} else {

					sendAuthResult(line == "AUTH OAUTHBEARER " + sm_acceptedResponse);
				}

			} else if (verb == "QUIT") {

				localSend("221 test.vmime.org Service closing transmission channel\r\n");

			} else {

				localSend("502 Command not implemented\r\n");
			}
		}

		processCommand();
	}

private:

	void sendAuthResult(const bool accepted) {

		if (accepted) {

			localSend("235 2.7.0 Authentication successful\r\n");

		} else {

			// Error message (RFC 7628, section 3.2.2):
			//    {"status":"invalid_token","scope":"https://mail.vmime.org/"}
			localSend("334 eyJzdGF0dXMiOiJpbnZhbGlkX3Rva2VuIiwic2NvcGUiOiJodHRwczovL21haWwudm1pbWUub3JnLyJ9\r\n");

			m_errorSent = true;
		}
	}

	bool m_promptSent;
	bool m_errorSent;

	static vmime::string sm_acceptedResponse;
	static std::vector <vmime::string> sm_authLines;
};


vmime::string OAUTHBEARERSMTPTestSocket::sm_acceptedResponse;
std::vector <vmime::string> OAUTHBEARERSMTPTestSocket::sm_authLines;



VMIME_TEST_SUITE_BEGIN(SMTPTransportTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConnectToInvalidServer)
		VMIME_TEST(testGreetingError)
		VMIME_TEST(testMAILandRCPT)
		VMIME_TEST(testChunking)
		VMIME_TEST(testSize_Chunking)
		VMIME_TEST(testSize_NoChunking)
		VMIME_TEST(testSMTPUTF8_available)
		VMIME_TEST(testSMTPUTF8_notAvailable)
		VMIME_TEST(testSMTPUTF8_without8BITMIME)
		VMIME_TEST(testSMTPUTF8_message)
		VMIME_TEST(testSMTPUTF8_headerAddress)
		VMIME_TEST(testSMTPUTF8_internationalizedMessageOption)
		VMIME_TEST(test8BITMIME_7bitMessage)
		VMIME_TEST(test8BITMIME_8bitPart)
		VMIME_TEST(test8BITMIME_notSupported)
		VMIME_TEST(test8BITMIME_converted)
		VMIME_TEST(test8BITMIME_undeclared)
		VMIME_TEST(test8BITMIME_undeclaredAfterChunks)
		VMIME_TEST(test8BITMIME_rawData)
		VMIME_TEST(testRawData_Chunking)
		VMIME_TEST(testBINARYMIME_binaryPart)
		VMIME_TEST(testBINARYMIME_binaryPartConverted)
		VMIME_TEST(testBINARYMIME_conversionDisabled)
		VMIME_TEST(testBINARYMIME_messagePart)
		VMIME_TEST(testBINARYMIME_undeclaredBinaryPart)
		VMIME_TEST(testBINARYMIME_undeclaredBinaryStream)
		VMIME_TEST(testBINARYMIME_rawData)
		VMIME_TEST(testBINARYMIME_undeclaredRawData)
		VMIME_TEST(test8BITMIME_messageAttachment)
		VMIME_TEST(testCanonicalLineEndings)
		VMIME_TEST(testSize_NotLowerThanActual)
		VMIME_TEST(testExtensionKeywordsCase)
		VMIME_TEST(testReconnectAfterTimeout)
		VMIME_TEST(testDisconnectAfterTimeout)
		VMIME_TEST(testSendTimeout)
		VMIME_TEST(testConnectionLostDuringEnvelope)
		VMIME_TEST(testConnectionLostDuringData)
		VMIME_TEST(testConnectionLostDuringChunking)
		VMIME_TEST(testInputStreamErrorDuringData)
		VMIME_TEST(testInputStreamErrorDuringChunking)
		VMIME_TEST(testGenerationErrorDuringChunking)
		VMIME_TEST(testSASLInitialResponse)
		VMIME_TEST(testSASLInitialResponse_MaxLength)
		VMIME_TEST(testSASLInitialResponse_TooLong)
		VMIME_TEST(testSASL_PlaintextNotAllowed)
		VMIME_TEST(testSASL_LOGIN)
		VMIME_TEST(testSASL_PLAINPreferredToLOGIN)
		VMIME_TEST(testSASL_OAUTHBEARER)
		VMIME_TEST(testSASL_OAUTHBEARER_Error)
		VMIME_TEST(testSASL_OAUTHBEARER_Prompt)
	VMIME_TEST_LIST_END


	// The server supports CHUNKING, but the message data is sent with DATA
	// unless chunking is enabled (the server does not handle BDAT data)
	static vmime::shared_ptr <vmime::net::transport> createFailingTransport(const bool chunking = false) {

		failingSMTPTestSocket::reset();

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <failingSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());
		tr->setProperty("options.chunking", chunking);

		return tr;
	}

	static void sendTestMessage(const vmime::shared_ptr <vmime::net::transport>& tr) {

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::string data("Message data");
		vmime::utility::inputStreamStringAdapter is(data);

		tr->send(exp, recips, is, data.length());
	}


	void testConnectToInvalidServer() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();

		vmime::utility::url url("smtp://invalid-smtp-server");
		vmime::shared_ptr <vmime::net::transport> store = sess->getTransport(url);

		VASSERT_THROW("connect", store->connect(), vmime::exceptions::connection_error);
	}

	void testGreetingError() {

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <greetingErrorSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		VASSERT_THROW(
			"Connection",
			tr->connect(),
			vmime::exceptions::connection_greeting_error
		);
	}

	void testMAILandRCPT() {

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <MAILandRCPTSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		VASSERT_NO_THROW("Connection", tr->connect());

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient1@test.vmime.org"));
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient2@test.vmime.org"));
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient3@test.vmime.org"));

		vmime::string data("Message data");
		vmime::utility::inputStreamStringAdapter is(data);

		tr->send(exp, recips, is, 0);
	}

	void testChunking() {

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <chunkingSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		tr->connect();

		VASSERT(
			"Test server should report it supports the CHUNKING extension!",
			vmime::dynamicCast <vmime::net::smtp::SMTPTransport>(tr)->getConnection()->hasExtension("CHUNKING")
		);

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <SMTPTestMessage>();

		tr->send(msg, exp, recips);
	}

	void testSize_Chunking() {

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <bigMessageSMTPTestSocket <true> > >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		tr->connect();

		VASSERT(
			"Test server should report it supports the SIZE extension!",
			vmime::dynamicCast <vmime::net::smtp::SMTPTransport>(tr)->getConnection()->hasExtension("SIZE")
		);

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <SMTPBigTestMessage4MB>();

		VASSERT_THROW(
			"Max size limit exception",
			tr->send(msg, exp, recips),
			vmime::net::smtp::SMTPMessageSizeExceedsMaxLimitsException
		);
	}

	void testSize_NoChunking() {

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <bigMessageSMTPTestSocket <false> > >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		tr->connect();

		VASSERT(
			"Test server should report it supports the SIZE extension!",
			vmime::dynamicCast <vmime::net::smtp::SMTPTransport>(tr)->getConnection()->hasExtension("SIZE")
		);

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <SMTPBigTestMessage4MB>();

		VASSERT_THROW(
			"Max size limit exception",
			tr->send(msg, exp, recips),
			vmime::net::smtp::SMTPMessageSizeExceedsMaxLimitsException
		);
	}

	template <bool SUPPORTS_UTF8, bool SUPPORTS_8BITMIME = SUPPORTS_UTF8>
	static vmime::shared_ptr <vmime::net::transport> createUTF8Transport() {

		UTF8SMTPTestSocket <SUPPORTS_UTF8, SUPPORTS_8BITMIME>::reset();

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <UTF8SMTPTestSocket <SUPPORTS_UTF8, SUPPORTS_8BITMIME> > >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		tr->connect();

		return tr;
	}

	static vmime::shared_ptr <vmime::mailbox> createUTF8Mailbox(
		const vmime::string& localName,
		const vmime::string& domainName
	) {

		return vmime::make_shared <vmime::mailbox>(
			vmime::emailAddress(
				vmime::word(localName, vmime::charsets::UTF_8),
				vmime::word(domainName, vmime::charsets::UTF_8)
			)
		);
	}

	static void sendUTF8TestData(
		const vmime::shared_ptr <vmime::net::transport>& tr,
		const vmime::mailbox& exp,
		const vmime::mailboxList& recips,
		const vmime::net::transport::sendOptions& options = vmime::net::transport::sendOptions()
	) {

		vmime::string data("Message data");
		vmime::utility::inputStreamStringAdapter is(data);

		tr->send(exp, recips, is, data.length(), NULL, vmime::mailbox(), options);
	}

	static vmime::shared_ptr <vmime::message> createUTF8TestMessage(const vmime::mailbox& recipient) {

		vmime::messageBuilder mb;
		mb.setExpeditor(vmime::mailbox("expediteur@test.vmime.org"));
		mb.getRecipients().appendAddress(vmime::make_shared <vmime::mailbox>(recipient));
		mb.setSubject(vmime::text("Grüße", vmime::charsets::UTF_8));
		mb.getTextPart()->setText(vmime::make_shared <vmime::stringContentHandler>("Message data"));

		return mb.construct();
	}

	void testSMTPUTF8_available() {

		// Non-ASCII sender and recipient
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));
			recips.appendMailbox(createUTF8Mailbox("récepteur", "test.vmime.org"));

			sendUTF8TestData(tr, *createUTF8Mailbox("expéditeur", "test.vmime.org"), recips);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 3, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expéditeur@test.vmime.org> SMTPUTF8 BODY=8BITMIME", lines[0]);
			VASSERT_EQ("RCPT 1", "RCPT TO:<recipient@test.vmime.org>", lines[1]);
			VASSERT_EQ("RCPT 2", "RCPT TO:<récepteur@test.vmime.org>", lines[2]);
		}

		// Non-ASCII recipient only
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));
			recips.appendMailbox(createUTF8Mailbox("récepteur", "test.vmime.org"));

			sendUTF8TestData(tr, vmime::mailbox("expediteur@test.vmime.org"), recips);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 3, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org> SMTPUTF8 BODY=8BITMIME", lines[0]);
			VASSERT_EQ("RCPT 1", "RCPT TO:<recipient@test.vmime.org>", lines[1]);
			VASSERT_EQ("RCPT 2", "RCPT TO:<récepteur@test.vmime.org>", lines[2]);
		}

		// Non-ASCII display name only: it is not part of the envelope,
		// so SMTPUTF8 is not needed (RFC 6531, section 3.4)
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			vmime::text name;
			name.appendWord(vmime::make_shared <vmime::word>("Expéditeur", vmime::charsets::UTF_8));
			name.appendWord(vmime::make_shared <vmime::word>(" Test", vmime::charsets::US_ASCII));

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

			sendUTF8TestData(tr, vmime::mailbox(name, vmime::emailAddress("expediteur@test.vmime.org")), recips);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org>", lines[0]);
			VASSERT_EQ("RCPT", "RCPT TO:<recipient@test.vmime.org>", lines[1]);
		}

		// Non-ASCII domain only: it is written as IDNA A-labels, so SMTPUTF8
		// is not needed (RFC 6531, section 3.2)
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			vmime::mailboxList recips;
			recips.appendMailbox(createUTF8Mailbox("recipient", "tést.vmime.org"));

			sendUTF8TestData(tr, vmime::mailbox("expediteur@test.vmime.org"), recips);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org>", lines[0]);
			VASSERT_EQ("RCPT", "RCPT TO:<recipient@xn--tst-bma.vmime.org>", lines[1]);
		}
	}

	// An SMTPUTF8-aware client must not send non-ASCII addresses to a server
	// which does not support SMTPUTF8 (RFC 6531, sections 3.2 and 3.5)
	void testSMTPUTF8_notAvailable() {

		// Non-ASCII sender
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <false>();

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

			VASSERT_THROW(
				"Send",
				sendUTF8TestData(tr, *createUTF8Mailbox("expéditeur", "test.vmime.org"), recips),
				vmime::net::smtp::SMTPUTF8ExtensionNotSupportedException
			);

			VASSERT_TRUE("No command sent", UTF8SMTPTestSocket <false>::getEnvelopeLines().empty());
			VASSERT_TRUE("Still connected", tr->isConnected());
		}

		// Non-ASCII recipient only
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <false>();

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));
			recips.appendMailbox(createUTF8Mailbox("récepteur", "test.vmime.org"));

			VASSERT_THROW(
				"Send",
				sendUTF8TestData(tr, vmime::mailbox("expediteur@test.vmime.org"), recips),
				vmime::net::smtp::SMTPUTF8ExtensionNotSupportedException
			);

			VASSERT_TRUE("No command sent", UTF8SMTPTestSocket <false>::getEnvelopeLines().empty());
			VASSERT_TRUE("Still connected", tr->isConnected());
		}

		// Non-ASCII domain only: it is written as IDNA A-labels
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <false>();

			vmime::mailboxList recips;
			recips.appendMailbox(createUTF8Mailbox("recipient", "tést.vmime.org"));

			sendUTF8TestData(tr, vmime::mailbox("expediteur@test.vmime.org"), recips);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <false>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org>", lines[0]);
			VASSERT_EQ("RCPT", "RCPT TO:<recipient@xn--tst-bma.vmime.org>", lines[1]);
		}
	}

	// A server which supports SMTPUTF8 must also announce 8BITMIME (RFC 6531,
	// section 3.1), without which 8-bit data must not be sent (RFC 6152,
	// section 3): SMTPUTF8 is not usable with a server which does not
	void testSMTPUTF8_without8BITMIME() {

		// Non-ASCII recipient
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true, false>();

			vmime::mailboxList recips;
			recips.appendMailbox(createUTF8Mailbox("récepteur", "test.vmime.org"));

			VASSERT_THROW(
				"Send",
				sendUTF8TestData(tr, vmime::mailbox("expediteur@test.vmime.org"), recips),
				vmime::net::smtp::SMTPUTF8ExtensionNotSupportedException
			);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true, false>::getEnvelopeLines();

			VASSERT_TRUE("No command sent", lines.empty());
			VASSERT_TRUE("Still connected", tr->isConnected());
		}

		// ASCII addresses: SMTPUTF8 is not needed, and the non-ASCII subject is encoded
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true, false>();

			tr->send(createUTF8TestMessage(vmime::mailbox("recipient@test.vmime.org")));

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true, false>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org>", lines[0]);
			VASSERT_EQ("RCPT", "RCPT TO:<recipient@test.vmime.org>", lines[1]);

			const vmime::string& data = UTF8SMTPTestSocket <true, false>::getMessageData();

			VASSERT(data, data.find("Subject: =?utf-8?B?R3LDvMOfZQ==?=\r\n") != vmime::string::npos);
		}
	}

	// The message is generated with UTF-8 header fields (RFC 6532) only if
	// it is sent with SMTPUTF8 (RFC 6531, section 3.4)
	void testSMTPUTF8_message() {

		// ASCII addresses: the non-ASCII subject is encoded, and SMTPUTF8
		// is not used, even if the server supports it
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			tr->send(createUTF8TestMessage(vmime::mailbox("recipient@test.vmime.org")));

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org>", lines[0]);
			VASSERT_EQ("RCPT", "RCPT TO:<recipient@test.vmime.org>", lines[1]);

			const vmime::string& data = UTF8SMTPTestSocket <true>::getMessageData();

			VASSERT(data, data.find("Subject: =?utf-8?B?R3LDvMOfZQ==?=\r\n") != vmime::string::npos);
		}

		// Non-ASCII recipient: SMTPUTF8 is used, and the subject is sent as UTF-8
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			tr->send(createUTF8TestMessage(*createUTF8Mailbox("récepteur", "test.vmime.org")));

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org> SMTPUTF8 BODY=8BITMIME", lines[0]);
			VASSERT_EQ("RCPT", "RCPT TO:<récepteur@test.vmime.org>", lines[1]);

			const vmime::string& data = UTF8SMTPTestSocket <true>::getMessageData();

			VASSERT(data, data.find("Subject: Grüße\r\n") != vmime::string::npos);
			VASSERT(data, data.find("To: récepteur@test.vmime.org\r\n") != vmime::string::npos);
		}
	}

	// A non-ASCII address in a header field which is not part of the envelope
	// also needs SMTPUTF8 (RFC 6531, section 3.6)
	void testSMTPUTF8_headerAddress() {

		vmime::addressList replyTo;
		replyTo.appendAddress(createUTF8Mailbox("réponse", "test.vmime.org"));

		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			vmime::shared_ptr <vmime::message> msg =
				createUTF8TestMessage(vmime::mailbox("recipient@test.vmime.org"));

			msg->getHeader()->ReplyTo()->setValue(replyTo);

			tr->send(msg);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org> SMTPUTF8 BODY=8BITMIME", lines[0]);
			VASSERT_EQ("RCPT", "RCPT TO:<recipient@test.vmime.org>", lines[1]);

			const vmime::string& data = UTF8SMTPTestSocket <true>::getMessageData();

			VASSERT(data, data.find("Reply-To: réponse@test.vmime.org\r\n") != vmime::string::npos);
		}

		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <false>();

			vmime::shared_ptr <vmime::message> msg =
				createUTF8TestMessage(vmime::mailbox("recipient@test.vmime.org"));

			msg->getHeader()->ReplyTo()->setValue(replyTo);

			VASSERT_THROW("Send", tr->send(msg), vmime::net::smtp::SMTPUTF8ExtensionNotSupportedException);

			VASSERT_TRUE("No command sent", UTF8SMTPTestSocket <false>::getEnvelopeLines().empty());
		}
	}

	// The caller can tell that the message has UTF-8 header fields, which
	// VMime cannot know when sending raw message data
	void testSMTPUTF8_internationalizedMessageOption() {

		vmime::net::smtp::SMTPSendOptions options;
		options.setInternationalizedMessage(true);

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		// Raw message data, ASCII addresses
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			sendUTF8TestData(tr, vmime::mailbox("expediteur@test.vmime.org"), recips, options);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org> SMTPUTF8 BODY=8BITMIME", lines[0]);
			VASSERT_EQ("RCPT", "RCPT TO:<recipient@test.vmime.org>", lines[1]);
		}

		// Server without SMTPUTF8
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <false>();

			VASSERT_THROW(
				"Send",
				sendUTF8TestData(tr, vmime::mailbox("expediteur@test.vmime.org"), recips, options),
				vmime::net::smtp::SMTPUTF8ExtensionNotSupportedException
			);

			VASSERT_TRUE("No command sent", UTF8SMTPTestSocket <false>::getEnvelopeLines().empty());
		}

		// Message object: it is generated with UTF-8 header fields
		{
			vmime::shared_ptr <vmime::net::transport> tr = createUTF8Transport <true>();

			tr->send(createUTF8TestMessage(vmime::mailbox("recipient@test.vmime.org")), NULL, options);

			const std::vector <vmime::string>& lines = UTF8SMTPTestSocket <true>::getEnvelopeLines();

			VASSERT_EQ("Count", 2, lines.size());
			VASSERT_EQ("MAIL", "MAIL FROM:<expediteur@test.vmime.org> SMTPUTF8 BODY=8BITMIME", lines[0]);

			const vmime::string& data = UTF8SMTPTestSocket <true>::getMessageData();

			VASSERT(data, data.find("Subject: Grüße\r\n") != vmime::string::npos);
		}
	}

	static vmime::shared_ptr <vmime::net::transport> create8bitTransport(
		const std::vector <vmime::string>& extensions
	) {

		eightBitSMTPTestSocket::reset(extensions);

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <eightBitSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		tr->connect();

		return tr;
	}

	// A message as received, with a text part in the specified encoding
	static vmime::shared_ptr <vmime::message> create8bitTestMessage(
		const vmime::string& encoding,
		const vmime::string& body
	) {

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();

		msg->parse(
			"From: expediteur@test.vmime.org\r\n"
			"To: recipient@test.vmime.org\r\n"
			"Subject: Test\r\n"
			"MIME-Version: 1.0\r\n"
			"Content-Type: text/plain; charset=utf-8\r\n"
			"Content-Transfer-Encoding: " + encoding + "\r\n"
			"\r\n" + body
		);

		return msg;
	}

	// A message as received, with a part of the specified type, in the
	// specified encoding
	static vmime::shared_ptr <vmime::message> createTypedTestMessage(
		const vmime::string& type,
		const vmime::string& encoding,
		const vmime::string& body
	) {

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <vmime::message>();

		msg->parse(
			"From: expediteur@test.vmime.org\r\n"
			"To: recipient@test.vmime.org\r\n"
			"Subject: Test\r\n"
			"MIME-Version: 1.0\r\n"
			"Content-Type: " + type + "\r\n"
			"Content-Transfer-Encoding: " + encoding + "\r\n"
			"\r\n" + body
		);

		return msg;
	}

	// Return the header and the decoded body of a message received by
	// the server
	static std::pair <vmime::string, vmime::string> parseReceivedMessage(const vmime::string& data) {

		vmime::message msg;
		msg.parse(data);

		vmime::string body;
		vmime::utility::outputStreamStringAdapter os(body);

		msg.getBody()->getContents()->extract(os);

		return std::make_pair(msg.getHeader()->generate(), body);
	}

	static void send8bitTestData(
		const vmime::shared_ptr <vmime::net::transport>& tr,
		const vmime::string& data,
		const vmime::net::transport::sendOptions& options = vmime::net::transport::sendOptions()
	) {

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::utility::inputStreamStringAdapter is(data);

		tr->send(
			vmime::mailbox("expediteur@test.vmime.org"), recips, is, data.length(),
			NULL, vmime::mailbox(), options
		);
	}

	// A 7-bit message is not declared as 8-bit, even if the server supports it
	void test8BITMIME_7bitMessage() {

		// DATA
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME" });

			tr->send(create8bitTestMessage("7bit", "Hello\r\n"));

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

			VASSERT_EQ("1.1", 3, commands.size());
			VASSERT_EQ("1.2", "MAIL FROM:<expediteur@test.vmime.org>", commands[0]);
			VASSERT_EQ("1.3", 1, eightBitSMTPTestSocket::getMessages().size());
		}

		// CHUNKING
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME", "CHUNKING" });

			tr->send(create8bitTestMessage("7bit", "Hello\r\n"));

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

			VASSERT_EQ("2.1", 3, commands.size());
			VASSERT_EQ("2.2", "MAIL FROM:<expediteur@test.vmime.org>", commands[0]);
			VASSERT_EQ("2.3", 1, eightBitSMTPTestSocket::getMessages().size());
		}
	}

	// A message which has a part declared as 8-bit is declared as 8-bit
	// (RFC 6152, section 3)
	void test8BITMIME_8bitPart() {

		const vmime::string text8bit = "Gr\xc3\xbc\xc3\x9f" "e\r\n";

		// DATA
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME" });

			tr->send(create8bitTestMessage("8bit", text8bit));

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("1.1", 3, commands.size());
			VASSERT_EQ("1.2", "MAIL FROM:<expediteur@test.vmime.org> BODY=8BITMIME", commands[0]);
			VASSERT_EQ("1.3", 1, messages.size());
			VASSERT("1.4", messages[0].find(text8bit) != vmime::string::npos);
		}

		// CHUNKING
		{
			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "CHUNKING", "PIPELINING" });

			tr->send(create8bitTestMessage("8bit", text8bit));

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("2.1", 3, commands.size());
			VASSERT_EQ("2.2", "MAIL FROM:<expediteur@test.vmime.org> BODY=8BITMIME", commands[0]);
			VASSERT_EQ("2.3", 1, messages.size());
			VASSERT("2.4", messages[0].find(text8bit) != vmime::string::npos);
		}
	}

	// 8-bit data must not be sent to a server which does not support
	// 8BITMIME (RFC 6152, section 3)
	void test8BITMIME_notSupported() {

		const vmime::string text8bit = "Gr\xc3\xbc\xc3\x9f" "e\r\n";

		// The 8-bit part could be converted, but the conversion is disabled
		vmime::net::smtp::SMTPSendOptions options;
		options.setConvertParts(false);

		// DATA
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({});

			VASSERT_THROW(
				"1.1",
				tr->send(create8bitTestMessage("8bit", text8bit), NULL, options),
				vmime::net::smtp::SMTP8BITMIMEExtensionNotSupportedException
			);

			VASSERT_TRUE("1.2 No command sent", eightBitSMTPTestSocket::getCommands().empty());
			VASSERT_TRUE("1.3 Still connected", tr->isConnected());
		}

		// CHUNKING
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "CHUNKING" });

			VASSERT_THROW(
				"2.1",
				tr->send(create8bitTestMessage("8bit", text8bit), NULL, options),
				vmime::net::smtp::SMTP8BITMIMEExtensionNotSupportedException
			);

			VASSERT_TRUE("2.2 No command sent", eightBitSMTPTestSocket::getCommands().empty());
			VASSERT_TRUE("2.3 Still connected", tr->isConnected());
		}
	}

	// Without 8BITMIME, the parts which contain 8-bit data are converted to
	// 7-bit data (RFC 6152, section 3)
	void test8BITMIME_converted() {

		const vmime::string text8bit = "Gr\xc3\xbc\xc3\x9f" "e\r\n";

		// DATA, then CHUNKING
		for (int chunking = 0 ; chunking < 2 ; ++chunking) {

			const std::string label = chunking ? "CHUNKING: " : "DATA: ";

			const std::vector <vmime::string> extensions =
				chunking ? std::vector <vmime::string>({ "CHUNKING" }) : std::vector <vmime::string>();

			// Text, declared as 8bit or 7bit: Quoted-Printable
			for (const vmime::string encoding : { "8bit", "7bit" }) {

				vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport(extensions);
				vmime::shared_ptr <vmime::message> msg = create8bitTestMessage(encoding, text8bit);

				tr->send(msg);

				const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
				const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

				VASSERT_EQ(label + encoding + ", MAIL", "MAIL FROM:<expediteur@test.vmime.org>", commands[0]);
				VASSERT_EQ(label + encoding + ", Delivered", 1, messages.size());

				const std::pair <vmime::string, vmime::string> received = parseReceivedMessage(messages[0]);

				VASSERT(
					label + encoding + ", Quoted-Printable",
					received.first.find("Content-Transfer-Encoding: quoted-printable\r\n") != vmime::string::npos
				);

				VASSERT_EQ(label + encoding + ", Data", text8bit, received.second);

				// The message object itself is not modified
				VASSERT_EQ(label + encoding + ", Not modified", encoding, msg->getBody()->getEncoding().generate());
			}

			// Other data: Base64
			{
				const vmime::string data = "\xff\xfe\r\n";

				vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport(extensions);

				tr->send(createTypedTestMessage("application/octet-stream", "8bit", data));

				const std::pair <vmime::string, vmime::string> received =
					parseReceivedMessage(eightBitSMTPTestSocket::getMessages().at(0));

				VASSERT(
					label + "Base64",
					received.first.find("Content-Transfer-Encoding: base64\r\n") != vmime::string::npos
				);

				VASSERT_EQ(label + "Base64 data", data, received.second);
			}

			// A message cannot be encoded (RFC 2046, section 5.2.1)
			{
				vmime::shared_ptr <vmime::message> msg = create8bitTestMessage("7bit", "See the attached message.\r\n");
				vmime::attachmentHelper::addAttachment(msg, create8bitTestMessage("8bit", text8bit));

				vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport(extensions);

				VASSERT_THROW(
					label + "Message",
					tr->send(msg),
					vmime::net::smtp::SMTP8BITMIMEExtensionNotSupportedException
				);

				VASSERT_TRUE(label + "No command sent", eightBitSMTPTestSocket::getCommands().empty());
			}
		}
	}

	// 8-bit data in a part declared as 7-bit, as sent by some old mailers
	void test8BITMIME_undeclared() {

		const vmime::string text8bit = "Gr\xc3\xbc\xc3\x9f" "e\r\n";

		// DATA: the whole message is generated before it is sent, so it is
		// declared as 8-bit
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME" });

			tr->send(create8bitTestMessage("7bit", text8bit));

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

			VASSERT_EQ("1.1", 3, commands.size());
			VASSERT_EQ("1.2", "MAIL FROM:<expediteur@test.vmime.org> BODY=8BITMIME", commands[0]);
			VASSERT_EQ("1.3", 1, eightBitSMTPTestSocket::getMessages().size());
		}

		// CHUNKING: the message is generated while it is sent, so the transfer
		// is cancelled, and the message can be sent again as 8-bit
		{
			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "CHUNKING", "PIPELINING" });

			vmime::shared_ptr <vmime::message> msg = create8bitTestMessage("7bit", text8bit);

			VASSERT_THROW("2.1", tr->send(msg), vmime::net::smtp::SMTPUndeclared8bitDataException);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

			VASSERT_EQ("2.2", 3, commands.size());
			VASSERT_EQ("2.3", "MAIL FROM:<expediteur@test.vmime.org>", commands[0]);
			VASSERT_EQ("2.4", "RSET", commands[2]);
			VASSERT_TRUE("2.5 Not delivered", eightBitSMTPTestSocket::getMessages().empty());
			VASSERT_TRUE("2.6 Still connected", tr->isConnected());

			vmime::net::smtp::SMTPSendOptions options;
			options.set8bitMessage(true);

			tr->send(msg, NULL, options);

			VASSERT_EQ("2.7", 6, commands.size());
			VASSERT_EQ("2.8", "MAIL FROM:<expediteur@test.vmime.org> BODY=8BITMIME", commands[3]);
			VASSERT_EQ("2.9", 1, eightBitSMTPTestSocket::getMessages().size());
		}

		// CHUNKING, server without 8BITMIME: the message cannot be sent
		// (contents which can only be read once cannot be checked, and so
		// converted, before the message is sent)
		{
			vmime::shared_ptr <vmime::message> msg = create8bitTestMessage("7bit", "");

			msg->getBody()->setContents(
				vmime::make_shared <vmime::streamContentHandler>(
					vmime::make_shared <readOnceInputStream>(text8bit), text8bit.length()
				)
			);

			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "CHUNKING" });

			VASSERT_THROW(
				"3.1",
				tr->send(msg),
				vmime::net::smtp::SMTP8BITMIMEExtensionNotSupportedException
			);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

			VASSERT_EQ("3.2", 3, commands.size());
			VASSERT_EQ("3.3", "RSET", commands[2]);
			VASSERT_TRUE("3.4 Not delivered", eightBitSMTPTestSocket::getMessages().empty());
			VASSERT_TRUE("3.5 Still connected", tr->isConnected());
		}
	}

	// When undeclared 8-bit data is found after some chunks have been sent,
	// they are discarded by resetting the transaction (RFC 3030, section 2)
	void test8BITMIME_undeclaredAfterChunks() {

		// 7-bit text, longer than one chunk, then 8-bit text
		vmime::string body;

		for (int i = 0 ; i < 4000 ; ++i) {
			body += vmime::string(78, 'A') + "\r\n";
		}

		body += "Gr\xc3\xbc\xc3\x9f" "e\r\n";

		// The responses for the chunks sent must be read first, if they
		// were pipelined
		for (int pipelining = 0 ; pipelining < 2 ; ++pipelining) {

			std::vector <vmime::string> extensions = { "8BITMIME", "CHUNKING" };

			if (pipelining) {
				extensions.push_back("PIPELINING");
			}

			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport(extensions);

			VASSERT_THROW(
				"Send",
				tr->send(create8bitTestMessage("7bit", body)),
				vmime::net::smtp::SMTPUndeclared8bitDataException
			);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

			VASSERT_EQ("Count", 4, commands.size());
			VASSERT_EQ("BDAT", "BDAT 262144", commands[2]);
			VASSERT_EQ("RSET", "RSET", commands[3]);
			VASSERT_TRUE("Not delivered", eightBitSMTPTestSocket::getMessages().empty());

			// The connection is still usable
			tr->send(create8bitTestMessage("7bit", "Hello\r\n"));

			VASSERT_EQ("Delivered", 1, eightBitSMTPTestSocket::getMessages().size());
		}
	}

	// Raw message data is not parsed: it is declared as 8-bit only with the
	// send option, and otherwise checked while it is sent
	void test8BITMIME_rawData() {

		const vmime::string data7bit = "Subject: Test\r\n\r\nHello\r\n";
		const vmime::string data8bit = "Subject: Test\r\n\r\nGr\xc3\xbc\xc3\x9f" "e\r\n";

		vmime::net::smtp::SMTPSendOptions options;
		options.set8bitMessage(true);

		// 7-bit data
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME" });

			send8bitTestData(tr, data7bit);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

			VASSERT_EQ("1.1", "MAIL FROM:<expediteur@test.vmime.org>", commands[0]);
			VASSERT_EQ("1.2", 1, eightBitSMTPTestSocket::getMessages().size());
		}

		// 8-bit data, declared with the option
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME" });

			send8bitTestData(tr, data8bit, options);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("2.1", "MAIL FROM:<expediteur@test.vmime.org> BODY=8BITMIME", commands[0]);
			VASSERT_EQ("2.2", 1, messages.size());
			VASSERT("2.3", messages[0].find(data8bit) == 0);
		}

		// 8-bit data, not declared: the message is not delivered, and as the
		// DATA transfer cannot be cancelled, the connection is closed
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME" });

			VASSERT_THROW(
				"3.1",
				send8bitTestData(tr, data8bit),
				vmime::net::smtp::SMTPUndeclared8bitDataException
			);

			VASSERT_TRUE("3.2 Not delivered", eightBitSMTPTestSocket::getMessages().empty());
			VASSERT_FALSE("3.3 Disconnected", tr->isConnected());
		}

		// 8-bit data, server without 8BITMIME
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({});

			VASSERT_THROW(
				"4.1",
				send8bitTestData(tr, data8bit),
				vmime::net::smtp::SMTP8BITMIMEExtensionNotSupportedException
			);

			VASSERT_TRUE("4.2 Not delivered", eightBitSMTPTestSocket::getMessages().empty());
			VASSERT_FALSE("4.3 Disconnected", tr->isConnected());
		}

		// 8-bit data, declared with the option, server without 8BITMIME:
		// nothing is sent
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({});

			VASSERT_THROW(
				"5.1",
				send8bitTestData(tr, data8bit, options),
				vmime::net::smtp::SMTP8BITMIMEExtensionNotSupportedException
			);

			VASSERT_TRUE("5.2 No command sent", eightBitSMTPTestSocket::getCommands().empty());
			VASSERT_TRUE("5.3 Still connected", tr->isConnected());
		}
	}

	// Raw message data is sent by chunks if the server supports CHUNKING
	// (RFC 3030), so that the transfer can be cancelled without closing the
	// connection, if it contains undeclared 8-bit data
	void testRawData_Chunking() {

		// 7-bit data, with LF only
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "CHUNKING" });

			send8bitTestData(tr, "Subject: Test\n\nLine 1\nLine 2\n");

			const vmime::string expected = "Subject: Test\r\n\r\nLine 1\r\nLine 2\r\n";

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("1.1", 3, commands.size());
			VASSERT_EQ("1.2", "BDAT " + std::to_string(expected.length()) + " LAST", commands[2]);
			VASSERT_EQ("1.3", 1, messages.size());
			VASSERT_EQ("1.4", expected, messages[0]);
		}

		// Undeclared 8-bit data: the transfer is cancelled, and the message
		// can be sent again as 8-bit on the same connection
		{
			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "CHUNKING", "PIPELINING" });

			const vmime::string data8bit = "Subject: Test\r\n\r\nGr\xc3\xbc\xc3\x9f" "e\r\n";

			VASSERT_THROW(
				"2.1",
				send8bitTestData(tr, data8bit),
				vmime::net::smtp::SMTPUndeclared8bitDataException
			);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("2.2", 3, commands.size());
			VASSERT_EQ("2.3", "RSET", commands[2]);
			VASSERT_TRUE("2.4 Not delivered", messages.empty());
			VASSERT_TRUE("2.5 Still connected", tr->isConnected());

			vmime::net::smtp::SMTPSendOptions options;
			options.set8bitMessage(true);

			send8bitTestData(tr, data8bit, options);

			VASSERT_EQ("2.6", "MAIL FROM:<expediteur@test.vmime.org> BODY=8BITMIME", commands[3]);
			VASSERT_EQ("2.7", 1, messages.size());
			VASSERT_EQ("2.8", data8bit, messages[0]);
		}
	}

	// Binary data, with NUL, CR and LF alone (RFC 2045, section 2.9)
	static vmime::string getBinaryTestData() {

		static const char data[] = "Binary\0data,\rwith CR\nand LF alone\r\n";
		return vmime::string(data, sizeof(data) - 1);
	}

	// Parts declared as binary are sent as is with BINARYMIME, which can
	// only be used with CHUNKING (RFC 3030, section 3)
	void testBINARYMIME_binaryPart() {

		const vmime::string binaryData = getBinaryTestData();

		vmime::shared_ptr <vmime::net::transport> tr =
			create8bitTransport({ "8BITMIME", "BINARYMIME", "CHUNKING", "PIPELINING" });

		tr->send(createTypedTestMessage("application/octet-stream", "binary", binaryData));

		const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
		const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

		VASSERT_EQ("1", 3, commands.size());
		VASSERT_EQ("2", "MAIL FROM:<expediteur@test.vmime.org> BODY=BINARYMIME", commands[0]);
		VASSERT_EQ("3", 1, messages.size());

		// The data is not altered (eg. its line endings)
		VASSERT("4", messages[0].find("\r\n\r\n" + binaryData) != vmime::string::npos);
	}

	// Otherwise, parts declared as binary are encoded in Base64, which keeps
	// their data as is (RFC 3030, section 3)
	void testBINARYMIME_binaryPartConverted() {

		const vmime::string binaryData = getBinaryTestData();

		// No BINARYMIME, BINARYMIME without CHUNKING, and BINARYMIME with
		// CHUNKING disabled
		for (int i = 0 ; i < 3 ; ++i) {

			const std::string label = "Server " + std::to_string(i) + ": ";

			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport(
					i == 0 ? std::vector <vmime::string>({ "8BITMIME", "CHUNKING" })
					: i == 1 ? std::vector <vmime::string>({ "8BITMIME", "BINARYMIME" })
					: std::vector <vmime::string>({ "8BITMIME", "BINARYMIME", "CHUNKING" })
				);

			if (i == 2) {
				tr->setProperty("options.chunking", false);
			}

			vmime::shared_ptr <vmime::message> msg =
				createTypedTestMessage("application/octet-stream", "binary", binaryData);

			tr->send(msg);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ(label + "MAIL", "MAIL FROM:<expediteur@test.vmime.org>", commands[0]);
			VASSERT_EQ(label + "Delivered", 1, messages.size());

			const std::pair <vmime::string, vmime::string> received = parseReceivedMessage(messages[0]);

			VASSERT(label + "Base64", received.first.find("Content-Transfer-Encoding: base64\r\n") != vmime::string::npos);
			VASSERT_EQ(label + "Data", binaryData, received.second);

			// The message object itself is not modified
			VASSERT_EQ(label + "Not modified", "binary", msg->getBody()->getEncoding().generate());
		}
	}

	// If the conversion is disabled, a part declared as binary cannot be sent
	// to a server which does not support BINARYMIME
	void testBINARYMIME_conversionDisabled() {

		vmime::net::smtp::SMTPSendOptions options;
		options.setConvertParts(false);

		vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME", "CHUNKING" });

		VASSERT_THROW(
			"1",
			tr->send(createTypedTestMessage("application/octet-stream", "binary", getBinaryTestData()), NULL, options),
			vmime::net::smtp::SMTPBINARYMIMEExtensionNotSupportedException
		);

		VASSERT_TRUE("2 No command sent", eightBitSMTPTestSocket::getCommands().empty());
		VASSERT_TRUE("3 Still connected", tr->isConnected());
	}

	// A message cannot be encoded (RFC 2046, section 5.2.1), so a message/rfc822
	// part which contains binary data needs BINARYMIME
	void testBINARYMIME_messagePart() {

		vmime::shared_ptr <vmime::message> attached =
			createTypedTestMessage("application/octet-stream", "binary", getBinaryTestData());

		// With BINARYMIME
		{
			vmime::shared_ptr <vmime::message> msg = create8bitTestMessage("7bit", "See the attached message.\r\n");
			vmime::attachmentHelper::addAttachment(msg, attached);

			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "BINARYMIME", "CHUNKING" });

			tr->send(msg);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

			VASSERT_EQ("1.1", "MAIL FROM:<expediteur@test.vmime.org> BODY=BINARYMIME", commands[0]);
			VASSERT_EQ("1.2", 1, eightBitSMTPTestSocket::getMessages().size());
		}

		// Without BINARYMIME
		{
			vmime::shared_ptr <vmime::message> msg = create8bitTestMessage("7bit", "See the attached message.\r\n");
			vmime::attachmentHelper::addAttachment(msg, attached);

			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME", "CHUNKING" });

			VASSERT_THROW("2.1", tr->send(msg), vmime::net::smtp::SMTPBINARYMIMEExtensionNotSupportedException);
			VASSERT_TRUE("2.2 No command sent", eightBitSMTPTestSocket::getCommands().empty());
		}
	}

	// Parts declared as 7bit or 8bit which contain binary data are not valid
	// (RFC 2045, sections 2.7 and 2.8): they are encoded, even if the server
	// supports BINARYMIME
	void testBINARYMIME_undeclaredBinaryPart() {

		const vmime::string longLine = vmime::string(1500, 'x') + "\r\n";

		// Text with a long line: Quoted-Printable
		{
			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "BINARYMIME", "CHUNKING" });

			tr->send(createTypedTestMessage("text/plain", "7bit", longLine));

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("1.1", "MAIL FROM:<expediteur@test.vmime.org>", commands[0]);
			VASSERT_EQ("1.2", 1, messages.size());

			const std::pair <vmime::string, vmime::string> received = parseReceivedMessage(messages[0]);

			VASSERT("1.3", received.first.find("Content-Transfer-Encoding: quoted-printable\r\n") != vmime::string::npos);
			VASSERT_EQ("1.4", longLine, received.second);
		}

		// Other data with NUL: Base64
		{
			static const char raw[] = "Binary\0data\r\n";
			const vmime::string data(raw, sizeof(raw) - 1);

			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "BINARYMIME", "CHUNKING" });

			tr->send(createTypedTestMessage("application/octet-stream", "7bit", data));

			const std::pair <vmime::string, vmime::string> received =
				parseReceivedMessage(eightBitSMTPTestSocket::getMessages().at(0));

			VASSERT("2.1", received.first.find("Content-Transfer-Encoding: base64\r\n") != vmime::string::npos);
			VASSERT_EQ("2.2", data, received.second);
		}

		// Conversion disabled
		{
			vmime::net::smtp::SMTPSendOptions options;
			options.setConvertParts(false);

			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "BINARYMIME", "CHUNKING" });

			VASSERT_THROW(
				"3.1",
				tr->send(createTypedTestMessage("text/plain", "7bit", longLine), NULL, options),
				vmime::net::smtp::SMTPUndeclaredBinaryDataException
			);

			VASSERT_TRUE("3.2 No command sent", eightBitSMTPTestSocket::getCommands().empty());
		}
	}

	// Contents which can only be read once cannot be checked before the
	// message is sent: their undeclared binary data is found while it is sent
	void testBINARYMIME_undeclaredBinaryStream() {

		static const char raw[] = "Binary\0data\r\n";

		vmime::shared_ptr <vmime::message> msg = create8bitTestMessage("7bit", "Hello\r\n");

		msg->getBody()->setContents(
			vmime::make_shared <vmime::streamContentHandler>(
				vmime::make_shared <readOnceInputStream>(vmime::string(raw, sizeof(raw) - 1)),
				sizeof(raw) - 1
			)
		);

		vmime::shared_ptr <vmime::net::transport> tr =
			create8bitTransport({ "8BITMIME", "BINARYMIME", "CHUNKING" });

		VASSERT_THROW("1", tr->send(msg), vmime::net::smtp::SMTPUndeclaredBinaryDataException);

		const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

		VASSERT_EQ("2", 3, commands.size());
		VASSERT_EQ("3", "RSET", commands[2]);
		VASSERT_TRUE("4 Not delivered", eightBitSMTPTestSocket::getMessages().empty());
		VASSERT_TRUE("5 Still connected", tr->isConnected());
	}

	// Raw message data declared as binary is sent as is with BINARYMIME
	void testBINARYMIME_rawData() {

		const vmime::string data = "Subject: Test\r\n\r\n" + getBinaryTestData();

		vmime::net::smtp::SMTPSendOptions options;
		options.setBinaryMessage(true);

		// With BINARYMIME
		{
			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "BINARYMIME", "CHUNKING" });

			send8bitTestData(tr, data, options);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("1.1", "MAIL FROM:<expediteur@test.vmime.org> BODY=BINARYMIME", commands[0]);
			VASSERT_EQ("1.2", "BDAT " + std::to_string(data.length()) + " LAST", commands[2]);
			VASSERT_EQ("1.3", 1, messages.size());
			VASSERT_EQ("1.4", data, messages[0]);  // line endings not converted
		}

		// Without BINARYMIME: nothing is sent
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME", "CHUNKING" });

			VASSERT_THROW(
				"2.1",
				send8bitTestData(tr, data, options),
				vmime::net::smtp::SMTPBINARYMIMEExtensionNotSupportedException
			);

			VASSERT_TRUE("2.2 No command sent", eightBitSMTPTestSocket::getCommands().empty());
			VASSERT_TRUE("2.3 Still connected", tr->isConnected());
		}
	}

	// Undeclared binary data in raw message data is found while it is sent
	void testBINARYMIME_undeclaredRawData() {

		const vmime::string binaryData = "Subject: Test\r\n\r\n" + getBinaryTestData();
		const vmime::string longLineData = "Subject: Test\r\n\r\n" + vmime::string(1500, 'x') + "\r\n";

		// With BINARYMIME: the transfer is cancelled, and the message can be
		// sent again as binary on the same connection
		for (const vmime::string& data : { binaryData, longLineData }) {

			vmime::shared_ptr <vmime::net::transport> tr =
				create8bitTransport({ "8BITMIME", "BINARYMIME", "CHUNKING", "PIPELINING" });

			VASSERT_THROW(
				"1.1",
				send8bitTestData(tr, data),
				vmime::net::smtp::SMTPUndeclaredBinaryDataException
			);

			const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();
			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("1.2", 3, commands.size());
			VASSERT_EQ("1.3", "RSET", commands[2]);
			VASSERT_TRUE("1.4 Not delivered", messages.empty());
			VASSERT_TRUE("1.5 Still connected", tr->isConnected());

			vmime::net::smtp::SMTPSendOptions options;
			options.setBinaryMessage(true);

			send8bitTestData(tr, data, options);

			VASSERT_EQ("1.6", 1, messages.size());
			VASSERT_EQ("1.7", data, messages[0]);
		}

		// Without BINARYMIME: the transfer is cancelled, but the message
		// cannot be sent
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME", "CHUNKING" });

			VASSERT_THROW(
				"2.1",
				send8bitTestData(tr, binaryData),
				vmime::net::smtp::SMTPBINARYMIMEExtensionNotSupportedException
			);

			VASSERT_EQ("2.2", "RSET", eightBitSMTPTestSocket::getCommands().at(2));
			VASSERT_TRUE("2.3 Not delivered", eightBitSMTPTestSocket::getMessages().empty());
			VASSERT_TRUE("2.4 Still connected", tr->isConnected());
		}

		// With DATA, which cannot be cancelled: the connection is closed
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME" });

			VASSERT_THROW(
				"3.1",
				send8bitTestData(tr, binaryData),
				vmime::net::smtp::SMTPBINARYMIMEExtensionNotSupportedException
			);

			VASSERT_TRUE("3.2 Not delivered", eightBitSMTPTestSocket::getMessages().empty());
			VASSERT_FALSE("3.3 Disconnected", tr->isConnected());
		}
	}

	// A forwarded message which contains 8-bit data is declared as 8-bit
	// (RFC 2046, section 5.2.1), so the message is known to be 8-bit
	// before it is sent
	void test8BITMIME_messageAttachment() {

		vmime::shared_ptr <vmime::message> msg =
			create8bitTestMessage("7bit", "See the attached message.\r\n");

		vmime::attachmentHelper::addAttachment(
			msg, create8bitTestMessage("8bit", "Gr\xc3\xbc\xc3\x9f" "e\r\n")
		);

		vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME", "CHUNKING" });

		tr->send(msg);

		const std::vector <vmime::string>& commands = eightBitSMTPTestSocket::getCommands();

		VASSERT_EQ("1", "MAIL FROM:<expediteur@test.vmime.org> BODY=8BITMIME", commands[0]);
		VASSERT_EQ("2", 1, eightBitSMTPTestSocket::getMessages().size());
	}

	// Return whether CR and LF only appear as CRLF
	static bool hasCanonicalLineEndings(const vmime::string& data) {

		for (size_t i = 0 ; i < data.length() ; ++i) {

			if (data[i] == '\n' && (i == 0 || data[i - 1] != '\r')) {
				return false;
			} else if (data[i] == '\r' && (i + 1 == data.length() || data[i + 1] != '\n')) {
				return false;
			}
		}

		return true;
	}

	// Send a message with CHUNKING, and return the size declared in the
	// MAIL command and the size actually sent
	static std::pair <size_t, size_t> sendAndGetSizes(const vmime::shared_ptr <vmime::message>& msg) {

		vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({ "8BITMIME", "CHUNKING", "SIZE" });

		tr->send(msg);

		const vmime::string& mail = eightBitSMTPTestSocket::getCommands()[0];
		const size_t pos = mail.find(" SIZE=");

		VASSERT("SIZE", pos != vmime::string::npos);

		return std::make_pair(
			static_cast <size_t>(std::stoul(mail.substr(pos + 6))),
			eightBitSMTPTestSocket::getMessages().at(0).length()
		);
	}

	// The declared size of a message should not be lower than its actual size
	// (RFC 1870, section 6), even when the contents of a part are copied with
	// their line endings converted to CRLF
	void testSize_NotLowerThanActual() {

		vmime::string body;

		for (int i = 0 ; i < 1000 ; ++i) {
			body += "Line\n";
		}

		const std::pair <size_t, size_t> sizes = sendAndGetSizes(create8bitTestMessage("7bit", body));

		VASSERT(
			"Declared " + std::to_string(sizes.first) + ", actual " + std::to_string(sizes.second),
			sizes.first >= sizes.second
		);

		// Attachment encoded in Base64 by VMime, with lines cut
		vmime::string data;

		for (size_t i = 0 ; i < 100000 ; ++i) {
			data += static_cast <char>((i * 7919) % 256);
		}

		vmime::messageBuilder mb;
		mb.setExpeditor(vmime::mailbox("expediteur@test.vmime.org"));
		mb.getRecipients().appendAddress(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));
		mb.setSubject(vmime::text("Test"));
		mb.getTextPart()->setText(vmime::make_shared <vmime::stringContentHandler>("Hello\r\n"));
		mb.appendAttachment(
			vmime::make_shared <vmime::defaultAttachment>(
				vmime::make_shared <vmime::stringContentHandler>(data),
				vmime::mediaType("application/octet-stream")
			)
		);

		const std::pair <size_t, size_t> sizes2 = sendAndGetSizes(mb.construct());

		VASSERT(
			"Declared " + std::to_string(sizes2.first) + ", actual " + std::to_string(sizes2.second),
			sizes2.first >= sizes2.second
		);
	}

	// Lines are sent with CRLF (RFC 5321, section 2.3.8), whatever the
	// convention used by the message (eg. LF only, as stored on Unix)
	void testCanonicalLineEndings() {

		// Raw data
		{
			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport({});

			send8bitTestData(tr, "Subject: Test\n\nLine 1\nLine 2\n");

			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("1.1", 1, messages.size());
			VASSERT_EQ("1.2", "Subject: Test\r\n\r\nLine 1\r\nLine 2\r\n", messages[0]);
		}

		// Message object, with DATA then with CHUNKING
		for (int chunking = 0 ; chunking < 2 ; ++chunking) {

			std::vector <vmime::string> extensions;

			if (chunking) {
				extensions.push_back("CHUNKING");
			}

			vmime::shared_ptr <vmime::net::transport> tr = create8bitTransport(extensions);

			tr->send(create8bitTestMessage("7bit", "Line 1\nLine 2\n"));

			const std::vector <vmime::string>& messages = eightBitSMTPTestSocket::getMessages();

			VASSERT_EQ("2.1", 1, messages.size());
			VASSERT("2.2", hasCanonicalLineEndings(messages[0]));
			VASSERT("2.3", messages[0].find("\r\n\r\nLine 1\r\nLine 2\r\n") != vmime::string::npos);
		}
	}

	// Extension keywords are case-insensitive (RFC 5321, section 2.4)
	void testExtensionKeywordsCase() {

		mixedCaseEHLOSMTPTestSocket::reset();

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();
		session->getProperties()["transport.smtp.options.need-authentication"] = true;
		session->getProperties()["transport.smtp.auth.allow-plaintext"] = true;

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://user:pass@localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <mixedCaseEHLOSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		// Authentication fails if "auth" is not recognized
		VASSERT_NO_THROW("Connect", tr->connect());
		VASSERT_EQ("AUTH", "AUTH PLAIN AHVzZXIAcGFzcw==", mixedCaseEHLOSMTPTestSocket::getAuthLine());

		vmime::shared_ptr <vmime::net::smtp::SMTPConnection> conn =
			vmime::dynamicCast <vmime::net::smtp::SMTPTransport>(tr)->getConnection();

		VASSERT_TRUE("PIPELINING", conn->hasExtension("PIPELINING"));

		std::vector <vmime::string> params;

		VASSERT_TRUE("SIZE", conn->hasExtension("size", &params));
		VASSERT_EQ("SIZE params", 1, params.size());
		VASSERT_EQ("SIZE param", "1000000", params[0]);

		tr->disconnect();
	}

	void testReconnectAfterTimeout() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport();
		tr->connect();

		failingSMTPTestSocket::failOn("NOOP");

		VASSERT_THROW("NOOP", tr->noop(), vmime::exceptions::operation_timed_out);

		// Connection is not usable anymore after a time-out
		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("Connection closed", 0, failingSMTPTestSocket::getConnectedCount());

		failingSMTPTestSocket::failOn("");

		VASSERT_NO_THROW("Reconnect", tr->connect());
		VASSERT_TRUE("Connected", tr->isConnected());
		VASSERT_NO_THROW("NOOP", tr->noop());

		VASSERT_NO_THROW("Disconnect", tr->disconnect());
		VASSERT_EQ("All connections closed", 0, failingSMTPTestSocket::getConnectedCount());
	}

	void testDisconnectAfterTimeout() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport();
		tr->connect();

		failingSMTPTestSocket::failOn("NOOP");

		VASSERT_THROW("NOOP", tr->noop(), vmime::exceptions::operation_timed_out);

		VASSERT_NO_THROW("Disconnect", tr->disconnect());
		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("All connections closed", 0, failingSMTPTestSocket::getConnectedCount());
	}

	void testSendTimeout() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport();
		tr->connect();

		failingSMTPTestSocket::failOn("RCPT");

		VASSERT_THROW("Send", sendTestMessage(tr), vmime::exceptions::operation_timed_out);

		// Connection is not usable anymore after a time-out
		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("Connection closed", 0, failingSMTPTestSocket::getConnectedCount());

		failingSMTPTestSocket::failOn("");

		VASSERT_NO_THROW("Reconnect", tr->connect());
		VASSERT_NO_THROW("Send after reconnection", sendTestMessage(tr));

		VASSERT_NO_THROW("Disconnect", tr->disconnect());
		VASSERT_EQ("All connections closed", 0, failingSMTPTestSocket::getConnectedCount());
	}

	void testConnectionLostDuringEnvelope() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport();
		tr->connect();

		// Connection is lost after MAIL: sending RCPT fails
		failingSMTPTestSocket::failOn("MAIL", failingSMTPTestSocket::FAILURE_CONNECTION_LOST);

		VASSERT_THROW("Send", sendTestMessage(tr), vmime::exceptions::socket_exception);

		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("Connection closed", 0, failingSMTPTestSocket::getConnectedCount());
	}

	void testConnectionLostDuringData() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport();
		tr->connect();

		failingSMTPTestSocket::failOn("DATA", failingSMTPTestSocket::FAILURE_CONNECTION_LOST);

		VASSERT_THROW("Send", sendTestMessage(tr), vmime::exceptions::socket_exception);

		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("Connection closed", 0, failingSMTPTestSocket::getConnectedCount());
	}

	void testConnectionLostDuringChunking() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport(/* chunking */ true);
		tr->connect();

		failingSMTPTestSocket::failOn("BDAT", failingSMTPTestSocket::FAILURE_CONNECTION_LOST);

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <SMTPTestMessage>();

		VASSERT_THROW("Send", tr->send(msg, exp, recips), vmime::exceptions::socket_exception);

		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("Connection closed", 0, failingSMTPTestSocket::getConnectedCount());
	}

	void testInputStreamErrorDuringData() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport();
		tr->connect();

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		failingInputStream is("Message data\r\n");

		VASSERT_THROW("Send", tr->send(exp, recips, is, 0), vmime::exception);

		// Server still expects the end of the message data: connection is not usable
		// anymore, and the incomplete message must not be delivered
		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("Connection closed", 0, failingSMTPTestSocket::getConnectedCount());
		VASSERT_EQ("Message not delivered", 0, failingSMTPTestSocket::getDeliveredCount());
	}

	// Raw message data is also sent by chunks
	void testInputStreamErrorDuringChunking() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport(/* chunking */ true);
		tr->connect();

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		failingInputStream is("Message data\r\n");

		VASSERT_THROW("Send", tr->send(exp, recips, is, 0), vmime::exception);

		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("Connection closed", 0, failingSMTPTestSocket::getConnectedCount());
		VASSERT_EQ("Message not delivered", 0, failingSMTPTestSocket::getDeliveredCount());
	}

	void testGenerationErrorDuringChunking() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport(/* chunking */ true);
		tr->connect();

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::shared_ptr <vmime::message> msg = vmime::make_shared <failingMessage>();

		VASSERT_THROW("Send", tr->send(msg, exp, recips), vmime::exception);

		VASSERT_FALSE("Connected", tr->isConnected());
		VASSERT_EQ("Connection closed", 0, failingSMTPTestSocket::getConnectedCount());
		VASSERT_EQ("Message not delivered", 0, failingSMTPTestSocket::getDeliveredCount());
	}

	static void connectSASL(const vmime::string& password, const bool allowPlaintext = true) {

		SASLSMTPTestSocket::reset();

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();
		session->getProperties()["transport.smtp.options.need-authentication"] = true;
		session->getProperties()["transport.smtp.auth.allow-plaintext"] = allowPlaintext;

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://user:" + password + "@localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <SASLSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		tr->connect();
		tr->disconnect();
	}

	// Return the Base64-encoded response of the PLAIN mechanism
	static vmime::string encodePLAINResponse(const vmime::string& password) {

		const vmime::string resp = vmime::string(1, '\0') + "user" + vmime::string(1, '\0') + password;

		return vmime::security::sasl::SASLContext::create()->encodeB64(
			reinterpret_cast <const vmime::byte_t*>(resp.data()), resp.length()
		);
	}

	// Issue #319: initial response is sent with the AUTH command
	void testSASLInitialResponse() {

		connectSASL("pass");

		const std::vector <vmime::string>& lines = SASLSMTPTestSocket::getAuthLines();

		VASSERT_EQ("Count", 1, lines.size());
		VASSERT_EQ("AUTH", "AUTH PLAIN AHVzZXIAcGFzcw==", lines[0]);
	}

	// RFC 4954: the AUTH command with the initial response fits in a
	// command line (512 octets, RFC 5321)
	void testSASLInitialResponse_MaxLength() {

		const vmime::string password(366, 'p');

		connectSASL(password);

		const std::vector <vmime::string>& lines = SASLSMTPTestSocket::getAuthLines();

		VASSERT_EQ("Count", 1, lines.size());
		VASSERT_EQ("AUTH", "AUTH PLAIN " + encodePLAINResponse(password), lines[0]);
		VASSERT_EQ("Length", 509, lines[0].length() + 2);  // including CRLF
	}

	// RFC 4954: the initial response must not be sent with the AUTH command
	// if the command would exceed 512 octets
	void testSASLInitialResponse_TooLong() {

		const vmime::string password(367, 'p');

		connectSASL(password);

		const std::vector <vmime::string>& lines = SASLSMTPTestSocket::getAuthLines();

		VASSERT_EQ("Count", 2, lines.size());
		VASSERT_EQ("AUTH", "AUTH PLAIN", lines[0]);
		VASSERT_EQ("Response", encodePLAINResponse(password), lines[1]);
		VASSERT_EQ("Length", 513, lines[0].length() + 1 + lines[1].length() + 2);  // with initial response
	}

	// PLAIN must not be used on a connection which is not secured (RFC 4616, section 1)
	void testSASL_PlaintextNotAllowed() {

		VASSERT_THROW(
			"Connect",
			connectSASL("pass", /* allowPlaintext */ false),
			vmime::exceptions::plaintext_authentication_not_allowed
		);

		VASSERT_EQ("Count", 0, SASLSMTPTestSocket::getAuthLines().size());
	}

	static void connectLOGIN(const vmime::string& mechs) {

		LOGINSMTPTestSocket::reset(mechs);

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();
		session->getProperties()["transport.smtp.options.need-authentication"] = true;
		session->getProperties()["transport.smtp.auth.allow-plaintext"] = true;

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://user:pass@localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <LOGINSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		tr->connect();
		tr->disconnect();
	}

	// Some servers only support LOGIN (and XOAUTH2, which is not registered here)
	void testSASL_LOGIN() {

		connectLOGIN("LOGIN XOAUTH2");

		const std::vector <vmime::string>& lines = LOGINSMTPTestSocket::getAuthLines();

		VASSERT_EQ("Count", 3, lines.size());
		VASSERT_EQ("AUTH", "AUTH LOGIN", lines[0]);
		VASSERT_EQ("Username", "dXNlcg==", lines[1]);
		VASSERT_EQ("Password", "cGFzcw==", lines[2]);
	}

	// Clients SHOULD use PLAIN whenever offered by a server (draft-murchison-sasl-login)
	void testSASL_PLAINPreferredToLOGIN() {

		connectLOGIN("LOGIN PLAIN");

		const std::vector <vmime::string>& lines = LOGINSMTPTestSocket::getAuthLines();

		VASSERT_EQ("Count", 1, lines.size());
		VASSERT_EQ("AUTH", "AUTH PLAIN AHVzZXIAcGFzcw==", lines[0]);
	}

	// Return the Base64-encoded initial client response of the OAUTHBEARER mechanism
	static vmime::string encodeOAUTHBEARERResponse(const vmime::string& accessToken) {

		const vmime::string resp = "n,a=user@vmime.org,\x01" "auth=Bearer " + accessToken + "\x01\x01";

		return vmime::security::sasl::SASLContext::create()->encodeB64(
			reinterpret_cast <const vmime::byte_t*>(resp.data()), resp.length()
		);
	}

	static void connectOAUTHBEARER(
		const vmime::string& accessToken,
		const vmime::string& validAccessToken = "vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg=="
	) {

		// The OAUTHBEARER mechanism is not registered by default
		vmime::security::sasl::SASLMechanismFactory::getInstance()->
			registerMechanism <vmime::security::sasl::OAuthBearerSASLMechanism>("OAUTHBEARER");

		OAUTHBEARERSMTPTestSocket::reset(encodeOAUTHBEARERResponse(validAccessToken));

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();
		session->getProperties()["transport.smtp.options.need-authentication"] = true;
		session->getProperties()["transport.smtp.auth.allow-plaintext"] = true;
		session->getProperties()["transport.smtp.auth.username"] = "user@vmime.org";
		session->getProperties()["transport.smtp.auth.accesstoken"] = accessToken;

		vmime::shared_ptr <vmime::net::transport> tr = session->getTransport(
			vmime::utility::url("smtp://localhost"),
			vmime::make_shared <vmime::security::sasl::OAuth2SASLAuthenticator>(
				vmime::security::sasl::OAuth2SASLAuthenticator::MODE_EXCLUSIVE
			)
		);

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <OAUTHBEARERSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		tr->connect();
		tr->disconnect();
	}

	// RFC 7628, section 3.1: initial client response
	void testSASL_OAUTHBEARER() {

		connectOAUTHBEARER("vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg==");

		const std::vector <vmime::string>& lines = OAUTHBEARERSMTPTestSocket::getAuthLines();

		VASSERT_EQ("Count", 1, lines.size());
		VASSERT_EQ(
			"AUTH",
			"AUTH OAUTHBEARER bixhPXVzZXJAdm1pbWUub3JnLAFhdXRoPUJlYXJlciB2RjlkZnQ0cW1UYzJOdmIzUmxja0JoYkhSaGRtbHpkR0V1WTI5dENnPT0BAQ==",
			lines[0]
		);
	}

	// RFC 7628, section 3.2.3: on error, the client sends a response consisting
	// of a single %x01 character, and the server finishes the exchange
	void testSASL_OAUTHBEARER_Error() {

		VASSERT_THROW(
			"Connect",
			connectOAUTHBEARER("expired"),
			vmime::exceptions::authentication_error
		);

		const std::vector <vmime::string>& lines = OAUTHBEARERSMTPTestSocket::getAuthLines();

		VASSERT_EQ("Count", 2, lines.size());
		VASSERT_EQ("AUTH", "AUTH OAUTHBEARER " + encodeOAUTHBEARERResponse("expired"), lines[0]);
		VASSERT_EQ("Response", "AQ==", lines[1]);
	}

	// Yahoo access tokens are about 1200 characters long, so the initial response
	// is always sent after the first challenge (RFC 4954, section 4). Yahoo sends
	// a prompt ("OAuthBearer:") as this first challenge, instead of an empty one.
	void testSASL_OAUTHBEARER_Prompt() {

		const vmime::string accessToken(1200, 'Y');

		connectOAUTHBEARER(accessToken, accessToken);

		const std::vector <vmime::string>& lines = OAUTHBEARERSMTPTestSocket::getAuthLines();

		VASSERT_EQ("Count", 2, lines.size());
		VASSERT_EQ("AUTH", "AUTH OAUTHBEARER", lines[0]);
		VASSERT_EQ("Response", encodeOAUTHBEARERResponse(accessToken), lines[1]);
	}

VMIME_TEST_SUITE_END
