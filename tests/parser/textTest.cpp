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

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "tests/parser/testUtils.hpp"

using namespace unitpp;


namespace
{
	class textTest : public suite
	{
		void testConstructors()
		{
			vmime::text t1;

			assert_eq("1.1", 0, t1.getWordCount());

			vmime::text t2("Test\xa9\xc3");

			assert_eq("2.1", 1, t2.getWordCount());
			assert_eq("2.2", "Test\xa9\xc3", t2.getWordAt(0)->getBuffer());
			assert_eq("2.3", vmime::charset::getLocaleCharset(), t2.getWordAt(0)->getCharset());

			vmime::text t3("Test\xa9\xc3", vmime::charset(vmime::charsets::ISO8859_13));

			assert_eq("3.1", 1, t3.getWordCount());
			assert_eq("3.2", "Test\xa9\xc3", t3.getWordAt(0)->getBuffer());
			assert_eq("3.3", vmime::charset(vmime::charsets::ISO8859_13), t3.getWordAt(0)->getCharset());

			vmime::word w1("Test", vmime::charset(vmime::charsets::UTF_8));
			vmime::text t4(w1);

			assert_eq("4.1", 1, t4.getWordCount());
			assert_eq("4.2", w1.getBuffer(), t4.getWordAt(0)->getBuffer());
			assert_eq("4.3", w1.getCharset(), t4.getWordAt(0)->getCharset());

			vmime::word w2("Other", vmime::charset(vmime::charsets::US_ASCII));
			t4.appendWord(new vmime::word(w2));

			vmime::text t5(t4);

			assert_eq("5.1", 2, t5.getWordCount());
			assert_eq("5.2", w1.getBuffer(), t5.getWordAt(0)->getBuffer());
			assert_eq("5.3", w1.getCharset(), t5.getWordAt(0)->getCharset());
			assert_eq("5.4", w2.getBuffer(), t5.getWordAt(1)->getBuffer());
			assert_eq("5.5", w2.getCharset(), t5.getWordAt(1)->getCharset());
		}

		void testCopy()
		{
			vmime::text t1("Test: \xa9\xc3");

			assert_true("operator==", t1 == t1);
			assert_true("clone", *(t1.clone()) == t1);

			vmime::text t2;
			t2.copyFrom(t1);

			assert_true("copyFrom", t1 == t2);
		}

		void testNewFromString()
		{
			vmime::string s1 = "only ASCII characters";
			vmime::charset c1("test");
			vmime::text t1;

			vmime::text::newFromString(s1, c1, &t1);

			assert_eq("1.1", 1, t1.getWordCount());
			assert_eq("1.2", s1, t1.getWordAt(0)->getBuffer());
			assert_eq("1.3", vmime::charset(vmime::charsets::US_ASCII), t1.getWordAt(0)->getCharset());

			vmime::string s2_1 = "some ASCII characters and special chars: ";
			vmime::string s2_2 = "\xf1\xf2\xf3\xf4 ";
			vmime::string s2_3 = "and then more ASCII chars.";
			vmime::string s2 = s2_1 + s2_2 + s2_3;
			vmime::charset c2("test");
			vmime::text t2;

			vmime::text::newFromString(s2, c2, &t2);

			assert_eq("2.1", 3, t2.getWordCount());
			assert_eq("2.2", s2_1, t2.getWordAt(0)->getBuffer());
			assert_eq("2.3", vmime::charset(vmime::charsets::US_ASCII), t2.getWordAt(0)->getCharset());
			assert_eq("2.4", s2_2, t2.getWordAt(1)->getBuffer());
			assert_eq("2.5", c2, t2.getWordAt(1)->getCharset());
			assert_eq("2.6", s2_3, t2.getWordAt(2)->getBuffer());
			assert_eq("2.7", vmime::charset(vmime::charsets::US_ASCII), t2.getWordAt(2)->getCharset());
		}

		static const vmime::string getDisplayText(const vmime::text& t)
		{
			vmime::string res;

			for (int i = 0 ; i < t.getWordCount() ; ++i)
				res += t.getWordAt(i)->getBuffer();

			return res;
		}

		void testDisplayForm()  // from RFC-2047
		{
#define DISPLAY_FORM(x) getDisplayText(*vmime::text::decodeAndUnfold(x))

			assert_eq("1", "a", DISPLAY_FORM("=?ISO-8859-1?Q?a?="));
			assert_eq("2", "a b", DISPLAY_FORM("=?ISO-8859-1?Q?a?= b"));
			assert_eq("3", "ab", DISPLAY_FORM("=?ISO-8859-1?Q?a?= =?ISO-8859-1?Q?b?="));
			assert_eq("4", "ab", DISPLAY_FORM("=?ISO-8859-1?Q?a?= \t  =?ISO-8859-1?Q?b?="));
			assert_eq("5", "ab", DISPLAY_FORM("=?ISO-8859-1?Q?a?= \r\n  \t =?ISO-8859-1?Q?b?="));
			assert_eq("6", "a b", DISPLAY_FORM("=?ISO-8859-1?Q?a_b?="));
			assert_eq("7", "a b", DISPLAY_FORM("=?ISO-8859-1?Q?a?= =?ISO-8859-2?Q?_b?="));

#undef DISPLAY_FORM
		}

		// TODO: tests for encodeAndFold() and decodeAndUnfold()

	public:

		textTest() : suite("vmime::text")
		{
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Constructors", testcase(this, "Constructors", &textTest::testConstructors));
			add("Copy", testcase(this, "Copy", &textTest::testCopy));
			add("NewFromString", testcase(this, "NewFromString", &textTest::testNewFromString));
			add("DisplatForm", testcase(this, "DisplayForm", &textTest::testDisplayForm));

			suite::main().add("vmime::text", this);
		}

	};

	textTest* theTest = new textTest();
}
