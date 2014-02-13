//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

#include "vmime/net/pop3/POP3Command.hpp"


using namespace vmime::net::pop3;


VMIME_TEST_SUITE_BEGIN(POP3CommandTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testCreateCommand)
		VMIME_TEST(testCreateCommandParams)
		VMIME_TEST(testCAPA)
		VMIME_TEST(testNOOP)
		VMIME_TEST(testAUTH)
		VMIME_TEST(testAUTH_InitialResponse)
		VMIME_TEST(testSTLS)
		VMIME_TEST(testAPOP)
		VMIME_TEST(testUSER)
		VMIME_TEST(testPASS)
		VMIME_TEST(testSTAT)
		VMIME_TEST(testLIST)
		VMIME_TEST(testLISTMessage)
		VMIME_TEST(testUIDL)
		VMIME_TEST(testUIDLMessage)
		VMIME_TEST(testDELE)
		VMIME_TEST(testRETR)
		VMIME_TEST(testTOP)
		VMIME_TEST(testRSET)
		VMIME_TEST(testQUIT)
		VMIME_TEST(testWriteToSocket)
	VMIME_TEST_LIST_END


	void testCreateCommand()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::createCommand("MY_COMMAND");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MY_COMMAND", cmd->getText());
	}

	void testCreateCommandParams()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::createCommand("MY_COMMAND param1 param2");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MY_COMMAND param1 param2", cmd->getText());
	}

	void testCAPA()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::CAPA();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "CAPA", cmd->getText());
	}

	void testNOOP()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::NOOP();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "NOOP", cmd->getText());
	}

	void testAUTH()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::AUTH("saslmechanism");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "AUTH saslmechanism", cmd->getText());
	}

	void testAUTH_InitialResponse()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::AUTH("saslmechanism", "initial-response");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "AUTH saslmechanism initial-response", cmd->getText());
	}

	void testSTLS()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::STLS();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "STLS", cmd->getText());
	}

	void testAPOP()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::APOP("user", "digest");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "APOP user digest", cmd->getText());
	}

	void testUSER()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::USER("user");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "USER user", cmd->getText());
	}

	void testPASS()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::PASS("pass");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "PASS pass", cmd->getText());
	}

	void testSTAT()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::STAT();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "STAT", cmd->getText());
	}

	void testLIST()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::LIST();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "LIST", cmd->getText());
	}

	void testLISTMessage()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::LIST(42);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "LIST 42", cmd->getText());
	}

	void testUIDL()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::UIDL();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "UIDL", cmd->getText());
	}

	void testUIDLMessage()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::UIDL(42);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "UIDL 42", cmd->getText());
	}

	void testDELE()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::DELE(42);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "DELE 42", cmd->getText());
	}

	void testRETR()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::RETR(42);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RETR 42", cmd->getText());
	}

	void testTOP()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::TOP(42, 567);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "TOP 42 567", cmd->getText());
	}

	void testRSET()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::RSET();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RSET", cmd->getText());
	}

	void testQUIT()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::QUIT();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "QUIT", cmd->getText());
	}

	void testWriteToSocket()
	{
		vmime::shared_ptr <POP3Command> cmd = POP3Command::createCommand("MY_COMMAND param1 param2");

		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();
		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(sok),
			 vmime::shared_ptr <vmime::net::timeoutHandler>());

		cmd->send(conn);

		vmime::string response;
		sok->localReceive(response);

		VASSERT_EQ("Sent buffer", "MY_COMMAND param1 param2\r\n", response);
	}

VMIME_TEST_SUITE_END
