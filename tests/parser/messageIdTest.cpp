//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "tests/parser/testUtils.hpp"

using namespace unitpp;


namespace
{
	class messageIdTest : public suite
	{
		void testParse()
		{
			vmime::messageId m1;
			m1.parse("<a@b>");

			assert_eq("1.1", "a", m1.getLeft());
			assert_eq("1.2", "b", m1.getRight());
		}

		void testGenerate()
		{
			vmime::messageId m1;

			assert_eq("1", "<@>", m1.generate());

			vmime::messageId m2;
			m2.setLeft("a");

			assert_eq("2", "<a@>", m2.generate());

			vmime::messageId m3;
			m3.setRight("b");

			assert_eq("3", "<@b>", m3.generate());

			vmime::messageId m4;
			m4.setLeft("a");
			m4.setRight("b");

			assert_eq("4", "<a@b>", m4.generate());
		}

	public:

		messageIdTest() : suite("vmime::messageId")
		{
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse", testcase(this, "Parse", &messageIdTest::testParse));
			add("Generate", testcase(this, "Generate", &messageIdTest::testGenerate));

			suite::main().add("vmime::messageId", this);
		}

	};

	messageIdTest* theTest = new messageIdTest();
}
