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

#include <atomic>
#include <chrono>
#include <thread>


using namespace vmime::net::imap;


/** IMAP test server with 3 messages in the INBOX. Message 2 is expunged
  * by EXPUNGE, and also by NOOP, which then reports a new message. As
  * allowed by RFC 9051, no EXISTS response follows EXPUNGE.
  *
  * The root folder contains "Inbox" (not in upper case) and "Archive".
  */
class folderIMAPTestSocket : public lineBasedTestSocket {

public:

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

		if (verb == "LOGIN") {

			localSend(tag + " OK LOGIN completed\r\n");

		} else if (verb == "CAPABILITY") {

			localSend("* CAPABILITY IMAP4rev1\r\n");
			localSend(tag + " OK CAPABILITY completed\r\n");

		} else if (verb == "LIST" && cmd.find('%') != vmime::string::npos) {

			// Sub-folders of the root folder
			localSend("* LIST (\\HasNoChildren) \"/\" \"Inbox\"\r\n");
			localSend("* LIST (\\HasNoChildren) \"/\" \"Archive\"\r\n");
			localSend(tag + " OK LIST completed\r\n");

		} else if (verb == "LIST") {

			localSend("* LIST (\\Noselect) \"/\" \"\"\r\n");
			localSend(tag + " OK LIST completed\r\n");

		} else if (verb == "SELECT") {

			localSend("* 3 EXISTS\r\n");
			localSend("* 0 RECENT\r\n");
			localSend("* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)\r\n");
			localSend("* OK [UIDVALIDITY 1] UIDs valid\r\n");
			localSend(tag + " OK [READ-WRITE] SELECT completed\r\n");

		} else if (verb == "EXPUNGE") {

			localSend("* 2 EXPUNGE\r\n");
			localSend(tag + " OK EXPUNGE completed\r\n");

		} else if (verb == "NOOP") {

			localSend("* 2 EXPUNGE\r\n");
			localSend("* 3 EXISTS\r\n");
			localSend(tag + " OK NOOP completed\r\n");

		} else if (verb == "LOGOUT") {

			localSend("* BYE test.vmime.org IMAP4rev1 server logging out\r\n");
			localSend(tag + " OK LOGOUT completed\r\n");

		} else {

			localSend(tag + " BAD Command not recognized\r\n");
		}

		processCommand();
	}
};


/** IMAP test server which supports IDLE (RFC 2177), with 3 messages in
  * the INBOX. The responses to IDLE and DONE are set with static members,
  * as the test cannot access the socket created by the factory.
  */
class idleIMAPTestSocket : public lineBasedTestSocket {

public:

	idleIMAPTestSocket(const vmime::shared_ptr <vmime::net::timeoutHandler>& th)
		: m_timeoutHandler(th),
		  m_receiveCount(0) {

	}

	static void reset() {

		sm_capabilities = "IMAP4rev1 IDLE";
		sm_rejectIdle = false;
		sm_endIdle = false;
		sm_idleResponses.clear();
		sm_delayedIdleResponses.clear();
		sm_doneResponses.clear();
		sm_idleCount = 0;
		sm_doneCount = 0;
		sm_idleReceived = false;
	}

	void onConnected() {

		localSend("* OK test.vmime.org IMAP4rev1 server ready\r\n");
	}

	// Simulate responses sent by the server after the client has been
	// waiting for some time
	void receive(vmime::string& buffer) {

		if (!m_delayedResponses.empty() && ++m_receiveCount == 5) {

			localSend(m_delayedResponses);
			m_delayedResponses.clear();
		}

		lineBasedTestSocket::receive(buffer);
	}

	// Like a real socket, check the time-out handler while waiting
	bool waitForRead(const int /* msecs */) {

		if (m_timeoutHandler && m_timeoutHandler->isTimeOut()) {

			if (!m_timeoutHandler->handleTimeOut()) {
				throw vmime::exceptions::operation_timed_out();
			}

			m_timeoutHandler->resetTimeOut();
		}

		return true;
	}

	void processCommand() {

		if (!haveMoreLines()) {
			return;
		}

		const vmime::string line = getNextLine();

		// Terminates IDLE (this line has no tag)
		if (line == "DONE") {

			++sm_doneCount;

			localSend(sm_doneResponses);
			localSend(m_idleTag + " OK IDLE terminated\r\n");

			processCommand();
			return;
		}

		const vmime::size_t sp = line.find(' ');

		const vmime::string tag = line.substr(0, sp);
		const vmime::string cmd = line.substr(sp + 1);

		const vmime::string verb =
			vmime::utility::stringUtils::toUpper(cmd.substr(0, cmd.find(' ')));

		if (verb == "LOGIN") {

			localSend(tag + " OK LOGIN completed\r\n");

		} else if (verb == "CAPABILITY") {

			localSend("* CAPABILITY " + sm_capabilities + "\r\n");
			localSend(tag + " OK CAPABILITY completed\r\n");

		} else if (verb == "LIST") {

			localSend("* LIST (\\Noselect) \"/\" \"\"\r\n");
			localSend(tag + " OK LIST completed\r\n");

		} else if (verb == "SELECT") {

			localSend("* 3 EXISTS\r\n");
			localSend("* 0 RECENT\r\n");
			localSend("* OK [UIDVALIDITY 1] UIDs valid\r\n");
			localSend(tag + " OK [READ-WRITE] SELECT completed\r\n");

		} else if (verb == "IDLE") {

			++sm_idleCount;
			sm_idleReceived = true;

			if (sm_rejectIdle) {

				localSend(tag + " BAD IDLE not allowed\r\n");

			} else {

				m_idleTag = tag;
				m_delayedResponses = sm_delayedIdleResponses;
				m_receiveCount = 0;

				localSend("+ idling\r\n");
				localSend(sm_idleResponses);

				if (sm_endIdle) {
					localSend(tag + " OK IDLE terminated by server\r\n");
				}
			}

		} else if (verb == "LOGOUT") {

			localSend("* BYE test.vmime.org IMAP4rev1 server logging out\r\n");
			localSend(tag + " OK LOGOUT completed\r\n");

		} else {

			localSend(tag + " BAD Command not recognized\r\n");
		}

		processCommand();
	}

	static vmime::string sm_capabilities;
	static bool sm_rejectIdle;                    // reject the IDLE command
	static bool sm_endIdle;                       // terminate IDLE without waiting for DONE
	static vmime::string sm_idleResponses;        // sent after the continuation request
	static vmime::string sm_delayedIdleResponses; // sent later, while idling
	static vmime::string sm_doneResponses;        // sent after DONE, before the tagged response
	static int sm_idleCount;
	static int sm_doneCount;
	static std::atomic <bool> sm_idleReceived;

private:

	vmime::shared_ptr <vmime::net::timeoutHandler> m_timeoutHandler;

	vmime::string m_idleTag;
	vmime::string m_delayedResponses;
	int m_receiveCount;
};


/** Creates idleIMAPTestSocket objects, which use the time-out handler.
  */
class idleIMAPTestSocketFactory : public vmime::net::socketFactory {

public:

	vmime::shared_ptr <vmime::net::socket> create() {

		return vmime::make_shared <idleIMAPTestSocket>(vmime::null);
	}

	vmime::shared_ptr <vmime::net::socket> create(
		const vmime::shared_ptr <vmime::net::timeoutHandler>& th
	) {

		return vmime::make_shared <idleIMAPTestSocket>(th);
	}
};


vmime::string idleIMAPTestSocket::sm_capabilities;
bool idleIMAPTestSocket::sm_rejectIdle = false;
bool idleIMAPTestSocket::sm_endIdle = false;
vmime::string idleIMAPTestSocket::sm_idleResponses;
vmime::string idleIMAPTestSocket::sm_delayedIdleResponses;
vmime::string idleIMAPTestSocket::sm_doneResponses;
int idleIMAPTestSocket::sm_idleCount = 0;
int idleIMAPTestSocket::sm_doneCount = 0;
std::atomic <bool> idleIMAPTestSocket::sm_idleReceived(false);


/** Creates time-out handlers with a delay of 2 seconds.
  */
class shortTimeoutHandlerFactory : public vmime::net::timeoutHandlerFactory {

public:

	vmime::shared_ptr <vmime::net::timeoutHandler> create() {

		return vmime::make_shared <testTimeoutHandler>(2);
	}
};


/** Records the numbers of the messages added and removed.
  */
class recordingMessageCountListener : public vmime::net::events::messageCountListener {

public:

	void messagesAdded(const vmime::shared_ptr <vmime::net::events::messageCountEvent>& event) {

		added.push_back(event->getNumbers());
	}

	void messagesRemoved(const vmime::shared_ptr <vmime::net::events::messageCountEvent>& event) {

		removed.push_back(event->getNumbers());
	}

	std::vector <std::vector <size_t> > added;
	std::vector <std::vector <size_t> > removed;
};


/** Records the numbers of the messages changed.
  */
class recordingMessageChangedListener : public vmime::net::events::messageChangedListener {

public:

	void messageChanged(const vmime::shared_ptr <vmime::net::events::messageChangedEvent>& event) {

		changed.push_back(event->getNumbers());
	}

	std::vector <std::vector <size_t> > changed;
};



VMIME_TEST_SUITE_BEGIN(IMAPFolderTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testExpunge_MessageCount)
		VMIME_TEST(testExpunge_NewMessage)
		VMIME_TEST(testInbox_SpecialUse)
		VMIME_TEST(testInbox_Rename)
		VMIME_TEST(testIdle_NewMessage)
		VMIME_TEST(testIdle_ExpungeAndFlags)
		VMIME_TEST(testIdle_DelayedChange)
		VMIME_TEST(testIdle_KeepAliveAndTimeout)
		VMIME_TEST(testIdle_TimeoutHandler)
		VMIME_TEST(testIdle_EndedByServer)
		VMIME_TEST(testIdle_NotSupported)
		VMIME_TEST(testIdle_NotOpen)
		VMIME_TEST(testIdle_Rejected)
		VMIME_TEST(testIdle_Bye)
		VMIME_TEST(testIdle_InterruptBeforeIdle)
		VMIME_TEST(testIdle_InterruptFromOtherThread)
	VMIME_TEST_LIST_END


	// Connect to the test server. The store is kept in m_store, as folders
	// only hold a weak reference to it.
	template <typename SOCKET_FACTORY = testSocketFactory <folderIMAPTestSocket> >
	void connect(
		const vmime::shared_ptr <vmime::net::timeoutHandlerFactory>& thf =
			vmime::make_shared <testTimeoutHandlerFactory>()
	) {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.imap.auth.allow-plaintext"] = true;

		m_store = vmime::dynamicCast <IMAPStore>(
			sess->getStore(vmime::utility::url("imap://user:pass@localhost"))
		);

		m_store->setSocketFactory(vmime::make_shared <SOCKET_FACTORY>());
		m_store->setTimeoutHandlerFactory(thf);
		m_store->connect();
	}

	// Connect to the test server and open the INBOX
	template <typename SOCKET_FACTORY = testSocketFactory <folderIMAPTestSocket> >
	vmime::shared_ptr <IMAPFolder> openFolder(
		const vmime::shared_ptr <vmime::net::timeoutHandlerFactory>& thf =
			vmime::make_shared <testTimeoutHandlerFactory>()
	) {

		connect <SOCKET_FACTORY>(thf);

		vmime::shared_ptr <IMAPFolder> folder =
			vmime::dynamicCast <IMAPFolder>(m_store->getDefaultFolder());

		folder->open(vmime::net::folder::MODE_READ_WRITE);

		return folder;
	}


	void testExpunge_MessageCount() {

		vmime::shared_ptr <IMAPFolder> folder = openFolder();

		recordingMessageCountListener listener;
		folder->addMessageCountListener(&listener);

		VASSERT_EQ("Count before", 3, folder->getMessageCount());

		folder->expunge();

		VASSERT_EQ("Count after", 2, folder->getMessageCount());

		VASSERT_EQ("Removed events", 1, listener.removed.size());
		VASSERT_EQ("Removed count", 1, listener.removed[0].size());
		VASSERT_EQ("Removed number", 2, listener.removed[0][0]);
		VASSERT_EQ("Added events", 0, listener.added.size());

		folder->removeMessageCountListener(&listener);
		folder->close(false);
		m_store->disconnect();
	}

	void testExpunge_NewMessage() {

		vmime::shared_ptr <IMAPFolder> folder = openFolder();

		recordingMessageCountListener listener;
		folder->addMessageCountListener(&listener);

		// Message 2 is expunged, then a new message arrives (number 3)
		folder->noop();

		VASSERT_EQ("Count", 3, folder->getMessageCount());

		VASSERT_EQ("Removed events", 1, listener.removed.size());
		VASSERT_EQ("Removed count", 1, listener.removed[0].size());
		VASSERT_EQ("Removed number", 2, listener.removed[0][0]);

		VASSERT_EQ("Added events", 1, listener.added.size());
		VASSERT_EQ("Added count", 1, listener.added[0].size());
		VASSERT_EQ("Added number", 3, listener.added[0][0]);

		folder->removeMessageCountListener(&listener);
		folder->close(false);
		m_store->disconnect();
	}

	// INBOX is case-insensitive (RFC 9051, section 5.1)
	void testInbox_SpecialUse() {

		connect();

		const std::vector <vmime::shared_ptr <vmime::net::folder> > folders =
			m_store->getRootFolder()->getFolders(false);

		VASSERT_EQ("Count", 2, folders.size());

		VASSERT_EQ("Name 1", "Inbox", folders[0]->getName().getBuffer());
		VASSERT_EQ(
			"Special use 1",
			vmime::net::folderAttributes::SPECIALUSE_INBOX,
			folders[0]->getAttributes().getSpecialUse()
		);

		VASSERT_EQ("Name 2", "Archive", folders[1]->getName().getBuffer());
		VASSERT_EQ(
			"Special use 2",
			vmime::net::folderAttributes::SPECIALUSE_NONE,
			folders[1]->getAttributes().getSpecialUse()
		);

		m_store->disconnect();
	}

	void testInbox_Rename() {

		connect();

		vmime::shared_ptr <vmime::net::folder> inbox = m_store->getFolder(
			vmime::net::folder::path::component("inbox", vmime::charsets::UTF_8)
		);

		VASSERT_THROW(
			"Rename",
			inbox->rename(vmime::net::folder::path::component("Other", vmime::charsets::UTF_8)),
			vmime::exceptions::illegal_operation
		);

		m_store->disconnect();
	}

	void testIdle_NewMessage() {

		idleIMAPTestSocket::reset();
		idleIMAPTestSocket::sm_idleResponses = "* 4 EXISTS\r\n";

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		recordingMessageCountListener listener;
		folder->addMessageCountListener(&listener);

		VASSERT_EQ("Result", IMAPFolder::IDLE_CHANGES, folder->idle(10));

		VASSERT_EQ("IDLE", 1, idleIMAPTestSocket::sm_idleCount);
		VASSERT_EQ("DONE", 1, idleIMAPTestSocket::sm_doneCount);

		VASSERT_EQ("Count", 4, folder->getMessageCount());

		VASSERT_EQ("Added events", 1, listener.added.size());
		VASSERT_EQ("Added count", 1, listener.added[0].size());
		VASSERT_EQ("Added number", 4, listener.added[0][0]);
		VASSERT_EQ("Removed events", 0, listener.removed.size());

		folder->removeMessageCountListener(&listener);
		folder->close(false);
		m_store->disconnect();
	}

	// Changes reported while idling and after DONE are all processed
	void testIdle_ExpungeAndFlags() {

		idleIMAPTestSocket::reset();
		idleIMAPTestSocket::sm_idleResponses = "* 2 EXPUNGE\r\n";
		idleIMAPTestSocket::sm_doneResponses = "* 1 FETCH (FLAGS (\\Seen))\r\n";

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		recordingMessageCountListener countListener;
		folder->addMessageCountListener(&countListener);

		recordingMessageChangedListener changedListener;
		folder->addMessageChangedListener(&changedListener);

		VASSERT_EQ("Result", IMAPFolder::IDLE_CHANGES, folder->idle(10));

		VASSERT_EQ("Count", 2, folder->getMessageCount());

		VASSERT_EQ("Removed events", 1, countListener.removed.size());
		VASSERT_EQ("Removed number", 2, countListener.removed[0][0]);
		VASSERT_EQ("Added events", 0, countListener.added.size());

		VASSERT_EQ("Changed events", 1, changedListener.changed.size());
		VASSERT_EQ("Changed number", 1, changedListener.changed[0][0]);

		folder->removeMessageChangedListener(&changedListener);
		folder->removeMessageCountListener(&countListener);
		folder->close(false);
		m_store->disconnect();
	}

	// The change is not received with the continuation request
	void testIdle_DelayedChange() {

		idleIMAPTestSocket::reset();
		idleIMAPTestSocket::sm_delayedIdleResponses = "* 4 EXISTS\r\n";

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		recordingMessageCountListener listener;
		folder->addMessageCountListener(&listener);

		VASSERT_EQ("Result", IMAPFolder::IDLE_CHANGES, folder->idle(10));

		VASSERT_EQ("DONE", 1, idleIMAPTestSocket::sm_doneCount);
		VASSERT_EQ("Added events", 1, listener.added.size());
		VASSERT_EQ("Added number", 4, listener.added[0][0]);

		folder->removeMessageCountListener(&listener);
		folder->close(false);
		m_store->disconnect();
	}

	// An untagged status response (eg. keep-alive) does not end IDLE
	void testIdle_KeepAliveAndTimeout() {

		idleIMAPTestSocket::reset();
		idleIMAPTestSocket::sm_idleResponses = "* OK Still here\r\n";

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		recordingMessageCountListener listener;
		folder->addMessageCountListener(&listener);

		VASSERT_EQ("Result", IMAPFolder::IDLE_TIMEOUT, folder->idle(1));

		VASSERT_EQ("DONE", 1, idleIMAPTestSocket::sm_doneCount);
		VASSERT_EQ("Count", 3, folder->getMessageCount());
		VASSERT_EQ("Added events", 0, listener.added.size());
		VASSERT_EQ("Removed events", 0, listener.removed.size());

		folder->removeMessageCountListener(&listener);
		folder->close(false);
		m_store->disconnect();
	}

	// Waiting longer than the time-out handler delay is not a time-out
	void testIdle_TimeoutHandler() {

		idleIMAPTestSocket::reset();

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>(
			vmime::make_shared <shortTimeoutHandlerFactory>()
		);

		VASSERT_EQ("Result", IMAPFolder::IDLE_TIMEOUT, folder->idle(3));
		VASSERT_EQ("DONE", 1, idleIMAPTestSocket::sm_doneCount);

		folder->close(false);
		m_store->disconnect();
	}

	// The server sends the tagged response without waiting for DONE
	void testIdle_EndedByServer() {

		idleIMAPTestSocket::reset();
		idleIMAPTestSocket::sm_endIdle = true;

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		VASSERT_EQ("Result", IMAPFolder::IDLE_TIMEOUT, folder->idle(10));
		VASSERT_EQ("DONE", 0, idleIMAPTestSocket::sm_doneCount);

		folder->close(false);
		m_store->disconnect();
	}

	void testIdle_NotSupported() {

		idleIMAPTestSocket::reset();
		idleIMAPTestSocket::sm_capabilities = "IMAP4rev1";

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		VASSERT_THROW("Not supported", folder->idle(10), vmime::exceptions::operation_not_supported);
		VASSERT_EQ("IDLE", 0, idleIMAPTestSocket::sm_idleCount);

		folder->close(false);
		m_store->disconnect();
	}

	void testIdle_NotOpen() {

		idleIMAPTestSocket::reset();

		connect <idleIMAPTestSocketFactory>();

		vmime::shared_ptr <IMAPFolder> folder =
			vmime::dynamicCast <IMAPFolder>(m_store->getDefaultFolder());

		VASSERT_THROW("Not open", folder->idle(10), vmime::exceptions::illegal_state);
		VASSERT_EQ("IDLE", 0, idleIMAPTestSocket::sm_idleCount);

		m_store->disconnect();
	}

	void testIdle_Rejected() {

		idleIMAPTestSocket::reset();
		idleIMAPTestSocket::sm_rejectIdle = true;

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		VASSERT_THROW("Rejected", folder->idle(10), vmime::exceptions::command_error);

		folder->close(false);
		m_store->disconnect();
	}

	void testIdle_Bye() {

		idleIMAPTestSocket::reset();
		idleIMAPTestSocket::sm_idleResponses = "* BYE Server shutting down\r\n";

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		VASSERT_THROW("BYE", folder->idle(10), vmime::exceptions::command_error);
		VASSERT_EQ("DONE", 0, idleIMAPTestSocket::sm_doneCount);

		m_store->disconnect();
	}

	// An interruption requested before idle() is not lost
	void testIdle_InterruptBeforeIdle() {

		idleIMAPTestSocket::reset();

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		folder->interruptIdle();

		VASSERT_EQ("Result 1", IMAPFolder::IDLE_INTERRUPTED, folder->idle(10));
		VASSERT_EQ("IDLE 1", 0, idleIMAPTestSocket::sm_idleCount);

		// The interruption has been consumed
		idleIMAPTestSocket::sm_idleResponses = "* 4 EXISTS\r\n";

		VASSERT_EQ("Result 2", IMAPFolder::IDLE_CHANGES, folder->idle(10));
		VASSERT_EQ("IDLE 2", 1, idleIMAPTestSocket::sm_idleCount);

		folder->close(false);
		m_store->disconnect();
	}

	void testIdle_InterruptFromOtherThread() {

		idleIMAPTestSocket::reset();

		vmime::shared_ptr <IMAPFolder> folder = openFolder <idleIMAPTestSocketFactory>();

		const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

		IMAPFolder::IdleResult result = IMAPFolder::IDLE_CHANGES;
		bool failed = false;

		std::thread idleThread([&]() {

			try {
				result = folder->idle(60);
			} catch (...) {
				failed = true;
			}
		});

		// Wait until the server has received IDLE
		for (int i = 0 ; i < 1000 && !idleIMAPTestSocket::sm_idleReceived ; ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		folder->interruptIdle();

		idleThread.join();

		const std::chrono::steady_clock::duration elapsed = std::chrono::steady_clock::now() - start;

		VASSERT("No error", !failed);
		VASSERT_EQ("Result", IMAPFolder::IDLE_INTERRUPTED, result);
		VASSERT_EQ("IDLE", 1, idleIMAPTestSocket::sm_idleCount);
		VASSERT_EQ("DONE", 1, idleIMAPTestSocket::sm_doneCount);
		VASSERT("Elapsed", elapsed < std::chrono::seconds(30));

		folder->close(false);
		m_store->disconnect();
	}

private:

	vmime::shared_ptr <IMAPStore> m_store;

VMIME_TEST_SUITE_END
