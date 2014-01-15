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

#include "vmime/platform.hpp"

#include <locale>
#include <clocale>


VMIME_TEST_SUITE_BEGIN(emailAddressTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testParseASCII)
		VMIME_TEST(testParseEAI)
		VMIME_TEST(testParseInvalid)
		VMIME_TEST(testGenerateASCII)
		VMIME_TEST(testGenerateEAI)
		VMIME_TEST(testParseSpecialChars)
		VMIME_TEST(testParseCommentInLocalPart)
		VMIME_TEST(testParseCommentInDomainPart)
		VMIME_TEST(testGenerateSpecialChars)
	VMIME_TEST_LIST_END


	void setUp()
	{
		// Set the global C and C++ locale to the user-configured locale.
		// The locale should use UTF-8 encoding for these tests to run successfully.
		try
		{
			std::locale::global(std::locale(""));
		}
		catch (std::exception &)
		{
			std::setlocale(LC_ALL, "");
		}
	}

	void tearDown()
	{
		// Restore default locale
		std::locale::global(std::locale("C"));
	}


	void testParseASCII()
	{
		vmime::emailAddress eml1("local@domain");
		VASSERT_EQ("1/local", "local", eml1.getLocalName());
		VASSERT_EQ("1/domain", "domain", eml1.getDomainName());

		// When not specified, domain should be local host name
		vmime::emailAddress eml2("local");
		VASSERT_EQ("2/local", "local", eml2.getLocalName());
		VASSERT_EQ("2/domain", vmime::platform::getHandler()->getHostName(), eml2.getDomainName());
	}

	void testParseEAI()
	{
		// Examples taken from Wikipedia (http://en.wikipedia.org/wiki/Email_address)

		// Latin Alphabet (with diacritics):
		vmime::emailAddress eml1("Pelé@example.com");
		VASSERT_EQ("1/local", "Pelé", eml1.getLocalName());
		VASSERT_EQ("1/domain", "example.com", eml1.getDomainName());

		// Greek Alphabet
		vmime::emailAddress eml2("δοκιμή@παράδειγμα.δοκιμή");
		VASSERT_EQ("2/local", "δοκιμή", eml2.getLocalName());
		VASSERT_EQ("2/domain", "παράδειγμα.δοκιμή", eml2.getDomainName());

		// Japanese Characters
		vmime::emailAddress eml3("甲斐@黒川.日本");
		VASSERT_EQ("3/local", "甲斐", eml3.getLocalName());
		VASSERT_EQ("3/domain", "黒川.日本", eml3.getDomainName());

		// Cyrillic Characters
		vmime::emailAddress eml4("чебурашка@ящик-с-апельсинами.рф");
		VASSERT_EQ("4/local", "чебурашка", eml4.getLocalName());
		VASSERT_EQ("4/domain", "ящик-с-апельсинами.рф", eml4.getDomainName());
	}

	void testParseInvalid()
	{
		// Only one @ is allowed outside quotation marks
		vmime::emailAddress eml1("local@part@domain");
		VASSERT_EQ("1/local", "local", eml1.getLocalName());
		VASSERT_EQ("1/domain", "part@domain", eml1.getDomainName());

		// Quoted strings must be dot separated, or the only element making up
		// the local-part: should be parsed correctly, but it still represents
		// an invalid email address
		vmime::emailAddress eml2("Just\"not\"right@example.com");
		VASSERT_EQ("2/local", "Just\"not\"right", eml2.getLocalName());
		VASSERT_EQ("2/domain", "example.com", eml2.getDomainName());

		// An @ character must separate the local and domain parts
		vmime::emailAddress eml3("Abc.example.com");
		VASSERT_EQ("3/local", "Abc.example.com", eml3.getLocalName());
		VASSERT_EQ("3/domain", vmime::platform::getHandler()->getHostName(), eml3.getDomainName());

		// Whitespace must be escaped
		vmime::emailAddress eml4("local part@domain");
		VASSERT_EQ("4/local", "local", eml4.getLocalName());
		VASSERT_EQ("4/domain", vmime::platform::getHandler()->getHostName(), eml4.getDomainName());

		vmime::emailAddress eml5("this\\ still\\\"not\\\\allowed@example.com");
		VASSERT_EQ("5/local", "this\\", eml5.getLocalName());
		VASSERT_EQ("5/domain", vmime::platform::getHandler()->getHostName(), eml5.getDomainName());
	}

	void testParseSpecialChars()
	{
		// Examples taken from Wikipedia (http://en.wikipedia.org/wiki/Email_address)

		vmime::emailAddress eml1("\" \"@example.org");
		VASSERT_EQ("1/local", " ", eml1.getLocalName());
		VASSERT_EQ("1/domain", "example.org", eml1.getDomainName());

		vmime::emailAddress eml2("\"()<>[]:,;@\\\\\\\"!#$%&'*+-/=?^_`{}| ~.a\"@example.org");
		VASSERT_EQ("2/local", "()<>[]:,;@\\\"!#$%&'*+-/=?^_`{}| ~.a", eml2.getLocalName());
		VASSERT_EQ("3/domain", "example.org", eml2.getDomainName());

		vmime::emailAddress eml3("!#$%&'*+-/=?^_`{}|~@example.org");
		VASSERT_EQ("3/local", "!#$%&'*+-/=?^_`{}|~", eml3.getLocalName());
		VASSERT_EQ("3/domain", "example.org", eml3.getDomainName());

		vmime::emailAddress eml4("!#$%&'*+-/=?^_`{}|~@example.org");
		VASSERT_EQ("4/local", "!#$%&'*+-/=?^_`{}|~", eml4.getLocalName());
		VASSERT_EQ("4/domain", "example.org", eml4.getDomainName());

		vmime::emailAddress eml5("\"very.unusual.@.unusual.com\"@example.com");
		VASSERT_EQ("5/local", "very.unusual.@.unusual.com", eml5.getLocalName());
		VASSERT_EQ("5/domain", "example.com", eml5.getDomainName());

		vmime::emailAddress eml6("\"very.(),:;<>[]\\\".VERY.\\\"very@\\\\ \\\"very\\\".unusual\"@strange.example.com");
		VASSERT_EQ("6/local", "very.(),:;<>[]\".VERY.\"very@\\ \"very\".unusual", eml6.getLocalName());
		VASSERT_EQ("6/domain", "strange.example.com", eml6.getDomainName());
	}

	void testParseCommentInLocalPart()
	{
		vmime::emailAddress eml1("john.smith(comment)@example.com");
		VASSERT_EQ("1/local", "john.smith", eml1.getLocalName());
		VASSERT_EQ("1/domain", "example.com", eml1.getDomainName());

		vmime::emailAddress eml2("(comment)john.smith@example.com");
		VASSERT_EQ("2/local", "john.smith", eml2.getLocalName());
		VASSERT_EQ("2/domain", "example.com", eml2.getDomainName());

		vmime::emailAddress eml3("(comment (comment in comment))john.smith@example.com");
		VASSERT_EQ("3/local", "john.smith", eml3.getLocalName());
		VASSERT_EQ("3/domain", "example.com", eml3.getDomainName());

		vmime::emailAddress eml4("(comment \\) end comment)john.smith@example.com");
		VASSERT_EQ("4/local", "john.smith", eml4.getLocalName());
		VASSERT_EQ("4/domain", "example.com", eml4.getDomainName());
	}

	void testParseCommentInDomainPart()
	{
		vmime::emailAddress eml1("john.smith@(comment)example.com");
		VASSERT_EQ("1/local", "john.smith", eml1.getLocalName());
		VASSERT_EQ("1/domain", "example.com", eml1.getDomainName());

		vmime::emailAddress eml2("john.smith@example.com(comment)");
		VASSERT_EQ("2/local", "john.smith", eml2.getLocalName());
		VASSERT_EQ("2/domain", "example.com", eml2.getDomainName());

		vmime::emailAddress eml3("john.smith@(comment (comment in comment))example.com");
		VASSERT_EQ("3/local", "john.smith", eml3.getLocalName());
		VASSERT_EQ("3/domain", "example.com", eml3.getDomainName());

		vmime::emailAddress eml4("john.smith@(comment \\) end comment)example.com");
		VASSERT_EQ("4/local", "john.smith", eml4.getLocalName());
		VASSERT_EQ("4/domain", "example.com", eml4.getDomainName());
	}

	void testGenerateASCII()
	{
		VASSERT_EQ("email 1", "local@domain", vmime::emailAddress("local", "domain").generate());

		VASSERT_EQ("email 2", "=?utf-8?Q?Pel=C3=A9?=@example.com",
			vmime::emailAddress("Pelé", "example.com").generate());
		VASSERT_EQ("email 3", "=?utf-8?B?55Sy5paQ?=@xn--5rtw95l.xn--wgv71a",
			vmime::emailAddress("甲斐", "黒川.日本").generate());
		VASSERT_EQ("email 4", "mailtest@xn--r8jz45g.xn--zckzah",
			vmime::emailAddress("mailtest", "例え.テスト").generate());
		VASSERT_EQ("email 5", "mailtest@xn--mgbh0fb.xn--kgbechtv",
			vmime::emailAddress("mailtest", "مثال.إختبار").generate());
	}

	void testGenerateEAI()
	{
		vmime::generationContext ctx(vmime::generationContext::getDefaultContext());
		ctx.setInternationalizedEmailSupport(true);

		vmime::generationContext::switcher <vmime::generationContext> contextSwitcher(ctx);

		VASSERT_EQ("email 1", "Pelé@example.com",
			vmime::emailAddress("Pelé", "example.com").generate());
		VASSERT_EQ("email 2", "δοκιμή@παράδειγμα.δοκιμή",
			vmime::emailAddress("δοκιμή", "παράδειγμα.δοκιμή").generate());
		VASSERT_EQ("email 3", "甲斐@黒川.日本",
			vmime::emailAddress("甲斐", "黒川.日本").generate());
		VASSERT_EQ("email 4", "чебурашка@ящик-с-апельсинами.рф",
			vmime::emailAddress("чебурашка", "ящик-с-апельсинами.рф").generate());
	}

	void testGenerateSpecialChars()
	{
		VASSERT_EQ("email 1", "\"very.unusual.@.unusual.com\"@example.com",
			vmime::emailAddress("very.unusual.@.unusual.com", "example.com").generate());

		VASSERT_EQ("email 2", "\"very.(),:;<>[]\\\".VERY.\\\"very@\\\\ \\\"very\\\".unusual\"@strange.example.com",
			vmime::emailAddress("very.(),:;<>[]\".VERY.\"very@\\ \"very\".unusual", "strange.example.com").generate());

		VASSERT_EQ("email 3", "\" \"@example.com",
			vmime::emailAddress(" ", "example.com").generate());
	}

VMIME_TEST_SUITE_END
