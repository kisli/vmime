//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/utility/datetimeUtils.hpp"


namespace vmime {
namespace utility {


#ifndef VMIME_BUILDING_DOC

static inline void nextMonth(datetime& d)
{
	if (d.getMonth() >= 12)
	{
		d.setMonth(1);
		d.setYear(d.getYear() + 1);
	}
	else
	{
		d.setMonth(d.getMonth() + 1);
	}
}


static inline void prevMonth(datetime& d)
{
	if (d.getMonth() <= 1)
	{
		d.setYear(d.getYear() - 1);
		d.setMonth(12);
	}
	else
	{
		d.setMonth(d.getMonth() - 1);
	}
}


static inline void nextDay(datetime& d)
{

	if (d.getDay() >= datetimeUtils::getDaysInMonth(d.getYear(), d.getMonth()))
	{
		d.setDay(1);
		nextMonth(d);
	}
	else
	{
		d.setDay(d.getDay() + 1);
	}
}


static inline void prevDay(datetime& d)
{
	if (d.getDay() <= 1)
	{
		prevMonth(d);
		d.setDay(datetimeUtils::getDaysInMonth(d.getYear(), d.getMonth()));
	}
	else
	{
		d.setDay(d.getDay() - 1);
	}
}


static inline void nextHour(datetime& d)
{
	if (d.getHour() >= 23)
	{
		d.setHour(0);
		nextDay(d);
	}
	else
	{
		d.setHour(d.getHour() + 1);
	}
}


static inline void prevHour(datetime& d)
{
	if (d.getHour() <= 0)
	{
		d.setHour(23);
		prevDay(d);
	}
	else
	{
		d.setHour(d.getHour() - 1);
	}
}


static inline void addHoursAndMinutes(datetime& d, const int h, const int m)
{
	d.setMinute(d.getMinute() + m);

	if (d.getMinute() >= 60)
	{
		d.setMinute(d.getMinute() - 60);
		nextHour(d);
	}

	d.setHour(d.getHour() + h);

	if (d.getHour() >= 24)
	{
		d.setHour(d.getHour() - 24);
		nextDay(d);
	}
}


static inline void substractHoursAndMinutes(datetime& d, const int h, const int m)
{
	if (m > d.getMinute())
	{
		d.setMinute(60 - (m - d.getMinute()));
		prevHour(d);
	}
	else
	{
		d.setMinute(d.getMinute() - m);
	}

	if (h > d.getHour())
	{
		d.setHour(24 - (h - d.getHour()));
		prevDay(d);
	}
	else
	{
		d.setHour(d.getHour() - h);
	}
}

#endif // VMIME_BUILDING_DOC


const datetime datetimeUtils::localTimeToUniversalTime(const datetime& date)
{
	if (date.getZone() == datetime::GMT)
		return (date);

	datetime nd(date);
	nd.setZone(datetime::GMT);

	const int z = date.getZone();
	const int h = (z < 0) ? (-z / 60) : (z / 60);
	const int m = (z < 0) ? (-z - h * 60) : (z - h * 60);

	if (z < 0)  // GMT-hhmm: add hours and minutes to date
		addHoursAndMinutes(nd, h, m);
	else        // GMT+hhmm: substract hours and minutes from date
		substractHoursAndMinutes(nd, h, m);

	return (nd);
}


const datetime datetimeUtils::universalTimeToLocalTime(const datetime& date, const int zone)
{
	if (zone == 0)
		return (date);

	datetime nd(date);
	nd.setZone(zone);

	const int z = zone;
	const int h = (z < 0) ? (-z / 60) : (z / 60);
	const int m = (z < 0) ? (-z - h * 60) : (z - h * 60);

	if (z < 0)  // GMT+hhmm: substract hours and minutes from date
		substractHoursAndMinutes(nd, h, m);
	else        // GMT-hhmm: add hours and minutes to date
		addHoursAndMinutes(nd, h, m);

	return (nd);
}


const bool datetimeUtils::isLeapYear(const int year)
{
	// From RFC 3339 - Appendix C. Leap Years:
	return ((year % 4) == 0 && (year % 100 != 0 || year % 400 == 0));
}


const int datetimeUtils::getDaysInMonth(const int year, const int month)
{
	static const int daysInMonth[12] =
		{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	static const int daysInMonthLeapYear[12] =
		{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	return (isLeapYear(year) ? daysInMonthLeapYear[month - 1] : daysInMonth[month - 1]);
}


const int datetimeUtils::getDayOfWeek(const int year, const int month, const int day)
{
	int y = year;
	int m = month;

	// From RFC-3339 - Appendix B. Day of the Week

	// Adjust months so February is the last one
	m -= 2;

	if (m < 1)
	{
		m += 12;
		--y;
	}

	// Split by century
	const int cent = y / 100;
	y %= 100;

	return (((26 * m - 2) / 10 + day + y + (y >> 2) + (cent >> 2) + 5 * cent) % 7);
}


} // utility
} // vmime
