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

#include "vmime/dateTime.hpp"
#include "vmime/utility/datetimeUtils.hpp"


#define VMIME_TEST_SUITE         datetimeUtilsTest
#define VMIME_TEST_SUITE_MODULE  "Utility"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testIsLeapYear)
		VMIME_TEST(testGetDaysInMonth)
		VMIME_TEST(testGetDaysInMonthLeapYear)
		VMIME_TEST(testToUniversalTime)
		VMIME_TEST(testToLocalTime)
		VMIME_TEST(testGetDayOfWeek)
		VMIME_TEST(testGetWeekOfYear)
	VMIME_TEST_LIST_END


	typedef vmime::utility::datetimeUtils datetimeUtils;


	void testIsLeapYear()
	{
		VASSERT_EQ("1", false, datetimeUtils::isLeapYear(1999));
		VASSERT_EQ("2", false, datetimeUtils::isLeapYear(1800));
		VASSERT_EQ("3", false, datetimeUtils::isLeapYear(1900));
		VASSERT_EQ("4", false, datetimeUtils::isLeapYear(2100));
		VASSERT_EQ("5", false, datetimeUtils::isLeapYear(2200));

		VASSERT_EQ("6", true, datetimeUtils::isLeapYear(1996));
		VASSERT_EQ("7", true, datetimeUtils::isLeapYear(2000));
	}

	void testGetDaysInMonth()
	{
		VASSERT_EQ("1",  31, datetimeUtils::getDaysInMonth(2006, 1));
		VASSERT_EQ("2",  28, datetimeUtils::getDaysInMonth(2006, 2));
		VASSERT_EQ("3",  31, datetimeUtils::getDaysInMonth(2006, 3));
		VASSERT_EQ("4",  30, datetimeUtils::getDaysInMonth(2006, 4));
		VASSERT_EQ("5",  31, datetimeUtils::getDaysInMonth(2006, 5));
		VASSERT_EQ("6",  30, datetimeUtils::getDaysInMonth(2006, 6));
		VASSERT_EQ("7",  31, datetimeUtils::getDaysInMonth(2006, 7));
		VASSERT_EQ("8",  31, datetimeUtils::getDaysInMonth(2006, 8));
		VASSERT_EQ("9",  30, datetimeUtils::getDaysInMonth(2006, 9));
		VASSERT_EQ("10", 31, datetimeUtils::getDaysInMonth(2006, 10));
		VASSERT_EQ("11", 30, datetimeUtils::getDaysInMonth(2006, 11));
		VASSERT_EQ("12", 31, datetimeUtils::getDaysInMonth(2006, 12));
	}

	void testGetDaysInMonthLeapYear()
	{
		VASSERT_EQ("1",  31, datetimeUtils::getDaysInMonth(2004, 1));
		VASSERT_EQ("2",  29, datetimeUtils::getDaysInMonth(2004, 2));
		VASSERT_EQ("3",  31, datetimeUtils::getDaysInMonth(2004, 3));
		VASSERT_EQ("4",  30, datetimeUtils::getDaysInMonth(2004, 4));
		VASSERT_EQ("5",  31, datetimeUtils::getDaysInMonth(2004, 5));
		VASSERT_EQ("6",  30, datetimeUtils::getDaysInMonth(2004, 6));
		VASSERT_EQ("7",  31, datetimeUtils::getDaysInMonth(2004, 7));
		VASSERT_EQ("8",  31, datetimeUtils::getDaysInMonth(2004, 8));
		VASSERT_EQ("9",  30, datetimeUtils::getDaysInMonth(2004, 9));
		VASSERT_EQ("10", 31, datetimeUtils::getDaysInMonth(2004, 10));
		VASSERT_EQ("11", 30, datetimeUtils::getDaysInMonth(2004, 11));
		VASSERT_EQ("12", 31, datetimeUtils::getDaysInMonth(2004, 12));
	}

	void testToUniversalTime()
	{
		const vmime::datetime local
			(2005, 12, 2, 12, 34, 56, -789);

		const vmime::datetime gmt =
			datetimeUtils::toUniversalTime(local);

		// 789 is 13 hours, 9 minutes later
		VASSERT_EQ("1", 2005, gmt.getYear());
		VASSERT_EQ("2",   12, gmt.getMonth());
		VASSERT_EQ("3",    3, gmt.getDay());
		VASSERT_EQ("4",    1, gmt.getHour());
		VASSERT_EQ("5",   43, gmt.getMinute());
		VASSERT_EQ("6",   56, gmt.getSecond());
		VASSERT_EQ("7",    0, gmt.getZone());
	}

	void testToLocalTime()
	{
		const vmime::datetime date
			(2005, 12, 2, 12, 34, 56, -120); // GMT-2

		const vmime::datetime local =
			datetimeUtils::toLocalTime(date, 120); // GMT+2

		VASSERT_EQ("1", 2005, local.getYear());
		VASSERT_EQ("2",   12, local.getMonth());
		VASSERT_EQ("3",    2, local.getDay());
		VASSERT_EQ("4",   16, local.getHour());
		VASSERT_EQ("5",   34, local.getMinute());
		VASSERT_EQ("6",   56, local.getSecond());
		VASSERT_EQ("7",  120, local.getZone());
	}

	void testGetDayOfWeek()
	{
		VASSERT_EQ("1", vmime::datetime::WEDNESDAY, datetimeUtils::getDayOfWeek(1969, 12, 31));
		VASSERT_EQ("2", vmime::datetime::FRIDAY,    datetimeUtils::getDayOfWeek(1976,  4,  9));
		VASSERT_EQ("3", vmime::datetime::TUESDAY,   datetimeUtils::getDayOfWeek(1987,  6, 23));
		VASSERT_EQ("4", vmime::datetime::SATURDAY,  datetimeUtils::getDayOfWeek(1990,  1, 13));
		VASSERT_EQ("5", vmime::datetime::MONDAY,    datetimeUtils::getDayOfWeek(1999,  9, 20));
		VASSERT_EQ("6", vmime::datetime::THURSDAY,  datetimeUtils::getDayOfWeek(2003,  2, 27));
		VASSERT_EQ("7", vmime::datetime::SATURDAY,  datetimeUtils::getDayOfWeek(2005, 11, 19));
		VASSERT_EQ("8", vmime::datetime::WEDNESDAY, datetimeUtils::getDayOfWeek(2012,  5, 16));
		VASSERT_EQ("9", vmime::datetime::FRIDAY,    datetimeUtils::getDayOfWeek(2027,  3, 12));
	}

	void testGetWeekOfYear()
	{
		VASSERT_EQ("1.1", 52, datetimeUtils::getWeekOfYear(2003, 12, 27));
		VASSERT_EQ("1.2", 52, datetimeUtils::getWeekOfYear(2003, 12, 28));
		VASSERT_EQ("1.3",  1, datetimeUtils::getWeekOfYear(2003, 12, 29, true));
		VASSERT_EQ("1.4", 53, datetimeUtils::getWeekOfYear(2003, 12, 29, false));
		VASSERT_EQ("1.5",  1, datetimeUtils::getWeekOfYear(2004,  1,  4));
		VASSERT_EQ("1.6",  2, datetimeUtils::getWeekOfYear(2004,  1,  5));
		VASSERT_EQ("1.7",  2, datetimeUtils::getWeekOfYear(2004,  1, 11));

		VASSERT_EQ("2.1", 52, datetimeUtils::getWeekOfYear(2004, 12, 26));
		VASSERT_EQ("2.2", 53, datetimeUtils::getWeekOfYear(2004, 12, 27));
		VASSERT_EQ("2.3", 53, datetimeUtils::getWeekOfYear(2005,  1,  2));
		VASSERT_EQ("2.4",  1, datetimeUtils::getWeekOfYear(2005,  1,  3));
		VASSERT_EQ("2.5",  1, datetimeUtils::getWeekOfYear(2005,  1,  4));
		VASSERT_EQ("2.6",  2, datetimeUtils::getWeekOfYear(2005,  1, 11));

		VASSERT_EQ("3.1",  9, datetimeUtils::getWeekOfYear(2027,  3,  7));
		VASSERT_EQ("3.2", 10, datetimeUtils::getWeekOfYear(2027,  3,  8));
		VASSERT_EQ("3.3", 10, datetimeUtils::getWeekOfYear(2027,  3, 14));
		VASSERT_EQ("3.4", 11, datetimeUtils::getWeekOfYear(2027,  3, 15));
	}

VMIME_TEST_SUITE_END

