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

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"
#include "vmime/utility/stringUtils.hpp"

using namespace unitpp;


namespace
{
	class stringUtilsTest : public suite
	{
		void testIsStringEqualNoCase1()
		{
			assert_eq("1", true, vmime::stringUtils::isStringEqualNoCase(vmime::string("foo"), "foo", 3));
			assert_eq("2", true, vmime::stringUtils::isStringEqualNoCase(vmime::string("FOo"), "foo", 3));

			assert_eq("3", false, vmime::stringUtils::isStringEqualNoCase(vmime::string("foo"), "FOo", 3));
			assert_eq("4", false, vmime::stringUtils::isStringEqualNoCase(vmime::string("foo"), "bar", 3));
		}

		void testIsStringEqualNoCase2()
		{
			assert_eq("1", true, vmime::stringUtils::isStringEqualNoCase(vmime::string("foo"), vmime::string("foo")));
			assert_eq("2", true, vmime::stringUtils::isStringEqualNoCase(vmime::string("FOo"), vmime::string("foo")));
			assert_eq("3", true, vmime::stringUtils::isStringEqualNoCase(vmime::string("foO"), vmime::string("FOo")));
		}

		void testIsStringEqualNoCase3()
		{
			vmime::string str1("FooBar");

			assert_eq("1", true, vmime::stringUtils::isStringEqualNoCase(str1.begin(), str1.end(), "foobar", 6));
			assert_eq("2", false, vmime::stringUtils::isStringEqualNoCase(str1.begin(), str1.end(), "FooBar", 6));
			assert_eq("3", true, vmime::stringUtils::isStringEqualNoCase(str1.begin(), str1.end(), "fooBar", 3));
			assert_eq("4", false, vmime::stringUtils::isStringEqualNoCase(str1.begin(), str1.begin() + 3, "fooBar", 6));
		}

		void testToLower()
		{
			assert_eq("1", "foo", vmime::stringUtils::toLower("FOO"));
			assert_eq("2", "foo", vmime::stringUtils::toLower("foO"));
			assert_eq("3", "foo", vmime::stringUtils::toLower("foo"));
		}

		void testTrim()
		{
			assert_eq("1", "foo", vmime::stringUtils::trim("  foo"));
			assert_eq("2", "foo", vmime::stringUtils::trim("\t\tfoo"));
			assert_eq("3", "foo", vmime::stringUtils::trim(" \t \tfoo"));
			assert_eq("4", "foo", vmime::stringUtils::trim(" \r\n\tfoo"));

			assert_eq("5", "foo", vmime::stringUtils::trim("foo  "));
			assert_eq("6", "foo", vmime::stringUtils::trim("foo\t\t"));
			assert_eq("7", "foo", vmime::stringUtils::trim("foo \t \t"));
			assert_eq("8", "foo", vmime::stringUtils::trim("foo \r\n\t"));

			assert_eq( "9", "foo", vmime::stringUtils::trim("foo  "));
			assert_eq("10", "foo", vmime::stringUtils::trim(" foo  "));
			assert_eq("11", "foo", vmime::stringUtils::trim(" foo\t\t"));
			assert_eq("12", "foo", vmime::stringUtils::trim("\tfoo \r \t"));
			assert_eq("13", "foo", vmime::stringUtils::trim("\r \tfoo \n\t"));
		}

		void testCountASCIIChars()
		{
			vmime::string str1("foo");
			assert_eq("1", static_cast <vmime::string::size_type>(3),
				vmime::stringUtils::countASCIIchars(str1.begin(), str1.end()));

			vmime::string str2("f=?oo");
			assert_eq("2", static_cast <vmime::string::size_type>(3 + 1),
				vmime::stringUtils::countASCIIchars(str2.begin(), str2.end()));

			vmime::string str3("foo\x7f");
			assert_eq("3", static_cast <vmime::string::size_type>(4),
				vmime::stringUtils::countASCIIchars(str3.begin(), str3.end()));

			vmime::string str4("foo\x80");
			assert_eq("4", static_cast <vmime::string::size_type>(3),
				vmime::stringUtils::countASCIIchars(str4.begin(), str4.end()));
		}

	public:

		stringUtilsTest() : suite("vmime::utility::stringUtils")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("IsStringEqualNoCase1", testcase(this, "IsStringEqualNoCase1", &stringUtilsTest::testIsStringEqualNoCase1));
			add("IsStringEqualNoCase2", testcase(this, "IsStringEqualNoCase2", &stringUtilsTest::testIsStringEqualNoCase2));
			add("IsStringEqualNoCase3", testcase(this, "IsStringEqualNoCase3", &stringUtilsTest::testIsStringEqualNoCase3));

			add("ToLower", testcase(this, "ToLower", &stringUtilsTest::testToLower));

			add("Trim", testcase(this, "Trim", &stringUtilsTest::testTrim));

			add("CountASCIIChars", testcase(this, "CountASCIIChars", &stringUtilsTest::testCountASCIIChars));

			suite::main().add("vmime::utility::stringUtils", this);
		}

	};

	stringUtilsTest* theTest = new stringUtilsTest();
}
