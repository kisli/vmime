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
	class pathTest : public suite
	{
		void testParse()
		{
			vmime::path p1;
			p1.parse("<>");

			assert_eq("1.1", "", p1.getLocalPart());
			assert_eq("1.2", "", p1.getDomain());

			vmime::path p2;
			p2.parse("<domain>");

			assert_eq("2.1", "", p2.getLocalPart());
			assert_eq("2.2", "domain", p2.getDomain());

			vmime::path p3;
			p3.parse("<local@domain>");

			assert_eq("3.1", "local", p3.getLocalPart());
			assert_eq("3.2", "domain", p3.getDomain());
		}

		void testParse2()
		{
			// Test some invalid paths (no '<>')
			vmime::path p1;
			p1.parse("");

			assert_eq("1.1", "", p1.getLocalPart());
			assert_eq("1.2", "", p1.getDomain());

			vmime::path p2;
			p2.parse("domain");

			assert_eq("2.1", "", p2.getLocalPart());
			assert_eq("2.2", "domain", p2.getDomain());

			vmime::path p3;
			p3.parse("local@domain");

			assert_eq("3.1", "local", p3.getLocalPart());
			assert_eq("3.2", "domain", p3.getDomain());
		}

		void testGenerate()
		{
			vmime::path p1;

			assert_eq("1", "<>", p1.generate());

			vmime::path p2;
			p2.setLocalPart("local");

			assert_eq("2", "<local@>", p2.generate());

			vmime::path p3;
			p3.setDomain("domain");

			assert_eq("3", "<@domain>", p3.generate());

			vmime::path p4;
			p4.setLocalPart("local");
			p4.setDomain("domain");

			assert_eq("4", "<local@domain>", p4.generate());
		}

	public:

		pathTest() : suite("vmime::path")
		{
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse", testcase(this, "Parse", &pathTest::testParse));
			add("Parse2", testcase(this, "Parse2", &pathTest::testParse2));
			add("Generate", testcase(this, "Generate", &pathTest::testGenerate));

			suite::main().add("vmime::path", this);
		}

	};

	pathTest* theTest = new pathTest();
}
