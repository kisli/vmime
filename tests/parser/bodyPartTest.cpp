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

using namespace unitpp;


namespace
{
	class bodyPartTest : public suite
	{
		static const vmime::string extractComponentString
			(const vmime::string& buffer, const vmime::component& c)
		{
			return vmime::string(buffer.begin() + c.getParsedOffset(),
			                     buffer.begin() + c.getParsedOffset() + c.getParsedLength());
		}


		void testParse()
		{
			vmime::string str1 = "HEADER\r\n\r\nBODY";
			vmime::bodyPart p1;
			p1.parse(str1);

			assert_eq("1", "HEADER\r\n\r\n", extractComponentString(str1, *p1.getHeader()));
			assert_eq("2", "BODY", extractComponentString(str1, *p1.getBody()));

			vmime::string str2 = "HEADER\n\nBODY";
			vmime::bodyPart p2;
			p2.parse(str2);

			assert_eq("3", "HEADER\n\n", extractComponentString(str2, *p2.getHeader()));
			assert_eq("4", "BODY", extractComponentString(str2, *p2.getBody()));

			vmime::string str3 = "HEADER\r\n\nBODY";
			vmime::bodyPart p3;
			p3.parse(str3);

			assert_eq("5", "HEADER\r\n\n", extractComponentString(str3, *p3.getHeader()));
			assert_eq("6", "BODY", extractComponentString(str3, *p3.getBody()));
		}

		void testGenerate()
		{
			vmime::bodyPart p1;
			p1.getHeader()->getField("Foo")->setValue(vmime::string("bar"));
			p1.getBody()->setContents(vmime::stringContentHandler("Baz"));

			assert_eq("1", "Foo: bar\r\n\r\nBaz", p1.generate());
		}

	public:

		bodyPartTest() : suite("vmime::bodyPart")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse", testcase(this, "Parse", &bodyPartTest::testParse));

			add("Generate", testcase(this, "Generate", &bodyPartTest::testGenerate));

			suite::main().add("vmime::bodyPart", this);
		}

	};

	bodyPartTest* theTest = new bodyPartTest();
}
