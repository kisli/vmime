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

#include "vmime/net/smtp/SMTPResponse.hpp"


VMIME_TEST_SUITE_BEGIN(SMTPResponseTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testSingleLineResponse)
		VMIME_TEST(testSingleLineResponseLF)
		VMIME_TEST(testMultiLineResponse)
		VMIME_TEST(testMultiLineResponseDifferentCode)
		VMIME_TEST(testIncompleteMultiLineResponse)
		VMIME_TEST(testNoResponseText)
		VMIME_TEST(testEnhancedStatusCode)
		VMIME_TEST(testNoEnhancedStatusCode)
		VMIME_TEST(testInvalidEnhancedStatusCode)
	VMIME_TEST_LIST_END


	void testSingleLineResponse()
	{
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>();

		socket->localSend("123 Response Text\r\n");

		vmime::net::smtp::SMTPResponse::state responseState;

		vmime::shared_ptr <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState);

		VASSERT_EQ("Code", 123, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
	}

	void testSingleLineResponseLF()
	{
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>();

		socket->localSend("123 Response Text\n");

		vmime::net::smtp::SMTPResponse::state responseState;

		vmime::shared_ptr <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState);

		VASSERT_EQ("Code", 123, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "Response Text", resp->getText());
	}

	void testMultiLineResponse()
	{
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>();

		socket->localSend
		(
			"123-Response\r\n"
			"123 Text\r\n"
		);

		vmime::net::smtp::SMTPResponse::state responseState;

		vmime::shared_ptr <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState);

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
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>();

		socket->localSend
		(
			"123-Response\r\n"
			"456 Text\r\n"
		);

		vmime::net::smtp::SMTPResponse::state responseState;

		vmime::shared_ptr <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState);

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
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>(1);

		socket->localSend
		(
			"123-Response\r\n"
			"123-Text\r\n"
			// Missing data
		);

		vmime::net::smtp::SMTPResponse::state responseState;

		VASSERT_THROW("Incomplete response",
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState),
			vmime::exceptions::operation_timed_out);
	}

	void testNoResponseText()
	{
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>(1);

		socket->localSend
		(
			"250\r\n"
		);

		vmime::net::smtp::SMTPResponse::state responseState;

		vmime::shared_ptr <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState);

		VASSERT_EQ("Code", 250, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "", resp->getText());
	}

	void testEnhancedStatusCode()
	{
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>();

		socket->localSend("250 2.1.5 OK fu13sm4720601wic.7 - gsmtp\r\n");

		vmime::net::smtp::SMTPResponse::state responseState;

		vmime::shared_ptr <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState);

		VASSERT_EQ("Code", 250, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "2.1.5 OK fu13sm4720601wic.7 - gsmtp", resp->getText());
		VASSERT_EQ("Enh.class", 2, resp->getEnhancedCode().klass);
		VASSERT_EQ("Enh.subject", 1, resp->getEnhancedCode().subject);
		VASSERT_EQ("Enh.detail", 5, resp->getEnhancedCode().detail);
	}

	void testNoEnhancedStatusCode()
	{
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>();

		socket->localSend("354  Go ahead fu13sm4720601wic.7 - gsmtp\r\n");

		vmime::net::smtp::SMTPResponse::state responseState;

		vmime::shared_ptr <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState);

		VASSERT_EQ("Code", 354, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "Go ahead fu13sm4720601wic.7 - gsmtp", resp->getText());
		VASSERT_EQ("Enh.class", 0, resp->getEnhancedCode().klass);
		VASSERT_EQ("Enh.subject", 0, resp->getEnhancedCode().subject);
		VASSERT_EQ("Enh.detail", 0, resp->getEnhancedCode().detail);
	}

	void testInvalidEnhancedStatusCode()
	{
		vmime::shared_ptr <vmime::net::tracer> tracer;
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh =
			vmime::make_shared <testTimeoutHandler>();

		socket->localSend("250 4.2 xxx\r\n");

		vmime::net::smtp::SMTPResponse::state responseState;

		vmime::shared_ptr <vmime::net::smtp::SMTPResponse> resp =
			vmime::net::smtp::SMTPResponse::readResponse(tracer, socket, toh, responseState);

		VASSERT_EQ("Code", 250, resp->getCode());
		VASSERT_EQ("Lines", 1, resp->getLineCount());
		VASSERT_EQ("Text", "4.2 xxx", resp->getText());
		VASSERT_EQ("Enh.class", 0, resp->getEnhancedCode().klass);
		VASSERT_EQ("Enh.subject", 0, resp->getEnhancedCode().subject);
		VASSERT_EQ("Enh.detail", 0, resp->getEnhancedCode().detail);
	}

VMIME_TEST_SUITE_END

