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


#define VMIME_TEST_SUITE         messageIdTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testParseInvalid)
		VMIME_TEST(testGenerate)
	VMIME_TEST_LIST_END


	void testParse()
	{
		vmime::messageId m1;
		m1.parse("<a@b>");

		VASSERT_EQ("1.1", "a", m1.getLeft());
		VASSERT_EQ("1.2", "b", m1.getRight());
	}

	void testParseInvalid()
	{
		vmime::messageId m1;
		m1.parse("foo@bar");

		VASSERT_EQ("1.1", "foo", m1.getLeft());
		VASSERT_EQ("1.2", "bar", m1.getRight());
	}

	void testGenerate()
	{
		vmime::messageId m1;

		VASSERT_EQ("1", "<>", m1.generate());

		vmime::messageId m2;
		m2.setLeft("a");

		VASSERT_EQ("2", "<a>", m2.generate());

		vmime::messageId m3;
		m3.setRight("b");

		VASSERT_EQ("3", "<@b>", m3.generate());

		vmime::messageId m4;
		m4.setLeft("a");
		m4.setRight("b");

		VASSERT_EQ("4", "<a@b>", m4.generate());
	}

VMIME_TEST_SUITE_END

