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

#include "vmime/net/smtp/SMTPCommand.hpp"


using namespace vmime::net::smtp;


#define VMIME_TEST_SUITE         SMTPCommandTest
#define VMIME_TEST_SUITE_MODULE  "Net/SMTP"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testCreateCommand)
		VMIME_TEST(testCreateCommandParams)
		VMIME_TEST(testHELO)
		VMIME_TEST(testEHLO)
		VMIME_TEST(testAUTH)
		VMIME_TEST(testSTARTTLS)
		VMIME_TEST(testMAIL)
		VMIME_TEST(testRCPT)
		VMIME_TEST(testRSET)
		VMIME_TEST(testDATA)
		VMIME_TEST(testNOOP)
		VMIME_TEST(testQUIT)
		VMIME_TEST(testWriteToSocket)
	VMIME_TEST_LIST_END


	void testCreateCommand()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::createCommand("MY_COMMAND");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MY_COMMAND", cmd->getText());
	}

	void testCreateCommandParams()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::createCommand("MY_COMMAND param1 param2");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MY_COMMAND param1 param2", cmd->getText());
	}

	void testHELO()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::HELO("hostname");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "HELO hostname", cmd->getText());
	}

	void testEHLO()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::EHLO("hostname");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "EHLO hostname", cmd->getText());
	}

	void testAUTH()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::AUTH("saslmechanism");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "AUTH saslmechanism", cmd->getText());
	}

	void testSTARTTLS()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::STARTTLS();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "STARTTLS", cmd->getText());
	}

	void testMAIL()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::MAIL(vmime::mailbox("me@vmime.org"));

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MAIL FROM:<me@vmime.org>", cmd->getText());
	}

	void testRCPT()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::RCPT(vmime::mailbox("someone@vmime.org"));

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RCPT TO:<someone@vmime.org>", cmd->getText());
	}

	void testRSET()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::RSET();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RSET", cmd->getText());
	}

	void testDATA()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::DATA();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "DATA", cmd->getText());
	}

	void testNOOP()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::NOOP();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "NOOP", cmd->getText());
	}

	void testQUIT()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::QUIT();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "QUIT", cmd->getText());
	}

	void testWriteToSocket()
	{
		vmime::ref <SMTPCommand> cmd = SMTPCommand::createCommand("MY_COMMAND param1 param2");

		vmime::ref <testSocket> sok = vmime::create <testSocket>();
		cmd->writeToSocket(sok);

		vmime::string response;
		sok->localReceive(response);

		VASSERT_EQ("Sent buffer", "MY_COMMAND param1 param2\r\n", response);
	}

VMIME_TEST_SUITE_END
