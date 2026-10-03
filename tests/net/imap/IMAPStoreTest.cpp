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
#include "vmime/net/imap/IMAPConnection.hpp"


using namespace vmime::net::imap;


/** IMAP test server.
  *
  * Supports the ID extension (RFC 2971) and records the commands
  * sent by the client (without the tag).
  */
class IDIMAPTestSocket : public lineBasedTestSocket {

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

		m_commands.push_back(cmd);

		const vmime::string verb =
			vmime::utility::stringUtils::toUpper(cmd.substr(0, cmd.find(' ')));

		if (verb == "LOGIN") {

			localSend(tag + " OK LOGIN completed\r\n");

		} else if (verb == "CAPABILITY") {

			localSend(vmime::string("* CAPABILITY IMAP4rev1") + (supportsID() ? " ID" : "") + "\r\n");
			localSend(tag + " OK CAPABILITY completed\r\n");

		} else if (verb == "ID") {

			processID(tag);

		} else if (verb == "LIST") {

			localSend("* LIST (\\Noselect) \"/\" \"\"\r\n");
			localSend(tag + " OK LIST completed\r\n");

		} else if (verb == "LOGOUT") {

			localSend("* BYE test.vmime.org IMAP4rev1 server logging out\r\n");
			localSend(tag + " OK LOGOUT completed\r\n");

		} else {

			localSend(tag + " BAD Command not recognized\r\n");
		}

		processCommand();
	}

	/** Returns the index of the first command starting with the
	  * specified verb, or -1 if the command has not been sent.
	  */
	int findCommand(const vmime::string& verb) const {

		for (size_t i = 0 ; i < m_commands.size() ; ++i) {

			if (vmime::utility::stringUtils::isStringEqualNoCase(
					m_commands[i].substr(0, m_commands[i].find(' ')), verb)) {

				return static_cast <int>(i);
			}
		}

		return -1;
	}

	const std::vector <vmime::string>& getCommands() const {

		return m_commands;
	}

protected:

	virtual bool supportsID() const {

		return true;
	}

	virtual void processID(const vmime::string& tag) {

		localSend("* ID (\"Name\" \"vmime-test-server\" \"version\" \"4.2\" \"vendor\" NIL)\r\n");
		localSend(tag + " OK ID completed\r\n");
	}

private:

	std::vector <vmime::string> m_commands;
};


/** IMAP test server which does not support the ID extension.
  */
class noIDIMAPTestSocket : public IDIMAPTestSocket {

protected:

	bool supportsID() const {

		return false;
	}
};


/** IMAP test server which rejects the ID command.
  */
class rejectIDIMAPTestSocket : public IDIMAPTestSocket {

protected:

	void processID(const vmime::string& tag) {

		localSend(tag + " NO ID not allowed\r\n");
	}
};



VMIME_TEST_SUITE_BEGIN(IMAPStoreTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testID)
		VMIME_TEST(testID_NoFields)
		VMIME_TEST(testID_Disabled)
		VMIME_TEST(testID_NotSupportedByServer)
		VMIME_TEST(testID_RejectedByServer)
	VMIME_TEST_LIST_END


	template <typename SOCKET>
	static vmime::shared_ptr <IMAPStore> createStore(
		const vmime::shared_ptr <vmime::net::session>& sess
	) {

		vmime::shared_ptr <IMAPStore> store = vmime::dynamicCast <IMAPStore>(
			sess->getStore(vmime::utility::url("imap://user:pass@localhost"))
		);

		store->setSocketFactory(vmime::make_shared <testSocketFactory <SOCKET> >());
		store->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		return store;
	}

	static vmime::shared_ptr <const IDIMAPTestSocket> getSocket(
		const vmime::shared_ptr <IMAPStore>& store
	) {

		return vmime::dynamicCast <const IDIMAPTestSocket>(store->getConnection()->getSocket());
	}


	void testID() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.imap.options.id"] = true;
		sess->getProperties()["store.imap.options.id.name"] = "vmime-test";
		sess->getProperties()["store.imap.options.id.version"] = "1.0";

		vmime::shared_ptr <IMAPStore> store = createStore <IDIMAPTestSocket>(sess);

		VASSERT_NO_THROW("Connect", store->connect());

		vmime::shared_ptr <const IDIMAPTestSocket> sok = getSocket(store);

		const int loginIndex = sok->findCommand("LOGIN");
		const int idIndex = sok->findCommand("ID");
		const int listIndex = sok->findCommand("LIST");

		VASSERT("ID sent", idIndex != -1);
		VASSERT("ID sent after LOGIN", loginIndex < idIndex);
		VASSERT("ID sent before LIST", idIndex < listIndex);

		VASSERT_EQ(
			"ID command",
			"ID (\"name\" \"vmime-test\" \"version\" \"1.0\")",
			sok->getCommands()[idIndex]
		);

		const std::map <vmime::string, vmime::string> serverId = store->getServerIdentification();

		VASSERT_EQ("Server ID size", 3, serverId.size());
		VASSERT_EQ("Server ID name", "vmime-test-server", serverId.at("name"));
		VASSERT_EQ("Server ID version", "4.2", serverId.at("version"));
		VASSERT_EQ("Server ID vendor", "", serverId.at("vendor"));

		store->disconnect();
	}

	void testID_NoFields() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.imap.options.id"] = true;

		vmime::shared_ptr <IMAPStore> store = createStore <IDIMAPTestSocket>(sess);

		VASSERT_NO_THROW("Connect", store->connect());

		vmime::shared_ptr <const IDIMAPTestSocket> sok = getSocket(store);

		const int idIndex = sok->findCommand("ID");

		VASSERT("ID sent", idIndex != -1);
		VASSERT_EQ("ID command", "ID NIL", sok->getCommands()[idIndex]);

		store->disconnect();
	}

	void testID_Disabled() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.imap.options.id.name"] = "vmime-test";

		vmime::shared_ptr <IMAPStore> store = createStore <IDIMAPTestSocket>(sess);

		VASSERT_NO_THROW("Connect", store->connect());

		VASSERT_EQ("ID not sent", -1, getSocket(store)->findCommand("ID"));
		VASSERT_EQ("Server ID", 0, store->getServerIdentification().size());

		store->disconnect();
	}

	void testID_NotSupportedByServer() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.imap.options.id"] = true;
		sess->getProperties()["store.imap.options.id.name"] = "vmime-test";

		vmime::shared_ptr <IMAPStore> store = createStore <noIDIMAPTestSocket>(sess);

		VASSERT_NO_THROW("Connect", store->connect());

		VASSERT_EQ("ID not sent", -1, getSocket(store)->findCommand("ID"));
		VASSERT_EQ("Server ID", 0, store->getServerIdentification().size());

		store->disconnect();
	}

	void testID_RejectedByServer() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.imap.options.id"] = true;
		sess->getProperties()["store.imap.options.id.name"] = "vmime-test";

		vmime::shared_ptr <IMAPStore> store = createStore <rejectIDIMAPTestSocket>(sess);

		// Identification is informational only: connection must succeed
		VASSERT_NO_THROW("Connect", store->connect());

		VASSERT("ID sent", getSocket(store)->findCommand("ID") != -1);
		VASSERT_EQ("Server ID", 0, store->getServerIdentification().size());

		store->disconnect();
	}

VMIME_TEST_SUITE_END
