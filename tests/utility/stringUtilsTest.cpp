//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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


VMIME_TEST_SUITE_BEGIN(stringUtilsTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testMakeStringFromBytes)
		VMIME_TEST(testAppendBytesToString)

		VMIME_TEST(testIsStringEqualNoCase1)
		VMIME_TEST(testIsStringEqualNoCase2)
		VMIME_TEST(testIsStringEqualNoCase3)

		VMIME_TEST(testToLower)

		VMIME_TEST(testTrim)

		VMIME_TEST(testCountASCIIChars)

		VMIME_TEST(testUnquote)
	VMIME_TEST_LIST_END


	typedef vmime::utility::stringUtils stringUtils;


	void testMakeStringFromBytes()
	{
		vmime::byte_t bytes[] = { 0x12, 0x34, 0x56, 0x78 };
		vmime::string str = vmime::utility::stringUtils::makeStringFromBytes(bytes, 3);

		VASSERT_EQ("length", 3, str.length());
		VASSERT_EQ("byte1", '\x12', str[0]);
		VASSERT_EQ("byte2", '\x34', str[1]);
		VASSERT_EQ("byte3", '\x56', str[2]);
	}

	void testAppendBytesToString()
	{
		vmime::byte_t bytes[] = { 0x42, 0x56, 0x12, 0x00, 'f', 'o', 'o' };

		vmime::string str = "test";
		vmime::utility::stringUtils::appendBytesToString(str, bytes, 7);

		VASSERT_EQ("length", 4 + 7, str.length());
		VASSERT_EQ("byte1", 't', str[0]);
		VASSERT_EQ("byte2", 'e', str[1]);
		VASSERT_EQ("byte3", 's', str[2]);
		VASSERT_EQ("byte4", 't', str[3]);
		VASSERT_EQ("byte5", '\x42', str[4]);
		VASSERT_EQ("byte6", '\x56', str[5]);
		VASSERT_EQ("byte7", '\x12', str[6]);
		VASSERT_EQ("byte8", '\0', str[7]);
		VASSERT_EQ("byte9", 'f', str[8]);
		VASSERT_EQ("byte10", 'o', str[9]);
		VASSERT_EQ("byte11", 'o', str[10]);
	}

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
		VASSERT_EQ("1", static_cast <vmime::size_t>(3),
			stringUtils::countASCIIchars(str1.begin(), str1.end()));

		vmime::string str2("f=?oo");
		VASSERT_EQ("2", static_cast <vmime::size_t>(3 + 1),
			stringUtils::countASCIIchars(str2.begin(), str2.end()));

		vmime::string str3("foo\x7f");
		VASSERT_EQ("3", static_cast <vmime::size_t>(4),
			stringUtils::countASCIIchars(str3.begin(), str3.end()));

		vmime::string str4("foo\x80");
		VASSERT_EQ("4", static_cast <vmime::size_t>(3),
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

