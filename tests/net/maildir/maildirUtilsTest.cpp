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

#include "vmime/net/maildir/maildirUtils.hpp"


using namespace vmime::net::maildir;


VMIME_TEST_SUITE_BEGIN(maildirUtilsTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testMessageSetToNumberList)
	VMIME_TEST_LIST_END


	void testMessageSetToNumberList()
	{
		const std::vector <size_t> msgNums =
			maildirUtils::messageSetToNumberList
				(vmime::net::messageSet::byNumber(5, -1), /* msgCount */ 8);

		VASSERT_EQ("Count", 4, msgNums.size());
		VASSERT_EQ("1", 5, msgNums[0]);
		VASSERT_EQ("2", 6, msgNums[1]);
		VASSERT_EQ("3", 7, msgNums[2]);
		VASSERT_EQ("4", 8, msgNums[3]);
	}

VMIME_TEST_SUITE_END

