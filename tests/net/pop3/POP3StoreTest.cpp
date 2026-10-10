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

#include "tests/net/pop3/POP3TestUtils.hpp"

#include "vmime/net/pop3/POP3Store.hpp"
#include "vmime/net/pop3/POP3SStore.hpp"


/** POP3 test server which simulates a network time-out.
  *
  * When the client sends the command set with failOn(), the server
  * stops responding and any further read on the socket throws an
  * operation_timed_out exception, as a real socket would.
  *
  * The number of sockets currently connected and the number of QUIT
  * commands received are tracked, to check that all connections are
  * properly closed by the client.
  */
class failingPOP3TestSocket : public lineBasedTestSocket {

public:

	failingPOP3TestSocket()
		: m_failed(false) {

	}

	static void reset() {

		sm_failCommand.clear();
		sm_connectedCount = 0;
		sm_quitCount = 0;
	}

	static void failOn(const vmime::string& verb) {

		sm_failCommand = verb;
	}

	static int getConnectedCount() {

		return sm_connectedCount;
	}

	static int getQuitCount() {

		return sm_quitCount;
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

	void receive(vmime::string& buffer) {

		lineBasedTestSocket::receive(buffer);

		if (buffer.empty() && m_failed) {
			throw vmime::exceptions::operation_timed_out();
		}
	}

	void onConnected() {

		localSend("+OK test.vmime.org POP3 server ready\r\n");
	}

	void processCommand() {

		if (!haveMoreLines()) {
			return;
		}

		const vmime::string line = getNextLine();
		const vmime::string verb =
			vmime::utility::stringUtils::toUpper(line.substr(0, line.find(' ')));

		if (verb == "QUIT") {
			++sm_quitCount;
		}

		if (m_failed) {

			// Server does not respond anymore

		} else if (verb == sm_failCommand) {

			m_failed = true;

		} else if (verb == "USER" || verb == "PASS" || verb == "NOOP" || verb == "RSET") {

			localSend("+OK\r\n");

		} else if (verb == "STAT") {

			localSend("+OK 1 12\r\n");

		} else if (verb == "RETR") {

			localSend("+OK 12 octets\r\n");
			localSend("Message data\r\n");
			localSend(".\r\n");

		} else if (verb == "QUIT") {

			localSend("+OK test.vmime.org POP3 server signing off\r\n");

		} else {

			localSend("-ERR Command not recognized\r\n");
		}

		processCommand();
	}

private:

	bool m_failed;

	static vmime::string sm_failCommand;
	static int sm_connectedCount;
	static int sm_quitCount;
};


vmime::string failingPOP3TestSocket::sm_failCommand;
int failingPOP3TestSocket::sm_connectedCount = 0;
int failingPOP3TestSocket::sm_quitCount = 0;



VMIME_TEST_SUITE_BEGIN(POP3StoreTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testCreateFromURL)
		VMIME_TEST(testConnectToInvalidServer)
		VMIME_TEST(testDisconnect)
		VMIME_TEST(testConnectTimeout)
#if VMIME_HAVE_TLS_SUPPORT
		VMIME_TEST(testDestroyedAfterConnectFailure)
#endif // VMIME_HAVE_TLS_SUPPORT
		VMIME_TEST(testReconnectAfterConnectTimeout)
		VMIME_TEST(testReconnectAfterTimeout)
		VMIME_TEST(testDisconnectAfterTimeout)
		VMIME_TEST(testReconnectDetachesFolders)
		VMIME_TEST(testFolderCloseAfterTimeout)
		VMIME_TEST(testFolderDestroyedAfterTimeout)
		VMIME_TEST(testRetrWriteError)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::net::store> createFailingStore() {

		failingPOP3TestSocket::reset();

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.pop3.options.apop"] = false;
		sess->getProperties()["store.pop3.options.sasl"] = false;

		vmime::shared_ptr <vmime::net::store> store =
			sess->getStore(vmime::utility::url("pop3://user:pass@localhost"));

		store->setSocketFactory(vmime::make_shared <testSocketFactory <failingPOP3TestSocket> >());
		store->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		return store;
	}


	void testCreateFromURL() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();

		// POP3
		vmime::utility::url url("pop3://pop3.vmime.org");
		vmime::shared_ptr <vmime::net::store> store = sess->getStore(url);

		VASSERT_TRUE("pop3", typeid(*store) == typeid(vmime::net::pop3::POP3Store));

		// POP3S
		vmime::utility::url url2("pop3s://pop3s.vmime.org");
		vmime::shared_ptr <vmime::net::store> store2 = sess->getStore(url2);

		VASSERT_TRUE("pop3s", typeid(*store2) == typeid(vmime::net::pop3::POP3SStore));
	}

	void testConnectToInvalidServer() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();

		vmime::utility::url url("pop3://invalid-pop3-server");
		vmime::shared_ptr <vmime::net::store> store = sess->getStore(url);

		VASSERT_THROW("connect", store->connect(), vmime::exceptions::connection_error);
	}

	void testDisconnect() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();

		VASSERT_NO_THROW("Connect", store->connect());
		VASSERT_TRUE("Connected", store->isConnected());

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_FALSE("Connected", store->isConnected());

		VASSERT_EQ("QUIT sent", 1, failingPOP3TestSocket::getQuitCount());
		VASSERT_EQ("Connection closed", 0, failingPOP3TestSocket::getConnectedCount());
	}

	void testConnectTimeout() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();

		failingPOP3TestSocket::failOn("PASS");

		VASSERT_THROW("Connect", store->connect(), vmime::exceptions::operation_timed_out);
		VASSERT_FALSE("Connected", store->isConnected());

		// Connection is not usable anymore after a time-out
		VASSERT_EQ("Connection closed", 0, failingPOP3TestSocket::getConnectedCount());

		store = vmime::null;

		VASSERT_EQ("QUIT not sent", 0, failingPOP3TestSocket::getQuitCount());
	}

#if VMIME_HAVE_TLS_SUPPORT

	void testDestroyedAfterConnectFailure() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();

		store->getSession()->getProperties()["store.pop3.connection.tls"] = true;
		store->getSession()->getProperties()["store.pop3.connection.tls.required"] = true;

		// STLS is not supported by the server
		VASSERT_THROW("Connect", store->connect(), vmime::exceptions::command_error);
		VASSERT_FALSE("Connected", store->isConnected());

		// Connection is not authenticated, so it is only closed
		// when the store is destroyed
		store = vmime::null;

		VASSERT_EQ("QUIT sent", 1, failingPOP3TestSocket::getQuitCount());
		VASSERT_EQ("Connection closed", 0, failingPOP3TestSocket::getConnectedCount());
	}

#endif // VMIME_HAVE_TLS_SUPPORT

	void testReconnectAfterConnectTimeout() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();

		failingPOP3TestSocket::failOn("PASS");

		VASSERT_THROW("Connect", store->connect(), vmime::exceptions::operation_timed_out);

		failingPOP3TestSocket::failOn("");

		VASSERT_NO_THROW("Reconnect", store->connect());
		VASSERT_TRUE("Connected", store->isConnected());
		VASSERT_EQ("First connection closed", 1, failingPOP3TestSocket::getConnectedCount());

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingPOP3TestSocket::getConnectedCount());
	}

	void testReconnectAfterTimeout() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();
		store->connect();

		failingPOP3TestSocket::failOn("NOOP");

		VASSERT_THROW("NOOP", store->noop(), vmime::exceptions::operation_timed_out);

		// Connection is not usable anymore after a time-out
		VASSERT_FALSE("Connected", store->isConnected());
		VASSERT_EQ("Connection closed", 0, failingPOP3TestSocket::getConnectedCount());

		failingPOP3TestSocket::failOn("");

		VASSERT_NO_THROW("Reconnect", store->connect());
		VASSERT_TRUE("Connected", store->isConnected());
		VASSERT_NO_THROW("NOOP", store->noop());

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingPOP3TestSocket::getConnectedCount());
	}

	void testDisconnectAfterTimeout() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();
		store->connect();

		failingPOP3TestSocket::failOn("NOOP");

		VASSERT_THROW("NOOP", store->noop(), vmime::exceptions::operation_timed_out);

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_FALSE("Connected", store->isConnected());
		VASSERT_EQ("All connections closed", 0, failingPOP3TestSocket::getConnectedCount());
	}

	void testReconnectDetachesFolders() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();
		store->connect();

		vmime::shared_ptr <vmime::net::folder> folder = store->getDefaultFolder();

		failingPOP3TestSocket::failOn("NOOP");

		VASSERT_THROW("NOOP", store->noop(), vmime::exceptions::operation_timed_out);

		failingPOP3TestSocket::failOn("");

		VASSERT_NO_THROW("Reconnect", store->connect());

		// Folders obtained before reconnection belong to the previous session
		VASSERT_THROW(
			"Open old folder",
			folder->open(vmime::net::folder::MODE_READ_WRITE),
			vmime::exceptions::illegal_state
		);

		folder = store->getDefaultFolder();

		VASSERT_NO_THROW("Open new folder", folder->open(vmime::net::folder::MODE_READ_WRITE));

		folder = vmime::null;

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingPOP3TestSocket::getConnectedCount());
	}

	void testFolderCloseAfterTimeout() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();
		store->connect();

		vmime::shared_ptr <vmime::net::folder> folder = store->getDefaultFolder();
		folder->open(vmime::net::folder::MODE_READ_WRITE);

		failingPOP3TestSocket::failOn("NOOP");

		VASSERT_THROW("NOOP", store->noop(), vmime::exceptions::operation_timed_out);

		VASSERT_NO_THROW("Close", folder->close(false));
		VASSERT_FALSE("Open", folder->isOpen());

		folder = vmime::null;

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingPOP3TestSocket::getConnectedCount());
	}

	void testFolderDestroyedAfterTimeout() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();
		store->connect();

		vmime::shared_ptr <vmime::net::folder> folder = store->getDefaultFolder();
		folder->open(vmime::net::folder::MODE_READ_WRITE);

		failingPOP3TestSocket::failOn("RSET");

		// Folder must unregister itself from the store, even if it
		// could not be closed properly (store would access a deleted
		// object otherwise)
		folder = vmime::null;

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingPOP3TestSocket::getConnectedCount());
	}

	void testRetrWriteError() {

		vmime::shared_ptr <vmime::net::store> store = createFailingStore();
		store->connect();

		vmime::shared_ptr <vmime::net::folder> folder = store->getDefaultFolder();
		folder->open(vmime::net::folder::MODE_READ_WRITE);

		vmime::shared_ptr <vmime::net::message> msg = folder->getMessage(1);

		// Error while writing message data (eg. disk full)
		failingOutputStream os;

		VASSERT_THROW("Extract", msg->extract(os), vmime::exception);

		// Response has not been read completely: connection is not usable anymore
		VASSERT_FALSE("Connected", store->isConnected());
		VASSERT_EQ("Connection closed", 0, failingPOP3TestSocket::getConnectedCount());

		VASSERT_NO_THROW("Close", folder->close(false));
	}

VMIME_TEST_SUITE_END
