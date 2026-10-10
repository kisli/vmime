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

#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/prefixedInputStream.hpp"
#include "vmime/utility/stringUtils.hpp"


using namespace vmime::utility;


VMIME_TEST_SUITE_BEGIN(prefixedInputStreamTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testRead)
		VMIME_TEST(testReadPartialPrefix)
		VMIME_TEST(testReadEmptyPrefix)
		VMIME_TEST(testReadEmptyStream)
		VMIME_TEST(testEOF)
		VMIME_TEST(testSkip)
		VMIME_TEST(testReset)
		VMIME_TEST(testUnderlyingStreamError)
	VMIME_TEST_LIST_END


	static const vmime::string readAll(inputStream& is) {

		vmime::string res;
		vmime::byte_t buffer[100];

		while (!is.eof()) {

			const vmime::size_t read = is.read(buffer, sizeof(buffer));

			if (read == 0) {
				break;
			}

			res += vmime::utility::stringUtils::makeStringFromBytes(buffer, read);
		}

		return res;
	}


	void testRead() {

		inputStreamStringAdapter data("DATA");
		prefixedInputStream stream("PREFIX ", data);

		vmime::byte_t buffer[100];
		std::fill(vmime::begin(buffer), vmime::end(buffer), 0);

		// Prefix is returned first, without reading from the underlying stream
		VASSERT_EQ("Read 1", 7, stream.read(buffer, sizeof(buffer)));
		VASSERT_EQ("Buffer 1", "PREFIX ", vmime::utility::stringUtils::makeStringFromBytes(buffer, 7));
		VASSERT_FALSE("EOF 1", stream.eof());

		VASSERT_EQ("Read 2", 4, stream.read(buffer, sizeof(buffer)));
		VASSERT_EQ("Buffer 2", "DATA", vmime::utility::stringUtils::makeStringFromBytes(buffer, 4));
		VASSERT_TRUE("EOF 2", stream.eof());
	}

	void testReadPartialPrefix() {

		inputStreamStringAdapter data("DATA");
		prefixedInputStream stream("PREFIX ", data);

		vmime::byte_t buffer[100];
		std::fill(vmime::begin(buffer), vmime::end(buffer), 0);

		VASSERT_EQ("Read 1", 3, stream.read(buffer, 3));
		VASSERT_EQ("Buffer 1", "PRE", vmime::utility::stringUtils::makeStringFromBytes(buffer, 3));

		VASSERT_EQ("Read 2", 4, stream.read(buffer, sizeof(buffer)));
		VASSERT_EQ("Buffer 2", "FIX ", vmime::utility::stringUtils::makeStringFromBytes(buffer, 4));

		VASSERT_EQ("Data", "DATA", readAll(stream));
	}

	void testReadEmptyPrefix() {

		inputStreamStringAdapter data("DATA");
		prefixedInputStream stream("", data);

		VASSERT_EQ("Data", "DATA", readAll(stream));
		VASSERT_TRUE("EOF", stream.eof());
	}

	void testReadEmptyStream() {

		inputStreamStringAdapter data("");
		prefixedInputStream stream("PREFIX", data);

		VASSERT_FALSE("EOF", stream.eof());
		VASSERT_EQ("Data", "PREFIX", readAll(stream));
		VASSERT_TRUE("EOF", stream.eof());
	}

	void testEOF() {

		inputStreamStringAdapter data("DATA");
		prefixedInputStream stream("PREFIX", data);

		// Underlying stream is at end, but the prefix has not been read yet
		data.skip(4);

		VASSERT_TRUE("Underlying stream EOF", data.eof());
		VASSERT_FALSE("EOF 1", stream.eof());

		vmime::byte_t buffer[100];

		VASSERT_EQ("Read", 6, stream.read(buffer, sizeof(buffer)));
		VASSERT_TRUE("EOF 2", stream.eof());
	}

	void testSkip() {

		inputStreamStringAdapter data("DATA");
		prefixedInputStream stream("PREFIX ", data);

		// Skip in prefix
		VASSERT_EQ("Skip 1", 3, stream.skip(3));

		// Skip the rest of the prefix only
		VASSERT_EQ("Skip 2", 4, stream.skip(100));

		// Skip in the underlying stream
		VASSERT_EQ("Skip 3", 2, stream.skip(2));

		VASSERT_EQ("Data", "TA", readAll(stream));
	}

	void testReset() {

		inputStreamStringAdapter data("DATA");
		prefixedInputStream stream("PREFIX ", data);

		VASSERT_EQ("Data 1", "PREFIX DATA", readAll(stream));

		stream.reset();

		VASSERT_FALSE("EOF", stream.eof());
		VASSERT_EQ("Data 2", "PREFIX DATA", readAll(stream));
	}

	void testUnderlyingStreamError() {

		failingInputStream data("");
		prefixedInputStream stream("PREFIX", data);

		vmime::byte_t buffer[100];
		std::fill(vmime::begin(buffer), vmime::end(buffer), 0);

		// Prefix data is not lost if the underlying stream fails
		VASSERT_EQ("Read", 6, stream.read(buffer, sizeof(buffer)));
		VASSERT_EQ("Buffer", "PREFIX", vmime::utility::stringUtils::makeStringFromBytes(buffer, 6));

		VASSERT_THROW("Error", stream.read(buffer, sizeof(buffer)), vmime::exception);
	}

VMIME_TEST_SUITE_END
