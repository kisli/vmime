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

#include "vmime/utility/outputStreamStringAdapter.hpp"


VMIME_TEST_SUITE_BEGIN(outputStreamTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testWriteChar)
		VMIME_TEST(testWriteByte)
		VMIME_TEST(testWriteString)
		VMIME_TEST(testWriteNumber)
	VMIME_TEST_LIST_END


	void testWriteChar() {

		vmime::string str;
		vmime::utility::outputStreamStringAdapter os(str);

		os << 'a' << '\0' << '\xff';
		os.flush();

		VASSERT_EQ("Data", vmime::string("a\0\xff", 3), str);
	}

	void testWriteByte() {

		vmime::string str;
		vmime::utility::outputStreamStringAdapter os(str);

		os << static_cast <vmime::byte_t>(0x41) << static_cast <vmime::byte_t>(0xff);
		os.flush();

		VASSERT_EQ("Data", "A\xff", str);
	}

	void testWriteString() {

		vmime::string str;
		vmime::utility::outputStreamStringAdapter os(str);

		const char* cstr = "ghi";

		os << vmime::string("abc") << "def" << cstr;
		os.flush();

		VASSERT_EQ("Data", "abcdefghi", str);
	}

	void testWriteNumber() {

		// Numbers must be formatted without digit grouping, whatever the global locale
		digitGroupingLocale loc;

		vmime::string str;
		vmime::utility::outputStreamStringAdapter os(str);

		os << 12288 << ' ' << static_cast <vmime::size_t>(1048576);
		os.flush();

		VASSERT_EQ("Data", "12288 1048576", str);
	}

VMIME_TEST_SUITE_END
