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

#include "vmime/utility/stringUtils.hpp"


#define VMIME_TEST_SUITE         stringUtilsTest
#define VMIME_TEST_SUITE_MODULE  "Utility"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testIsStringEqualNoCase1)
		VMIME_TEST(testIsStringEqualNoCase2)
		VMIME_TEST(testIsStringEqualNoCase3)

		VMIME_TEST(testToLower)

		VMIME_TEST(testTrim)

		VMIME_TEST(testCountASCIIChars)

		VMIME_TEST(testUnquote)
	VMIME_TEST_LIST_END


	typedef vmime::utility::stringUtils stringUtils;


	void testIsStringEqualNoCase1()
	{
		VASSERT_EQ("1", true, stringUtils::isStringEqualNoCase(vmime::string("foo"), "foo", 3));
		VASSERT_EQ("2", true, stringUtils::isStringEqualNoCase(vmime::string("FOo"), "foo", 3));

		VASSERT_EQ("3", false, stringUtils::isStringEqualNoCase(vmime::string("foo"), "FOo", 3));
		VASSERT_EQ("4", false, stringUtils::isStringEqualNoCase(vmime::string("foo"), "bar", 3));
	}

	void testIsStringEqualNoCase2()
	{
		VASSERT_EQ("1", true, stringUtils::isStringEqualNoCase(vmime::string("foo"), vmime::string("foo")));
		VASSERT_EQ("2", true, stringUtils::isStringEqualNoCase(vmime::string("FOo"), vmime::string("foo")));
		VASSERT_EQ("3", true, stringUtils::isStringEqualNoCase(vmime::string("foO"), vmime::string("FOo")));
	}

	void testIsStringEqualNoCase3()
	{
		vmime::string str1("FooBar");

		VASSERT_EQ("1", true, stringUtils::isStringEqualNoCase(str1.begin(), str1.end(), "foobar", 6));
		VASSERT_EQ("2", false, stringUtils::isStringEqualNoCase(str1.begin(), str1.end(), "FooBar", 6));
		VASSERT_EQ("3", true, stringUtils::isStringEqualNoCase(str1.begin(), str1.end(), "fooBar", 3));
		VASSERT_EQ("4", false, stringUtils::isStringEqualNoCase(str1.begin(), str1.begin() + 3, "fooBar", 6));
	}

	void testToLower()
	{
		VASSERT_EQ("1", "foo", stringUtils::toLower("FOO"));
		VASSERT_EQ("2", "foo", stringUtils::toLower("foO"));
		VASSERT_EQ("3", "foo", stringUtils::toLower("foo"));
	}

	void testTrim()
	{
		VASSERT_EQ("1", "foo", stringUtils::trim("  foo"));
		VASSERT_EQ("2", "foo", stringUtils::trim("\t\tfoo"));
		VASSERT_EQ("3", "foo", stringUtils::trim(" \t \tfoo"));
		VASSERT_EQ("4", "foo", stringUtils::trim(" \r\n\tfoo"));

		VASSERT_EQ("5", "foo", stringUtils::trim("foo  "));
		VASSERT_EQ("6", "foo", stringUtils::trim("foo\t\t"));
		VASSERT_EQ("7", "foo", stringUtils::trim("foo \t \t"));
		VASSERT_EQ("8", "foo", stringUtils::trim("foo \r\n\t"));

		VASSERT_EQ( "9", "foo", stringUtils::trim("foo  "));
		VASSERT_EQ("10", "foo", stringUtils::trim(" foo  "));
		VASSERT_EQ("11", "foo", stringUtils::trim(" foo\t\t"));
		VASSERT_EQ("12", "foo", stringUtils::trim("\tfoo \r \t"));
		VASSERT_EQ("13", "foo", stringUtils::trim("\r \tfoo \n\t"));
	}

	void testCountASCIIChars()
	{
		vmime::string str1("foo");
		VASSERT_EQ("1", static_cast <vmime::string::size_type>(3),
			stringUtils::countASCIIchars(str1.begin(), str1.end()));

		vmime::string str2("f=?oo");
		VASSERT_EQ("2", static_cast <vmime::string::size_type>(3 + 1),
			stringUtils::countASCIIchars(str2.begin(), str2.end()));

		vmime::string str3("foo\x7f");
		VASSERT_EQ("3", static_cast <vmime::string::size_type>(4),
			stringUtils::countASCIIchars(str3.begin(), str3.end()));

		vmime::string str4("foo\x80");
		VASSERT_EQ("4", static_cast <vmime::string::size_type>(3),
			stringUtils::countASCIIchars(str4.begin(), str4.end()));
	}

	void testUnquote()
	{
		VASSERT_EQ("1", "quoted", stringUtils::unquote("\"quoted\""));  // "quoted"
		VASSERT_EQ("2", "\"not quoted", stringUtils::unquote("\"not quoted"));  // "not quoted
		VASSERT_EQ("3", "not quoted\"", stringUtils::unquote("not quoted\""));  // not quoted"
		VASSERT_EQ("4", "quoted with \"escape\"", stringUtils::unquote("\"quoted with \\\"escape\\\"\""));  // "quoted with \"escape\""
	}

VMIME_TEST_SUITE_END

