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

#include <algorithm>
#include <map>
#include <sstream>


using namespace vmime::net::imap;


/** IMAP test server which holds two messages: a multipart message
  * with a base64-encoded attachment (message 1) and a single part
  * message (message 2).
  *
  * FETCH supports the items used by IMAPMessage, and body sections
  * are served from the raw message data.
  */
class messageIMAPTestSocket : public lineBasedTestSocket {

public:

	messageIMAPTestSocket() {

		// Message 1: multipart/mixed
		const vmime::string header1 =
			"From: from@vmime.org\r\n"
			"To: to@vmime.org\r\n"
			"Subject: Multipart\r\n"
			"MIME-Version: 1.0\r\n"
			"Content-Type: multipart/mixed; boundary=\"BOUNDARY\"\r\n"
			"\r\n";

		const vmime::string part1Header =
			"Content-Type: text/plain; charset=us-ascii\r\n"
			"Content-Transfer-Encoding: 7bit\r\n"
			"\r\n";

		const vmime::string part1Body = "Hello";

		const vmime::string part2Header =
			"Content-Type: text/html; charset=us-ascii\r\n"
			"Content-Transfer-Encoding: 7bit\r\n"
			"\r\n";

		const vmime::string part2Body = "<p>Hello</p>";

		const vmime::string part3Header =
			"Content-Type: application/octet-stream; name=\"hello.bin\"\r\n"
			"Content-Transfer-Encoding: base64\r\n"
			"\r\n";

		const vmime::string part3Body = "SGVsbG8gV29ybGQh";  // "Hello World!"

		const vmime::string text1 =
			"--BOUNDARY\r\n" + part1Header + part1Body + "\r\n"
			"--BOUNDARY\r\n" + part2Header + part2Body + "\r\n"
			"--BOUNDARY\r\n" + part3Header + part3Body + "\r\n"
			"--BOUNDARY--\r\n";

		m_structure[1] =
			"((\"TEXT\" \"PLAIN\" (\"CHARSET\" \"us-ascii\") NIL NIL \"7BIT\" 5 0 NIL NIL NIL NIL)"
			"(\"TEXT\" \"HTML\" (\"CHARSET\" \"us-ascii\") NIL NIL \"7BIT\" 12 0 NIL NIL NIL NIL)"
			"(\"APPLICATION\" \"OCTET-STREAM\" (\"NAME\" \"hello.bin\") NIL NIL \"BASE64\" 16 NIL NIL NIL NIL)"
			" \"MIXED\" (\"BOUNDARY\" \"BOUNDARY\") NIL NIL NIL)";

		m_sections[1][""] = header1 + text1;
		m_sections[1]["HEADER"] = header1;
		m_sections[1]["TEXT"] = text1;
		m_sections[1]["1.MIME"] = part1Header;
		m_sections[1]["1"] = part1Body;
		m_sections[1]["2.MIME"] = part2Header;
		m_sections[1]["2"] = part2Body;
		m_sections[1]["3.MIME"] = part3Header;
		m_sections[1]["3"] = part3Body;

		// Message 2: text/plain
		const vmime::string header2 =
			"From: from@vmime.org\r\n"
			"To: to@vmime.org\r\n"
			"Subject: Single part\r\n"
			"MIME-Version: 1.0\r\n"
			"Content-Type: text/plain; charset=us-ascii\r\n"
			"Content-Transfer-Encoding: 7bit\r\n"
			"\r\n";

		const vmime::string text2 = "Hello\r\n";

		m_structure[2] =
			"(\"TEXT\" \"PLAIN\" (\"CHARSET\" \"us-ascii\") NIL NIL \"7BIT\" 7 1 NIL NIL NIL NIL)";

		m_sections[2][""] = header2 + text2;
		m_sections[2]["HEADER"] = header2;
		m_sections[2]["TEXT"] = text2;
		m_sections[2]["1"] = text2;
	}

	/** Return the data of a body section, as sent by the server.
	  *
	  * @param num message number
	  * @param section section specifier (eg. "", "HEADER", "1.MIME")
	  */
	const vmime::string& getSection(const int num, const vmime::string& section) const {

		return m_sections.at(num).at(section);
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
		vmime::string cmd = line.substr(sp + 1);

		// UIDs are the same as message numbers
		if (cmd.compare(0, 4, "UID ") == 0) {
			cmd = cmd.substr(4);
		}

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

		} else if (verb == "SELECT" || verb == "EXAMINE") {

			localSend("* 2 EXISTS\r\n");
			localSend("* 0 RECENT\r\n");
			localSend("* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)\r\n");
			localSend("* OK [UIDVALIDITY 1] UIDs valid\r\n");
			localSend(tag + " OK [READ-WRITE] " + verb + " completed\r\n");

		} else if (verb == "FETCH") {

			processFetch(tag, cmd.substr(cmd.find(' ') + 1));

		} else if (verb == "LOGOUT") {

			localSend("* BYE test.vmime.org IMAP4rev1 server logging out\r\n");
			localSend(tag + " OK LOGOUT completed\r\n");

		} else {

			localSend(tag + " BAD Command not recognized\r\n");
		}

		processCommand();
	}

private:

	void processFetch(const vmime::string& tag, const vmime::string& args) {

		// "num ITEM" or "num (ITEM1 ITEM2...)": the tests fetch only one
		// message at a time, and never request items containing spaces
		const vmime::size_t sp = args.find(' ');
		const int num = std::stoi(args.substr(0, sp));

		vmime::string list = args.substr(sp + 1);

		if (list[0] == '(') {
			list = list.substr(1, list.length() - 2);
		}

		std::istringstream items(list);

		vmime::string resp = "* " + std::to_string(num) + " FETCH (";
		vmime::string it;

		while (items >> it) {

			if (resp.back() != '(') {
				resp += " ";
			}

			if (it == "UID") {

				resp += "UID " + std::to_string(num);

			} else if (it == "FLAGS") {

				resp += "FLAGS ()";

			} else if (it == "RFC822.SIZE") {

				resp += "RFC822.SIZE " + std::to_string(m_sections[num][""].length());

			} else if (it == "RFC822.HEADER") {

				resp += "RFC822.HEADER " + literal(m_sections[num]["HEADER"]);

			} else if (it == "BODYSTRUCTURE") {

				resp += "BODYSTRUCTURE " + m_structure[num];

			} else {

				// BODY[section] or BODY.PEEK[section], optionally followed
				// by <origin.length> for a partial fetch
				const vmime::size_t start = it.find('[');
				const vmime::size_t end = it.find(']');
				const vmime::string section = it.substr(start + 1, end - start - 1);

				vmime::string data = m_sections[num][section];
				resp += "BODY[" + section + "]";

				if (end + 1 < it.length() && it[end + 1] == '<') {

					const vmime::size_t dot = it.find('.', end);
					const vmime::size_t origin = std::stoul(it.substr(end + 2, dot - end - 2));
					const vmime::size_t length = std::stoul(it.substr(dot + 1));

					data = data.substr(std::min(origin, data.length()), length);
					resp += "<" + std::to_string(origin) + ">";
				}

				resp += " " + literal(data);
			}
		}

		localSend(resp + ")\r\n");
		localSend(tag + " OK FETCH completed\r\n");
	}

	static const vmime::string literal(const vmime::string& data) {

		return "{" + std::to_string(data.length()) + "}\r\n" + data;
	}


	std::map <int, vmime::string> m_structure;
	std::map <int, std::map <vmime::string, vmime::string> > m_sections;
};



VMIME_TEST_SUITE_BEGIN(IMAPMessageTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testGetParsedMessage_Multipart)
		VMIME_TEST(testGetParsedMessage_Base64Part)
		VMIME_TEST(testGetParsedMessage_SinglePart)
		VMIME_TEST(testExtract)
		VMIME_TEST(testExtract_Partial)
		VMIME_TEST(testExtract_Peek)
		VMIME_TEST(testExtractPart)
		VMIME_TEST(testExtractPart_Partial)
		VMIME_TEST(testExtractPart_Peek)
		VMIME_TEST(testExtractPart_Root)
		VMIME_TEST(testPartContents_Extract)
		VMIME_TEST(testPartContents_ExtractRaw)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <IMAPStore> createStore() {

		vmime::shared_ptr <vmime::net::session> sess = vmime::net::session::create();
		sess->getProperties()["store.imap.auth.allow-plaintext"] = true;

		vmime::shared_ptr <IMAPStore> store = vmime::dynamicCast <IMAPStore>(
			sess->getStore(vmime::utility::url("imap://user:pass@localhost"))
		);

		store->setSocketFactory(vmime::make_shared <testSocketFactory <messageIMAPTestSocket> >());
		store->setTimeoutHandlerFactory(vmime::make_shared <testTimeoutHandlerFactory>());

		return store;
	}

	// Get the parsed message from the server, as in the bug report:
	// fetch the header, flags and size first, then call getParsedMessage()
	// and generate the message
	static const vmime::string generateParsedMessage(const vmime::size_t num) {

		vmime::shared_ptr <IMAPStore> store = createStore();
		store->connect();

		vmime::shared_ptr <vmime::net::folder> folder = store->getDefaultFolder();
		folder->open(vmime::net::folder::MODE_READ_WRITE);

		std::vector <vmime::shared_ptr <vmime::net::message> > msgs =
			folder->getMessages(vmime::net::messageSet::byNumber(num));

		folder->fetchMessages(
			msgs,
			vmime::net::fetchAttributes::FULL_HEADER |
			vmime::net::fetchAttributes::ENVELOPE |
			vmime::net::fetchAttributes::FLAGS |
			vmime::net::fetchAttributes::SIZE
		);

		const vmime::string generated = msgs[0]->getParsedMessage()->generate();

		folder->close(false);
		store->disconnect();

		return generated;
	}

	static const vmime::string extractContents(const vmime::shared_ptr <const vmime::body>& body) {

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		body->getContents()->extract(os);

		return data;
	}

	// Connect to the test server and open the default folder. The store is
	// kept in m_store, as the folder only holds a weak reference to it, and
	// the commands sent are recorded by m_tracerFactory.
	vmime::shared_ptr <vmime::net::folder> openFolder() {

		m_tracerFactory = vmime::make_shared <recordingTracerFactory>();

		m_store = createStore();
		m_store->setTracerFactory(m_tracerFactory);
		m_store->connect();

		vmime::shared_ptr <vmime::net::folder> folder = m_store->getDefaultFolder();
		folder->open(vmime::net::folder::MODE_READ_WRITE);

		return folder;
	}

	// Return whether the specified command has been sent (ignoring the tag)
	bool isCommandSent(const vmime::string& cmd) const {

		for (const auto& line : m_tracerFactory->getTracer()->getSentLines()) {

			const vmime::size_t sp = line.find(' ');

			if (sp != vmime::string::npos && line.substr(sp + 1) == cmd) {
				return true;
			}
		}

		return false;
	}

	// Return the first part (text/plain) of message 1
	static vmime::shared_ptr <const vmime::net::messagePart> getFirstPart(
		const vmime::shared_ptr <vmime::net::message>& msg
	) {

		return msg->getStructure()->getPartAt(0)->getStructure()->getPartAt(0);
	}


	void testGetParsedMessage_Multipart() {

		const vmime::string generated = generateParsedMessage(1);

		vmime::message msg;
		msg.parse(generated);

		VASSERT_EQ("Subject", "Multipart", msg.getHeader()->Subject()->getValue <vmime::text>()->getWholeBuffer());
		VASSERT_EQ("Part count", 3, msg.getBody()->getPartCount());

		VASSERT_EQ("Part 1", "Hello", extractContents(msg.getBody()->getPartAt(0)->getBody()));
		VASSERT_EQ("Part 2", "<p>Hello</p>", extractContents(msg.getBody()->getPartAt(1)->getBody()));
	}

	void testGetParsedMessage_Base64Part() {

		const vmime::string generated = generateParsedMessage(1);

		vmime::message msg;
		msg.parse(generated);

		VASSERT_EQ("Part count", 3, msg.getBody()->getPartCount());
		VASSERT_EQ("Part 3", "Hello World!", extractContents(msg.getBody()->getPartAt(2)->getBody()));
	}

	void testGetParsedMessage_SinglePart() {

		const vmime::string generated = generateParsedMessage(2);

		vmime::message msg;
		msg.parse(generated);

		VASSERT_EQ("From count", 1, msg.getHeader()->findAllFields("From").size());
		VASSERT_EQ("Subject", "Single part", msg.getHeader()->Subject()->getValue <vmime::text>()->getWholeBuffer());

		VASSERT_EQ("Contents", "Hello\r\n", extractContents(msg.getBody()));
	}

	void testExtract() {

		const messageIMAPTestSocket server;

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> msg = folder->getMessage(1);

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		msg->extract(os);

		VASSERT_EQ("Data", server.getSection(1, ""), data);
		VASSERT_TRUE("Command", isCommandSent("FETCH 1 BODY[]"));
	}

	void testExtract_Partial() {

		const messageIMAPTestSocket server;

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> msg = folder->getMessage(1);

		vmime::string data1;
		vmime::utility::outputStreamStringAdapter os1(data1);

		msg->extract(os1, NULL, 10, 20);

		VASSERT_EQ("Data 1", server.getSection(1, "").substr(10, 20), data1);
		VASSERT_TRUE("Command 1", isCommandSent("FETCH 1 BODY[]<10.20>"));

		// Up to the end
		vmime::string data2;
		vmime::utility::outputStreamStringAdapter os2(data2);

		msg->extract(os2, NULL, 10);

		VASSERT_EQ("Data 2", server.getSection(1, "").substr(10), data2);
		VASSERT_TRUE("Command 2", isCommandSent("FETCH 1 BODY[]<10.4294967295>"));
	}

	void testExtract_Peek() {

		const messageIMAPTestSocket server;

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> msg = folder->getMessage(1);

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		msg->extract(os, NULL, 0, -1, /* peek */ true);

		VASSERT_EQ("Data", server.getSection(1, ""), data);
		VASSERT_TRUE("Command", isCommandSent("FETCH 1 BODY.PEEK[]"));
	}

	void testExtractPart() {

		const messageIMAPTestSocket server;

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> msg = folder->getMessage(1);

		folder->fetchMessage(msg, vmime::net::fetchAttributes::STRUCTURE);

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		msg->extractPart(getFirstPart(msg), os);

		// Header + contents of the part
		VASSERT_EQ("Data", server.getSection(1, "1.MIME") + server.getSection(1, "1"), data);
		VASSERT_TRUE("Header", isCommandSent("FETCH 1 BODY[1.MIME]"));
		VASSERT_TRUE("Contents", isCommandSent("FETCH 1 BODY[1]"));
	}

	// Partial fetch across the header and the contents of the part
	void testExtractPart_Partial() {

		const messageIMAPTestSocket server;

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> msg = folder->getMessage(1);

		folder->fetchMessage(msg, vmime::net::fetchAttributes::STRUCTURE);

		const vmime::string part = server.getSection(1, "1.MIME") + server.getSection(1, "1");
		const vmime::size_t start = server.getSection(1, "1.MIME").length() - 4;

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		msg->extractPart(getFirstPart(msg), os, NULL, start, 6);

		VASSERT_EQ("Data", part.substr(start, 6), data);
		VASSERT_TRUE("Contents", isCommandSent("FETCH 1 BODY[1]<0.2>"));
	}

	void testExtractPart_Peek() {

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> msg = folder->getMessage(1);

		folder->fetchMessage(msg, vmime::net::fetchAttributes::STRUCTURE);

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		msg->extractPart(getFirstPart(msg), os, NULL, 0, -1, /* peek */ true);

		VASSERT_TRUE("Header", isCommandSent("FETCH 1 BODY.PEEK[1.MIME]"));
		VASSERT_TRUE("Contents", isCommandSent("FETCH 1 BODY.PEEK[1]"));
	}

	// The root part is the whole message
	void testExtractPart_Root() {

		const messageIMAPTestSocket server;

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> msg = folder->getMessage(1);

		folder->fetchMessage(msg, vmime::net::fetchAttributes::STRUCTURE);

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		msg->extractPart(msg->getStructure()->getPartAt(0), os);

		VASSERT_EQ("Data", server.getSection(1, ""), data);
		VASSERT_TRUE("Command", isCommandSent("FETCH 1 BODY[]"));
	}

	// Contents of a part of the parsed message: decoded data
	void testPartContents_Extract() {

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> netMsg = folder->getMessage(1);
		vmime::shared_ptr <vmime::message> msg = netMsg->getParsedMessage();

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		msg->getBody()->getPartAt(2)->getBody()->getContents()->extract(os);

		VASSERT_EQ("Data", "Hello World!", data);
	}

	// Contents of a part of the parsed message: encoded data, without header
	void testPartContents_ExtractRaw() {

		const messageIMAPTestSocket server;

		vmime::shared_ptr <vmime::net::folder> folder = openFolder();
		vmime::shared_ptr <vmime::net::message> netMsg = folder->getMessage(1);
		vmime::shared_ptr <vmime::message> msg = netMsg->getParsedMessage();

		vmime::string data;
		vmime::utility::outputStreamStringAdapter os(data);

		msg->getBody()->getPartAt(2)->getBody()->getContents()->extractRaw(os);

		VASSERT_EQ("Data", server.getSection(1, "3"), data);
	}

private:

	vmime::shared_ptr <IMAPStore> m_store;
	vmime::shared_ptr <recordingTracerFactory> m_tracerFactory;

VMIME_TEST_SUITE_END
