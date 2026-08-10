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


VMIME_TEST_SUITE_BEGIN(datetimeTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParse)
		VMIME_TEST(testParseZoneName)
		VMIME_TEST(testParseWellFormed)
		VMIME_TEST(testParseMalformed)
		VMIME_TEST(testGenerate)
		VMIME_TEST(testFromTime)
		VMIME_TEST(testCompare)
	VMIME_TEST_LIST_END


	void testParse() {

		struct datetimePair {
			vmime::string parseBuffer;
			vmime::datetime result;
		};

		// Here, we can't test all the possible structures for date/time,
		// so we test some cases. Don't forget to add a new test case
		// each time you encounter a bug in date/time parsing (after
		// you have fixed it).
		datetimePair pairs[] = {

			{ /* 1 */ "Mon, 8 Nov 2004 13:42:56 +0000 (GMT)",
			  vmime::datetime(2004, 11, 8, 13, 42, 56, vmime::datetime::GMT) },

			{ /* 2 */ "Sun,  7 Nov 2004 00:43:22 -0500 (EST)",
			  vmime::datetime(2004, 11, 7, 0, 43, 22, vmime::datetime::GMT_5) },

			{ /* 3 */ "Thu Nov 18 12:11:16 2004",
			  vmime::datetime(2004, 11, 18, 12, 11, 16, vmime::datetime::GMT) },

			{ /* 4 */ "Sat, 18, 2004 22:36:32 -0400",
			  vmime::datetime(2004, 1, 18, 22, 36, 32, vmime::datetime::GMT_4) },

			{ /* 5 */ "Mon Dec 13 21:57:18 2004",
			  vmime::datetime(2004, 12, 13, 21, 57, 18, vmime::datetime::GMT) },

			{ /* 6 */ "18 Nov 2004 21:44:54 +0300",
			  vmime::datetime(2004, 11, 18, 21, 44, 54, vmime::datetime::GMT3) },

			{ /* 7 */ "Thu Nov 18 12:11:16 2004 +0100",
			  vmime::datetime(2004, 11, 18, 12, 11, 16, vmime::datetime::GMT1) }
		};

		for (unsigned int i = 0 ; i < sizeof(pairs) / sizeof(pairs[0]) ; ++i) {

			vmime::datetime d;
			d.parse(pairs[i].parseBuffer);

			std::ostringstream oss;
			oss << (i + 1);

			VASSERT_EQ(oss.str(), pairs[i].result, d);
		}
	}

	void testParseZoneName() {

		vmime::datetime d;

		d.parse("3 Jul 2026 05:00:00 EST");
		VASSERT_EQ("1", vmime::datetime::EST, d.getZone());
		d.parse("3 Jul 2026 05:00:00 pdt");
		VASSERT_EQ("2", vmime::datetime::PDT, d.getZone());
		d.parse("3 Jul 2026 10:00:00 UT");
		VASSERT_EQ("3", 0, d.getZone());

		// RFC 5322 section 4.3: to be treated as -0000
		d.parse("3 Jul 2026 10:00:00 A");
		VASSERT_EQ("4", 0, d.getZone());
		d.parse("3 Jul 2026 10:00:00 IST");
		VASSERT_EQ("5", 0, d.getZone());
		d.parse("3 Jul 2026 10:00:00 CEST");
		VASSERT_EQ("6", 0, d.getZone());
		d.parse("3 Jul 2026 10:00:00 E");
		VASSERT_EQ("7", 0, d.getZone());
	}

	static size_t parseEnd(vmime::datetime& d, const vmime::string& s) {

		vmime::parsingContext ctx;
		size_t newPos = 0;

		d.parse(ctx, s, 0, s.length(), &newPos);

		return newPos;
	}

	void testParseWellFormed() {

		vmime::datetime d;
		vmime::string s;

		s = "Fri, 3 Jul 2026 10:00:00 +0000";
		VASSERT_EQ("1", s.length(), parseEnd(d, s));
		VASSERT_EQ("1v", vmime::datetime(2026, 7, 3, 10, 0, 0, 0), d);

		s = " (c) Fri (c) , 3 (c) jul 2026 10 : 00 (c) +0200 (a (nested) comment) ";
		VASSERT_EQ("2", s.length(), parseEnd(d, s));
		VASSERT_EQ("2v", vmime::datetime(2026, 7, 3, 8, 0, 0, 0), d);

		s = "Friday, 3 July 2026 10:00 GMT";
		VASSERT_EQ("3", s.length(), parseEnd(d, s));
		VASSERT_EQ("3v", vmime::datetime(2026, 7, 3, 10, 0, 0, 0), d);

		// RFC 5322 section 4.3 year rules
		s = "3 Jul 49 10:00:00 -0000";
		VASSERT_EQ("4", s.length(), parseEnd(d, s));
		VASSERT_EQ("4v", 2049, d.getYear());
		s = "3 Jul 50 10:00:00 -0000";
		VASSERT_EQ("5", s.length(), parseEnd(d, s));
		VASSERT_EQ("5v", 1950, d.getYear());
		s = "3 Jul 126 10:00:00 -0000";
		VASSERT_EQ("6", s.length(), parseEnd(d, s));
		VASSERT_EQ("6v", 2026, d.getYear());

		s = "31 Dec 2016 23:59:60 +0000";
		VASSERT_EQ("7", s.length(), parseEnd(d, s));
		VASSERT_EQ("7v", 60, d.getSecond());

		s = "29 Feb 2024 10:00:00 +0000";
		VASSERT_EQ("8", s.length(), parseEnd(d, s));

		s = "3 Jul 2026 12:00:00 +0200 CEST";
		VASSERT_EQ("11", s.length(), parseEnd(d, s));
		VASSERT_EQ("11v", vmime::datetime(2026, 7, 3, 10, 0, 0, 0), d);
		s = "3 Jul 2026 12:00:00 +0200 CEST (c)";
		VASSERT_EQ("12", s.length(), parseEnd(d, s));

		// Trailing garbage is not consumed
		s = "3 Jul 2026 10:00:00 +0000 ;xyz";
		VASSERT_EQ("9", s.length() - 4, parseEnd(d, s));
		VASSERT_EQ("9v", vmime::datetime(2026, 7, 3, 10, 0, 0, 0), d);

		s = "3 Jul 2026 10:00:00 GMT+0200";
		VASSERT_EQ("13", s.length() - 5, parseEnd(d, s));
		s = "3 Jul 2026 10:00:00 +0200 CEST x";
		VASSERT_EQ("14", s.length() - 6, parseEnd(d, s));

		// Bounds are honored
		s = "x 3 Jul 2026 10:00:00 +0000 x";
		size_t newPos = 0;
		vmime::parsingContext ctx;
		d.parse(ctx, s, 2, s.length() - 2, &newPos);
		VASSERT_EQ("10", s.length() - 2, newPos);
	}

	void testParseMalformed() {

		static const char* const inputs[] = {
			"",
			"   ",
			"not a date",
			"by mx1.example.com id 12345",
			"May 4 2026 10:00:00 +0000",
			"Fri 3 Jul 2026 10:00:00 +0000",
			"3 Jul 2026",
			"3 Jul 2026 10:00:00",
			"3 Jul 2026 10:00:00 +000",
			"3 Jul 2026 10:00:00 +00000",
			"3 Jul 2026 10:00:00 +0060",
			"3 Jul 2026 24:00:00 +0000",
			"3 Jul 2026 10:60:00 +0000",
			"3 Jul 2026 10:00:61 +0000",
			"0 Jul 2026 10:00:00 +0000",
			"31 Jun 2026 10:00:00 +0000",
			"29 Feb 2026 10:00:00 +0000",
			"3 Jux 2026 10:00:00 +0000",
			"3 Jul 2 10:00:00 +0000",
			"Sat, 18, 2004 22:36:32 -0400",
		};

		for (unsigned int i = 0 ; i < sizeof(inputs) / sizeof(inputs[0]) ; ++i) {

			vmime::datetime d;

			VASSERT_EQ(inputs[i], 0, parseEnd(d, inputs[i]));
		}
	}

	void testGenerate() {

		vmime::datetime d1(2005, 7, 8, 4, 5, 6, 1 * 60 + 23);

		VASSERT_EQ("1", "Fri, 8 Jul 2005 04:05:06 +0123", d1.generate());
	}

	void testFromTime() {

		VASSERT_EQ("1", "Fri, 3 Jul 2026 10:00:00 +0000",
			vmime::datetime(1783072800).generate());
		VASSERT_EQ("2", "Fri, 3 Jul 2026 12:00:00 +0200",
			vmime::datetime(1783072800, vmime::datetime::GMT2).generate());
		VASSERT_EQ("3", "Thu, 2 Jul 2026 23:30:00 -1030",
			vmime::datetime(1783072800, -630).generate());
	}

	void testCompare() {

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
