//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
	static const bool isLeapYear(const int year);

	/** Return the number of days in the specified month.
	  *
	  * @param year year in 4-digit format (this is needed to check
	  * for leap years)
	  * @param month month, January is 1, December is 12 (see datetime::Months enum)
	  * @return the number of days in the month
	  */
	static const int getDaysInMonth(const int year, const int month);

	/** Convert the specified local time and date to UT (GMT).
	  *
	  * @param date local date/time
	  * @return GMT date/time
	  */
	static const datetime localTimeToUniversalTime(const datetime& date);

	/** Convert the specified UT to local time and date.
	  *
	  * @param date GMT date/time
	  * @param zone local zone to convert to (see datetime::TimeZones enum)
	  * @return local time and date
	  */
	static const datetime universalTimeToLocalTime(const datetime& date, const int zone);
};


} // utility
} // vmime


#endif // VMIME_DATETIMEUTILS_HPP_INCLUDED
