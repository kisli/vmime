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


#define VMIME_TEST_SUITE         messageIdSequenceTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testGenerate)
	VMIME_TEST_LIST_END


	void testParse()
	{
		vmime::messageIdSequence s1;
		s1.parse("");

		VASSERT_EQ("1", 0, s1.getMessageIdCount());

		vmime::messageIdSequence s2;
		s2.parse("   \t  ");

		VASSERT_EQ("2", 0, s2.getMessageIdCount());

		vmime::messageIdSequence s3;
		s3.parse("<a@b>");

		VASSERT_EQ("3.1", 1, s3.getMessageIdCount());
		VASSERT_EQ("3.2", "a", s3.getMessageIdAt(0)->getLeft());
		VASSERT_EQ("3.3", "b", s3.getMessageIdAt(0)->getRight());

		vmime::messageIdSequence s4;
		s4.parse("<a@b>  \r\n\t<c@d>");

		VASSERT_EQ("4.1", 2, s4.getMessageIdCount());
		VASSERT_EQ("4.2", "a", s4.getMessageIdAt(0)->getLeft());
		VASSERT_EQ("4.3", "b", s4.getMessageIdAt(0)->getRight());
		VASSERT_EQ("4.4", "c", s4.getMessageIdAt(1)->getLeft());
		VASSERT_EQ("4.5", "d", s4.getMessageIdAt(1)->getRight());
	}

	void testGenerate()
	{
		vmime::messageIdSequence s1;
		s1.appendMessageId(vmime::create <vmime::messageId>("a", "b"));

		VASSERT_EQ("1", "<a@b>", s1.generate());

		vmime::messageIdSequence s2;
		s2.appendMessageId(vmime::create <vmime::messageId>("a", "b"));
		s2.appendMessageId(vmime::create <vmime::messageId>("c", "d"));

		VASSERT_EQ("2", "<a@b> <c@d>", s2.generate());
	}

VMIME_TEST_SUITE_END

