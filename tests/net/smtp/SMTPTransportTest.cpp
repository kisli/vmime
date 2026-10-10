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
  * that all connections are properly closed by the client.
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
	}

	static void failOn(const vmime::string& verb, const FailureMode mode = FAILURE_NO_RESPONSE) {

		sm_failCommand = verb;
		sm_failureMode = mode;
	}

	static int getConnectedCount() {

		return sm_connectedCount;
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
};


vmime::string failingSMTPTestSocket::sm_failCommand;
failingSMTPTestSocket::FailureMode failingSMTPTestSocket::sm_failureMode =
	failingSMTPTestSocket::FAILURE_NO_RESPONSE;
int failingSMTPTestSocket::sm_connectedCount = 0;



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

VMIME_TEST_SUITE_END
