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


#define VMIME_TEST_SUITE         charsetTest
#define VMIME_TEST_SUITE_MODULE  "Parser"


// Excerpt from http://www.gnu.org/
static const vmime::charset inputCharset("gb2312");
static const char inputBytes[] =
	"\xbb\xb6\xd3\xad\xc0\xb4\xb5\xbd\x20\x47\x4e\x55\x20\xb9\xa4\xb3"
	"\xcc\xb5\xc4\xcd\xf8\xd2\xb3\xcb\xc5\xb7\xfe\xd6\xf7\xbb\xfa\x20"
	"\x77\x77\x77\x2e\x67\x6e\x75\x2e\x6f\x72\x67\x20\xa1\xa3\x20\x47"
	"\x4e\x55\x20\xb9\xa4\xb3\xcc\x20\xbf\xaa\xca\xbc\xec\xb6\xd2\xbb"
	"\xbe\xc5\xb0\xcb\xcb\xc4\xc4\xea\xa3\xac\xd6\xbc\xd4\xda\xb7\xa2"
	"\xd5\xb9\xd2\xbb\xb8\xf6\xc0\xe0\xcb\xc6\x20\x55\x6e\x69\x78\x20"
	"\xa3\xac\xc7\xd2\xce\xaa\x20\xd7\xd4\xd3\xc9\xc8\xed\xbc\xfe\x20"
	"\xb5\xc4\xcd\xea\xd5\xfb\xb2\xd9\xd7\xf7\xcf\xb5\xcd\xb3\xa3\xba"
	"\x20\x47\x4e\x55\x20\xcf\xb5\xcd\xb3\xa1\xa3\xa3\xa8\x47\x4e\x55"
	"\x20\xca\xc7\xd3\xc9\xa1\xb0\x47\x4e\x55\x27\x73\x20\x4e\x6f\x74"
	"\x20\x55\x6e\x69\x78\xa1\xb1\xcb\xf9\xb5\xdd\xbb\xd8\xb6\xa8\xd2"
	"\xe5\xb3\xf6\xb5\xc4\xca\xd7\xd7\xd6\xc4\xb8\xcb\xf5\xd0\xb4\xd3"
	"\xef\xa3\xbb\xcb\xfc\xb5\xc4\xb7\xa2\xd2\xf4\xce\xaa\xa1\xb0\x67"
	"\x75\x68\x2d\x4e\x45\x57\xa1\xb1\xa3\xa9\xa1\xa3\xb8\xf7\xd6\xd6"
	"\xca\xb9\xd3\xc3\x20\x4c\x69\x6e\x75\x78\x20\xd7\xf7\xce\xaa\xc4"
	"\xda\xba\xcb\xb5\xc4\x20\x47\x4e\x55\x20\xb2\xd9\xd7\xf7\xcf\xb5"
	"\xcd\xb3\xd5\xfd\xb1\xbb\xb9\xe3\xb7\xba\xb5\xd8\xca\xb9\xd3\xc3"
	"\xd6\xf8\xa3\xbb\xcb\xe4\xc8\xbb\xd5\xe2\xd0\xa9\xcf\xb5\xcd\xb3"
	"\xcd\xa8\xb3\xa3\xb1\xbb\xb3\xc6\xd7\xf7\xce\xaa\xa1\xb0\x4c\x69"
	"\x6e\x75\x78\xa1\xb1\xa3\xac\xb5\xab\xca\xc7\xcb\xfc\xc3\xc7\xd3"
	"\xa6\xb8\xc3\xb8\xfc\xbe\xab\xc8\xb7\xb5\xd8\xb1\xbb\xb3\xc6\xce"
	"\xaa\x20\x47\x4e\x55\x2f\x4c\x69\x6e\x75\x78\x20\xcf\xb5\xcd\xb3"
	"\x20\xa1\xa3\x0a";

static const vmime::charset outputCharset("utf-8");
static const char outputBytes[] =
	"\xe6\xac\xa2\xe8\xbf\x8e\xe6\x9d\xa5\xe5\x88\xb0\x20\x47\x4e\x55"
	"\x20\xe5\xb7\xa5\xe7\xa8\x8b\xe7\x9a\x84\xe7\xbd\x91\xe9\xa1\xb5"
	"\xe4\xbc\xba\xe6\x9c\x8d\xe4\xb8\xbb\xe6\x9c\xba\x20\x77\x77\x77"
	"\x2e\x67\x6e\x75\x2e\x6f\x72\x67\x20\xe3\x80\x82\x20\x47\x4e\x55"
	"\x20\xe5\xb7\xa5\xe7\xa8\x8b\x20\xe5\xbc\x80\xe5\xa7\x8b\xe6\x96"
	"\xbc\xe4\xb8\x80\xe4\xb9\x9d\xe5\x85\xab\xe5\x9b\x9b\xe5\xb9\xb4"
	"\xef\xbc\x8c\xe6\x97\xa8\xe5\x9c\xa8\xe5\x8f\x91\xe5\xb1\x95\xe4"
	"\xb8\x80\xe4\xb8\xaa\xe7\xb1\xbb\xe4\xbc\xbc\x20\x55\x6e\x69\x78"
	"\x20\xef\xbc\x8c\xe4\xb8\x94\xe4\xb8\xba\x20\xe8\x87\xaa\xe7\x94"
	"\xb1\xe8\xbd\xaf\xe4\xbb\xb6\x20\xe7\x9a\x84\xe5\xae\x8c\xe6\x95"
	"\xb4\xe6\x93\x8d\xe4\xbd\x9c\xe7\xb3\xbb\xe7\xbb\x9f\xef\xbc\x9a"
	"\x20\x47\x4e\x55\x20\xe7\xb3\xbb\xe7\xbb\x9f\xe3\x80\x82\xef\xbc"
	"\x88\x47\x4e\x55\x20\xe6\x98\xaf\xe7\x94\xb1\xe2\x80\x9c\x47\x4e"
	"\x55\x27\x73\x20\x4e\x6f\x74\x20\x55\x6e\x69\x78\xe2\x80\x9d\xe6"
	"\x89\x80\xe9\x80\x92\xe5\x9b\x9e\xe5\xae\x9a\xe4\xb9\x89\xe5\x87"
	"\xba\xe7\x9a\x84\xe9\xa6\x96\xe5\xad\x97\xe6\xaf\x8d\xe7\xbc\xa9"
	"\xe5\x86\x99\xe8\xaf\xad\xef\xbc\x9b\xe5\xae\x83\xe7\x9a\x84\xe5"
	"\x8f\x91\xe9\x9f\xb3\xe4\xb8\xba\xe2\x80\x9c\x67\x75\x68\x2d\x4e"
	"\x45\x57\xe2\x80\x9d\xef\xbc\x89\xe3\x80\x82\xe5\x90\x84\xe7\xa7"
	"\x8d\xe4\xbd\xbf\xe7\x94\xa8\x20\x4c\x69\x6e\x75\x78\x20\xe4\xbd"
	"\x9c\xe4\xb8\xba\xe5\x86\x85\xe6\xa0\xb8\xe7\x9a\x84\x20\x47\x4e"
	"\x55\x20\xe6\x93\x8d\xe4\xbd\x9c\xe7\xb3\xbb\xe7\xbb\x9f\xe6\xad"
	"\xa3\xe8\xa2\xab\xe5\xb9\xbf\xe6\xb3\x9b\xe5\x9c\xb0\xe4\xbd\xbf"
	"\xe7\x94\xa8\xe8\x91\x97\xef\xbc\x9b\xe8\x99\xbd\xe7\x84\xb6\xe8"
	"\xbf\x99\xe4\xba\x9b\xe7\xb3\xbb\xe7\xbb\x9f\xe9\x80\x9a\xe5\xb8"
	"\xb8\xe8\xa2\xab\xe7\xa7\xb0\xe4\xbd\x9c\xe4\xb8\xba\xe2\x80\x9c"
	"\x4c\x69\x6e\x75\x78\xe2\x80\x9d\xef\xbc\x8c\xe4\xbd\x86\xe6\x98"
	"\xaf\xe5\xae\x83\xe4\xbb\xac\xe5\xba\x94\xe8\xaf\xa5\xe6\x9b\xb4"
	"\xe7\xb2\xbe\xe7\xa1\xae\xe5\x9c\xb0\xe8\xa2\xab\xe7\xa7\xb0\xe4"
	"\xb8\xba\x20\x47\x4e\x55\x2f\x4c\x69\x6e\x75\x78\x20\xe7\xb3\xbb"
	"\xe7\xbb\x9f\x20\xe3\x80\x82\x0a";



VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		// Test valid input
		VMIME_TEST(testConvertStringValid)
		VMIME_TEST(testConvertStreamValid)
		VMIME_TEST(testFilterValid1)
		VMIME_TEST(testFilterValid2)
		VMIME_TEST(testFilterValid3)
		VMIME_TEST(testEncodingHebrew1255)

		// Test invalid input
		VMIME_TEST(testFilterInvalid1)

		// TODO: more tests
	VMIME_TEST_LIST_END


	void testConvertStringValid()
	{
		vmime::string in(inputBytes, sizeof(inputBytes) - 1);
		vmime::string expectedOut(outputBytes, sizeof(outputBytes) - 1);
		vmime::string actualOut;

		vmime::charset::convert
			(in, actualOut, inputCharset, outputCharset);

		VASSERT_EQ("1", toHex(expectedOut), toHex(actualOut));
	}

	void testConvertStreamValid()
	{
		vmime::string in(inputBytes, sizeof(inputBytes) - 1);
		vmime::string expectedOut(outputBytes, sizeof(outputBytes) - 1);

		vmime::string actualOut;
		vmime::utility::outputStreamStringAdapter os(actualOut);

		vmime::utility::inputStreamStringAdapter is(in);

		vmime::charset::convert
			(is, os, inputCharset, outputCharset);

		os.flush();

		VASSERT_EQ("1", toHex(expectedOut), toHex(actualOut));
	}

	// Using 'bufferedStreamCopy'
	void testFilterValid1()
	{
		vmime::string in(inputBytes, sizeof(inputBytes) - 1);
		vmime::string expectedOut(outputBytes, sizeof(outputBytes) - 1);

		vmime::string actualOut;
		vmime::utility::outputStreamStringAdapter osa(actualOut);
		vmime::utility::charsetFilteredOutputStream os
			(inputCharset, outputCharset, osa);

		vmime::utility::inputStreamStringAdapter is(in);

		vmime::utility::bufferedStreamCopy(is, os);

		os.flush();

		VASSERT_EQ("1", toHex(expectedOut), toHex(actualOut));
	}

	// One byte at a time
	void testFilterValid2()
	{
		vmime::string in(inputBytes, sizeof(inputBytes) - 1);
		vmime::string expectedOut(outputBytes, sizeof(outputBytes) - 1);

		vmime::string actualOut;
		vmime::utility::outputStreamStringAdapter osa(actualOut);
		vmime::utility::charsetFilteredOutputStream os
			(inputCharset, outputCharset, osa);

		vmime::utility::inputStreamStringAdapter is(in);

		vmime::utility::stream::value_type buffer[16];

		for (int i = 0 ; !is.eof() ; ++i)
			os.write(buffer, is.read(buffer, 1));

		os.flush();

		VASSERT_EQ("1", toHex(expectedOut), toHex(actualOut));
	}

	// Variable chunks
	void testFilterValid3()
	{
		vmime::string in(inputBytes, sizeof(inputBytes) - 1);
		vmime::string expectedOut(outputBytes, sizeof(outputBytes) - 1);

		vmime::string actualOut;
		vmime::utility::outputStreamStringAdapter osa(actualOut);
		vmime::utility::charsetFilteredOutputStream os
			(inputCharset, outputCharset, osa);

		vmime::utility::inputStreamStringAdapter is(in);

		vmime::utility::stream::value_type buffer[16];

		for (int i = 0 ; !is.eof() ; ++i)
			os.write(buffer, is.read(buffer, (i % 5) + 1));

		os.flush();

		VASSERT_EQ("1", toHex(expectedOut), toHex(actualOut));
	}

	void testFilterInvalid1()
	{
		vmime::string in("foo\xab\xcd\xef bar");
		vmime::string expectedOut("foo??? bar");

		vmime::string actualOut;
		vmime::utility::outputStreamStringAdapter osa(actualOut);
		vmime::utility::charsetFilteredOutputStream os
			(vmime::charset("utf-8"),
			 vmime::charset("iso-8859-1"), osa);

		vmime::utility::inputStreamStringAdapter is(in);

		vmime::utility::stream::value_type buffer[16];

		for (int i = 0 ; !is.eof() ; ++i)
			os.write(buffer, is.read(buffer, 1));

		os.flush();

		VASSERT_EQ("1", toHex(expectedOut), toHex(actualOut));
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

	// Conversion to hexadecimal for easier debugging
	static const vmime::string toHex(const vmime::string str)
	{
		static const char hexChars[] = "0123456789abcdef";

		vmime::string res = "\n";

		for (unsigned int i = 0 ; i < str.length() ; i += 16)
		{
			unsigned int r = std::min
				(static_cast <size_t>(16), str.length() - i);

			vmime::string hex;
			vmime::string chr;

			for (unsigned int j = 0 ; j < r ; ++j)
			{
				const unsigned char c = str[i + j];

				hex += hexChars[c / 16];
				hex += hexChars[c % 16];
				hex += " ";

				if (c >= 32 && c <= 127)
					chr += c;
				else
					chr += '.';
			}

			for (unsigned int j = r ; j < 16 ; ++j)
				hex += "   ";

			res += hex + "  " + chr + "\n";
		}

		return res;
	}

VMIME_TEST_SUITE_END

