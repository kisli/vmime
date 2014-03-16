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

#include "vmime/net/smtp/SMTPCommandSet.hpp"
#include "vmime/net/smtp/SMTPCommand.hpp"


using namespace vmime::net::smtp;


VMIME_TEST_SUITE_BEGIN(SMTPCommandSetTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testCreate)
		VMIME_TEST(testCreatePipeline)
		VMIME_TEST(testAddCommand)
		VMIME_TEST(testAddCommandPipeline)
		VMIME_TEST(testWriteToSocket)
		VMIME_TEST(testWriteToSocketPipeline)
		VMIME_TEST(testGetLastCommandSent)
		VMIME_TEST(testGetLastCommandSentPipeline)
	VMIME_TEST_LIST_END


	void testCreate()
	{
		vmime::shared_ptr <SMTPCommandSet> cset = SMTPCommandSet::create(/* pipelining */ false);

		VASSERT_NOT_NULL("Not null", cset);
		VASSERT_FALSE("Finished", cset->isFinished());
	}

	void testCreatePipeline()
	{
		vmime::shared_ptr <SMTPCommandSet> cset = SMTPCommandSet::create(/* pipelining */ true);

		VASSERT_NOT_NULL("Not null", cset);
		VASSERT_FALSE("Finished", cset->isFinished());
	}

	void testAddCommand()
	{
		vmime::shared_ptr <SMTPCommandSet> cset = SMTPCommandSet::create(/* pipelining */ false);

		VASSERT_NO_THROW("No throw 1", cset->addCommand(SMTPCommand::createCommand("MY_COMMAND1")));
		VASSERT_EQ("Text", "MY_COMMAND1\r\n", cset->getText());
		VASSERT_NO_THROW("No throw 2", cset->addCommand(SMTPCommand::createCommand("MY_COMMAND2")));
		VASSERT_EQ("Text", "MY_COMMAND1\r\nMY_COMMAND2\r\n", cset->getText());

		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();

		cset->writeToSocket(sok, tracer);
		VASSERT_FALSE("Finished", cset->isFinished());

		// Can't add commands when writing to socket has started
		VASSERT_THROW("Throw", cset->addCommand(SMTPCommand::createCommand("MY_COMMAND3")), std::runtime_error);

		cset->writeToSocket(sok, tracer);
		VASSERT_TRUE("Finished", cset->isFinished());
	}

	void testAddCommandPipeline()
	{
		vmime::shared_ptr <SMTPCommandSet> cset = SMTPCommandSet::create(/* pipelining */ true);

		VASSERT_NO_THROW("No throw 1", cset->addCommand(SMTPCommand::createCommand("MY_COMMAND1")));
		VASSERT_EQ("Text", "MY_COMMAND1\r\n", cset->getText());
		VASSERT_NO_THROW("No throw 2", cset->addCommand(SMTPCommand::createCommand("MY_COMMAND2")));
		VASSERT_EQ("Text", "MY_COMMAND1\r\nMY_COMMAND2\r\n", cset->getText());

		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();
		vmime::string response;

		cset->writeToSocket(sok, tracer);
		VASSERT_TRUE("Finished", cset->isFinished());

		sok->localReceive(response);
		VASSERT_EQ("Receive cmds", "MY_COMMAND1\r\nMY_COMMAND2\r\n", response);

		// Can't add commands when writing to socket has started
		VASSERT_THROW("Throw", cset->addCommand(SMTPCommand::createCommand("MY_COMMAND3")), std::runtime_error);
	}

	void testWriteToSocket()
	{
		vmime::shared_ptr <SMTPCommandSet> cset = SMTPCommandSet::create(/* pipelining */ false);

		cset->addCommand(SMTPCommand::createCommand("MY_COMMAND1"));
		cset->addCommand(SMTPCommand::createCommand("MY_COMMAND2"));

		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();
		vmime::string response;

		cset->writeToSocket(sok, tracer);

		sok->localReceive(response);
		VASSERT_EQ("Receive cmd 1", "MY_COMMAND1\r\n", response);

		cset->writeToSocket(sok, tracer);

		sok->localReceive(response);
		VASSERT_EQ("Receive cmd 2", "MY_COMMAND2\r\n", response);
	}

	void testWriteToSocketPipeline()
	{
		vmime::shared_ptr <SMTPCommandSet> cset = SMTPCommandSet::create(/* pipelining */ true);

		cset->addCommand(SMTPCommand::createCommand("MY_COMMAND1"));
		cset->addCommand(SMTPCommand::createCommand("MY_COMMAND2"));

		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();
		vmime::string response;

		cset->writeToSocket(sok, tracer);

		sok->localReceive(response);
		VASSERT_EQ("Receive cmds", "MY_COMMAND1\r\nMY_COMMAND2\r\n", response);
	}

	void testGetLastCommandSent()
	{
		vmime::shared_ptr <SMTPCommandSet> cset = SMTPCommandSet::create(/* pipelining */ false);

		cset->addCommand(SMTPCommand::createCommand("MY_COMMAND1"));
		cset->addCommand(SMTPCommand::createCommand("MY_COMMAND2"));

		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();

		cset->writeToSocket(sok, tracer);
		VASSERT_EQ("Cmd 1", "MY_COMMAND1", cset->getLastCommandSent()->getText());

		cset->writeToSocket(sok, tracer);
		VASSERT_EQ("Cmd 2", "MY_COMMAND2", cset->getLastCommandSent()->getText());
	}

	void testGetLastCommandSentPipeline()
	{
		vmime::shared_ptr <SMTPCommandSet> cset = SMTPCommandSet::create(/* pipelining */ true);

		cset->addCommand(SMTPCommand::createCommand("MY_COMMAND1"));
		cset->addCommand(SMTPCommand::createCommand("MY_COMMAND2"));

		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> sok = vmime::make_shared <testSocket>();

		cset->writeToSocket(sok, tracer);
		VASSERT_EQ("Cmd 1", "MY_COMMAND1", cset->getLastCommandSent()->getText());

		cset->writeToSocket(sok, tracer);
		VASSERT_EQ("Cmd 2", "MY_COMMAND2", cset->getLastCommandSent()->getText());
	}

VMIME_TEST_SUITE_END
