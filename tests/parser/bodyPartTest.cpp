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


#define VMIME_TEST_SUITE         bodyPartTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testGenerate)
		VMIME_TEST(testParseMissingLastBoundary)
	VMIME_TEST_LIST_END


	static const vmime::string extractComponentString
		(const vmime::string& buffer, const vmime::component& c)
	{
		return vmime::string(buffer.begin() + c.getParsedOffset(),
		                     buffer.begin() + c.getParsedOffset() + c.getParsedLength());
	}

	static const vmime::string extractContents(const vmime::ref <const vmime::contentHandler> cts)
	{
		std::ostringstream oss;
		vmime::utility::outputStreamAdapter os(oss);

		cts->extract(os);

		return oss.str();
	}


	void testParse()
	{
		vmime::string str1 = "HEADER\r\n\r\nBODY";
		vmime::bodyPart p1;
		p1.parse(str1);

		VASSERT_EQ("1", "HEADER\r\n\r\n", extractComponentString(str1, *p1.getHeader()));
		VASSERT_EQ("2", "BODY", extractComponentString(str1, *p1.getBody()));

		vmime::string str2 = "HEADER\n\nBODY";
		vmime::bodyPart p2;
		p2.parse(str2);

		VASSERT_EQ("3", "HEADER\n\n", extractComponentString(str2, *p2.getHeader()));
		VASSERT_EQ("4", "BODY", extractComponentString(str2, *p2.getBody()));

		vmime::string str3 = "HEADER\r\n\nBODY";
		vmime::bodyPart p3;
		p3.parse(str3);

		VASSERT_EQ("5", "HEADER\r\n\n", extractComponentString(str3, *p3.getHeader()));
		VASSERT_EQ("6", "BODY", extractComponentString(str3, *p3.getBody()));
	}

	void testParseMissingLastBoundary()
	{
		vmime::string str =
			"Content-Type: multipart/mixed; boundary=\"MY-BOUNDARY\""
			"\r\n\r\n"
			"--MY-BOUNDARY\r\nHEADER1\r\n\r\nBODY1"
			"--MY-BOUNDARY\r\nHEADER2\r\n\r\nBODY2";

		vmime::bodyPart p;
		p.parse(str);

		VASSERT_EQ("count", 2, p.getBody()->getPartCount());

		VASSERT_EQ("part1-body", "BODY1", extractContents(p.getBody()->getPartAt(0)->getBody()->getContents()));
		VASSERT_EQ("part2-body", "BODY2", extractContents(p.getBody()->getPartAt(1)->getBody()->getContents()));
	}

	void testGenerate()
	{
		vmime::bodyPart p1;
		p1.getHeader()->getField("Foo")->setValue(vmime::string("bar"));
		p1.getBody()->setContents(vmime::create <vmime::stringContentHandler>("Baz"));

		VASSERT_EQ("1", "Foo: bar\r\n\r\nBaz", p1.generate());
	}

VMIME_TEST_SUITE_END

