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
	VMIME_TEST_LIST_END


	// For Apple iCloud IMAP server
	void testExtraSpaceInCapaResponse()
	{
		vmime::ref <testSocket> socket = vmime::create <testSocket>();
		vmime::ref <vmime::net::timeoutHandler> toh = vmime::create <testTimeoutHandler>();

		vmime::ref <vmime::net::imap::IMAPTag> tag =
			vmime::create <vmime::net::imap::IMAPTag>();

		socket->localSend(
			"* CAPABILITY IMAP4rev1 AUTH=ATOKEN AUTH=PLAIN \r\n"  // extra space at end
			"a001 OK Capability completed.\r\n");

		vmime::ref <vmime::net::imap::IMAPParser> parser =
			vmime::create <vmime::net::imap::IMAPParser>(tag, socket.dynamicCast <vmime::net::socket>(), toh);

		parser->setStrict(false);
		VASSERT_NO_THROW("non-strict mode", parser->readResponse(/* literalHandler */ NULL));

		++(*tag);

		socket->localSend(
			"* CAPABILITY IMAP4rev1 AUTH=ATOKEN AUTH=PLAIN \r\n"  // extra space at end
			"a002 OK Capability completed.\r\n");

		parser->setStrict(true);
		VASSERT_THROW("strict mode", parser->readResponse(/* literalHandler */ NULL), vmime::exceptions::invalid_response);
	}

VMIME_TEST_SUITE_END
