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
		VMIME_TEST(testReconnectAfterTimeout)
		VMIME_TEST(testDisconnectAfterTimeout)
		VMIME_TEST(testSendTimeout)
		VMIME_TEST(testConnectionLostDuringEnvelope)
		VMIME_TEST(testConnectionLostDuringData)
		VMIME_TEST(testConnectionLostDuringChunking)
		VMIME_TEST(testInputStreamErrorDuringData)
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


	static vmime::shared_ptr <vmime::net::transport> createFailingTransport() {

		failingSMTPTestSocket::reset();

		vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

		vmime::shared_ptr <vmime::net::transport> tr =
			session->getTransport(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::make_shared <testSocketFactory <failingSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

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

	void testSMTPUTF8_available() {

		// Test with UTF8 sender
		{
			vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

			vmime::shared_ptr <vmime::net::transport> tr =
				session->getTransport(vmime::utility::url("smtp://localhost"));

			tr->setSocketFactory(vmime::make_shared <testSocketFactory <UTF8SMTPTestSocket <true> > >());
			tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

			VASSERT_NO_THROW("Connection", tr->connect());

			vmime::mailbox exp(
				vmime::emailAddress(
					vmime::word("expéditeur", vmime::charsets::UTF_8),
					vmime::word("test.vmime.org")
				)
			);

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient1@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient2@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>(
				vmime::emailAddress(
					vmime::word("récepteur", vmime::charsets::UTF_8),
					vmime::word("test.vmime.org")
				)
			));

			vmime::string data("Message data");
			vmime::utility::inputStreamStringAdapter is(data);

			tr->send(exp, recips, is, 0);
		}

		// Test with UTF8 recipient only
		{
			vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

			vmime::shared_ptr <vmime::net::transport> tr =
				session->getTransport(vmime::utility::url("smtp://localhost"));

			tr->setSocketFactory(vmime::make_shared <testSocketFactory <UTF8SMTPTestSocket <true> > >());
			tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

			VASSERT_NO_THROW("Connection", tr->connect());

			vmime::mailbox exp("expediteur@test.vmime.org");

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient1@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient2@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>(
				vmime::emailAddress(
					vmime::word("récepteur", vmime::charsets::UTF_8),
					vmime::word("test.vmime.org")
				)
			));

			vmime::string data("Message data");
			vmime::utility::inputStreamStringAdapter is(data);

			tr->send(exp, recips, is, 0);
		}

		// Test with UTF8 display name only (non-ASCII word followed by an ASCII word)
		{
			vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

			vmime::shared_ptr <vmime::net::transport> tr =
				session->getTransport(vmime::utility::url("smtp://localhost"));

			tr->setSocketFactory(vmime::make_shared <testSocketFactory <UTF8SMTPTestSocket <true, false> > >());
			tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

			VASSERT_NO_THROW("Connection", tr->connect());

			vmime::text name;
			name.appendWord(vmime::make_shared <vmime::word>("Expéditeur", vmime::charsets::UTF_8));
			name.appendWord(vmime::make_shared <vmime::word>(" Test", vmime::charsets::US_ASCII));

			vmime::mailbox exp(name, vmime::emailAddress("expediteur@test.vmime.org"));

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient1@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient2@test.vmime.org"));

			vmime::string data("Message data");
			vmime::utility::inputStreamStringAdapter is(data);

			tr->send(exp, recips, is, 0);
		}
	}

	void testSMTPUTF8_notAvailable() {

		// Test with UTF8 sender
		{
			vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

			vmime::shared_ptr <vmime::net::transport> tr =
				session->getTransport(vmime::utility::url("smtp://localhost"));

			tr->setSocketFactory(vmime::make_shared <testSocketFactory <UTF8SMTPTestSocket <false> > >());
			tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

			VASSERT_NO_THROW("Connection", tr->connect());

			vmime::mailbox exp(
				vmime::emailAddress(
					vmime::word("expéditeur", vmime::charsets::UTF_8),
					vmime::word("test.vmime.org")
				)
			);

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient1@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient2@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>(
				vmime::emailAddress(
					vmime::word("récepteur", vmime::charsets::UTF_8),
					vmime::word("test.vmime.org")
				)
			));

			vmime::string data("Message data");
			vmime::utility::inputStreamStringAdapter is(data);

			tr->send(exp, recips, is, 0);
		}

		// Test with UTF8 recipient only
		{
			vmime::shared_ptr <vmime::net::session> session = vmime::net::session::create();

			vmime::shared_ptr <vmime::net::transport> tr =
				session->getTransport(vmime::utility::url("smtp://localhost"));

			tr->setSocketFactory(vmime::make_shared <testSocketFactory <UTF8SMTPTestSocket <false> > >());
			tr->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

			VASSERT_NO_THROW("Connection", tr->connect());

			vmime::mailbox exp("expediteur@test.vmime.org");

			vmime::mailboxList recips;
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient1@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>("recipient2@test.vmime.org"));
			recips.appendMailbox(vmime::make_shared <vmime::mailbox>(
				vmime::emailAddress(
					vmime::word("récepteur", vmime::charsets::UTF_8),
					vmime::word("test.vmime.org")
				)
			));

			vmime::string data("Message data");
			vmime::utility::inputStreamStringAdapter is(data);

			tr->send(exp, recips, is, 0);
		}
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

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport();
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

	void testGenerationErrorDuringChunking() {

		vmime::shared_ptr <vmime::net::transport> tr = createFailingTransport();
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
	static const vmime::string encodePLAINResponse(const vmime::string& password) {

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
	static const vmime::string encodeOAUTHBEARERResponse(const vmime::string& accessToken) {

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
