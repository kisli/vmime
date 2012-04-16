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

#include <iomanip>

#include "vmime/dateTime.hpp"
#include "vmime/platform.hpp"
#include "vmime/parserHelpers.hpp"

#include "vmime/utility/datetimeUtils.hpp"


namespace vmime
{

/*

 RFC #822:
 5. DATE AND TIME SPECIFICATION

date-time = [ day "," ] date time                 ; dd mm yy
                                                  ; hh:mm:ss zzz
day = "Mon" / "Tue" / "Wed" / "Thu" /
      "Fri" / "Sat" / "Sun"

date = 1*2DIGIT month 2DIGIT                      ; day month year
                                                  ; e.g. 20 Jun 82
month = "Jan" / "Feb" / "Mar" / "Apr" /
        "May" / "Jun" / "Jul" / "Aug" /
        "Sep" / "Oct" / "Nov" / "Dec"

time = hour zone                                  ; ANSI and Military

hour = 2DIGIT ":" 2DIGIT [":" 2DIGIT]             ; 00:00:00 - 23:59:59

zone = "UT" / "GMT"                               ; Universal Time
                                                  ; North American : UT
       / "EST" / "EDT"                            ; Eastern: - 5/ - 4
       / "CST" / "CDT"                            ; Central: - 6/ - 5
       / "MST" / "MDT"                            ; Mountain: - 7/ - 6
       / "PST" / "PDT"                            ; Pacific: - 8/ - 7
       / 1ALPHA                                   ; Military: Z = UT;
                                                  ; A:-1; (J not used)
                                                  ; M:-12; N:+1; Y:+12
       / ( ("+" / "-") 4DIGIT )                   ; Local differential
                                                  ; hours+min. (HHMM)
*/


void datetime::parseImpl(const string& buffer, const string::size_type position,
	const string::size_type end, string::size_type* newPosition)
{
	const string::value_type* const pend = buffer.data() + end;
	const string::value_type* p = buffer.data() + position;

	// Parse the date and time value
	while (p < pend && parserHelpers::isSpace(*p)) ++p;

	if (p < pend)
	{
		if (parserHelpers::isAlpha(*p))
		{
			// Ignore week day
			while (p < pend && parserHelpers::isAlpha(*p)) ++p;
			while (p < pend && parserHelpers::isSpace(*p)) ++p;
			if (p < pend && *p == ',') ++p;
			while (p < pend && parserHelpers::isSpace(*p)) ++p;
		}

		bool dayParsed = false;

		if (parserHelpers::isAlpha(*p))
		{
			// Ill-formed date/time, this may be the month,
			// so we skip day parsing (will be done later)
		}
		else
		{
			while (p < pend && !parserHelpers::isDigit(*p)) ++p;

			if (p < pend && parserHelpers::isDigit(*p))
			{
				// Month day
				int day = 0;

				do
				{
					day = day * 10 + (*p - '0');
					++p;
				}
				while (p < pend && parserHelpers::isDigit(*p));

				m_day = (day >= 1 && day <= 31) ? day : 1;

				while (p < pend && !parserHelpers::isSpace(*p)) ++p;
				while (p < pend && parserHelpers::isSpace(*p)) ++p;
			}
			else
			{
				m_day = 1;

				// Skip everything to the next field
				while (p < pend && !parserHelpers::isSpace(*p)) ++p;
				while (p < pend && parserHelpers::isSpace(*p)) ++p;
			}

			dayParsed = true;
		}

		if (p < pend && parserHelpers::isAlpha(*p))
		{
			// Month
			char_t month[4] = { 0 };
			int monthLength = 0;

			do
			{
				month[monthLength++] = *p;
				++p;
			}
			while (monthLength < 3 && p < pend && parserHelpers::isAlpha(*p));

			while (p < pend && parserHelpers::isAlpha(*p)) ++p;

			switch (month[0])
			{
			case 'a':
			case 'A':
			{
				if (month[1] == 'u' || month[1] == 'U')
					m_month = AUGUST;
				else
					m_month = APRIL; // by default

				break;
			}
			case 'd':
			case 'D':
			{
				m_month = DECEMBER;
				break;
			}
			case 'f':
			case 'F':
			{
				m_month = FEBRUARY;
				break;
			}
			case 'j':
			case 'J':
			{
				if (month[1] == 'u' || month[1] == 'U')
				{
					if (month[2] == 'l' || month[2] == 'L')
						m_month = JULY;
					else // if (month[2] == 'n' || month[2] == 'N')
						m_month = JUNE;
				}
				else
				{
					m_month = JANUARY; // by default
				}

				break;
			}
			case 'm':
			case 'M':
			{
				if ((month[1] == 'a' || month[1] == 'A') &&
				    (month[2] == 'y' || month[2] == 'Y'))
				{
					m_month = MAY;
				}
				else
				{
					m_month = MARCH; // by default
				}

				break;
			}
			case 'n':
			case 'N':
			{
				m_month = NOVEMBER;
				break;
			}
			case 'o':
			case 'O':
			{
				m_month = OCTOBER;
				break;
			}
			case 's':
			case 'S':
			{
				m_month = SEPTEMBER;
				break;
			}
			default:
			{
				m_month = JANUARY; // by default
				break;
			}

			}

			while (p < pend && !parserHelpers::isSpace(*p)) ++p;
			while (p < pend && parserHelpers::isSpace(*p)) ++p;
		}
		else
		{
			m_month = JANUARY;

			if (parserHelpers::isDigit(*p))
			{
				// Here, we expected a month, but it maybe
				// a ill-formed date, so try to parse a year
				// (we don't skip anything).
			}
			else
			{
				// Skip everything to the next field
				while (p < pend && !parserHelpers::isSpace(*p)) ++p;
				while (p < pend && parserHelpers::isSpace(*p)) ++p;
			}
		}

		if (!dayParsed && p < pend && parserHelpers::isDigit(*p))
		{
			// Month day
			int day = 0;

			do
			{
				day = day * 10 + (*p - '0');
				++p;
			}
			while (p < pend && parserHelpers::isDigit(*p));

			m_day = (day >= 1 && day <= 31) ? day : 1;

			while (p < pend && !parserHelpers::isSpace(*p)) ++p;
			while (p < pend && parserHelpers::isSpace(*p)) ++p;
		}

		if (p < pend && parserHelpers::isDigit(*p))
		{
			// Check for ill-formed date/time and try to recover
			if (p + 2 < pend && *(p + 2) == ':')
			{
				// Skip year (default to current), and advance
				// to time parsing
				m_year = now().getYear();
			}
			else
			{
				// Year
				int year = 0;

				do
				{
					year = year * 10 + (*p - '0');
					++p;
				}
				while (p < pend && parserHelpers::isDigit(*p));

				if (year < 70)         m_year = year + 2000;
				else if (year < 1000)  m_year = year + 1900;
				else                   m_year = year;

				while (p < pend && !parserHelpers::isSpace(*p)) ++p;
				while (p < pend && parserHelpers::isSpace(*p)) ++p;
			}
		}
		else
		{
			m_year = 1970;

			// Skip everything to the next field
			while (p < pend && !parserHelpers::isSpace(*p)) ++p;
			while (p < pend && parserHelpers::isSpace(*p)) ++p;
		}

		if (p < pend && parserHelpers::isDigit(*p))
		{
			// Hour
			int hour = 0;

			do
			{
				hour = hour * 10 + (*p - '0');
				++p;
			}
			while (p < pend && parserHelpers::isDigit(*p));

			m_hour = (hour >= 0 && hour <= 23) ? hour : 0;

			while (p < pend && parserHelpers::isSpace(*p)) ++p;

			if (p < pend && *p == ':')
			{
				++p;

				while (p < pend && parserHelpers::isSpace(*p)) ++p;

				if (p < pend && parserHelpers::isDigit(*p))
				{
					// Minute
					int minute = 0;

					do
					{
						minute = minute * 10 + (*p - '0');
						++p;
					}
					while (p < pend && parserHelpers::isDigit(*p));

					m_minute = (minute >= 0 && minute <= 59) ? minute : 0;

					while (p < pend && parserHelpers::isSpace(*p)) ++p;

					if (p < pend && *p == ':')
					{
						++p;

						while (p < pend && parserHelpers::isSpace(*p)) ++p;

						if (p < pend && parserHelpers::isDigit(*p))
						{
							// Second
							int second = 0;

							do
							{
								second = second * 10 + (*p - '0');
								++p;
							}
							while (p < pend && parserHelpers::isDigit(*p));

							m_second = (second >= 0 && second <= 59) ? second : 0;

							while (p < pend && !parserHelpers::isSpace(*p)) ++p;
							while (p < pend && parserHelpers::isSpace(*p)) ++p;
						}
						else
						{
							m_second = 0;
						}
					}
					else
					{
						m_second = 0;
					}
				}
				else
				{
					m_minute = 0;
				}
			}
			else
			{
				m_minute = 0;
			}
		}
		else
		{
			m_hour = 0;

			// Skip everything to the next field
			while (p < pend && !parserHelpers::isSpace(*p)) ++p;
			while (p < pend && parserHelpers::isSpace(*p)) ++p;
		}

		if (p + 1 < pend && (*p == '+' || *p == '-') && parserHelpers::isDigit(*(p + 1)))
		{
			const char_t sign = *p;
			++p;

			// Zone offset (in hour/minutes)
			int offset = 0;

			do
			{
				offset = offset * 10 + (*p - '0');
				++p;
			}
			while (p < pend && parserHelpers::isDigit(*p));

			const int hourOff = offset / 100;
			const int minOff = offset % 100;

			if (sign == '+')
				m_zone = hourOff * 60 + minOff;
			else
				m_zone = -(hourOff * 60 + minOff);
		}
		else if (p < pend && isalpha(*p))
		{
			bool done = false;

			// Zone offset (Time zone name)
			char_t zone[4] = { 0 };
			int zoneLength = 0;

			do
			{
				zone[zoneLength++] = *p;
				++p;
			}
			while (zoneLength < 3 && p < pend);

			switch (zone[0])
			{
			case 'c':
			case 'C':
			{
				if (zoneLength >= 2)
				{
					if (zone[1] == 's' || zone[1] == 'S')
						m_zone = CST;
					else
						m_zone = CDT;

					done = true;
				}

				break;
			}
			case 'e':
			case 'E':
			{
				if (zoneLength >= 2)
				{
					if (zone[1] == 's' || zone[1] == 'S')
						m_zone = EST;
					else
						m_zone = EDT;

					done = true;
				}

				break;
			}
			case 'm':
			case 'M':
			{
				if (zoneLength >= 2)
				{
					if (zone[1] == 's' || zone[1] == 'S')
						m_zone = MST;
					else
						m_zone = MDT;

					done = true;
				}

				break;
			}
			case 'p':
			case 'P':
			{
				if (zoneLength >= 2)
				{
					if (zone[1] == 's' || zone[1] == 'S')
						m_zone = PST;
					else
						m_zone = PDT;

					done = true;
				}

				break;
			}
			case 'g':
			case 'G':
			case 'u':
			case 'U':
			{
				if (zoneLength >= 2)
				{
					m_zone = GMT;  // = UTC
					done = true;
				}

				break;
			}

			}

			if (!done)
			{
				const char_t z = zone[0];

				// Military time zone
				if (z != 'j' && z != 'J')
				{
					typedef std::map <char_t, int> Map;
					static const Map::value_type offsetMapInit[] =
					{
						Map::value_type('a', -60),
						Map::value_type('b', -120),
						Map::value_type('c', -180),
						Map::value_type('d', -240),
						Map::value_type('e', -300),
						Map::value_type('f', -360),
						Map::value_type('g', -420),
						Map::value_type('h', -480),
						Map::value_type('i', -540),
						Map::value_type('k', -600),
						Map::value_type('l', -660),
						Map::value_type('m', -720),

						Map::value_type('n', 60),
						Map::value_type('o', 120),
						Map::value_type('p', 180),
						Map::value_type('q', 240),
						Map::value_type('r', 300),
						Map::value_type('s', 360),
						Map::value_type('t', 420),
						Map::value_type('u', 480),
						Map::value_type('v', 540),
						Map::value_type('w', 600),
						Map::value_type('x', 660),
						Map::value_type('y', 720),

						Map::value_type('z', 0),
					};
					static const Map offsetMap
						(::vmime::begin(offsetMapInit),
						 ::vmime::end(offsetMapInit));

					Map::const_iterator pos =
						offsetMap.find(parserHelpers::toLower(z));

					if (pos != offsetMap.end())
						m_zone = (*pos).second;
					else
						m_zone = GMT;
				}
				else
				{
					m_zone = GMT;
				}
			}
		}
		else
		{
			m_zone = 0;
		}
	}
	else
	{
		m_year = 1970;
		m_month = JANUARY;
		m_day = 1;

		m_hour = 0;
		m_minute = 0;
		m_second = 0;

		m_zone = 0;
	}

	setParsedBounds(position, end);

	if (newPosition)
		*newPosition = end;
}


void datetime::generateImpl(utility::outputStream& os, const string::size_type /* maxLineLength */,
	const string::size_type curLinePos, string::size_type* newLinePos) const
{
	static const string::value_type* dayNames[] =
		{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const string::value_type* monthNames[] =
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	const int z = ((m_zone < 0) ? -m_zone : m_zone);
	const int zh = z / 60;
	const int zm = z % 60;

	std::ostringstream oss;
	oss.imbue(std::locale::classic());

	oss << dayNames[getWeekDay()] << ", "
	    << m_day << " " << monthNames[m_month - 1] << " " << m_year
	    << " " << std::setfill('0') << std::setw(2) << m_hour << ":"
	    << std::setfill('0') << std::setw(2) << m_minute << ":"
	    << std::setfill('0') << std::setw(2) << m_second
	    << " " << ((m_zone < 0) ? '-' : '+') << std::setfill('0') << std::setw(2) << zh
	    << std::setfill('0') << std::setw(2) << zm;

	const string& str = oss.str();
	os << str;

	if (newLinePos)
		*newLinePos = curLinePos + str.length();
}


datetime::datetime()
	: m_year(1970), m_month(1), m_day(1),
	  m_hour(0), m_minute(0), m_second(0), m_zone(0)
{
}


datetime::datetime(const int year, const int month, const int day)
	: m_year(year), m_month(month), m_day(day),
	  m_hour(0), m_minute(0), m_second(0), m_zone(0)
{
}


datetime::datetime(const int year, const int month, const int day,
                   const int hour, const int minute, const int second,
                   const int zone)
	: m_year(year), m_month(month), m_day(day),
	  m_hour(hour), m_minute(minute), m_second(second), m_zone(zone)
{
}


datetime::datetime(const datetime& d)
	: headerFieldValue(), m_year(d.m_year), m_month(d.m_month), m_day(d.m_day),
	  m_hour(d.m_hour), m_minute(d.m_minute), m_second(d.m_second), m_zone(d.m_zone)
{
}


datetime::datetime(const time_t t, const int zone)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
	// These functions are reentrant in MS C runtime library
	struct tm* gtm = gmtime(&t);
	struct tm* ltm = localtime(&t);

	struct tm tms;

	if (gtm)
		tms = *gtm;
	else if (ltm)
		tms = *ltm;
#elif defined(_REENTRANT)
	struct tm tms;

	if (!gmtime_r(&t, &tms))
		localtime_r(&t, &tms);
#else
	struct tm* gtm = gmtime(&t);
	struct tm* ltm = localtime(&t);

	struct tm tms;

	if (gtm)
		tms = *gtm;
	else if (ltm)
		tms = *ltm;
#endif // _REENTRANT

	m_year = tms.tm_year + 1900;
	m_month = tms.tm_mon + 1;
	m_day = tms.tm_mday;
	m_hour = tms.tm_hour;
	m_minute = tms.tm_min;
	m_second = tms.tm_sec;
	m_zone = zone;
}


datetime::datetime(const string& date)
{
	parse(date);
}


datetime::~datetime()
{
}


void datetime::copyFrom(const component& other)
{
	const datetime& d = dynamic_cast <const datetime&>(other);

	m_year = d.m_year;
	m_month = d.m_month;
	m_day = d.m_day;
	m_hour = d.m_hour;
	m_minute = d.m_minute;
	m_second = d.m_second;
	m_zone = d.m_zone;
}


datetime& datetime::operator=(const datetime& other)
{
	copyFrom(other);
	return (*this);
}


datetime& datetime::operator=(const string& s)
{
	parse(s);
	return (*this);
}


void datetime::getTime(int& hour, int& minute, int& second, int& zone) const
{
	hour = m_hour;
	minute = m_minute;
	second = m_second;
	zone = m_zone;
}


void datetime::getTime(int& hour, int& minute, int& second) const
{
	hour = m_hour;
	minute = m_minute;
	second = m_second;
}


void datetime::getDate(int& year, int& month, int& day) const
{
	year = m_year;
	month = m_month;
	day = m_day;
}


void datetime::setTime(const int hour, const int minute,
                       const int second, const int zone)
{
	m_hour = hour;
	m_minute = minute;
	m_second = second;
	m_zone = zone;
}


void datetime::setDate(const int year, const int month, const int day)
{
	m_year = year;
	m_month = month;
	m_day = day;
}


const datetime datetime::now()
{
	return (platform::getHandler()->getCurrentLocalTime());
}


ref <component> datetime::clone() const
{
	return vmime::create <datetime>(*this);
}


const std::vector <ref <component> > datetime::getChildComponents()
{
	return std::vector <ref <component> >();
}


int datetime::getYear() const { return (m_year); }
int datetime::getMonth() const { return (m_month); }
int datetime::getDay() const { return (m_day); }
int datetime::getHour() const { return (m_hour); }
int datetime::getMinute() const { return (m_minute); }
int datetime::getSecond() const { return (m_second); }
int datetime::getZone() const { return (m_zone); }
int datetime::getWeekDay() const { return (utility::datetimeUtils::getDayOfWeek(m_year, m_month, m_day)); }
int datetime::getWeek() const { return utility::datetimeUtils::getWeekOfYear(m_year, m_month, m_day); }

void datetime::setYear(const int year) { m_year = year; }
void datetime::setMonth(const int month) { m_month = std::min(std::max(month, 1), 12); }
void datetime::setDay(const int day) { m_day = day; }
void datetime::setHour(const int hour) { m_hour = hour; }
void datetime::setMinute(const int minute) { m_minute = minute; }
void datetime::setSecond(const int second) { m_second = second; }
void datetime::setZone(const int zone) { m_zone = zone; }


bool datetime::operator==(const datetime& other) const
{
	const datetime ut1 = utility::datetimeUtils::toUniversalTime(*this);
	const datetime ut2 = utility::datetimeUtils::toUniversalTime(other);

	return (ut1.m_year   == ut2.m_year   &&
	        ut1.m_month  == ut2.m_month  &&
	        ut1.m_day    == ut2.m_day    &&
	        ut1.m_hour   == ut2.m_hour   &&
	        ut1.m_minute == ut2.m_minute &&
	        ut1.m_second == ut2.m_second);
}


bool datetime::operator!=(const datetime& other) const
{
	const datetime ut1 = utility::datetimeUtils::toUniversalTime(*this);
	const datetime ut2 = utility::datetimeUtils::toUniversalTime(other);

	return (ut1.m_year   != ut2.m_year   ||
	        ut1.m_month  != ut2.m_month  ||
	        ut1.m_day    != ut2.m_day    ||
	        ut1.m_hour   != ut2.m_hour   ||
	        ut1.m_minute != ut2.m_minute ||
	        ut1.m_second != ut2.m_second);
}


bool datetime::operator<(const datetime& other) const
{
	const datetime ut1 = utility::datetimeUtils::toUniversalTime(*this);
	const datetime ut2 = utility::datetimeUtils::toUniversalTime(other);

	return ((ut1.m_year    <  ut2.m_year) ||
	        ((ut1.m_year   == ut2.m_year)   && ((ut1.m_month  < ut2.m_month)  ||
	        ((ut1.m_month  == ut2.m_month)  && ((ut1.m_day    < ut2.m_day)    ||
	        ((ut1.m_day    == ut2.m_day)    && ((ut1.m_hour   < ut2.m_hour)   ||
	        ((ut1.m_hour   == ut2.m_hour)   && ((ut1.m_minute < ut2.m_minute) ||
	        ((ut1.m_minute == ut2.m_minute) && ((ut1.m_second < ut2.m_second))))))))))));
}


bool datetime::operator<=(const datetime& other) const
{
	const datetime ut1 = utility::datetimeUtils::toUniversalTime(*this);
	const datetime ut2 = utility::datetimeUtils::toUniversalTime(other);

	return ((ut1.m_year    <  ut2.m_year) ||
	        ((ut1.m_year   == ut2.m_year)   && ((ut1.m_month  <  ut2.m_month)  ||
	        ((ut1.m_month  == ut2.m_month)  && ((ut1.m_day    <  ut2.m_day)    ||
	        ((ut1.m_day    == ut2.m_day)    && ((ut1.m_hour   <  ut2.m_hour)   ||
	        ((ut1.m_hour   == ut2.m_hour)   && ((ut1.m_minute <  ut2.m_minute) ||
	        ((ut1.m_minute == ut2.m_minute) && ((ut1.m_second <= ut2.m_second))))))))))));
}


bool datetime::operator>(const datetime& other) const
{
	const datetime ut1 = utility::datetimeUtils::toUniversalTime(*this);
	const datetime ut2 = utility::datetimeUtils::toUniversalTime(other);

	return ((ut1.m_year    >  ut2.m_year) ||
	        ((ut1.m_year   == ut2.m_year)   && ((ut1.m_month  > ut2.m_month)  ||
	        ((ut1.m_month  == ut2.m_month)  && ((ut1.m_day    > ut2.m_day)    ||
	        ((ut1.m_day    == ut2.m_day)    && ((ut1.m_hour   > ut2.m_hour)   ||
	        ((ut1.m_hour   == ut2.m_hour)   && ((ut1.m_minute > ut2.m_minute) ||
	        ((ut1.m_minute == ut2.m_minute) && (ut1.m_second > ut2.m_second)))))))))));
}


bool datetime::operator>=(const datetime& other) const
{
	const datetime ut1 = utility::datetimeUtils::toUniversalTime(*this);
	const datetime ut2 = utility::datetimeUtils::toUniversalTime(other);

	return ((ut1.m_year    >  ut2.m_year) ||
	        ((ut1.m_year   == ut2.m_year)   && ((ut1.m_month  >  ut2.m_month)  ||
	        ((ut1.m_month  == ut2.m_month)  && ((ut1.m_day    >  ut2.m_day)    ||
	        ((ut1.m_day    == ut2.m_day)    && ((ut1.m_hour   >  ut2.m_hour)   ||
	        ((ut1.m_hour   == ut2.m_hour)   && ((ut1.m_minute >  ut2.m_minute) ||
	        ((ut1.m_minute == ut2.m_minute) && (ut1.m_second >= ut2.m_second)))))))))));
}


} // vmime
