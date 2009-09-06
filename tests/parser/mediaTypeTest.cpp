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


#define VMIME_TEST_SUITE         mediaTypeTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConstructors)
		VMIME_TEST(testCopy)
		VMIME_TEST(testSetFromString)
		VMIME_TEST(testParse)
		VMIME_TEST(testGenerate)
	VMIME_TEST_LIST_END


	void testConstructors()
	{
		vmime::mediaType t1;

		VASSERT_EQ("1.1", vmime::mediaTypes::APPLICATION, t1.getType());
		VASSERT_EQ("1.2", vmime::mediaTypes::APPLICATION_OCTET_STREAM, t1.getSubType());

		vmime::mediaType t2("type", "sub");

		VASSERT_EQ("2.1", "type", t2.getType());
		VASSERT_EQ("2.2", "sub", t2.getSubType());

		vmime::mediaType t3("type/sub");

		VASSERT_EQ("3.1", "type", t3.getType());
		VASSERT_EQ("3.2", "sub", t3.getSubType());
	}

	void testCopy()
	{
		vmime::mediaType t1("type/sub");

		VASSERT_EQ("eq1", "type", t1.getType());
		VASSERT_EQ("eq2", "sub", t1.getSubType());

		VASSERT("operator==", t1 == t1);
		VASSERT("clone", t1 == *vmime::clone(t1));

		VASSERT_EQ("eq3", "type", vmime::clone(t1)->getType());
		VASSERT_EQ("eq4", "sub", vmime::clone(t1)->getSubType());

		vmime::mediaType t2;
		t2.copyFrom(t1);

		VASSERT("copyFrom", t1 == t2);
	}

	void testSetFromString()
	{
		vmime::mediaType t1;
		t1.setFromString("type/sub");

		VASSERT_EQ("1.1", "type", t1.getType());
		VASSERT_EQ("1.2", "sub", t1.getSubType());
	}

	void testParse()
	{
		vmime::mediaType t1;
		t1.parse("type/sub");

		VASSERT_EQ("1.1", "type", t1.getType());
		VASSERT_EQ("1.2", "sub", t1.getSubType());
	}

	void testGenerate()
	{
		vmime::mediaType t1("type", "sub");

		VASSERT_EQ("1", "type/sub", t1.generate());
	}

VMIME_TEST_SUITE_END

