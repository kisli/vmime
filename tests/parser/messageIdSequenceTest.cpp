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
	class messageIdSequenceTest : public suite
	{
		void testParse()
		{
			vmime::messageIdSequence s1;
			s1.parse("");

			assert_eq("1", 0, s1.getMessageIdCount());

			vmime::messageIdSequence s2;
			s2.parse("   \t  ");

			assert_eq("2", 0, s2.getMessageIdCount());

			vmime::messageIdSequence s3;
			s3.parse("<a@b>");

			assert_eq("3.1", 1, s3.getMessageIdCount());
			assert_eq("3.2", "a", s3.getMessageIdAt(0)->getLeft());
			assert_eq("3.3", "b", s3.getMessageIdAt(0)->getRight());

			vmime::messageIdSequence s4;
			s4.parse("<a@b>  \r\n\t<c@d>");

			assert_eq("4.1", 2, s4.getMessageIdCount());
			assert_eq("4.2", "a", s4.getMessageIdAt(0)->getLeft());
			assert_eq("4.3", "b", s4.getMessageIdAt(0)->getRight());
			assert_eq("4.4", "c", s4.getMessageIdAt(1)->getLeft());
			assert_eq("4.5", "d", s4.getMessageIdAt(1)->getRight());
		}

		void testGenerate()
		{
			vmime::messageIdSequence s1;
			s1.appendMessageId(new vmime::messageId("a", "b"));

			assert_eq("1", "<a@b>", s1.generate());

			vmime::messageIdSequence s2;
			s2.appendMessageId(new vmime::messageId("a", "b"));
			s2.appendMessageId(new vmime::messageId("c", "d"));

			assert_eq("2", "<a@b> <c@d>", s2.generate());
		}

	public:

		messageIdSequenceTest() : suite("vmime::messageIdSequence")
		{
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Parse", testcase(this, "Parse", &messageIdSequenceTest::testParse));
			add("Generate", testcase(this, "Generate", &messageIdSequenceTest::testGenerate));

			suite::main().add("vmime::messageIdSequence", this);
		}

	};

	messageIdSequenceTest* theTest = new messageIdSequenceTest();
}
