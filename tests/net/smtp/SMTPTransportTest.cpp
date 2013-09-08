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

#include "vmime/net/smtp/SMTPTransport.hpp"
#include "vmime/net/smtp/SMTPChunkingOutputStreamAdapter.hpp"
#include "vmime/net/smtp/SMTPExceptions.hpp"

#include "SMTPTransportTestUtils.hpp"


VMIME_TEST_SUITE_BEGIN(SMTPTransportTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConnectToInvalidServer)
		VMIME_TEST(testGreetingError)
		VMIME_TEST(testMAILandRCPT)
		VMIME_TEST(testChunking)
		VMIME_TEST(testSize_Chunking)
		VMIME_TEST(testSize_NoChunking)
	VMIME_TEST_LIST_END


	void testConnectToInvalidServer()
	{
		vmime::ref <vmime::net::session> sess
			= vmime::create <vmime::net::session>();

		vmime::utility::url url("smtp://invalid-smtp-server");
		vmime::ref <vmime::net::transport> store = sess->getTransport(url);

		VASSERT_THROW("connect", store->connect(), vmime::exceptions::connection_error);
	}

	void testGreetingError()
	{
		vmime::ref <vmime::net::session> session =
			vmime::create <vmime::net::session>();

		vmime::ref <vmime::net::transport> tr = session->getTransport
			(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::create <testSocketFactory <greetingErrorSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::create <testTimeoutHandlerFactory>());

		VASSERT_THROW("Connection", tr->connect(),
			vmime::exceptions::connection_greeting_error);
	}

	void testMAILandRCPT()
	{
		vmime::ref <vmime::net::session> session =
			vmime::create <vmime::net::session>();

		vmime::ref <vmime::net::transport> tr = session->getTransport
			(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::create <testSocketFactory <MAILandRCPTSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::create <testTimeoutHandlerFactory>());

		VASSERT_NO_THROW("Connection", tr->connect());

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient1@test.vmime.org"));
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient2@test.vmime.org"));
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient3@test.vmime.org"));

		vmime::string data("Message data");
		vmime::utility::inputStreamStringAdapter is(data);

		tr->send(exp, recips, is, 0);
	}

	void testChunking()
	{
		vmime::ref <vmime::net::session> session =
			vmime::create <vmime::net::session>();

		vmime::ref <vmime::net::transport> tr = session->getTransport
			(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::create <testSocketFactory <chunkingSMTPTestSocket> >());
		tr->setTimeoutHandlerFactory(vmime::create <testTimeoutHandlerFactory>());

		tr->connect();

		VASSERT("Test server should report it supports the CHUNKING extension!",
			tr.dynamicCast <vmime::net::smtp::SMTPTransport>()->getConnection()->hasExtension("CHUNKING"));

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::ref <vmime::message> msg = vmime::create <SMTPTestMessage>();

		tr->send(msg, exp, recips);
	}

	void testSize_Chunking()
	{
		vmime::ref <vmime::net::session> session =
			vmime::create <vmime::net::session>();

		vmime::ref <vmime::net::transport> tr = session->getTransport
			(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::create <testSocketFactory <bigMessageSMTPTestSocket <true> > >());
		tr->setTimeoutHandlerFactory(vmime::create <testTimeoutHandlerFactory>());

		tr->connect();

		VASSERT("Test server should report it supports the SIZE extension!",
			tr.dynamicCast <vmime::net::smtp::SMTPTransport>()->getConnection()->hasExtension("SIZE"));

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::ref <vmime::message> msg = vmime::create <SMTPBigTestMessage4MB>();

		VASSERT_THROW("Connection", tr->send(msg, exp, recips),
			vmime::net::smtp::SMTPMessageSizeExceedsMaxLimitsException);
	}

	void testSize_NoChunking()
	{
		vmime::ref <vmime::net::session> session =
			vmime::create <vmime::net::session>();

		vmime::ref <vmime::net::transport> tr = session->getTransport
			(vmime::utility::url("smtp://localhost"));

		tr->setSocketFactory(vmime::create <testSocketFactory <bigMessageSMTPTestSocket <false> > >());
		tr->setTimeoutHandlerFactory(vmime::create <testTimeoutHandlerFactory>());

		tr->connect();

		VASSERT("Test server should report it supports the SIZE extension!",
			tr.dynamicCast <vmime::net::smtp::SMTPTransport>()->getConnection()->hasExtension("SIZE"));

		vmime::mailbox exp("expeditor@test.vmime.org");

		vmime::mailboxList recips;
		recips.appendMailbox(vmime::create <vmime::mailbox>("recipient@test.vmime.org"));

		vmime::ref <vmime::message> msg = vmime::create <SMTPBigTestMessage4MB>();

		VASSERT_THROW("Connection", tr->send(msg, exp, recips),
			vmime::net::smtp::SMTPMessageSizeExceedsMaxLimitsException);
	}

VMIME_TEST_SUITE_END

