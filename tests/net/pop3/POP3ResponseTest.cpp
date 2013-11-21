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

#include "vmime/net/pop3/POP3Response.hpp"


using namespace vmime::net::pop3;


VMIME_TEST_SUITE_BEGIN(POP3ResponseTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testSingleLineResponseOK)
		VMIME_TEST(testSingleLineResponseERR)
		VMIME_TEST(testSingleLineResponseReady)
		VMIME_TEST(testSingleLineResponseInvalid)
		VMIME_TEST(testSingleLineResponseLF)
		VMIME_TEST(testMultiLineResponse)
		VMIME_TEST(testMultiLineResponseLF)
		VMIME_TEST(testLargeResponse)
	VMIME_TEST_LIST_END


	void testSingleLineResponseOK()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("+OK Response Text\r\n");

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readResponse(conn);

		VASSERT_EQ("Code", POP3Response::CODE_OK, resp->getCode());
		VASSERT_TRUE("Success", resp->isSuccess());
		VASSERT_EQ("Lines", 0, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
		VASSERT_EQ("First Line", "+OK Response Text", resp->getFirstLine());
	}

	void testSingleLineResponseERR()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("-ERR Response Text\r\n");

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readResponse(conn);

		VASSERT_EQ("Code", POP3Response::CODE_ERR, resp->getCode());
		VASSERT_FALSE("Success", resp->isSuccess());
		VASSERT_EQ("Lines", 0, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
		VASSERT_EQ("First Line", "-ERR Response Text", resp->getFirstLine());
	}

	void testSingleLineResponseReady()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("+ challenge_string\r\n");

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readResponse(conn);

		VASSERT_EQ("Code", POP3Response::CODE_READY, resp->getCode());
		VASSERT_FALSE("Success", resp->isSuccess());
		VASSERT_EQ("Lines", 0, resp->getLineCount());
		VASSERT_EQ("Text", "challenge_string", resp->getText());
		VASSERT_EQ("First Line", "+ challenge_string", resp->getFirstLine());
	}

	void testSingleLineResponseInvalid()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("Invalid Response Text\r\n");

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readResponse(conn);

		VASSERT_EQ("Code", POP3Response::CODE_ERR, resp->getCode());
		VASSERT_FALSE("Success", resp->isSuccess());
		VASSERT_EQ("Lines", 0, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
		VASSERT_EQ("First Line", "Invalid Response Text", resp->getFirstLine());
	}

	void testSingleLineResponseLF()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("+OK Response terminated by LF\n");

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readResponse(conn);

		VASSERT_EQ("Code", POP3Response::CODE_OK, resp->getCode());
		VASSERT_TRUE("Success", resp->isSuccess());
		VASSERT_EQ("Lines", 0, resp->getLineCount());
		VASSERT_EQ("Text", "Response terminated by LF", resp->getText());
		VASSERT_EQ("First Line", "+OK Response terminated by LF", resp->getFirstLine());
	}

	void testMultiLineResponse()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("+OK Response Text\r\n");
		socket->localSend("Line 1\r\n");
		socket->localSend("Line 2\r\n");
		socket->localSend(".\r\n");

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readMultilineResponse(conn);

		VASSERT_EQ("Code", POP3Response::CODE_OK, resp->getCode());
		VASSERT_TRUE("Success", resp->isSuccess());
		VASSERT_EQ("Lines", 2, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
		VASSERT_EQ("First Line", "+OK Response Text", resp->getFirstLine());
		VASSERT_EQ("Line 1", "Line 1", resp->getLineAt(0));
		VASSERT_EQ("Line 2", "Line 2", resp->getLineAt(1));
	}

	void testMultiLineResponseLF()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("+OK Response Text\n");
		socket->localSend("Line 1\n");
		socket->localSend("Line 2\n");
		socket->localSend(".\n");

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readMultilineResponse(conn);

		VASSERT_EQ("Code", POP3Response::CODE_OK, resp->getCode());
		VASSERT_TRUE("Success", resp->isSuccess());
		VASSERT_EQ("Lines", 2, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
		VASSERT_EQ("First Line", "+OK Response Text", resp->getFirstLine());
		VASSERT_EQ("Line 1", "Line 1", resp->getLineAt(0));
		VASSERT_EQ("Line 2", "Line 2", resp->getLineAt(1));
	}

	void testLargeResponse()
	{
		std::ostringstream data;

		for (unsigned int i = 0 ; i < 5000 ; ++i)
			data << "VMIME.VMIME\nVMIME\r\nVMIME_VMIME";

		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("+OK Large Response Follows\n");
		socket->localSend(data.str());
		socket->localSend("\r\n.\r\n");

		vmime::string receivedData;
		vmime::utility::outputStreamStringAdapter receivedDataStream(receivedData);

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readLargeResponse(conn, receivedDataStream, NULL, 0);

		VASSERT_EQ("Code", POP3Response::CODE_OK, resp->getCode());
		VASSERT_TRUE("Success", resp->isSuccess());
		VASSERT_EQ("Lines", 0, resp->getLineCount());
		VASSERT_EQ("Text", "Large Response Follows", resp->getText());
		VASSERT_EQ("Data Length", data.str().length(), receivedData.length());
		VASSERT_EQ("Data Bytes", data.str(), receivedData);
	}

VMIME_TEST_SUITE_END

