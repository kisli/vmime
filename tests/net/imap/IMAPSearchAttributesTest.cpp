//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#include "vmime/net/imap/IMAPSearchAttributes.hpp"


using namespace vmime::net::imap;


VMIME_TEST_SUITE_BEGIN(IMAPSearchAttributesTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testStringToken)
		VMIME_TEST(testHeaderToken)
	VMIME_TEST_LIST_END


	void testStringToken() {

		IMAPSearchAttributes attrs;
		attrs.add(IMAPSearchTokenFactory::SUBJECT("hello world"));
		attrs.add(IMAPSearchTokenFactory::FROM("say \"hello\" \\o/"));
		attrs.add(IMAPSearchTokenFactory::BODY("text\r\nA001 LOGOUT"));

		const std::vector <vmime::string> keys = attrs.generate();

		VASSERT_EQ("size", 3, keys.size());
		VASSERT_EQ("1", "SUBJECT \"hello world\"", keys[0]);
		VASSERT_EQ("2", "FROM \"say \\\"hello\\\" \\\\o/\"", keys[1]);
		VASSERT_EQ("3", "BODY \"textA001 LOGOUT\"", keys[2]);
	}

	void testHeaderToken() {

		IMAPSearchAttributes attrs;
		attrs.add(IMAPSearchTokenFactory::HEADER("X-Test"));
		attrs.add(IMAPSearchTokenFactory::HEADER("X-Test", "a \"quoted\" value"));

		const std::vector <vmime::string> keys = attrs.generate();

		VASSERT_EQ("size", 2, keys.size());
		VASSERT_EQ("1", "HEADER X-Test \"\"", keys[0]);
		VASSERT_EQ("2", "HEADER X-Test \"a \\\"quoted\\\" value\"", keys[1]);
	}

VMIME_TEST_SUITE_END
