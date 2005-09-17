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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "tests/testUtils.hpp"


#define VMIME_TEST_SUITE         textTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConstructors)
		VMIME_TEST(testCopy)
		VMIME_TEST(testNewFromString)
		VMIME_TEST(testDisplayForm)
		VMIME_TEST(testParse)
		VMIME_TEST(testGenerate)

		VMIME_TEST(testWordConstructors)
		VMIME_TEST(testWordParse)
		VMIME_TEST(testWordGenerate)
	VMIME_TEST_LIST_END


	void testConstructors()
	{
		vmime::text t1;

		VASSERT_EQ("1.1", 0, t1.getWordCount());

		vmime::text t2("Test\xa9\xc3");

		VASSERT_EQ("2.1", 1, t2.getWordCount());
		VASSERT_EQ("2.2", "Test\xa9\xc3", t2.getWordAt(0)->getBuffer());
		VASSERT_EQ("2.3", vmime::charset::getLocaleCharset(), t2.getWordAt(0)->getCharset());

		vmime::text t3("Test\xa9\xc3", vmime::charset(vmime::charsets::ISO8859_13));

		VASSERT_EQ("3.1", 1, t3.getWordCount());
		VASSERT_EQ("3.2", "Test\xa9\xc3", t3.getWordAt(0)->getBuffer());
		VASSERT_EQ("3.3", vmime::charset(vmime::charsets::ISO8859_13), t3.getWordAt(0)->getCharset());

		vmime::word w1("Test", vmime::charset(vmime::charsets::UTF_8));
		vmime::text t4(w1);

		VASSERT_EQ("4.1", 1, t4.getWordCount());
		VASSERT_EQ("4.2", w1.getBuffer(), t4.getWordAt(0)->getBuffer());
		VASSERT_EQ("4.3", w1.getCharset(), t4.getWordAt(0)->getCharset());

		vmime::word w2("Other", vmime::charset(vmime::charsets::US_ASCII));
		t4.appendWord(vmime::create <vmime::word>(w2));

		vmime::text t5(t4);

		VASSERT_EQ("5.1", 2, t5.getWordCount());
		VASSERT_EQ("5.2", w1.getBuffer(), t5.getWordAt(0)->getBuffer());
		VASSERT_EQ("5.3", w1.getCharset(), t5.getWordAt(0)->getCharset());
		VASSERT_EQ("5.4", w2.getBuffer(), t5.getWordAt(1)->getBuffer());
		VASSERT_EQ("5.5", w2.getCharset(), t5.getWordAt(1)->getCharset());
	}

	void testCopy()
	{
		vmime::text t1("Test: \xa9\xc3");

		VASSERT("operator==", t1 == t1);
		VASSERT("clone", *vmime::clone(t1) == t1);

		vmime::text t2;
		t2.copyFrom(t1);

		VASSERT("copyFrom", t1 == t2);
	}

	void testNewFromString()
	{
		vmime::string s1 = "only ASCII characters";
		vmime::charset c1("test");
		vmime::text t1;

		t1.createFromString(s1, c1);

		VASSERT_EQ("1.1", 1, t1.getWordCount());
		VASSERT_EQ("1.2", s1, t1.getWordAt(0)->getBuffer());
		VASSERT_EQ("1.3", vmime::charset(vmime::charsets::US_ASCII), t1.getWordAt(0)->getCharset());

		vmime::string s2_1 = "some ASCII characters and special chars: ";
		vmime::string s2_2 = "\xf1\xf2\xf3\xf4 ";
		vmime::string s2_3 = "and then more ASCII chars.";
		vmime::string s2 = s2_1 + s2_2 + s2_3;
		vmime::charset c2("test");
		vmime::text t2;

		t2.createFromString(s2, c2);

		VASSERT_EQ("2.1", 3, t2.getWordCount());
		VASSERT_EQ("2.2", s2_1, t2.getWordAt(0)->getBuffer());
		VASSERT_EQ("2.3", vmime::charset(vmime::charsets::US_ASCII), t2.getWordAt(0)->getCharset());
		VASSERT_EQ("2.4", s2_2, t2.getWordAt(1)->getBuffer());
		VASSERT_EQ("2.5", c2, t2.getWordAt(1)->getCharset());
		VASSERT_EQ("2.6", s2_3, t2.getWordAt(2)->getBuffer());
		VASSERT_EQ("2.7", vmime::charset(vmime::charsets::US_ASCII), t2.getWordAt(2)->getCharset());
	}

	static const vmime::string parseText(const vmime::string& buffer)
	{
		vmime::text t;
		t.parse(buffer);

		std::ostringstream oss;
		oss << t;

		return (oss.str());
	}

	void testParse()
	{
		// From RFC-2047
		VASSERT_EQ("1", "[text: [[word: charset=US-ASCII, buffer=Keith Moore]]]",
			parseText("=?US-ASCII?Q?Keith_Moore?="));

		VASSERT_EQ("2", "[text: [[word: charset=ISO-8859-1, buffer=Keld J\xf8rn Simonsen]]]",
			parseText("=?ISO-8859-1?Q?Keld_J=F8rn_Simonsen?="));

		VASSERT_EQ("3", "[text: [[word: charset=ISO-8859-1, buffer=Andr\xe9]," \
			                 "[word: charset=us-ascii, buffer= Pirard]]]",
			parseText("=?ISO-8859-1?Q?Andr=E9?= Pirard"));

		VASSERT_EQ("4", "[text: [[word: charset=ISO-8859-1, buffer=If you can read this yo]," \
			                 "[word: charset=ISO-8859-2, buffer=u understand the example.]]]",
			parseText("=?ISO-8859-1?B?SWYgeW91IGNhbiByZWFkIHRoaXMgeW8=?=\r\n " \
				"=?ISO-8859-2?B?dSB1bmRlcnN0YW5kIHRoZSBleGFtcGxlLg==?="));

		// Bugfix: in "=?charset?q?=XX=YY?=", the "?=" finish
		// sequence was not correctly found (should be the one
		// after '=YY' and not the one after '?q').
		VASSERT_EQ("5", "[text: [[word: charset=abc, buffer=\xe9\xe9]]]",
			parseText("=?abc?q?=E9=E9?="));

		// Question marks (?) in the middle of the string
		VASSERT_EQ("6", "[text: [[word: charset=iso-8859-1, buffer=Know wh\xe4t? It works!]]]",
			parseText("=?iso-8859-1?Q?Know_wh=E4t?_It_works!?="));

		// TODO: add more
	}

	void testGenerate()
	{
		// TODO
	}

	static const vmime::string getDisplayText(const vmime::text& t)
	{
		vmime::string res;

		for (int i = 0 ; i < t.getWordCount() ; ++i)
			res += t.getWordAt(i)->getBuffer();

		return res;
	}

	void testDisplayForm()
	{
#define DISPLAY_FORM(x) getDisplayText(*vmime::text::decodeAndUnfold(x))

		// From RFC-2047
		VASSERT_EQ("1", "a", DISPLAY_FORM("=?ISO-8859-1?Q?a?="));
		VASSERT_EQ("2", "a b", DISPLAY_FORM("=?ISO-8859-1?Q?a?= b"));
		VASSERT_EQ("3", "ab", DISPLAY_FORM("=?ISO-8859-1?Q?a?= =?ISO-8859-1?Q?b?="));
		VASSERT_EQ("4", "ab", DISPLAY_FORM("=?ISO-8859-1?Q?a?= \t  =?ISO-8859-1?Q?b?="));
		VASSERT_EQ("5", "ab", DISPLAY_FORM("=?ISO-8859-1?Q?a?= \r\n  \t =?ISO-8859-1?Q?b?="));
		VASSERT_EQ("6", "a b", DISPLAY_FORM("=?ISO-8859-1?Q?a_b?="));
		VASSERT_EQ("7", "a b", DISPLAY_FORM("=?ISO-8859-1?Q?a?= =?ISO-8859-2?Q?_b?="));

		// Some more tests...
		VASSERT_EQ("8", "a b", DISPLAY_FORM(" a =?ISO-8859-1?Q?b?=  "));
		VASSERT_EQ("9", "a b  ", DISPLAY_FORM(" \t  =?ISO-8859-1?Q?a?= b  "));
		VASSERT_EQ("10", "a b", DISPLAY_FORM("  a\r\n\t  b"));

#undef DISPLAY_FORM
	}

	void testWordConstructors()
	{
		VASSERT_EQ("1.1", vmime::charset::getLocaleCharset(), vmime::word().getCharset());
		VASSERT_EQ("1.2", "", vmime::word().getBuffer());

		VASSERT_EQ("2.1", vmime::charset::getLocaleCharset(), vmime::word("foo").getCharset());
		VASSERT_EQ("2.2", "foo", vmime::word("foo").getBuffer());

		VASSERT_EQ("3.1", "bar", vmime::word("foo", vmime::charset("bar")).getCharset().getName());
		VASSERT_EQ("3.2", "foo", vmime::word("foo", vmime::charset("bar")).getBuffer());
	}

	void testWordParse()
	{
		// Simple encoded word
		vmime::word w1;
		w1.parse("=?foo?q?bar=E9 baz?=");

		VASSERT_EQ("1.1", "foo", w1.getCharset().getName());
		VASSERT_EQ("1.2", "bar\xe9 baz", w1.getBuffer());

		// Unencoded text
		vmime::word w2;
		w2.parse("  foo bar \tbaz...");

		VASSERT_EQ("2.1", vmime::charset(vmime::charsets::US_ASCII), w2.getCharset());
		VASSERT_EQ("2.2", "  foo bar \tbaz...", w2.getBuffer());

		// Malformed word
		vmime::word w3;
		w3.parse("=?foo bar");

		VASSERT_EQ("3.1", vmime::charset(vmime::charsets::US_ASCII), w3.getCharset());
		VASSERT_EQ("3.2", "=?foo bar", w3.getBuffer());

		// Unknown encoding
		vmime::word w4;
		w4.parse("=?whatever?not_q_or_b?whatever?=");

		VASSERT_EQ("4.1", vmime::charset(vmime::charsets::US_ASCII), w4.getCharset());
		VASSERT_EQ("4.2", "=?whatever?not_q_or_b?whatever?=", w4.getBuffer());
	}

	void testWordGenerate()
	{
		VASSERT_EQ("1", "=?foo?Q?bar=E9_baz?=",
			vmime::word("bar\xe9 baz", vmime::charset("foo")).generate());

		VASSERT_EQ("2", "=?foo?B?8fLz9PU=?=",
			vmime::word("\xf1\xf2\xf3\xf4\xf5", vmime::charset("foo")).generate());
	}

VMIME_TEST_SUITE_END

