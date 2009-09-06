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


#define VMIME_TEST_SUITE         datetimeTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testGenerate)
		VMIME_TEST(testCompare)
	VMIME_TEST_LIST_END


	void testParse()
	{
		struct datetimePair
		{
			vmime::string parseBuffer;
			vmime::datetime result;
		};

		// Here, we can't test all the possible structures for date/time,
		// so we test some cases. Don't forget to add a new test case
		// each time you encounter a bug in date/time parsing (after
		// you have fixed it).
		datetimePair pairs[] =
		{
			{ /* 1 */ "Mon, 8 Nov 2004 13:42:56 +0000 (GMT)",
			  vmime::datetime(2004, 11, 8, 13, 42, 56, vmime::datetime::GMT) },

			{ /* 2 */ "Sun,  7 Nov 2004 00:43:22 -0500 (EST)",
			  vmime::datetime(2004, 11, 7, 0, 43, 22, vmime::datetime::GMT_5) },

			{ /* 3 */ "Thu Nov 18 12:11:16 2004",
			  vmime::datetime(vmime::datetime::now().getYear(), 11, 18, 12, 11, 16, vmime::datetime::GMT) },

			{ /* 4 */ "Sat, 18, 2004 22:36:32 -0400",
			  vmime::datetime(2004, 1, 18, 22, 36, 32, vmime::datetime::GMT_4) },

			{ /* 5 */ "Mon Dec 13 21:57:18 2004",
			  vmime::datetime(vmime::datetime::now().getYear(), 12, 13, 21, 57, 18, vmime::datetime::GMT) },

			{ /* 6 */ "18 Nov 2004 21:44:54 +0300",
			  vmime::datetime(2004, 11, 18, 21, 44, 54, vmime::datetime::GMT3) }
		};

		for (unsigned int i = 0 ; i < sizeof(pairs) / sizeof(pairs[0]) ; ++i)
		{
			vmime::datetime d;
			d.parse(pairs[i].parseBuffer);

			std::ostringstream oss;
			oss << (i + 1);

			VASSERT_EQ(oss.str(), pairs[i].result, d);
		}
	}

	void testGenerate()
	{
		vmime::datetime d1(2005, 7, 8, 4, 5, 6, 1 * 60 + 23);

		VASSERT_EQ("1", "Fri, 8 Jul 2005 04:05:06 +0123", d1.generate());
	}

	void testCompare()
	{
		// Date1 = Date2
		vmime::datetime d1(2005, 4, 22, 14, 6, 0, vmime::datetime::GMT2);
		vmime::datetime d2(2005, 4, 22, 10, 6, 0, vmime::datetime::GMT_2);

		VASSERT_EQ("1.1", true,  d1 == d2);
		VASSERT_EQ("1.2", false, d1 != d2);
		VASSERT_EQ("1.3", true,  d1 <= d2);
		VASSERT_EQ("1.4", false, d1 <  d2);
		VASSERT_EQ("1.5", true,  d1 >= d2);
		VASSERT_EQ("1.6", false, d1 >  d2);

		// Date1 < Date2
		vmime::datetime d3(2005, 4, 22, 14, 6, 0);
		vmime::datetime d4(2005, 4, 22, 15, 6, 0);

		VASSERT_EQ("2.1", false, d3 == d4);
		VASSERT_EQ("2.2", true,  d3 != d4);
		VASSERT_EQ("2.3", true,  d3 <= d4);
		VASSERT_EQ("2.4", true,  d3 <  d4);
		VASSERT_EQ("2.5", false, d3 >= d4);
		VASSERT_EQ("2.6", false, d3 >  d4);

		// Date1 > Date2
		vmime::datetime d5(2005, 4, 22, 15, 6, 0);
		vmime::datetime d6(2005, 4, 22, 14, 6, 0);

		VASSERT_EQ("3.1", false, d5 == d6);
		VASSERT_EQ("3.2", true,  d5 != d6);
		VASSERT_EQ("3.3", false, d5 <= d6);
		VASSERT_EQ("3.4", false, d5 <  d6);
		VASSERT_EQ("3.5", true,  d5 >= d6);
		VASSERT_EQ("3.6", true,  d5 >  d6);
	}

VMIME_TEST_SUITE_END

