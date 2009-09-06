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


#define VMIME_TEST_SUITE         pathTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testParse2)
		VMIME_TEST(testGenerate)
	VMIME_TEST_LIST_END


	void testParse()
	{
		vmime::path p1;
		p1.parse("<>");

		VASSERT_EQ("1.1", "", p1.getLocalPart());
		VASSERT_EQ("1.2", "", p1.getDomain());

		vmime::path p2;
		p2.parse("<domain>");

		VASSERT_EQ("2.1", "", p2.getLocalPart());
		VASSERT_EQ("2.2", "domain", p2.getDomain());

		vmime::path p3;
		p3.parse("<local@domain>");

		VASSERT_EQ("3.1", "local", p3.getLocalPart());
		VASSERT_EQ("3.2", "domain", p3.getDomain());
	}

	void testParse2()
	{
		// Test some invalid paths (no '<>')
		vmime::path p1;
		p1.parse("");

		VASSERT_EQ("1.1", "", p1.getLocalPart());
		VASSERT_EQ("1.2", "", p1.getDomain());

		vmime::path p2;
		p2.parse("domain");

		VASSERT_EQ("2.1", "", p2.getLocalPart());
		VASSERT_EQ("2.2", "domain", p2.getDomain());

		vmime::path p3;
		p3.parse("local@domain");

		VASSERT_EQ("3.1", "local", p3.getLocalPart());
		VASSERT_EQ("3.2", "domain", p3.getDomain());
	}

	void testGenerate()
	{
		vmime::path p1;

		VASSERT_EQ("1", "<>", p1.generate());

		vmime::path p2;
		p2.setLocalPart("local");

		VASSERT_EQ("2", "<local@>", p2.generate());

		vmime::path p3;
		p3.setDomain("domain");

		VASSERT_EQ("3", "<@domain>", p3.generate());

		vmime::path p4;
		p4.setLocalPart("local");
		p4.setDomain("domain");

		VASSERT_EQ("4", "<local@domain>", p4.generate());
	}

VMIME_TEST_SUITE_END

