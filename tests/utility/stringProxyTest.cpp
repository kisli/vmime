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

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

using namespace unitpp;


namespace
{
	class stringProxyTest : public suite
	{
		void testConstruct()
		{
			vmime::utility::stringProxy s;

			assert_eq("1", 0, s.length());
			assert_eq("2", 0, s.start());
			assert_eq("3", 0, s.end());
		}

		void testConstruct2()
		{
			vmime::string str("This is a test string.");

			vmime::utility::stringProxy s1(str);

			assert_eq("1", str.length(), s1.length());
			assert_eq("2", 0, s1.start());
			assert_eq("3", str.length(), s1.end());

			vmime::utility::stringProxy s2(str, 10);

			assert_eq("4", str.length() - 10, s2.length());
			assert_eq("5", 10, s2.start());
			assert_eq("6", str.length(), s2.end());

			vmime::utility::stringProxy s3(str, 10, 14);

			assert_eq("7", 4, s3.length());
			assert_eq("8", 10, s3.start());
			assert_eq("9", 14, s3.end());

			assert_eq("10", 't', *s3.it_begin());
			assert_eq("11", 'e', *(s3.it_begin() + 1));
			assert_eq("12", 's', *(s3.it_begin() + 2));
			assert_eq("13", 't', *(s3.it_begin() + 3));
		}

		void testDetach()
		{
			vmime::utility::stringProxy s;
			s = "foo";

			s.detach();

			assert_eq("1", 0, s.length());
			assert_eq("2", 0, s.start());
			assert_eq("3", 0, s.end());
		}

		void testSet()
		{
			vmime::string str("This is a test string.");

			vmime::utility::stringProxy s1;
			s1.set(str);

			assert_eq("1", str.length(), s1.length());
			assert_eq("2", 0, s1.start());
			assert_eq("3", str.length(), s1.end());

			vmime::utility::stringProxy s2;
			s2.set(str, 10);

			assert_eq("4", str.length() - 10, s2.length());
			assert_eq("5", 10, s2.start());
			assert_eq("6", str.length(), s2.end());

			vmime::utility::stringProxy s3;
			s3.set(str, 10, 14);

			assert_eq("7", 4, s3.length());
			assert_eq("8", 10, s3.start());
			assert_eq("9", 14, s3.end());

			assert_eq("10", 't', *s3.it_begin());
			assert_eq("11", 'e', *(s3.it_begin() + 1));
			assert_eq("12", 's', *(s3.it_begin() + 2));
			assert_eq("13", 't', *(s3.it_begin() + 3));
		}

		void testExtract()
		{
			vmime::string str("This is a test string.");

			vmime::utility::stringProxy s1(str, 10, 14);

			std::ostringstream oss1;
			vmime::utility::outputStreamAdapter osa1(oss1);

			s1.extract(osa1);

			assert_eq("1", "test", oss1.str());

			vmime::utility::stringProxy s2(str);

			std::ostringstream oss2;
			vmime::utility::outputStreamAdapter osa2(oss2);

			s2.extract(osa2);

			assert_eq("2", str, oss2.str());
		}

		void testOperatorLTLT1()
		{
			vmime::string str("This is a test string.");

			vmime::utility::stringProxy s1(str, 10, 14);

			std::ostringstream oss1;
			oss1 << s1;

			assert_eq("1", "test", oss1.str());

			vmime::utility::stringProxy s2(str);

			std::ostringstream oss2;
			oss2 << s2;

			assert_eq("2", str, oss2.str());
		}

		void testOperatorLTLT2()
		{
			vmime::string str("This is a test string.");

			vmime::utility::stringProxy s1(str, 10, 14);

			std::ostringstream oss1;
			vmime::utility::outputStreamAdapter osa1(oss1);

			osa1 << s1;

			assert_eq("1", "test", oss1.str());

			vmime::utility::stringProxy s2(str);

			std::ostringstream oss2;
			vmime::utility::outputStreamAdapter osa2(oss2);

			osa2 << s2;

			assert_eq("2", str, oss2.str());
		}

	public:

		stringProxyTest() : suite("vmime::utility::stringProxy")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("Construct", testcase(this, "Construct", &stringProxyTest::testConstruct));
			add("Construct2", testcase(this, "Construct2", &stringProxyTest::testConstruct2));

			add("Detach", testcase(this, "Detach", &stringProxyTest::testDetach));

			add("Set", testcase(this, "Set", &stringProxyTest::testSet));

			add("Extract", testcase(this, "Extract", &stringProxyTest::testExtract));

			add("Operator<<(1)", testcase(this, "Operator<<(1)", &stringProxyTest::testOperatorLTLT1));
			add("Operator<<(2)", testcase(this, "Operator<<(2)", &stringProxyTest::testOperatorLTLT2));

			suite::main().add("vmime::utility::stringProxy", this);
		}

	};

	stringProxyTest* theTest = new stringProxyTest();
}
