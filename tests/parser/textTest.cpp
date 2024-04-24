//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#include <locale>
#include <clocale>


VMIME_TEST_SUITE_BEGIN(textTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConstructors)
		VMIME_TEST(testCopy)
		VMIME_TEST(testRecommendedEncoding)
		VMIME_TEST(testNewFromString)
		VMIME_TEST(testDisplayForm)
		VMIME_TEST(testParse)
		VMIME_TEST(testGenerate)

		VMIME_TEST(testWordConstructors)
		VMIME_TEST(testWordParse)
		VMIME_TEST(testWordGenerate)
		VMIME_TEST(testWordGenerateSpace)
		VMIME_TEST(testWordGenerateSpace2)
		VMIME_TEST(testWordGenerateMultiBytes)
		VMIME_TEST(testWordGenerateQuote)
		VMIME_TEST(testWordGenerateSpecialCharsets)
		VMIME_TEST(testWordGenerateSpecials)

		VMIME_TEST(testWhitespace)
		VMIME_TEST(testWhitespaceMBox)

		VMIME_TEST(testFoldingAscii)
		VMIME_TEST(testForcedNonEncoding)

		VMIME_TEST(testBugFix20110511)

		VMIME_TEST(testInternationalizedEmail_specialChars)
		VMIME_TEST(testInternationalizedEmail_UTF8)
		VMIME_TEST(testInternationalizedEmail_nonUTF8)
		VMIME_TEST(testInternationalizedEmail_folding)
		VMIME_TEST(testInternationalizedEmail_whitespace)

		VMIME_TEST(testWronglyPaddedB64Words)
		VMIME_TEST(testFixBrokenWords)
		VMIME_TEST(testUnknownCharset)
	VMIME_TEST_LIST_END


	static const vmime::string getDisplayText(const vmime::text& t) {

		return t.getWholeBuffer();
	}

	static const vmime::string cleanGeneratedWords(const std::string& str) {

		std::istringstream iss(str);

		std::string res;
		std::string x;

		while (std::getline(iss, x)) {
			res += vmime::utility::stringUtils::trim(x);
		}

		return res;
	}


	void setUp() {

		// Set the global C and C++ locale to the user-configured locale.
		// The locale should use UTF-8 encoding for these tests to run successfully.
		try {
			std::locale::global(std::locale(""));
		} catch (std::exception &) {
			std::setlocale(LC_ALL, "");
		}
	}

	void tearDown() {

		// Restore default locale
		std::locale::global(std::locale("C"));
	}


	void testConstructors() {

		vmime::text t1;

		VASSERT_EQ("1.1", 0, t1.getWordCount());

		vmime::text t2("Test\xa9\xc3");

		VASSERT_EQ("2.1", 1, t2.getWordCount());
		VASSERT_EQ("2.2", "Test\xa9\xc3", t2.getWordAt(0)->getBuffer());
		VASSERT_EQ("2.3", vmime::charset::getLocalCharset(), t2.getWordAt(0)->getCharset());

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
		t4.appendWord(vmime::make_shared <vmime::word>(w2));

		vmime::text t5(t4);

		VASSERT_EQ("5.1", 2, t5.getWordCount());
		VASSERT_EQ("5.2", w1.getBuffer(), t5.getWordAt(0)->getBuffer());
		VASSERT_EQ("5.3", w1.getCharset(), t5.getWordAt(0)->getCharset());
		VASSERT_EQ("5.4", w2.getBuffer(), t5.getWordAt(1)->getBuffer());
		VASSERT_EQ("5.5", w2.getCharset(), t5.getWordAt(1)->getCharset());
	}

	void testCopy() {

		vmime::text t1("Test: \xa9\xc3");

		VASSERT("operator==", t1 == t1);
		VASSERT("clone", *vmime::clone(t1) == t1);

		vmime::text t2;
		t2.copyFrom(t1);

		VASSERT("copyFrom", t1 == t2);
	}

	void testRecommendedEncoding() {

		vmime::encoding enc;
		VASSERT_TRUE("1.1", vmime::charset("iso8859-1").getRecommendedEncoding(enc));
		VASSERT_TRUE("1.2", enc.getName() == vmime::encodingTypes::QUOTED_PRINTABLE);
		VASSERT_TRUE("1.2", vmime::charset("iso8859-2").getRecommendedEncoding(enc));
		VASSERT_TRUE("1.3", enc.getName() == vmime::encodingTypes::QUOTED_PRINTABLE);
		VASSERT_FALSE("1.3", vmime::charset(vmime::charsets::UTF_8).getRecommendedEncoding(enc));

	}

	void testNewFromString() {

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
		VASSERT_EQ("2.2", "some ASCII characters and special chars: ", t2.getWordAt(0)->getBuffer());
		VASSERT_EQ("2.3", vmime::charset(vmime::charsets::US_ASCII), t2.getWordAt(0)->getCharset());
		VASSERT_EQ("2.4", "\xf1\xf2\xf3\xf4", t2.getWordAt(1)->getBuffer());
		VASSERT_EQ("2.5", c2, t2.getWordAt(1)->getCharset());
		VASSERT_EQ("2.6", "and then more ASCII chars.", t2.getWordAt(2)->getBuffer());
		VASSERT_EQ("2.7", vmime::charset(vmime::charsets::US_ASCII), t2.getWordAt(2)->getCharset());
	}

	static const vmime::string parseText(const vmime::string& buffer) {

		vmime::text t;
		t.parse(buffer);

		std::ostringstream oss;
		oss << t;

		return oss.str();
	}

	void testParse() {

		// From RFC-2047
		VASSERT_EQ(
			"1",
			"[text: [[word: charset=US-ASCII, buffer=Keith Moore]]]",
			parseText("=?US-ASCII?Q?Keith_Moore?=")
		);

		VASSERT_EQ(
			"2",
			"[text: [[word: charset=ISO-8859-1, buffer=Keld J\xf8rn Simonsen]]]",
			parseText("=?ISO-8859-1?Q?Keld_J=F8rn_Simonsen?=")
		);

		VASSERT_EQ(
			"3",
			"[text: [[word: charset=ISO-8859-1, buffer=Andr\xe9]," \
			        "[word: charset=us-ascii, buffer= Pirard]]]",
			parseText("=?ISO-8859-1?Q?Andr=E9?= Pirard")
		);

		VASSERT_EQ(
			"4",
			"[text: [[word: charset=ISO-8859-1, buffer=If you can read this yo]," \
			        "[word: charset=ISO-8859-2, buffer=u understand the example.]]]",
			parseText(
				"=?ISO-8859-1?B?SWYgeW91IGNhbiByZWFkIHRoaXMgeW8=?=\r\n " \
				"=?ISO-8859-2?B?dSB1bmRlcnN0YW5kIHRoZSBleGFtcGxlLg==?="
			)
		);

		// Bugfix: in "=?charset?q?=XX=YY?=", the "?=" finish
		// sequence was not correctly found (should be the one
		// after '=YY' and not the one after '?q').
		VASSERT_EQ(
			"5",
			"[text: [[word: charset=abc, buffer=\xe9\xe9]]]",
			parseText("=?abc?q?=E9=E9?=")
		);

		// Question marks (?) in the middle of the string
		VASSERT_EQ(
			"6",
			"[text: [[word: charset=iso-8859-1, buffer=Know wh\xe4t? It works!]]]",
			parseText("=?iso-8859-1?Q?Know_wh=E4t?_It_works!?=")
		);

		// With language specifier
		VASSERT_EQ(
			"7",
			"[text: [[word: charset=US-ASCII, buffer=Keith Moore, lang=EN]]]",
			parseText("=?US-ASCII*EN?Q?Keith_Moore?=")
		);
	}

	void testGenerate() {

		// TODO

		// With language specifier
		vmime::word wlang1("Émeline", vmime::charset("UTF-8"), "FR");
		VASSERT_EQ("lang1", "=?UTF-8*FR?Q?=C3=89meline?=", wlang1.generate());

		vmime::word wlang2("Keith Moore", vmime::charset("US-ASCII"), "EN");
		VASSERT_EQ("lang2", "=?US-ASCII*EN?Q?Keith_Moore?=", wlang2.generate());
	}

	void testDisplayForm() {

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

		VASSERT_EQ("11", "a b c", DISPLAY_FORM("a =?ISO-8859-1?Q?b?= c"));
		VASSERT_EQ("12", "a b c ", DISPLAY_FORM("a =?ISO-8859-1?Q?b?= c "));
		VASSERT_EQ("13", "a b c ", DISPLAY_FORM(" a =?ISO-8859-1?Q?b?= c "));
		VASSERT_EQ("14", "a b c d", DISPLAY_FORM("a =?ISO-8859-1?Q?b?= c =?ISO-8859-1?Q?d?= "));
		VASSERT_EQ("15", "a b c d e", DISPLAY_FORM("a =?ISO-8859-1?Q?b?= c =?ISO-8859-1?Q?d?= e"));

		// Whitespaces and multiline
		VASSERT_EQ("16", "a b c d e", DISPLAY_FORM("=?ISO-8859-1?Q?a_b_?=c\n\t=?ISO-8859-1?Q?d_?=e"));

		// Ignored newlines
		VASSERT_EQ("17", "ab", DISPLAY_FORM("=?ISO-8859-1?Q?a?=\r\nb"));
		VASSERT_EQ("18", "a b", DISPLAY_FORM("=?ISO-8859-1?Q?a?= \r\nb"));

#undef DISPLAY_FORM
	}

	void testWordConstructors() {

		VASSERT_EQ("1.1", vmime::charset::getLocalCharset(), vmime::word().getCharset());
		VASSERT_EQ("1.2", "", vmime::word().getBuffer());

		VASSERT_EQ("2.1", vmime::charset::getLocalCharset(), vmime::word("foo").getCharset());
		VASSERT_EQ("2.2", "foo", vmime::word("foo").getBuffer());

		VASSERT_EQ("3.1", "bar", vmime::word("foo", vmime::charset("bar")).getCharset().getName());
		VASSERT_EQ("3.2", "foo", vmime::word("foo", vmime::charset("bar")).getBuffer());
	}

	void testWordParse() {

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

	void testWordGenerate() {

		VASSERT_EQ(
			"1",
			"=?foo?Q?bar=E9_baz?=",
			vmime::word("bar\xe9 baz", vmime::charset("foo")).generate()
		);

		VASSERT_EQ(
			"2",
			"=?foo?B?8fLz9PU=?=",
			vmime::word("\xf1\xf2\xf3\xf4\xf5", vmime::charset("foo")).generate()
		);
	}

	void testWordGenerateSpace() {

		// No white-space between an unencoded word and a encoded one
		VASSERT_EQ(
			"1",
			"Bonjour =?utf-8?Q?Fran=C3=A7ois?=",
			vmime::text::newFromString(
				"Bonjour Fran\xc3\xa7ois",
				vmime::charset("utf-8")
			)->generate()
		);

		// White-space between two encoded words
		vmime::text txt;
		txt.appendWord(vmime::make_shared <vmime::word>("\xc3\x89t\xc3\xa9", "utf-8"));
		txt.appendWord(vmime::make_shared <vmime::word>("Fran\xc3\xa7ois", "utf-8"));

		const vmime::string decoded = "\xc3\x89t\xc3\xa9""Fran\xc3\xa7ois";
		const vmime::string encoded = "=?utf-8?B?w4l0w6k=?= =?utf-8?Q?Fran=C3=A7ois?=";

		// -- test encoding
		VASSERT_EQ("2", encoded, txt.generate());

		// -- ensure no space is added when decoding
		vmime::text txt2;
		txt2.parse(encoded, 0, encoded.length());

		VASSERT_EQ("3", decoded, txt2.getWholeBuffer());

		// -- test rencoding
		VASSERT_EQ("4", encoded, txt2.generate());
	}

	void testWordGenerateSpace2() {

		// White-space between two encoded words (#2)
		vmime::text txt;
		txt.appendWord(vmime::make_shared <vmime::word>("Facture ", "utf-8"));
		txt.appendWord(vmime::make_shared <vmime::word>("\xc3\xa0", "utf-8"));
		txt.appendWord(vmime::make_shared <vmime::word>(" envoyer ", "utf-8"));
		txt.appendWord(vmime::make_shared <vmime::word>("\xc3\xa0", "utf-8"));
		txt.appendWord(vmime::make_shared <vmime::word>(" Martine", "utf-8"));

		const vmime::string decoded = "Facture ""\xc3\xa0"" envoyer ""\xc3\xa0"" Martine";
		const vmime::string encoded = "Facture =?utf-8?B?w6A=?= envoyer =?utf-8?B?w6A=?= Martine";

		// -- test encoding
		VASSERT_EQ("1", encoded, txt.generate());

		// -- ensure no space is added when decoding
		vmime::text txt2;
		txt2.parse(encoded, 0, encoded.length());

		VASSERT_EQ("2", decoded, txt2.getWholeBuffer());

		// -- test rencoding
		VASSERT_EQ("3", encoded, txt2.generate());
	}

	void testWordGenerateMultiBytes() {

		// Ensure we don't encode a non-integral number of characters
		VASSERT_EQ(
			"1",
			"=?utf-8?Q?aaa?==?utf-8?Q?=C3=A9?==?utf-8?Q?zzz?=",
			cleanGeneratedWords(
				vmime::word("aaa\xc3\xa9zzz", vmime::charset("utf-8")).generate(16)
			)
		);

		VASSERT_EQ(
			"2",
			"=?utf-8?Q?aaa=C3=A9?==?utf-8?Q?zzz?=",
			cleanGeneratedWords(
				vmime::word("aaa\xc3\xa9zzz", vmime::charset("utf-8")).generate(17)
			)
		);
	}

	void testWordGenerateQuote() {

		std::string str;
		vmime::utility::outputStreamStringAdapter os(str);

		vmime::generationContext ctx;
		ctx.setMaxLineLength(1000);

		// ASCII-only text is quotable
		str.clear();
		vmime::word("Quoted text")
			.generate(ctx, os, 0, NULL, vmime::text::QUOTE_IF_POSSIBLE, NULL);

		VASSERT_EQ("1", "\"Quoted text\"", cleanGeneratedWords(str));

		// Text with CR/LF is not quotable
		str.clear();
		vmime::word("Non-quotable\ntext", "us-ascii")
			.generate(ctx, os, 0, NULL, vmime::text::QUOTE_IF_POSSIBLE, NULL);

		VASSERT_EQ("2", "=?us-ascii?Q?Non-quotable=0Atext?=", cleanGeneratedWords(str));

		// Text with non-ASCII chars is not quotable
		str.clear();
		vmime::word("Non-quotable text \xc3\xa9")
			.generate(ctx, os, 0, NULL, vmime::text::QUOTE_IF_POSSIBLE, NULL);

		VASSERT_EQ("3", "=?UTF-8?Q?Non-quotable_text_=C3=A9?=", cleanGeneratedWords(str));
	}

	void testWordGenerateSpecialCharsets() {

		// ISO-2022-JP only uses 7-bit chars but should be encoded in Base64
		VASSERT_EQ(
			"1",
			"=?iso-2022-jp?B?XlskQiVRITwlPSVKJWshJiU9JVUlSCUmJSclIl5bKEI=?=",
			cleanGeneratedWords(
				vmime::word(
					"^[$B%Q!<%=%J%k!&%=%U%H%&%'%\"^[(B",
					vmime::charset("iso-2022-jp")
				).generate(100)
			)
		);
	}

	void testWordGenerateSpecials() {

		// In RFC-2047, quotation marks (ASCII 22h) should be encoded
		VASSERT_EQ(
			"1",
			"=?UTF-8?Q?=22=C3=9Cml=C3=A4ute=22?=",
			vmime::word(
				"\x22\xC3\x9Cml\xC3\xA4ute\x22",
				vmime::charset("UTF-8")
			).generate()
		);
	}

	void testWhitespace() {

		// Create
		vmime::text text;
		text.createFromString("Achim Br\xc3\xa4ndt", vmime::charsets::UTF_8);

		VASSERT_EQ("1", 2, text.getWordCount());
		VASSERT_EQ("2", "Achim ", text.getWordAt(0)->getBuffer());
		VASSERT_EQ("3", "us-ascii", text.getWordAt(0)->getCharset());
		VASSERT_EQ("4", "Br\xc3\xa4ndt", text.getWordAt(1)->getBuffer());
		VASSERT_EQ("5", "utf-8", text.getWordAt(1)->getCharset());

		// Generate
		VASSERT_EQ("6", "Achim =?utf-8?Q?Br=C3=A4ndt?=", text.generate());

		// Parse
		text.parse("=?us-ascii?Q?Achim_?= =?utf-8?Q?Br=C3=A4ndt?=");

		VASSERT_EQ("7", 2, text.getWordCount());
		VASSERT_EQ("8", "Achim ", text.getWordAt(0)->getBuffer());
		VASSERT_EQ("9", "us-ascii", text.getWordAt(0)->getCharset());
		VASSERT_EQ("10", "Br\xc3\xa4ndt", text.getWordAt(1)->getBuffer());
		VASSERT_EQ("11", "utf-8", text.getWordAt(1)->getCharset());
	}

	void testWhitespaceMBox() {

		// Space MUST be encoded inside a word
		vmime::mailbox mbox(vmime::text("Achim Br\xc3\xa4ndt", vmime::charsets::UTF_8), "me@vmime.org");
		VASSERT_EQ("generate1", "=?us-ascii?Q?Achim_?= =?utf-8?Q?Br=C3=A4ndt?= <me@vmime.org>", mbox.generate());

		vmime::text txt;
		txt.appendWord(vmime::make_shared <vmime::word>("Achim ", "us-ascii"));
		txt.appendWord(vmime::make_shared <vmime::word>("Br\xc3\xa4ndt", "utf-8"));
		mbox = vmime::mailbox(txt, "me@vmime.org");
		VASSERT_EQ("generate2", "=?us-ascii?Q?Achim_?= =?utf-8?Q?Br=C3=A4ndt?= <me@vmime.org>", mbox.generate());

		mbox.parse("=?us-ascii?Q?Achim?= =?utf-8?Q?Br=C3=A4ndt?= <me@vmime.org>");
		VASSERT_EQ("parse.name.count", 2, mbox.getName().getWordCount());
		VASSERT_EQ("parse.name.word1.buffer", "Achim", mbox.getName().getWordAt(0)->getBuffer());
		VASSERT_EQ("parse.name.word1.charset", "us-ascii", mbox.getName().getWordAt(0)->getCharset());
		VASSERT_EQ("parse.name.word2.buffer", "Br\xc3\xa4ndt", mbox.getName().getWordAt(1)->getBuffer());
		VASSERT_EQ("parse.name.word2.charset", "utf-8", mbox.getName().getWordAt(1)->getCharset());

		VASSERT_EQ("parse.email", "me@vmime.org", mbox.getEmail());
	}

	void testFoldingAscii() {

		// In this test, no encoding is needed, but line should be folded anyway
		vmime::word w("01234567890123456789012345678901234567890123456789"
		              "01234567890123456789012345678901234567890123456789", vmime::charset("us-ascii"));

		VASSERT_EQ(
			"fold.ascii",
			"=?us-ascii?Q?01234567890123456789012345678901234?=\r\n"
			" =?us-ascii?Q?5678901234567890123456789012345678?=\r\n"
			" =?us-ascii?Q?9012345678901234567890123456789?=", w.generate(50)
		);
	}

	void testForcedNonEncoding() {

		// Testing long unbreakable and unencodable header
		vmime::relay r;
		r.parse(
			" from User (Ee9GMqZQ8t7IQwftfAFHd2KyScCYRrFSJ50tKEoXv2bVCG4HcPU80GGWiFabAvG77FekpGgF1h@[127.0.0.1]) by servername.hostname.com\n\t"
			"with esmtp id 1NGTS9-2C0sqG0; Fri, 4 Dec 2009 09:23:49 +0100"
		);

		VASSERT_EQ(
			"received.long",
			"from User\r\n (Ee9GMqZQ8t7IQwftfAFHd2KyScCYRrFSJ50tKEoXv2bVCG4HcPU80GGWiFabAvG77FekpGgF1h@[127.0.0.1])\r\n by servername.hostname.com with esmtp id 1NGTS9-2C0sqG0; Fri, 4 Dec 2009\r\n 09:23:49 +0100",
			r.generate(78)
		);
	}

	void testBugFix20110511() {

		/*

		 Using the latest version of vmime (0.9.1), encoding the following string: Jean
		 Gwenaël Dutourd will result in:
		 Jean =?utf-8?Q?Gwena=C3=ABl_?= Dutourd
		 However, decoding this will result in Jean Gwenaël  Dutourd (notice two spaces
		 between the last 2 words).  The encoder adds a _ after the second word, but
		 since the last word is not encoded, the space between them is not ignored, and
		 is decoded into an additional space.

		 See: http://sourceforge.net/projects/vmime/forums/forum/237357/topic/4531365

		*/

		const std::string DECODED_TEXT = "Jean Gwenaël Dutourd";
		const std::string ENCODED_TEXT = "Jean =?utf-8?Q?Gwena=C3=ABl?= Dutourd";

		// Encode
		VASSERT_EQ(
			"encode",
			ENCODED_TEXT,
			vmime::text::newFromString(DECODED_TEXT, vmime::charset("utf-8"))->generate()
		);

		// Decode
		vmime::text t;
		t.parse(ENCODED_TEXT);

		// -- words
		std::ostringstream oss; oss << t;
		VASSERT_EQ(
			"decode1",
			"[text: [[word: charset=us-ascii, buffer=Jean ],"
			        "[word: charset=utf-8, buffer=Gwenaël],"
			        "[word: charset=us-ascii, buffer= Dutourd]]]",
			oss.str()
		);

		// -- getWholeBuffer
		VASSERT_EQ("decode2", DECODED_TEXT, t.getWholeBuffer());
	}

	void testInternationalizedEmail_specialChars() {

		vmime::generationContext ctx(vmime::generationContext::getDefaultContext());
		ctx.setInternationalizedEmailSupport(true);

		vmime::generationContext::switcher <vmime::generationContext> contextSwitcher(ctx);

		// Special sequence/chars should still be encoded
		VASSERT_EQ(
			"1",
			"=?us-ascii?Q?Test=3D=3Frfc2047_sequence?=",
			vmime::word("Test=?rfc2047 sequence", vmime::charset("us-ascii")).generate()
		);

		VASSERT_EQ(
			"2",
			"=?us-ascii?Q?Line_One=0ALine_Two?=",
			vmime::word("Line One\nLine Two", vmime::charset("us-ascii")).generate()
		);
	}

	void testInternationalizedEmail_UTF8() {

		vmime::generationContext ctx(vmime::generationContext::getDefaultContext());
		ctx.setInternationalizedEmailSupport(true);

		vmime::generationContext::switcher <vmime::generationContext> contextSwitcher(ctx);

		// Already UTF-8 encoded text should be left as is
		VASSERT_EQ(
			"1", "Achim Br\xc3\xa4ndt",
			vmime::word("Achim Br\xc3\xa4ndt", vmime::charset("utf-8")).generate()
		);
	}

	void testInternationalizedEmail_nonUTF8() {

		vmime::generationContext ctx(vmime::generationContext::getDefaultContext());
		ctx.setInternationalizedEmailSupport(true);

		vmime::generationContext::switcher <vmime::generationContext> contextSwitcher(ctx);

		// Non UTF-8 encoded text should first be converted to UTF-8
		VASSERT_EQ(
			"1", "Achim Br\xc3\xa4ndt",
			vmime::word("Achim Br\xe4ndt", vmime::charset("iso-8859-1")).generate()
		);
	}

	void testInternationalizedEmail_folding() {

		vmime::generationContext ctx(vmime::generationContext::getDefaultContext());
		ctx.setInternationalizedEmailSupport(true);

		vmime::generationContext::switcher <vmime::generationContext> contextSwitcher(ctx);

		// RFC-2047 encoding must be performed, as line folding is needed
		vmime::word w1("01234567890123456789\xc3\xa0x012345678901234567890123456789"
		               "01234567890123456789\xc3\xa0x012345678901234567890123456789", vmime::charset("utf-8"));

		VASSERT_EQ(
			"1",
			"=?utf-8?Q?01234567890123456789=C3=A0x01234567890?=\r\n"
			" =?utf-8?Q?1234567890123456789012345678901234567?=\r\n"
			" =?utf-8?Q?89=C3=A0x0123456789012345678901234567?=\r\n"
			" =?utf-8?Q?89?=",
			w1.generate(50)
		);

		// RFC-2047 encoding will not be forced, as words can be wrapped in a new line
		vmime::word w2("bla bla bla This is some '\xc3\xa0\xc3\xa7' UTF-8 encoded text", vmime::charset("utf-8"));

		VASSERT_EQ(
			"2",
			"bla bla bla This is\r\n"
			" some '\xc3\xa0\xc3\xa7' UTF-8\r\n"
			" encoded text",
			w2.generate(20)
		);
	}

	void testInternationalizedEmail_whitespace() {

		// Sanity checks for running this test
		{
			vmime::text t;
			t.parse("=?utf-8?Q?Adquisi=C3=B3n?= de Laptop y celular");

			VASSERT_EQ("parse", "Adquisión de Laptop y celular", t.getWholeBuffer());
		}

		{
			vmime::text t("Adquisi\xc3\xb3n de Laptop y celular", vmime::charset("UTF-8"));

			VASSERT_EQ("generate", "=?UTF-8?Q?Adquisi=C3=B3n?= de Laptop y celular", t.generate());
		}

		// Ensure a whitespace is added between encoded words in intl email support enabled
		{
			vmime::text t;
			t.parse("=?utf-8?Q?Adquisi=C3=B3n?= de Laptop y celular");

			std::ostringstream oss;
			vmime::utility::outputStreamAdapter ossAdapter(oss);
			vmime::generationContext gctx(vmime::generationContext::getDefaultContext());
			gctx.setInternationalizedEmailSupport(true);
			t.generate(gctx, ossAdapter);

			VASSERT_EQ("generate", "Adquisi\xc3\xb3n de Laptop y celular", oss.str());
		}

		{
			vmime::text t;
			t.parse("=?utf-8?Q?Adquisi=C3=B3n?= de Laptop =?utf-8?Q?y?= celular");

			std::ostringstream oss;
			vmime::utility::outputStreamAdapter ossAdapter(oss);
			vmime::generationContext gctx(vmime::generationContext::getDefaultContext());
			gctx.setInternationalizedEmailSupport(true);
			t.generate(gctx, ossAdapter);

			VASSERT_EQ("generate", "Adquisi\xc3\xb3n de Laptop y celular", oss.str());
		}

		{
			vmime::text t;
			t.parse("=?utf-8?Q?Adquisi=C3=B3n?= de Laptop =?utf-8?Q?y_celular?=");

			std::ostringstream oss;
			vmime::utility::outputStreamAdapter ossAdapter(oss);
			vmime::generationContext gctx(vmime::generationContext::getDefaultContext());
			gctx.setInternationalizedEmailSupport(true);
			t.generate(gctx, ossAdapter);

			VASSERT_EQ("generate", "Adquisi\xc3\xb3n de Laptop y celular", oss.str());
		}

		// Ensure no whitespace is added with non-encoded words
		{
			vmime::text t;
			t.parse("Laptop y celular");

			std::ostringstream oss;
			vmime::utility::outputStreamAdapter ossAdapter(oss);
			vmime::generationContext gctx(vmime::generationContext::getDefaultContext());
			gctx.setInternationalizedEmailSupport(true);
			t.generate(gctx, ossAdapter);

			VASSERT_EQ("generate", "Laptop y celular", oss.str());
		}

		{
			vmime::text t;
			t.parse("=?utf-8?Q?Laptop_y_celular?=");

			std::ostringstream oss;
			vmime::utility::outputStreamAdapter ossAdapter(oss);
			vmime::generationContext gctx(vmime::generationContext::getDefaultContext());
			gctx.setInternationalizedEmailSupport(true);
			t.generate(gctx, ossAdapter);

			VASSERT_EQ("generate", "Laptop y celular", oss.str());
		}
	}

	void testWronglyPaddedB64Words() {

		vmime::text outText;

		vmime::text::decodeAndUnfold("=?utf-8?B?5Lit5?=\n =?utf-8?B?paH?=", &outText);

		VASSERT_EQ(
			"1",
			"\xe4\xb8\xad\xe6\x96\x87",
			outText.getConvertedText(vmime::charset("utf-8"))
		);

		vmime::text::decodeAndUnfold("=?utf-8?B?5Lit5p?=\n =?utf-8?B?aH?=", &outText);

		VASSERT_EQ(
			"2",
			"\xe4\xb8\xad\xe6\x96\x87",
			outText.getConvertedText(vmime::charset("utf-8"))
		);

		vmime::text::decodeAndUnfold("=?utf-8?B?5Lit5pa?=\n =?utf-8?B?H?=", &outText);

		VASSERT_EQ(
			"3",
			"\xe4\xb8\xad\xe6\x96\x87",
			outText.getConvertedText(vmime::charset("utf-8"))
		);
	}

	// Ensure that words which encode a non-integral number of characters
	// are correctly decoded.
	void testFixBrokenWords() {

		vmime::text outText;

		vmime::charsetConverterOptions opts;
		opts.silentlyReplaceInvalidSequences = false;  // just to be sure that broken words are actually fixed

		// Test case 1
		vmime::text::decodeAndUnfold(
			"=?utf-8?Q?Gwena=C3?="
			"=?utf-8?Q?=ABl?=",
			&outText
		);

		VASSERT_EQ("1.1", 1, outText.getWordCount());
		VASSERT_EQ("1.2", "Gwena\xc3\xabl", outText.getWordAt(0)->getBuffer());
		VASSERT_EQ("1.3", vmime::charset("utf-8"), outText.getWordAt(0)->getCharset());

		// Test case 2
		vmime::text::decodeAndUnfold(
			"=?utf-8?B?5Lit6Yu85qmf5qKw6JGj5LqL5pyDMTAz5bm056ysMDXlsYbn?="
			"=?utf-8?B?rKwwN+asoeitsOeoiw==?=",
			&outText
		);

		VASSERT_EQ("2.1", 1, outText.getWordCount());
		VASSERT_EQ("2.2", "\xe4\xb8\xad\xe9\x8b\xbc\xe6\xa9\x9f\xe6\xa2\xb0"
			"\xe8\x91\xa3\xe4\xba\x8b\xe6\x9c\x83\x31\x30\x33\xe5\xb9\xb4"
			"\xe7\xac\xac\x30\x35\xe5\xb1\x86\xe7\xac\xac\x30\x37\xe6\xac"
			"\xa1\xe8\xad\xb0\xe7\xa8\x8b", outText.getWordAt(0)->getBuffer());
		VASSERT_EQ("2.3", vmime::charset("utf-8"), outText.getWordAt(0)->getCharset());

		// Test case 3 (a character spanning over 3 words: 'を' = E3 82 92)
		vmime::text::decodeAndUnfold(
			"=?utf-8?Q?abc=E3?="
			"=?utf-8?Q?=82?="
			"=?utf-8?Q?=92xyz?=",
			&outText
		);

		VASSERT_EQ("3.1", 1, outText.getWordCount());
		VASSERT_EQ("3.2", "abc\xe3\x82\x92xyz", outText.getWordAt(0)->getBuffer());
		VASSERT_EQ("3.3", vmime::charset("utf-8"), outText.getWordAt(0)->getCharset());

		// Test case 4 (remains invalid)
		vmime::text::decodeAndUnfold(
			"=?utf-8?Q?abc=E3?="
			"=?utf-8?Q?=82?="
			"=?utf-8?Q?xy?="
			"=?utf-8?Q?z?=",
			&outText
		);

		VASSERT_EQ("4.1", 2, outText.getWordCount());
		VASSERT_EQ("4.2", "abc", outText.getWordAt(0)->getBuffer());
		VASSERT_EQ("4.3", vmime::charset("utf-8"), outText.getWordAt(0)->getCharset());
		VASSERT_EQ("4.4", "\xe3\x82xyz", outText.getWordAt(1)->getBuffer());
		VASSERT_EQ("4.5", vmime::charset("utf-8"), outText.getWordAt(1)->getCharset());

		// Test case 5 (remains partially invalid)
		vmime::text::decodeAndUnfold(
			"=?utf-8?Q?abc=E3?="
			"=?utf-8?Q?=82?="
			"=?utf-8?Q?\x92xy?="
			"=?utf-8?Q?z\xc3?=",
			&outText
		);

		VASSERT_EQ("5.1", 2, outText.getWordCount());
		VASSERT_EQ("5.2", "abc\xe3\x82\x92xyz", outText.getWordAt(0)->getBuffer());
		VASSERT_EQ("5.3", vmime::charset("utf-8"), outText.getWordAt(0)->getCharset());
		VASSERT_EQ("5.4", "\xc3", outText.getWordAt(1)->getBuffer());
		VASSERT_EQ("5.5", vmime::charset("utf-8"), outText.getWordAt(1)->getCharset());
	}

	void testUnknownCharset() {

		vmime::text t;
		vmime::text::decodeAndUnfold("=?gb2312?B?wdaRY8PA?=", &t);

		VASSERT_EQ("1.1", 1, t.getWordCount());
		VASSERT_EQ("1.2", "\xc1\xd6\x91\x63\xc3\xc0", t.getWordAt(0)->getBuffer());
		VASSERT_EQ("1.3", vmime::charset("gb2312"), t.getWordAt(0)->getCharset());



		vmime::parsingContext ctx;

		const vmime::string hfieldBuffer = "From: '=?gb2312?B?wdaRY8PA?=' <a.b@c.de>";

		vmime::shared_ptr <vmime::headerField> hfield =
			vmime::headerField::parseNext(ctx, hfieldBuffer, 0, hfieldBuffer.size());

		vmime::shared_ptr <vmime::mailbox> hvalue =
			hfield->getValue <vmime::mailbox>();

		VASSERT_EQ("2.1", 3, hvalue->getName().getWordCount());
		VASSERT_EQ("2.2", "'", hvalue->getName().getWordAt(0)->getBuffer());
		VASSERT_EQ("2.3", vmime::charset("us-ascii"), hvalue->getName().getWordAt(0)->getCharset());
		VASSERT_EQ("2.4", "\xc1\xd6\x91\x63\xc3\xc0", hvalue->getName().getWordAt(1)->getBuffer());
		VASSERT_EQ("2.5", vmime::charset("gb2312"), hvalue->getName().getWordAt(1)->getCharset());
		VASSERT_EQ("2.6", "'", hvalue->getName().getWordAt(2)->getBuffer());
		VASSERT_EQ("2.7", vmime::charset("us-ascii"), hvalue->getName().getWordAt(2)->getCharset());
	}

VMIME_TEST_SUITE_END
