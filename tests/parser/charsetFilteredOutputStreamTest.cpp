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

#include "vmime/charset.hpp"
#include "vmime/charsetConverter.hpp"

#include "charsetTestSuites.hpp"


VMIME_TEST_SUITE_BEGIN(charsetFilteredOutputStreamTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testInputBufferUnderflow)
		VMIME_TEST(testInvalidInput1)
		VMIME_TEST(testStreamCopy)
		VMIME_TEST(testOneByteAtTime)
		VMIME_TEST(testVariableInputChunk)
	VMIME_TEST_LIST_END


	void testInputBufferUnderflow()
	{
		vmime::shared_ptr <vmime::charsetConverter> cc =
			vmime::charsetConverter::create("utf-8", "iso-8859-1");

		vmime::string output;
		vmime::utility::outputStreamStringAdapter os(output);
		vmime::shared_ptr <vmime::utility::filteredOutputStream> cfos = cc->getFilteredOutputStream(os);

		VASSERT_NOT_NULL("filteredOutputStream availability", cfos);

		// føo = 66 c3 b8 6f [UTF8]
		// føo = 66 f8 6f [latin1]

		cfos->write("\x66\xc3", 2);

		// Incomplete UTF-8 sequence was not converted
		VASSERT_EQ("chunk 1", toHex("f"), toHex(output));

		// Write second byte of UTF-8 sequence
		cfos->write("\xb8\x6f", 2);

		VASSERT_EQ("chunk 2", toHex("f\xf8o"), toHex(output));
	}

	void testInvalidInput1()
	{
		vmime::string in("foo\xab\xcd\xef bar");
		vmime::string expectedOut("foo??? bar");

		vmime::string actualOut;
		vmime::utility::outputStreamStringAdapter osa(actualOut);

		vmime::shared_ptr <vmime::charsetConverter> conv =
			vmime::charsetConverter::create
				(vmime::charset("utf-8"),
				 vmime::charset("iso-8859-1"));

		vmime::shared_ptr <vmime::utility::charsetFilteredOutputStream> os =
			conv->getFilteredOutputStream(osa);

		VASSERT_NOT_NULL("filteredOutputStream availability", os);

		vmime::utility::inputStreamStringAdapter is(in);

		vmime::byte_t buffer[16];

		for (int i = 0 ; !is.eof() ; ++i)
			os->write(buffer, is.read(buffer, 1));

		os->flush();

		VASSERT_EQ("1", toHex(expectedOut), toHex(actualOut));
	}

	// Using 'bufferedStreamCopy'
	void testStreamCopy()
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
			vmime::utility::outputStreamStringAdapter osa(actualOut);

			vmime::shared_ptr <vmime::charsetConverter> conv =
				vmime::charsetConverter::create(entry.fromCharset, entry.toCharset);

			vmime::shared_ptr <vmime::utility::charsetFilteredOutputStream> os =
				conv->getFilteredOutputStream(osa);

			VASSERT_NOT_NULL("filteredOutputStream availability", os);

			vmime::utility::inputStreamStringAdapter is(in);

			vmime::utility::bufferedStreamCopy(is, *os);

			os->flush();

			VASSERT_EQ(testName.str(), toHex(expectedOut), toHex(actualOut));
		}
	}

	// One byte at a time
	void testOneByteAtTime()
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
			vmime::utility::outputStreamStringAdapter osa(actualOut);

			vmime::shared_ptr <vmime::charsetConverter> conv =
				vmime::charsetConverter::create(entry.fromCharset, entry.toCharset);

			vmime::shared_ptr <vmime::utility::charsetFilteredOutputStream> os =
				conv->getFilteredOutputStream(osa);

			VASSERT_NOT_NULL("filteredOutputStream availability", os);

			vmime::utility::inputStreamStringAdapter is(in);

			vmime::byte_t buffer[16];

			for (int i = 0 ; !is.eof() ; ++i)
				os->write(buffer, is.read(buffer, 1));

			os->flush();

			VASSERT_EQ(testName.str(), toHex(expectedOut), toHex(actualOut));
		}
	}

	// Variable chunks
	void testVariableInputChunk()
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
			vmime::utility::outputStreamStringAdapter osa(actualOut);

			vmime::shared_ptr <vmime::charsetConverter> conv =
				vmime::charsetConverter::create(entry.fromCharset, entry.toCharset);

			vmime::shared_ptr <vmime::utility::charsetFilteredOutputStream> os =
				conv->getFilteredOutputStream(osa);

			VASSERT_NOT_NULL("filteredOutputStream availability", os);

			vmime::utility::inputStreamStringAdapter is(in);

			vmime::byte_t buffer[16];

			for (int i = 0 ; !is.eof() ; ++i)
				os->write(buffer, is.read(buffer, (i % 5) + 1));

			os->flush();

			VASSERT_EQ(testName.str(), toHex(expectedOut), toHex(actualOut));
		}
	}

VMIME_TEST_SUITE_END
