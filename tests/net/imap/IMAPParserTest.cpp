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

#include "vmime/net/imap/IMAPTag.hpp"
#include "vmime/net/imap/IMAPParser.hpp"


VMIME_TEST_SUITE_BEGIN(IMAPParserTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testExtraSpaceInCapaResponse)
		VMIME_TEST(testContinueReqWithoutSpace)
		VMIME_TEST(testNILValueInBodyFldEnc)
	VMIME_TEST_LIST_END


	// For Apple iCloud IMAP server
	void testExtraSpaceInCapaResponse()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
			vmime::make_shared <vmime::net::imap::IMAPTag>();

		socket->localSend(
			"* CAPABILITY IMAP4rev1 AUTH=ATOKEN AUTH=PLAIN \r\n"  // extra space at end
			"a001 OK Capability completed.\r\n");

		vmime::shared_ptr <vmime::net::imap::IMAPParser> parser =
			vmime::make_shared <vmime::net::imap::IMAPParser>();

		parser->setTag(tag);
		parser->setSocket(socket);
		parser->setTimeoutHandler(toh);

		parser->setStrict(false);
		VASSERT_NO_THROW("non-strict mode", parser->readResponse(/* literalHandler */ NULL));

		++(*tag);

		socket->localSend(
			"* CAPABILITY IMAP4rev1 AUTH=ATOKEN AUTH=PLAIN \r\n"  // extra space at end
			"a002 OK Capability completed.\r\n");

		parser->setStrict(true);
		VASSERT_THROW("strict mode", parser->readResponse(/* literalHandler */ NULL), vmime::exceptions::invalid_response);
	}

	// For Apple iCloud/Exchange IMAP server
	void testContinueReqWithoutSpace()
	{
		// continue_req   ::= "+" SPACE (resp_text / base64)
		//
		// Some servers do not send SPACE when response text is empty.
		// IMAP parser should allow this in non-strict mode.
		//
		// Eg:
		//
		//   C: a002 AUTHENTICATE xxx[CR][LF]
		//   S: +[CR][LF]

		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
				vmime::make_shared <vmime::net::imap::IMAPTag>();

		socket->localSend("+\r\n");

		vmime::shared_ptr <vmime::net::imap::IMAPParser> parser =
			vmime::make_shared <vmime::net::imap::IMAPParser>();

		parser->setTag(tag);
		parser->setSocket(socket);
		parser->setTimeoutHandler(toh);

		parser->setStrict(false);
		VASSERT_NO_THROW("non-strict mode", parser->readResponse());

		++(*tag);

		socket->localSend("+\r\n");

		parser->setStrict(true);
		VASSERT_THROW("strict mode", parser->readResponse(), vmime::exceptions::invalid_response);
	}

	// When an IMAP4 client sends a FETCH (bodystructure) request to a server
	// that is running the Exchange Server 2007 IMAP4 service, a corrupted
	// response is sent as a reply
	// --> http://support.microsoft.com/kb/975918/en-us
	void testNILValueInBodyFldEnc()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <vmime::net::imap::IMAPTag> tag =
				vmime::make_shared <vmime::net::imap::IMAPTag>();

		const char* resp = "* 7970 FETCH (UID 8036 FLAGS () BODYSTRUCTURE (\"text\" \"html\" (\"charset\" \"utf-8\") NIL NIL NIL 175501 1651 NIL NIL NIL NIL) RFC822.HEADER {3}\r\nx\r\n)\r\na001 OK FETCH complete\r\n";

		socket->localSend(resp);

		vmime::shared_ptr <vmime::net::imap::IMAPParser> parser =
			vmime::make_shared <vmime::net::imap::IMAPParser>();

		parser->setTag(tag);
		parser->setSocket(socket);
		parser->setTimeoutHandler(toh);

		parser->setStrict(false);
		VASSERT_NO_THROW("non-strict mode", parser->readResponse());

		++(*tag);

		socket->localSend(resp);

		parser->setStrict(true);
		VASSERT_THROW("strict mode", parser->readResponse(), vmime::exceptions::invalid_response);
	}

VMIME_TEST_SUITE_END
