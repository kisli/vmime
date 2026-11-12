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


/** IMAP test server with 3 messages in the INBOX. Message 2 is expunged
  * by EXPUNGE, and also by NOOP, which then reports a new message. As
  * allowed by RFC 9051, no EXISTS response follows EXPUNGE.
  */
class expungeIMAPTestSocket : public lineBasedTestSocket {

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



VMIME_TEST_SUITE_BEGIN(IMAPFolderTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testExpunge_MessageCount)
		VMIME_TEST(testExpunge_NewMessage)
	VMIME_TEST_LIST_END


	// Connect to the test server and open the INBOX. The store is kept in
	// m_store, as the folder only holds a weak reference to it.
	vmime::shared_ptr <IMAPFolder> openFolder() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.imap.auth.allow-plaintext"] = true;

		m_store = vmime::dynamicCast <IMAPStore>(
			sess->getStore(vmime::utility::url("imap://user:pass@localhost"))
		);

		m_store->setSocketFactory(vmime::make_shared <testSocketFactory <expungeIMAPTestSocket> >());
		m_store->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());
		m_store->connect();

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

private:

	vmime::shared_ptr <IMAPStore> m_store;

VMIME_TEST_SUITE_END
