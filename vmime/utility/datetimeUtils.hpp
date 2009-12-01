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

#ifndef VMIME_DATETIMEUTILS_HPP_INCLUDED
#define VMIME_DATETIMEUTILS_HPP_INCLUDED


#include "vmime/dateTime.hpp"


namespace vmime {
namespace utility {


/** Miscellaneous functions related to date/time.
  */

class datetimeUtils
{
public:

	/** Test whether the specified year is a leap year.
	  *
	  * @param year year in 4-digit format
	  * @return true if year is a leap year, false otherwise
	  */
	static bool isLeapYear(const int year);

	/** Return the number of days in the specified month.
	  *
	  * @param year year in 4-digit format (this is needed to check
	  * for leap years)
	  * @param month month, January is 1, December is 12 (see datetime::Months enum)
	  * @return the number of days in the month
	  */
	static int getDaysInMonth(const int year, const int month);

	/** Convert the specified date/time to UT (GMT).
	  *
	  * @param date date/time to convert
	  * @return GMT date/time
	  */
	static const datetime toUniversalTime(const datetime& date);

	/** Convert the specified date/time to the specified time zone.
	  *
	  * @param date date/time to convert
	  * @param zone local zone to convert to (see datetime::TimeZones enum)
	  * @return local time and date
	  */
	static const datetime toLocalTime(const datetime& date, const int zone);

	/** Return the day of the week from the specified date.
	  *
	  * @param year year in 4-digit format
	  * @param month month (1-12), January is 1, December is 12 (see datetime::Months enum)
	  * @param day month day (1-31)
	  * @return the day of the week, Sunday is 0, Monday is 1 (see datetime::DaysOfWeek enum)
	  */
	static int getDayOfWeek(const int year, const int month, const int day);

	/** Return the week number in the year (ISO 8601).
	  *
	  * @param year year in 4-digit format
	  * @param month month (1-12), January is 1, December is 12 (see datetime::Months enum)
	  * @param day month day (1-31)
	  * @param iso if TRUE, use ISO week-numbering year (default is to use calendar year).
	  * For more information, read here: http://en.wikipedia.org/wiki/ISO_8601#Week_dates
	  * @return the week number (1 is the first week of the year)
	  */
	static int getWeekOfYear(const int year, const int month, const int day, const bool iso = false);
};


} // utility
} // vmime


#endif // VMIME_DATETIMEUTILS_HPP_INCLUDED
