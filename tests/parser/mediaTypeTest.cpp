//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "../../src/vmime"
#include "../../src/platforms/posix/handler.hpp"

using namespace unitpp;


namespace
{
	class mediaTypeTest : public suite
	{
		void testConstructors()
		{
			vmime::mediaType t1;

			assert_eq("1.1", vmime::mediaTypes::APPLICATION, t1.getType());
			assert_eq("1.2", vmime::mediaTypes::APPLICATION_OCTET_STREAM, t1.getSubType());

			vmime::mediaType t2("type", "sub");

			assert_eq("2.1", "type", t2.getType());
			assert_eq("2.2", "sub", t2.getSubType());

			vmime::mediaType t3("type/sub");

			assert_eq("3.1", "type", t3.getType());
			assert_eq("3.2", "sub", t3.getSubType());
		}

		void testCopy()
		{
			vmime::mediaType t1("type/sub");

			assert_eq("eq1", "type", t1.getType());
			assert_eq("eq2", "sub", t1.getSubType());

			assert_true("operator==", t1 == t1);
			assert_true("clone", *(t1.clone()) == t1);

			assert_eq("eq3", "type", t1.clone()->getType());
			assert_eq("eq4", "sub", t1.clone()->getSubType());

			vmime::mediaType t2;
			t2.copyFrom(t1);

			assert_true("copyFrom", t1 == t2);
		}

		void testSetFromString()
		{
			vmime::mediaType t1;
			t1.setFromString("type/sub");

			assert_eq("1.1", "type", t1.getType());
			assert_eq("1.2", "sub", t1.getSubType());
		}

		void testParse()
		{
			vmime::mediaType t1;
			t1.parse("type/sub");

			assert_eq("1.1", "type", t1.getType());
			assert_eq("1.2", "sub", t1.getSubType());
		}

		void testGenerate()
		{
			vmime::mediaType t1("type", "sub");

			assert_eq("1", "type/sub", t1.generate());
		}

	public:

		mediaTypeTest() : suite("vmime::mediaType")
		{
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Constructors", testcase(this, "Constructors", &mediaTypeTest::testConstructors));
			add("Copy", testcase(this, "Copy", &mediaTypeTest::testCopy));
			add("SetFromString", testcase(this, "SetFromString", &mediaTypeTest::testSetFromString));
			add("Parse", testcase(this, "Parse", &mediaTypeTest::testParse));
			add("Generate", testcase(this, "Generate", &mediaTypeTest::testGenerate));

			suite::main().add("vmime::mediaType", this);
		}

	};

	mediaTypeTest* theTest = new mediaTypeTest();
}
