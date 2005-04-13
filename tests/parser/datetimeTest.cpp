//
// VMime library (http://www.vmime.org)
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

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "tests/parser/testUtils.hpp"

using namespace unitpp;


namespace
{
	class datetimeTest : public suite
	{
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

				assert_eq(oss.str(), pairs[i].result, d);
			}
		}

		void testGenerate()
		{
			vmime::datetime d1(2005, 7, 8, 4, 5, 6, 1 * 60 + 23);

			assert_eq("1", "Fri, 8 Jul 2005 04:05:06 +0123", d1.generate());
		}

	public:

		datetimeTest() : suite("vmime::datetime")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse", testcase(this, "Parse", &datetimeTest::testParse));
			add("Generate", testcase(this, "Generate", &datetimeTest::testGenerate));

			suite::main().add("vmime::datetime", this);
		}

	};

	datetimeTest* theTest = new datetimeTest();
}
