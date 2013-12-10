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

#include "vmime/utility/outputStreamStringAdapter.hpp"


VMIME_TEST_SUITE_BEGIN(outputStreamStringAdapterTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testWrite)
		VMIME_TEST(testWriteBinary)
		VMIME_TEST(testWriteCRLF)
	VMIME_TEST_LIST_END


	void testWrite()
	{
		vmime::string str("initial data");

		vmime::utility::outputStreamStringAdapter stream(str);
		stream << "additional data";
		stream.flush();

		VASSERT_EQ("Write 1 len", 27, str.length());
		VASSERT_EQ("Write 1 data", "initial dataadditional data", str);

		stream.write("more data");

		VASSERT_EQ("Write 2 len", 36, str.length());
		VASSERT_EQ("Write 2 data", "initial dataadditional datamore data", str);
	}

	void testWriteBinary()
	{
		const vmime::byte_t binaryData[] =
			"\xc5\x9a\xc3\xb8\xc9\xb1\xc9\x9b\x20\xc9\x93\xc9\xa8\xc9\xb2\xc9"
			"\x91\xc5\x95\xc9\xa3\x20\xc9\x96\xc9\x90\xca\x88\xc9\x92";

		vmime::string str;

		vmime::utility::outputStreamStringAdapter stream(str);
		stream.write(binaryData, sizeof(binaryData));
		stream.flush();

		VASSERT_EQ("Write", 0, memcmp(binaryData, str.data(), sizeof(binaryData)));
	}

	void testWriteCRLF()
	{
		vmime::string str;

		vmime::utility::outputStreamStringAdapter stream(str);
		stream << "some data";
		stream.flush();

		stream << "\nmore\r\ndata\r";
		stream.flush();

		VASSERT_EQ("Write", "some data\nmore\r\ndata\r", str);
	}

VMIME_TEST_SUITE_END

