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

#include "vmime/utility/parserInputStreamAdapter.hpp"


VMIME_TEST_SUITE_BEGIN(parserInputStreamAdapterTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testEndlessLoopBufferSize)
	VMIME_TEST_LIST_END


	void testEndlessLoopBufferSize()
	{
		static const unsigned int BUFFER_SIZE = 4096;  // same as in parserInputStreamAdapter::findNext()

		vmime::string str(BUFFER_SIZE, 'X');

		vmime::shared_ptr <vmime::utility::inputStreamStringAdapter> iss =
			vmime::make_shared <vmime::utility::inputStreamStringAdapter>(str);

		vmime::shared_ptr <vmime::utility::parserInputStreamAdapter> parser =
			vmime::make_shared <vmime::utility::parserInputStreamAdapter>(iss);

		VASSERT_EQ("Not found", vmime::string::npos, parser->findNext("token"));
	}

VMIME_TEST_SUITE_END
