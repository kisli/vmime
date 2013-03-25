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

#include "charsetTestSuites.hpp"


VMIME_TEST_SUITE_BEGIN(charsetTest)

	VMIME_TEST_LIST_BEGIN
		// Test valid input
		VMIME_TEST(testConvertStringValid)
		VMIME_TEST(testConvertStreamValid)
		VMIME_TEST(testEncodingHebrew1255)

		// IDNA
		VMIME_TEST(testEncodeIDNA)
		VMIME_TEST(testDecodeIDNA)

		VMIME_TEST(testUTF7Support)
	VMIME_TEST_LIST_END


	void testConvertStringValid()
	{
		for (unsigned int i = 0 ; i < charsetTestSuitesCount ; ++i)
		{
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

	void testConvertStreamValid()
	{
		for (unsigned int i = 0 ; i < charsetTestSuitesCount ; ++i)
		{
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

			vmime::charset::convert
				(is, os, entry.fromCharset, entry.toCharset);

			os.flush();

			VASSERT_EQ(testName.str(), toHex(expectedOut), toHex(actualOut));
		}
	}

	void testEncodingHebrew1255()
	{
		// hewbrew string in windows-1255 charset
		const char data[] = "\xe9\xf9\xf7\xf8\xe9\xf9\xf8\xf7\xe9\xe9\xf9";
		vmime::word w = vmime::word(data, "windows-1255");
		vmime::string encoded = w.generate();
		// less than 60% ascii, base64 received
		VASSERT_EQ("1", "=?windows-1255?B?6fn3+On5+Pfp6fk=?=", encoded);
	}

	static const vmime::string convertHelper
		(const vmime::string& in, const vmime::charset& csrc, const vmime::charset& cdest)
	{
		vmime::string out;
		vmime::charset::convert(in, out, csrc, cdest);

		return out;
	}

	void testEncodeIDNA()
	{
		VASSERT_EQ("1", "xn--espaol-zwa", convertHelper("español", "utf-8", "idna"));

		// Tests from ICANN
		VASSERT_EQ("2.1", "xn--hxajbheg2az3al", convertHelper("παράδειγμα", "utf-8", "idna"));
		VASSERT_EQ("2.2", "xn--jxalpdlp", convertHelper("δοκιμή", "utf-8", "idna"));

		VASSERT_EQ("3.1", "xn--mgbh0fb", convertHelper("مثال", "utf-8", "idna"));
		VASSERT_EQ("3.2", "xn--kgbechtv", convertHelper("إختبار", "utf-8", "idna"));
	}

	void testDecodeIDNA()
	{
		VASSERT_EQ("1", "español", convertHelper("xn--espaol-zwa", "idna", "utf-8"));

		// Tests from ICANN
		VASSERT_EQ("2.1", "παράδειγμα", convertHelper("xn--hxajbheg2az3al", "idna", "utf-8"));
		VASSERT_EQ("2.2", "δοκιμή", convertHelper("xn--jxalpdlp", "idna", "utf-8"));

		VASSERT_EQ("3.1", "مثال", convertHelper("xn--mgbh0fb", "idna", "utf-8"));
		VASSERT_EQ("3.2", "إختبار", convertHelper("xn--kgbechtv", "idna", "utf-8"));
	}

	void testUTF7Support()
	{
		// Ensure UTF-7 is supported, because it is used for IMAP
		VASSERT_EQ("1", "VMime +- UTF-7 encoding", convertHelper("VMime + UTF-7 encoding", "utf-8", "utf-7"));
		VASSERT_EQ("2", "f+APg-o", convertHelper("\x66\xc3\xb8\x6f", "utf-8", "utf-7"));
	}

VMIME_TEST_SUITE_END

