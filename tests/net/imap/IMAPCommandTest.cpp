//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2014 Vincent Richard <vincent@vmime.org>
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

#include "vmime/net/imap/IMAPCommand.hpp"
#include "vmime/net/imap/IMAPStore.hpp"
#include "vmime/net/imap/IMAPConnection.hpp"


using namespace vmime::net::imap;


VMIME_TEST_SUITE_BEGIN(IMAPCommandTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testCreateCommand)
		VMIME_TEST(testCreateCommandParams)
		VMIME_TEST(testLOGIN)
		VMIME_TEST(testAUTHENTICATE)
		VMIME_TEST(testAUTHENTICATE_InitialResponse)
		VMIME_TEST(testLIST)
		VMIME_TEST(testSELECT)
		VMIME_TEST(testSTATUS)
		VMIME_TEST(testCREATE)
		VMIME_TEST(testDELETE)
		VMIME_TEST(testRENAME)
		VMIME_TEST(testFETCH)
		VMIME_TEST(testSTORE)
		VMIME_TEST(testAPPEND)
		VMIME_TEST(testCOPY)
		VMIME_TEST(testSEARCH)
		VMIME_TEST(testSTARTTLS)
		VMIME_TEST(testCAPABILITY)
		VMIME_TEST(testNOOP)
		VMIME_TEST(testEXPUNGE)
		VMIME_TEST(testCLOSE)
		VMIME_TEST(testLOGOUT)
		VMIME_TEST(testSend)
	VMIME_TEST_LIST_END


	void testCreateCommand()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::createCommand("MY_COMMAND");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MY_COMMAND", cmd->getText());
	}

	void testCreateCommandParams()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::createCommand("MY_COMMAND param1 param2");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MY_COMMAND param1 param2", cmd->getText());
	}

	void testLOGIN()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::LOGIN("username", "password");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "LOGIN username password", cmd->getText());
		VASSERT_EQ("Trace Text", "LOGIN {username} {password}", cmd->getTraceText());
	}

	void testAUTHENTICATE()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::AUTHENTICATE("saslmechanism");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "AUTHENTICATE saslmechanism", cmd->getText());
	}

	void testAUTHENTICATE_InitialResponse()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::AUTHENTICATE("saslmechanism", "initial-response");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "AUTHENTICATE saslmechanism initial-response", cmd->getText());
	}

	void testLIST()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::LIST("ref-name", "mailbox-name");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "LIST ref-name mailbox-name", cmd->getText());

		vmime::shared_ptr <IMAPCommand> cmdQuote = IMAPCommand::LIST("ref name", "mailbox-name");

		VASSERT_NOT_NULL("Not null", cmdQuote);
		VASSERT_EQ("Text", "LIST \"ref name\" mailbox-name", cmdQuote->getText());
	}

	void testSELECT()
	{
		std::vector <vmime::string> params;
		params.push_back("param-1");
		params.push_back("param-2");


		vmime::shared_ptr <IMAPCommand> cmdRO = IMAPCommand::SELECT
			(/* readOnly */ true, "mailbox-name", std::vector <vmime::string>());

		VASSERT_NOT_NULL("Not null", cmdRO);
		VASSERT_EQ("Text", "EXAMINE mailbox-name", cmdRO->getText());

		vmime::shared_ptr <IMAPCommand> cmdROQuote = IMAPCommand::SELECT
			(/* readOnly */ true, "mailbox name", std::vector <vmime::string>());

		VASSERT_NOT_NULL("Not null", cmdROQuote);
		VASSERT_EQ("Text", "EXAMINE \"mailbox name\"", cmdROQuote->getText());


		vmime::shared_ptr <IMAPCommand> cmdRW = IMAPCommand::SELECT
			(/* readOnly */ false, "mailbox-name", std::vector <vmime::string>());

		VASSERT_NOT_NULL("Not null", cmdRW);
		VASSERT_EQ("Text", "SELECT mailbox-name", cmdRW->getText());

		vmime::shared_ptr <IMAPCommand> cmdRWParams = IMAPCommand::SELECT
			(/* readOnly */ false, "mailbox-name", params);

		VASSERT_NOT_NULL("Not null", cmdRWParams);
		VASSERT_EQ("Text", "SELECT mailbox-name (param-1 param-2)", cmdRWParams->getText());

		vmime::shared_ptr <IMAPCommand> cmdRWQuote = IMAPCommand::SELECT
			(/* readOnly */ false, "mailbox name", std::vector <vmime::string>());

		VASSERT_NOT_NULL("Not null", cmdRWQuote);
		VASSERT_EQ("Text", "SELECT \"mailbox name\"", cmdRWQuote->getText());
	}

	void testSTATUS()
	{
		std::vector <vmime::string> attribs;
		attribs.push_back("attrib-1");
		attribs.push_back("attrib-2");


		vmime::shared_ptr <IMAPCommand> cmd =
			IMAPCommand::STATUS("mailbox-name", attribs);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "STATUS mailbox-name (attrib-1 attrib-2)", cmd->getText());


		vmime::shared_ptr <IMAPCommand> cmdQuote =
			IMAPCommand::STATUS("mailbox name", attribs);

		VASSERT_NOT_NULL("Not null", cmdQuote);
		VASSERT_EQ("Text", "STATUS \"mailbox name\" (attrib-1 attrib-2)", cmdQuote->getText());
	}

	void testCREATE()
	{
		std::vector <vmime::string> params;
		params.push_back("param-1");
		params.push_back("param-2");


		vmime::shared_ptr <IMAPCommand> cmd =
			IMAPCommand::CREATE("mailbox-name", params);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "CREATE mailbox-name (param-1 param-2)", cmd->getText());


		vmime::shared_ptr <IMAPCommand> cmdQuote =
			IMAPCommand::CREATE("mailbox name", params);

		VASSERT_NOT_NULL("Not null", cmdQuote);
		VASSERT_EQ("Text", "CREATE \"mailbox name\" (param-1 param-2)", cmdQuote->getText());


		vmime::shared_ptr <IMAPCommand> cmdNoParam =
			IMAPCommand::CREATE("mailbox-name", std::vector <vmime::string>());

		VASSERT_NOT_NULL("Not null", cmdNoParam);
		VASSERT_EQ("Text", "CREATE mailbox-name", cmdNoParam->getText());
	}

	void testDELETE()
	{
		vmime::shared_ptr <IMAPCommand> cmd =
			IMAPCommand::DELETE("mailbox-name");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "DELETE mailbox-name", cmd->getText());


		vmime::shared_ptr <IMAPCommand> cmdQuote =
			IMAPCommand::DELETE("mailbox name");

		VASSERT_NOT_NULL("Not null", cmdQuote);
		VASSERT_EQ("Text", "DELETE \"mailbox name\"", cmdQuote->getText());
	}

	void testRENAME()
	{
		vmime::shared_ptr <IMAPCommand> cmd =
			IMAPCommand::RENAME("mailbox-name", "new-mailbox-name");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RENAME mailbox-name new-mailbox-name", cmd->getText());


		vmime::shared_ptr <IMAPCommand> cmdQuote =
			IMAPCommand::RENAME("mailbox name", "new mailbox name");

		VASSERT_NOT_NULL("Not null", cmdQuote);
		VASSERT_EQ("Text", "RENAME \"mailbox name\" \"new mailbox name\"", cmdQuote->getText());
	}

	void testFETCH()
	{
		std::vector <vmime::string> params;
		params.push_back("param-1");
		params.push_back("param-2");


		vmime::shared_ptr <IMAPCommand> cmdNum =
			IMAPCommand::FETCH(vmime::net::messageSet::byNumber(42), params);

		VASSERT_NOT_NULL("Not null", cmdNum);
		VASSERT_EQ("Text", "FETCH 42 (param-1 param-2)", cmdNum->getText());


		vmime::shared_ptr <IMAPCommand> cmdNums =
			IMAPCommand::FETCH(vmime::net::messageSet::byNumber(42, 47), params);

		VASSERT_NOT_NULL("Not null", cmdNums);
		VASSERT_EQ("Text", "FETCH 42:47 (param-1 param-2)", cmdNums->getText());


		vmime::shared_ptr <IMAPCommand> cmdUID =
			IMAPCommand::FETCH(vmime::net::messageSet::byUID(42), params);

		VASSERT_NOT_NULL("Not null", cmdUID);
		VASSERT_EQ("Text", "UID FETCH 42 (param-1 param-2)", cmdUID->getText());


		vmime::shared_ptr <IMAPCommand> cmdUIDs =
			IMAPCommand::FETCH(vmime::net::messageSet::byUID(42, 47), params);

		VASSERT_NOT_NULL("Not null", cmdUIDs);
		VASSERT_EQ("Text", "UID FETCH 42:47 (param-1 param-2)", cmdUIDs->getText());
	}

	void testSTORE()
	{
		std::vector <vmime::string> flags;
		flags.push_back("flag-1");
		flags.push_back("flag-2");


		vmime::shared_ptr <IMAPCommand> cmdNum = IMAPCommand::STORE
			(vmime::net::messageSet::byNumber(42), vmime::net::message::FLAG_MODE_SET, flags);

		VASSERT_NOT_NULL("Not null", cmdNum);
		VASSERT_EQ("Text", "STORE 42 FLAGS (flag-1 flag-2)", cmdNum->getText());


		vmime::shared_ptr <IMAPCommand> cmdNums = IMAPCommand::STORE
			(vmime::net::messageSet::byNumber(42, 47), vmime::net::message::FLAG_MODE_SET, flags);

		VASSERT_NOT_NULL("Not null", cmdNums);
		VASSERT_EQ("Text", "STORE 42:47 FLAGS (flag-1 flag-2)", cmdNums->getText());


		vmime::shared_ptr <IMAPCommand> cmdUID = IMAPCommand::STORE
			(vmime::net::messageSet::byUID(42), vmime::net::message::FLAG_MODE_SET, flags);

		VASSERT_NOT_NULL("Not null", cmdUID);
		VASSERT_EQ("Text", "UID STORE 42 FLAGS (flag-1 flag-2)", cmdUID->getText());


		vmime::shared_ptr <IMAPCommand> cmdUIDs = IMAPCommand::STORE
			(vmime::net::messageSet::byUID(42, 47), vmime::net::message::FLAG_MODE_SET, flags);

		VASSERT_NOT_NULL("Not null", cmdUIDs);
		VASSERT_EQ("Text", "UID STORE 42:47 FLAGS (flag-1 flag-2)", cmdUIDs->getText());


		vmime::shared_ptr <IMAPCommand> cmdAdd = IMAPCommand::STORE
			(vmime::net::messageSet::byUID(42, 47), vmime::net::message::FLAG_MODE_ADD, flags);

		VASSERT_NOT_NULL("Not null", cmdAdd);
		VASSERT_EQ("Text", "UID STORE 42:47 +FLAGS (flag-1 flag-2)", cmdAdd->getText());


		vmime::shared_ptr <IMAPCommand> cmdRem = IMAPCommand::STORE
			(vmime::net::messageSet::byUID(42, 47), vmime::net::message::FLAG_MODE_REMOVE, flags);

		VASSERT_NOT_NULL("Not null", cmdRem);
		VASSERT_EQ("Text", "UID STORE 42:47 -FLAGS (flag-1 flag-2)", cmdRem->getText());
	}

	void testAPPEND()
	{
		std::vector <vmime::string> flags;
		flags.push_back("flag-1");
		flags.push_back("flag-2");


		vmime::shared_ptr <IMAPCommand> cmd =
			IMAPCommand::APPEND("mailbox-name", flags, /* date */ NULL, 1234);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "APPEND mailbox-name (flag-1 flag-2) {1234}", cmd->getText());


		vmime::shared_ptr <IMAPCommand> cmdQuote =
			IMAPCommand::APPEND("mailbox name", flags, /* date */ NULL, 1234);

		VASSERT_NOT_NULL("Not null", cmdQuote);
		VASSERT_EQ("Text", "APPEND \"mailbox name\" (flag-1 flag-2) {1234}", cmdQuote->getText());


		vmime::datetime date(2014, 3, 15, 23, 11, 47, vmime::datetime::GMT2);
		vmime::shared_ptr <IMAPCommand> cmdDate =
			IMAPCommand::APPEND("mailbox name", flags, &date, 1234);

		VASSERT_NOT_NULL("Not null", cmdDate);
		VASSERT_EQ("Text", "APPEND \"mailbox name\" (flag-1 flag-2) \"15-Mar-2014 23:11:47 +0200\" {1234}", cmdDate->getText());
	}

	void testCOPY()
	{
		vmime::shared_ptr <IMAPCommand> cmdNum =
			IMAPCommand::COPY(vmime::net::messageSet::byNumber(42), "mailbox-name");

		VASSERT_NOT_NULL("Not null", cmdNum);
		VASSERT_EQ("Text", "COPY 42 mailbox-name", cmdNum->getText());


		vmime::shared_ptr <IMAPCommand> cmdNums =
			IMAPCommand::COPY(vmime::net::messageSet::byNumber(42, 47), "mailbox-name");

		VASSERT_NOT_NULL("Not null", cmdNums);
		VASSERT_EQ("Text", "COPY 42:47 mailbox-name", cmdNums->getText());


		vmime::shared_ptr <IMAPCommand> cmdUID =
			IMAPCommand::COPY(vmime::net::messageSet::byUID(42), "mailbox-name");

		VASSERT_NOT_NULL("Not null", cmdUID);
		VASSERT_EQ("Text", "UID COPY 42 mailbox-name", cmdUID->getText());


		vmime::shared_ptr <IMAPCommand> cmdUIDs =
			IMAPCommand::COPY(vmime::net::messageSet::byUID(42, 47), "mailbox-name");

		VASSERT_NOT_NULL("Not null", cmdUIDs);
		VASSERT_EQ("Text", "UID COPY 42:47 mailbox-name", cmdUIDs->getText());


		vmime::shared_ptr <IMAPCommand> cmdQuote =
			IMAPCommand::COPY(vmime::net::messageSet::byNumber(42, 47), "mailbox name");

		VASSERT_NOT_NULL("Not null", cmdQuote);
		VASSERT_EQ("Text", "COPY 42:47 \"mailbox name\"", cmdQuote->getText());
	}

	void testSEARCH()
	{
		std::vector <vmime::string> searchKeys;
		searchKeys.push_back("search-key-1");
		searchKeys.push_back("search-key-2");

		vmime::shared_ptr <IMAPCommand> cmd =
			IMAPCommand::SEARCH(searchKeys, /* charset */ NULL);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "SEARCH search-key-1 search-key-2", cmd->getText());


		vmime::charset cset("test-charset");

		vmime::shared_ptr <IMAPCommand> cmdCset =
			IMAPCommand::SEARCH(searchKeys, &cset);

		VASSERT_NOT_NULL("Not null", cmdCset);
		VASSERT_EQ("Text", "SEARCH CHARSET test-charset search-key-1 search-key-2", cmdCset->getText());
	}

	void testSTARTTLS()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::STARTTLS();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "STARTTLS", cmd->getText());
	}

	void testCAPABILITY()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::CAPABILITY();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "CAPABILITY", cmd->getText());
	}

	void testNOOP()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::NOOP();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "NOOP", cmd->getText());
	}

	void testEXPUNGE()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::EXPUNGE();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "EXPUNGE", cmd->getText());
	}

	void testCLOSE()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::CLOSE();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "CLOSE", cmd->getText());
	}

	void testLOGOUT()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::LOGOUT();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "LOGOUT", cmd->getText());
	}

	void testSend()
	{
		vmime::shared_ptr <IMAPCommand> cmd = IMAPCommand::createCommand("MY_COMMAND param1 param2");

		vmime::shared_ptr <vmime::net::session> sess =
			vmime::make_shared <vmime::net::session>();

		vmime::shared_ptr <vmime::security::authenticator> auth =
			vmime::make_shared <vmime::security::defaultAuthenticator>();

		vmime::shared_ptr <IMAPStore> store =
			vmime::make_shared <IMAPStore>(sess, auth, /* secured */ false);

		vmime::shared_ptr <IMAPConnection> conn =
			vmime::make_shared <IMAPConnection>(store, auth);

		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();
		conn->setSocket(sok);

		cmd->send(conn);

		vmime::string response;
		sok->localReceive(response);

		VASSERT_EQ("Sent buffer", vmime::string(*conn->getTag()) + " MY_COMMAND param1 param2\r\n", response);
	}

VMIME_TEST_SUITE_END
