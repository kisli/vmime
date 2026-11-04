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

#include "vmime/utility/details/classicIStringStream.hpp"


VMIME_TEST_SUITE_BEGIN(classicIStringStreamTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testClassicLocale)
	VMIME_TEST_LIST_END


	void testClassicLocale() {

		digitGroupingLocale loc;

		// A standard string stream uses the global locale (digit grouping)...
		std::istringstream iss("12 288");

		int value = 0;
		iss >> value;

		VASSERT_EQ("Global", 12288, value);

		// ...but not this one
		vmime::utility::details::classicIStringStream classicIss("12 288");

		int classicValue = 0;
		classicIss >> classicValue;

		VASSERT_EQ("Classic", 12, classicValue);
	}

VMIME_TEST_SUITE_END
