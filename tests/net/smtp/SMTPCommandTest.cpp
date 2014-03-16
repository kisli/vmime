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


VMIME_TEST_SUITE_BEGIN(SMTPCommandTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testCreateCommand)
		VMIME_TEST(testCreateCommandParams)
		VMIME_TEST(testHELO)
		VMIME_TEST(testEHLO)
		VMIME_TEST(testAUTH)
		VMIME_TEST(testAUTH_InitialResponse)
		VMIME_TEST(testSTARTTLS)
		VMIME_TEST(testMAIL)
		VMIME_TEST(testMAIL_Encoded)
		VMIME_TEST(testMAIL_UTF8)
		VMIME_TEST(testMAIL_SIZE)
		VMIME_TEST(testMAIL_SIZE_UTF8)
		VMIME_TEST(testRCPT)
		VMIME_TEST(testRCPT_Encoded)
		VMIME_TEST(testRCPT_UTF8)
		VMIME_TEST(testRSET)
		VMIME_TEST(testDATA)
		VMIME_TEST(testBDAT)
		VMIME_TEST(testNOOP)
		VMIME_TEST(testQUIT)
		VMIME_TEST(testWriteToSocket)
	VMIME_TEST_LIST_END


	void testCreateCommand()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::createCommand("MY_COMMAND");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MY_COMMAND", cmd->getText());
	}

	void testCreateCommandParams()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::createCommand("MY_COMMAND param1 param2");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MY_COMMAND param1 param2", cmd->getText());
	}

	void testHELO()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::HELO("hostname");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "HELO hostname", cmd->getText());
	}

	void testEHLO()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::EHLO("hostname");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "EHLO hostname", cmd->getText());
	}

	void testAUTH()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::AUTH("saslmechanism");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "AUTH saslmechanism", cmd->getText());
	}

	void testAUTH_InitialResponse()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::AUTH("saslmechanism", "initial-response");

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "AUTH saslmechanism initial-response", cmd->getText());
	}

	void testSTARTTLS()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::STARTTLS();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "STARTTLS", cmd->getText());
	}

	void testMAIL()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::MAIL(vmime::mailbox("me@vmime.org"), false);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MAIL FROM:<me@vmime.org>", cmd->getText());
	}

	void testMAIL_Encoded()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::MAIL
			(vmime::mailbox(vmime::emailAddress("mailtest", "例え.テスト")), false);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MAIL FROM:<mailtest@xn--r8jz45g.xn--zckzah>", cmd->getText());
	}

	void testMAIL_UTF8()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::MAIL
			(vmime::mailbox(vmime::emailAddress("mailtest", "例え.テスト")), true);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MAIL FROM:<mailtest@例え.テスト> SMTPUTF8", cmd->getText());
	}

	void testMAIL_SIZE()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::MAIL
			(vmime::mailbox("me@vmime.org"), false, 123456789);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MAIL FROM:<me@vmime.org> SIZE=123456789", cmd->getText());
	}

	void testMAIL_SIZE_UTF8()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::MAIL
			(vmime::mailbox(vmime::emailAddress("mailtest", "例え.テスト")), true, 123456789);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "MAIL FROM:<mailtest@例え.テスト> SMTPUTF8 SIZE=123456789", cmd->getText());
	}

	void testRCPT()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::RCPT(vmime::mailbox("someone@vmime.org"), false);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RCPT TO:<someone@vmime.org>", cmd->getText());
	}

	void testRCPT_Encoded()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::RCPT
			(vmime::mailbox(vmime::emailAddress("mailtest", "例え.テスト")), false);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RCPT TO:<mailtest@xn--r8jz45g.xn--zckzah>", cmd->getText());
	}

	void testRCPT_UTF8()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::RCPT
			(vmime::mailbox(vmime::emailAddress("mailtest", "例え.テスト")), true);

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RCPT TO:<mailtest@例え.テスト>", cmd->getText());
	}

	void testRSET()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::RSET();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "RSET", cmd->getText());
	}

	void testDATA()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::DATA();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "DATA", cmd->getText());
	}

	void testBDAT()
	{
		vmime::shared_ptr <SMTPCommand> cmd1 = SMTPCommand::BDAT(12345, false);

		VASSERT_NOT_NULL("Not null", cmd1);
		VASSERT_EQ("Text", "BDAT 12345", cmd1->getText());

		vmime::shared_ptr <SMTPCommand> cmd2 = SMTPCommand::BDAT(67890, true);

		VASSERT_NOT_NULL("Not null", cmd2);
		VASSERT_EQ("Text", "BDAT 67890 LAST", cmd2->getText());
	}

	void testNOOP()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::NOOP();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "NOOP", cmd->getText());
	}

	void testQUIT()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::QUIT();

		VASSERT_NOT_NULL("Not null", cmd);
		VASSERT_EQ("Text", "QUIT", cmd->getText());
	}

	void testWriteToSocket()
	{
		vmime::shared_ptr <SMTPCommand> cmd = SMTPCommand::createCommand("MY_COMMAND param1 param2");

		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();

		cmd->writeToSocket(sok, tracer);

		vmime::string response;
		sok->localReceive(response);

		VASSERT_EQ("Sent buffer", "MY_COMMAND param1 param2\r\n", response);
	}

VMIME_TEST_SUITE_END
