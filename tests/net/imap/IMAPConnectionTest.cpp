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

#include "vmime/net/imap/IMAPStore.hpp"
#include "vmime/net/imap/IMAPFolder.hpp"


using namespace vmime::net::imap;


/** IMAP test server which simulates a network failure.
  *
  * When the client sends the command set with failOn(), the server
  * stops responding and any further read on the socket throws an
  * exception, as a real socket would. Depending on the failure mode,
  * writing to the socket may fail as well, or the connection may be
  * closed by the server.
  *
  * The number of sockets currently connected is tracked, to check
  * that all connections are properly closed by the client.
  */
class failingIMAPTestSocket : public lineBasedTestSocket {

public:

	enum FailureMode {
		FAILURE_NO_RESPONSE,        /**< Server stops responding (reads time out). */
		FAILURE_DEAD_CONNECTION,    /**< Reads and writes time out. */
		FAILURE_CONNECTION_CLOSED   /**< Connection is closed by the server. */
	};

	using lineBasedTestSocket::send;

	failingIMAPTestSocket()
		: m_failed(false) {

	}

	static void reset() {

		sm_failCommand.clear();
		sm_failureMode = FAILURE_NO_RESPONSE;
		sm_connectedCount = 0;
	}

	static void failOn(const vmime::string& verb, const FailureMode mode) {

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

		if (m_failed && sm_failureMode == FAILURE_DEAD_CONNECTION) {
			throw vmime::exceptions::operation_timed_out();
		}

		lineBasedTestSocket::send(buffer);
	}

	void receive(vmime::string& buffer) {

		lineBasedTestSocket::receive(buffer);

		if (buffer.empty() && m_failed) {

			if (sm_failureMode == FAILURE_CONNECTION_CLOSED) {
				throw vmime::exceptions::socket_exception("Connection closed");
			} else {
				throw vmime::exceptions::operation_timed_out();
			}
		}
	}

	void onConnected() {

		localSend("* OK test.vmime.org IMAP4rev1 server ready\r\n");
	}

	void processCommand() {

		if (!haveMoreLines()) {
			return;
		}

		const vmime::string line = getNextLine();
		const vmime::size_t sp = line.find(' ');

		const vmime::string tag = line.substr(0, sp);
		const vmime::string cmd = line.substr(sp + 1);

		const vmime::string verb =
			vmime::utility::stringUtils::toUpper(cmd.substr(0, cmd.find(' ')));

		if (m_failed) {

			// Server does not respond anymore

		} else if (verb == sm_failCommand) {

			m_failed = true;

			if (sm_failureMode == FAILURE_CONNECTION_CLOSED) {
				disconnect();
			}

		} else if (verb == "LOGIN") {

			localSend(tag + " OK LOGIN completed\r\n");

		} else if (verb == "CAPABILITY") {

			localSend("* CAPABILITY IMAP4rev1\r\n");
			localSend(tag + " OK CAPABILITY completed\r\n");

		} else if (verb == "LIST") {

			localSend("* LIST (\\Noselect) \"/\" \"\"\r\n");
			localSend(tag + " OK LIST completed\r\n");

		} else if (verb == "SELECT" || verb == "EXAMINE") {

			localSend("* 0 EXISTS\r\n");
			localSend("* 0 RECENT\r\n");
			localSend("* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)\r\n");
			localSend("* OK [UIDVALIDITY 1] UIDs valid\r\n");
			localSend(tag + " OK [READ-WRITE] " + verb + " completed\r\n");

		} else if (verb == "NOOP") {

			localSend(tag + " OK NOOP completed\r\n");

		} else if (verb == "LOGOUT") {

			localSend("* BYE test.vmime.org IMAP4rev1 server logging out\r\n");
			localSend(tag + " OK LOGOUT completed\r\n");

		} else {

			localSend(tag + " BAD Command not recognized\r\n");
		}

		processCommand();
	}

private:

	bool m_failed;

	static vmime::string sm_failCommand;
	static FailureMode sm_failureMode;
	static int sm_connectedCount;
};


vmime::string failingIMAPTestSocket::sm_failCommand;
failingIMAPTestSocket::FailureMode failingIMAPTestSocket::sm_failureMode =
	failingIMAPTestSocket::FAILURE_NO_RESPONSE;
int failingIMAPTestSocket::sm_connectedCount = 0;



VMIME_TEST_SUITE_BEGIN(IMAPConnectionTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testFolderOpenTimeout)
		VMIME_TEST(testFolderCloseAfterTimeout)
		VMIME_TEST(testFolderDestroyedAfterTimeout)
		VMIME_TEST(testFolderDestroyedAfterConnectionLost)
		VMIME_TEST(testStoreDisconnectAfterTimeout)
		VMIME_TEST(testStoreDestroyedBeforeFolder)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <IMAPStore> createStore() {

		failingIMAPTestSocket::reset();

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();

		vmime::shared_ptr <IMAPStore> store = vmime::dynamicCast <IMAPStore>(
			sess->getStore(vmime::utility::url("imap://user:pass@localhost"))
		);

		store->setSocketFactory(vmime::make_shared <testSocketFactory <failingIMAPTestSocket> >());
		store->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		return store;
	}


	// Issue #234: time-out after the folder connection has been opened
	void testFolderOpenTimeout() {

		vmime::shared_ptr <IMAPStore> store = createStore();
		store->connect();

		vmime::shared_ptr <vmime::net::folder> folder = store->getDefaultFolder();

		failingIMAPTestSocket::failOn("SELECT", failingIMAPTestSocket::FAILURE_NO_RESPONSE);

		VASSERT_THROW(
			"Open",
			folder->open(vmime::net::folder::MODE_READ_WRITE),
			vmime::exceptions::operation_timed_out
		);

		VASSERT_FALSE("Open", folder->isOpen());
		VASSERT_EQ("Folder connection closed", 1, failingIMAPTestSocket::getConnectedCount());

		folder = vmime::null;

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingIMAPTestSocket::getConnectedCount());
	}

	void testFolderCloseAfterTimeout() {

		vmime::shared_ptr <IMAPStore> store = createStore();
		store->connect();

		vmime::shared_ptr <IMAPFolder> folder =
			vmime::dynamicCast <IMAPFolder>(store->getDefaultFolder());

		folder->open(vmime::net::folder::MODE_READ_WRITE);

		failingIMAPTestSocket::failOn("NOOP", failingIMAPTestSocket::FAILURE_DEAD_CONNECTION);

		VASSERT_THROW("NOOP", folder->noop(), vmime::exceptions::operation_timed_out);

		VASSERT_NO_THROW("Close", folder->close(false));
		VASSERT_FALSE("Open", folder->isOpen());
		VASSERT_EQ("Folder connection closed", 1, failingIMAPTestSocket::getConnectedCount());

		folder = vmime::null;

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingIMAPTestSocket::getConnectedCount());
	}

	void testFolderDestroyedAfterTimeout() {

		vmime::shared_ptr <IMAPStore> store = createStore();
		store->connect();

		vmime::shared_ptr <IMAPFolder> folder =
			vmime::dynamicCast <IMAPFolder>(store->getDefaultFolder());

		folder->open(vmime::net::folder::MODE_READ_WRITE);

		failingIMAPTestSocket::failOn("NOOP", failingIMAPTestSocket::FAILURE_DEAD_CONNECTION);

		VASSERT_THROW("NOOP", folder->noop(), vmime::exceptions::operation_timed_out);

		// Folder must unregister itself from the store, even if it
		// could not be closed properly
		folder = vmime::null;

		VASSERT_EQ("Folder connection closed", 1, failingIMAPTestSocket::getConnectedCount());

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingIMAPTestSocket::getConnectedCount());
	}

	void testFolderDestroyedAfterConnectionLost() {

		vmime::shared_ptr <IMAPStore> store = createStore();
		store->connect();

		vmime::shared_ptr <IMAPFolder> folder =
			vmime::dynamicCast <IMAPFolder>(store->getDefaultFolder());

		folder->open(vmime::net::folder::MODE_READ_WRITE);

		failingIMAPTestSocket::failOn("NOOP", failingIMAPTestSocket::FAILURE_CONNECTION_CLOSED);

		VASSERT_THROW("NOOP", folder->noop(), vmime::exceptions::socket_exception);

		// Folder cannot be closed, but it must unregister itself from
		// the store anyway (store would access a deleted object otherwise)
		folder = vmime::null;

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_EQ("All connections closed", 0, failingIMAPTestSocket::getConnectedCount());
	}

	void testStoreDisconnectAfterTimeout() {

		vmime::shared_ptr <IMAPStore> store = createStore();
		store->connect();

		failingIMAPTestSocket::failOn("NOOP", failingIMAPTestSocket::FAILURE_DEAD_CONNECTION);

		VASSERT_THROW("NOOP", store->noop(), vmime::exceptions::operation_timed_out);

		VASSERT_NO_THROW("Disconnect", store->disconnect());
		VASSERT_FALSE("Connected", store->isConnected());
		VASSERT_EQ("All connections closed", 0, failingIMAPTestSocket::getConnectedCount());
	}

	void testStoreDestroyedBeforeFolder() {

		vmime::shared_ptr <IMAPStore> store = createStore();
		store->connect();

		vmime::shared_ptr <vmime::net::folder> folder = store->getDefaultFolder();
		folder->open(vmime::net::folder::MODE_READ_WRITE);

		store = vmime::null;

		VASSERT_THROW("Status", folder->getStatus(), vmime::exceptions::illegal_state);

		folder = vmime::null;

		VASSERT_EQ("All connections closed", 0, failingIMAPTestSocket::getConnectedCount());
	}

VMIME_TEST_SUITE_END
