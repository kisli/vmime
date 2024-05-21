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

#include <algorithm>

#include "tests/testUtils.hpp"

#include "charsetTestSuites.hpp"


VMIME_TEST_SUITE_BEGIN(charsetTest)

	VMIME_TEST_LIST_BEGIN
		// Test valid input
		VMIME_TEST(testConvertStringValid)
		VMIME_TEST(testConvertStreamValid)
		VMIME_TEST(testConvertStreamExtract)
		VMIME_TEST(testEncodingHebrew1255)
		//VMIME_TEST(testEncodingSelectionOnASCII)

		// IDNA
		VMIME_TEST(testEncodeIDNA)
		VMIME_TEST(testDecodeIDNA)

		VMIME_TEST(testUTF7Support)

		VMIME_TEST(testReplaceInvalidSequence)
		VMIME_TEST(testStopOnInvalidSequence)

		VMIME_TEST(testStatus)
		VMIME_TEST(testStatusWithInvalidSequence)

		VMIME_TEST(testIsValidText)
	VMIME_TEST_LIST_END


	void testConvertStringValid() {

		for (unsigned int i = 0 ; i < charsetTestSuitesCount ; ++i) {

			const charsetTestSuiteStruct& entry = charsetTestSuites[i];

			std::ostringstream testName;
			testName << i << ": " << entry.fromCharset << " -> " << entry.toCharset;

			const unsigned long inLength = (entry.fromLength == 0 ? strlen(entry.fromBytes) : entry.fromLength);
			vmime::string in(entry.fromBytes, entry.fromBytes + inLength);

			const unsigned long outLength = (entry.toLength == 0 ? strlen(entry.toBytes) : entry.toLength);
			vmime::string expectedOut(entry.toBytes, entry.toBytes + outLength);

			vmime::string actualOut;

			vmime::charset::convert
				(in, actualOut, entry.fromCharset, entry.toCharset);

			VASSERT_EQ(testName.str(), toHex(expectedOut), toHex(actualOut));
		}
	}

	void testConvertStreamValid() {

		for (unsigned int i = 0 ; i < charsetTestSuitesCount ; ++i) {

			const charsetTestSuiteStruct& entry = charsetTestSuites[i];

			std::ostringstream testName;
			testName << i << ": " << entry.fromCharset << " -> " << entry.toCharset;

			const unsigned long inLength = (entry.fromLength == 0 ? strlen(entry.fromBytes) : entry.fromLength);
			vmime::string in(entry.fromBytes, entry.fromBytes + inLength);

			const unsigned long outLength = (entry.toLength == 0 ? strlen(entry.toBytes) : entry.toLength);
			vmime::string expectedOut(entry.toBytes, entry.toBytes + outLength);

			vmime::string actualOut;
			vmime::utility::outputStreamStringAdapter os(actualOut);

			vmime::utility::inputStreamStringAdapter is(in);

			vmime::charset::convert(is, os, entry.fromCharset, entry.toCharset);

			os.flush();

			VASSERT_EQ(testName.str(), toHex(expectedOut), toHex(actualOut));
		}
	}

	void testConvertStreamExtract() {
		vmime::bodyPart p;
		p.getBody()->setContents(
			vmime::make_shared <vmime::stringContentHandler>(
				"Foo éé\r\né bar\r\nbaz"
			),
			vmime::mediaType("text", "plain"),
			vmime::charset("utf-8"),
			vmime::encoding("quoted-printable")
		);

		vmime::string str;
		vmime::utility::outputStreamStringAdapter outStr(str);
		vmime::shared_ptr <vmime::charsetConverter> conv = vmime::charsetConverter::create(p.getBody()->getCharset(), vmime::charset("US-ASCII"));
		auto filteredStream = conv->getFilteredOutputStream(outStr);
		p.getBody()->getContents()->extract(*filteredStream);
		filteredStream->flush();
		VASSERT_EQ(
			"generate",
			"Foo ??\r\n"
			"? bar\r\n"
			"baz",
			str
		);
	}

	void testEncodingHebrew1255() {

		// hewbrew string in windows-1255 charset
		const char data[] = "\xe9\xf9\xf7\xf8\xe9\xf9\xf8\xf7\xe9\xe9\xf9";
		vmime::word w = vmime::word(data, "windows-1255");
		vmime::string encoded = w.generate();
		// less than 60% ascii, base64 received
		VASSERT_EQ("1", "=?windows-1255?B?6fn3+On5+Pfp6fk=?=", encoded);
	}

#if 0 /* decideImpl is not public */
	void testEncodingSelectionOnASCII() {

		const std::string a = "Hi\x01!", b = "Hi\x7f!";
		VASSERT_EQ("1", vmime::encoding::decideImpl(std::begin(a), std::end(a)),
		                vmime::encoding::decideImpl(std::begin(b), std::end(b)));

	}
#endif

	static const vmime::string convertHelper(
		const vmime::string& in,
		const vmime::charset& csrc,
		const vmime::charset& cdest,
		const vmime::charsetConverterOptions& opts = vmime::charsetConverterOptions(),
		vmime::charsetConverter::status* st = NULL
	) {

		vmime::shared_ptr <vmime::charsetConverter> conv =
			vmime::charsetConverter::create(csrc, cdest, opts);

		vmime::string out;
		conv->convert(in, out, st);

		return out;
	}

	void testEncodeIDNA() {

		VASSERT_EQ("1", "xn--espaol-zwa", convertHelper("español", "utf-8", "idna"));

		// Tests from ICANN
		VASSERT_EQ("2.1", "xn--hxajbheg2az3al", convertHelper("παράδειγμα", "utf-8", "idna"));
		VASSERT_EQ("2.2", "xn--jxalpdlp", convertHelper("δοκιμή", "utf-8", "idna"));

		VASSERT_EQ("3.1", "xn--mgbh0fb", convertHelper("مثال", "utf-8", "idna"));
		VASSERT_EQ("3.2", "xn--kgbechtv", convertHelper("إختبار", "utf-8", "idna"));
	}

	void testDecodeIDNA() {

		VASSERT_EQ("1", "español", convertHelper("xn--espaol-zwa", "idna", "utf-8"));

		// Tests from ICANN
		VASSERT_EQ("2.1", "παράδειγμα", convertHelper("xn--hxajbheg2az3al", "idna", "utf-8"));
		VASSERT_EQ("2.2", "δοκιμή", convertHelper("xn--jxalpdlp", "idna", "utf-8"));

		VASSERT_EQ("3.1", "مثال", convertHelper("xn--mgbh0fb", "idna", "utf-8"));
		VASSERT_EQ("3.2", "إختبار", convertHelper("xn--kgbechtv", "idna", "utf-8"));
	}

	void testUTF7Support() {

		// Ensure UTF-7 is supported, because it is used for IMAP
		VASSERT_EQ("1", "VMime +- UTF-7 encoding", convertHelper("VMime + UTF-7 encoding", "utf-8", "utf-7"));
		VASSERT_EQ("2", "f+APg-o", convertHelper("\x66\xc3\xb8\x6f", "utf-8", "utf-7"));
	}

	void testReplaceInvalidSequence() {

		vmime::charsetConverterOptions opts;
		opts.silentlyReplaceInvalidSequences = true;
		opts.invalidSequence = "?";

		vmime::string res = convertHelper(
			"\x61\xf1\x80\x80\xe1\x80\xc2\x62\x80\x63\x80\xbf\x64",
			"utf-8", "iso-8859-1", opts
		);

		// Result should be in the form "a???b?c??d" or "a??????b?c??d"...
		// Remove consecutive question marks for easier matching.
		res.erase(std::unique(res.begin(), res.end()), res.end());

		VASSERT_EQ(
			"Illegal UTF-8 sequence",
			"a?b?c?d",
			res
		);
	}

	void testStopOnInvalidSequence() {

		vmime::charsetConverterOptions opts;
		opts.silentlyReplaceInvalidSequences = false;

		VASSERT_THROW(
			"Illegal UTF-8 sequence",
			convertHelper("\x61\xf1\x80\x80\xe1\x80\xc2\x62\x80\x63\x80\xbf\x64", "utf-8", "iso-8859-1", opts),
			vmime::exceptions::illegal_byte_sequence_for_charset
		);
	}

	void testStatus() {

		vmime::charsetConverterOptions opts;
		opts.silentlyReplaceInvalidSequences = false;

		vmime::charsetConverter::status st;

		//             012345   6   7
		convertHelper("Gwena\xc3\xabl", "utf-8", "iso-8859-1", opts, &st);

		VASSERT_EQ("inputBytesRead", 8, st.inputBytesRead);
		VASSERT_EQ("outputBytesWritten", 7, st.outputBytesWritten);
	}

	void testStatusWithInvalidSequence() {

		vmime::charsetConverterOptions opts;
		opts.silentlyReplaceInvalidSequences = false;

		vmime::charsetConverter::status st;

		try {

			//             01234   5   6789   0   1
			convertHelper("Fran\xc3\xa7ois\xf1\x80\x65", "utf-8", "iso-8859-1", opts, &st);

		} catch (vmime::exceptions::illegal_byte_sequence_for_charset& e) {

		} catch (...) {

			throw;
		}

		VASSERT_EQ("inputBytesRead", 9, st.inputBytesRead);
		VASSERT_EQ("outputBytesWritten", 8, st.outputBytesWritten);
	}

	void testIsValidText() {

		// Invalid text
		const vmime::string invalidText("Fran\xc3\xa7ois\xf1\x80\x65");
		vmime::string::size_type firstInvalidByte;

		VASSERT_EQ("invalid.isValidText", false, vmime::charset("utf-8").isValidText(invalidText, &firstInvalidByte));
		VASSERT_EQ("invalid.firstInvalidByte", 9, firstInvalidByte);

		// Valid text
		const vmime::string validText("Gwena\xc3\xabl");

		VASSERT_EQ("valid.isValidText", true, vmime::charset("utf-8").isValidText(validText, &firstInvalidByte));
		VASSERT_EQ("valid.firstInvalidByte", 8, firstInvalidByte);
	}

VMIME_TEST_SUITE_END
