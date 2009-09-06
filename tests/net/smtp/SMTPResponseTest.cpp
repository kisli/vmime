//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/net/smtp/SMTPResponse.hpp"


#define VMIME_TEST_SUITE         SMTPResponseTest
#define VMIME_TEST_SUITE_MODULE  "Net/SMTP"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testSingleLineResponse)
		VMIME_TEST(testSingleLineResponseLF)
		VMIME_TEST(testMultiLineResponse)
		VMIME_TEST(testMultiLineResponseDifferentCode)
		VMIME_TEST(testIncompleteMultiLineResponse)
		VMIME_TEST(testIntermediateResponse)
		VMIME_TEST(testNoResponseText)
	VMIME_TEST_LIST_END


	void testSingleLineResponse()
	{
		vmime::ref <testSocket> socket = vmime::create <testSocket>();
		vmime::ref <vmime::net::timeoutHandler> toh =
			vmime::create <testTimeoutHandler>();

		socket->localSend("123 Response Text\r\n");

		vmime::ref <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(socket, toh);

		VASSERT_EQ("Code", 123, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
	}

	void testSingleLineResponseLF()
	{
		vmime::ref <testSocket> socket = vmime::create <testSocket>();
		vmime::ref <vmime::net::timeoutHandler> toh =
			vmime::create <testTimeoutHandler>();

		socket->localSend("123 Response Text\n");

		vmime::ref <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(socket, toh);

		VASSERT_EQ("Code", 123, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
	}

	void testMultiLineResponse()
	{
		vmime::ref <testSocket> socket = vmime::create <testSocket>();
		vmime::ref <vmime::net::timeoutHandler> toh =
			vmime::create <testTimeoutHandler>();

		socket->localSend
		(
			"123-Response\r\n"
			"123 Text\r\n"
		);

		vmime::ref <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(socket, toh);

		VASSERT_EQ("Code", 123, resp->getCode());
		VASSERT_EQ("Lines", 2, resp->getLineCount());
		VASSERT_EQ("Text", "Response\nText", resp->getText());

		VASSERT_EQ("Code", 123, resp->getLineAt(0).getCode());
		VASSERT_EQ("Text", "Response", resp->getLineAt(0).getText());

		VASSERT_EQ("Code", 123, resp->getLineAt(1).getCode());
		VASSERT_EQ("Text", "Text", resp->getLineAt(1).getText());
	}

	void testMultiLineResponseDifferentCode()
	{
		vmime::ref <testSocket> socket = vmime::create <testSocket>();
		vmime::ref <vmime::net::timeoutHandler> toh =
			vmime::create <testTimeoutHandler>();

		socket->localSend
		(
			"123-Response\r\n"
			"456 Text\r\n"
		);

		vmime::ref <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(socket, toh);

		VASSERT_EQ("Code", 0, resp->getCode());
		VASSERT_EQ("Lines", 2, resp->getLineCount());
		VASSERT_EQ("Text", "Response\nText", resp->getText());

		VASSERT_EQ("Code", 123, resp->getLineAt(0).getCode());
		VASSERT_EQ("Text", "Response", resp->getLineAt(0).getText());

		VASSERT_EQ("Code", 456, resp->getLineAt(1).getCode());
		VASSERT_EQ("Text", "Text", resp->getLineAt(1).getText());
	}

	void testIncompleteMultiLineResponse()
	{
		vmime::ref <testSocket> socket = vmime::create <testSocket>();
		vmime::ref <vmime::net::timeoutHandler> toh =
			vmime::create <testTimeoutHandler>(1);

		socket->localSend
		(
			"123-Response\r\n"
			"123-Text\r\n"
			// Missing data
		);

		VASSERT_THROW("Incomplete response",
			vmime::net::smtp::SMTPResponse::readResponse(socket, toh),
			vmime::exceptions::operation_timed_out);
	}

	void testIntermediateResponse()
	{
		vmime::ref <testSocket> socket = vmime::create <testSocket>();
		vmime::ref <vmime::net::timeoutHandler> toh =
			vmime::create <testTimeoutHandler>(1);

		socket->localSend
		(
			"334\r\n"
			"More information\r\n"
		);

		vmime::ref <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(socket, toh);

		VASSERT_EQ("Code", 334, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "More information", resp->getText());
	}

	void testNoResponseText()
	{
		vmime::ref <testSocket> socket = vmime::create <testSocket>();
		vmime::ref <vmime::net::timeoutHandler> toh =
			vmime::create <testTimeoutHandler>(1);

		socket->localSend
		(
			"250\r\n"
		);

		vmime::ref <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(socket, toh);

		VASSERT_EQ("Code", 250, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "", resp->getText());
	}


VMIME_TEST_SUITE_END

