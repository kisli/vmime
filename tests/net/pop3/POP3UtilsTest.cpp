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

#include "vmime/net/pop3/POP3Utils.hpp"
#include "vmime/net/pop3/POP3Response.hpp"


using namespace vmime::net::pop3;


VMIME_TEST_SUITE_BEGIN(POP3UtilsTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParseMultiListOrUidlResponse)
	VMIME_TEST_LIST_END


	void testParseMultiListOrUidlResponse()
	{
		vmime::shared_ptr <testSocket> socket = vmime::make_shared <testSocket>();
		vmime::shared_ptr <vmime::net::timeoutHandler> toh = vmime::make_shared <testTimeoutHandler>();

		vmime::shared_ptr <POP3ConnectionTest> conn = vmime::make_shared <POP3ConnectionTest>
			(vmime::dynamicCast <vmime::net::socket>(socket), toh);

		socket->localSend("+OK Response Text\r\n");
		socket->localSend("1 abcdef\r\n");
		socket->localSend("23    ghijkl\r\n");
		socket->localSend("4\tmnopqr\r\n");
		socket->localSend("567xx\tstuvwx\r\n");
		socket->localSend("8 yz   \r\n");
		socket->localSend(".\r\n");

		vmime::shared_ptr <POP3Response> resp =
			POP3Response::readMultilineResponse(conn);

		std::map <int, vmime::string> result;
		POP3Utils::parseMultiListOrUidlResponse(resp, result);

		VASSERT_EQ("Count", 5, result.size());
		VASSERT_EQ("1", "abcdef", result[1]);
		VASSERT_EQ("2 (multiple spaces)", "ghijkl", result[23]);
		VASSERT_EQ("3 (with tab)", "mnopqr", result[4]);
		VASSERT_EQ("4 (with invalid digit)", "stuvwx", result[567]);
		VASSERT_EQ("5 (with extra space)", "yz", result[8]);
	}

VMIME_TEST_SUITE_END

