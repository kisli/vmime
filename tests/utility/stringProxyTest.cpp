//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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


#define VMIME_TEST_SUITE         stringProxyTest
#define VMIME_TEST_SUITE_MODULE  "Utility"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testConstruct)
		VMIME_TEST(testConstruct2)

		VMIME_TEST(testDetach)

		VMIME_TEST(testSet)

		VMIME_TEST(testExtract)

		VMIME_TEST(testOperatorLTLT1)
		VMIME_TEST(testOperatorLTLT2)
	VMIME_TEST_LIST_END


	void testConstruct()
	{
		vmime::utility::stringProxy s;

		VASSERT_EQ("1", static_cast <vmime::utility::stringProxy::size_type>(0), s.length());
		VASSERT_EQ("2", static_cast <vmime::utility::stringProxy::size_type>(0), s.start());
		VASSERT_EQ("3", static_cast <vmime::utility::stringProxy::size_type>(0), s.end());
	}

	void testConstruct2()
	{
		vmime::string str("This is a test string.");

		vmime::utility::stringProxy s1(str);

		VASSERT_EQ("1", str.length(), s1.length());
		VASSERT_EQ("2", static_cast <vmime::utility::stringProxy::size_type>(0), s1.start());
		VASSERT_EQ("3", str.length(), s1.end());

		vmime::utility::stringProxy s2(str, 10);

		VASSERT_EQ("4", str.length() - 10, s2.length());
		VASSERT_EQ("5", static_cast <vmime::utility::stringProxy::size_type>(10), s2.start());
		VASSERT_EQ("6", str.length(), s2.end());

		vmime::utility::stringProxy s3(str, 10, 14);

		VASSERT_EQ("7", static_cast <vmime::utility::stringProxy::size_type>(4), s3.length());
		VASSERT_EQ("8", static_cast <vmime::utility::stringProxy::size_type>(10), s3.start());
		VASSERT_EQ("9", static_cast <vmime::utility::stringProxy::size_type>(14), s3.end());

		VASSERT_EQ("10", 't', *s3.it_begin());
		VASSERT_EQ("11", 'e', *(s3.it_begin() + 1));
		VASSERT_EQ("12", 's', *(s3.it_begin() + 2));
		VASSERT_EQ("13", 't', *(s3.it_begin() + 3));
	}

	void testDetach()
	{
		vmime::utility::stringProxy s;
		s = "foo";

		s.detach();

		VASSERT_EQ("1", static_cast <vmime::utility::stringProxy::size_type>(0), s.length());
		VASSERT_EQ("2", static_cast <vmime::utility::stringProxy::size_type>(0), s.start());
		VASSERT_EQ("3", static_cast <vmime::utility::stringProxy::size_type>(0), s.end());
	}

	void testSet()
	{
		vmime::string str("This is a test string.");

		vmime::utility::stringProxy s1;
		s1.set(str);

		VASSERT_EQ("1", str.length(), s1.length());
		VASSERT_EQ("2", static_cast <vmime::utility::stringProxy::size_type>(0), s1.start());
		VASSERT_EQ("3", str.length(), s1.end());

		vmime::utility::stringProxy s2;
		s2.set(str, 10);

		VASSERT_EQ("4", str.length() - 10, s2.length());
		VASSERT_EQ("5", static_cast <vmime::utility::stringProxy::size_type>(10), s2.start());
		VASSERT_EQ("6", str.length(), s2.end());

		vmime::utility::stringProxy s3;
		s3.set(str, 10, 14);

		VASSERT_EQ("7", static_cast <vmime::utility::stringProxy::size_type>(4), s3.length());
		VASSERT_EQ("8", static_cast <vmime::utility::stringProxy::size_type>(10), s3.start());
		VASSERT_EQ("9", static_cast <vmime::utility::stringProxy::size_type>(14), s3.end());

		VASSERT_EQ("10", 't', *s3.it_begin());
		VASSERT_EQ("11", 'e', *(s3.it_begin() + 1));
		VASSERT_EQ("12", 's', *(s3.it_begin() + 2));
		VASSERT_EQ("13", 't', *(s3.it_begin() + 3));
	}

	void testExtract()
	{
		vmime::string str("This is a test string.");

		vmime::utility::stringProxy s1(str, 10, 14);

		std::ostringstream oss1;
		vmime::utility::outputStreamAdapter osa1(oss1);

		s1.extract(osa1);

		VASSERT_EQ("1", "test", oss1.str());

		vmime::utility::stringProxy s2(str);

		std::ostringstream oss2;
		vmime::utility::outputStreamAdapter osa2(oss2);

		s2.extract(osa2);

		VASSERT_EQ("2", str, oss2.str());
	}

	void testOperatorLTLT1()
	{
		vmime::string str("This is a test string.");

		vmime::utility::stringProxy s1(str, 10, 14);

		std::ostringstream oss1;
		oss1 << s1;

		VASSERT_EQ("1", "test", oss1.str());

		vmime::utility::stringProxy s2(str);

		std::ostringstream oss2;
		oss2 << s2;

		VASSERT_EQ("2", str, oss2.str());
	}

	void testOperatorLTLT2()
	{
		vmime::string str("This is a test string.");

		vmime::utility::stringProxy s1(str, 10, 14);

		std::ostringstream oss1;
		vmime::utility::outputStreamAdapter osa1(oss1);

		osa1 << s1;

		VASSERT_EQ("1", "test", oss1.str());

		vmime::utility::stringProxy s2(str);

		std::ostringstream oss2;
		vmime::utility::outputStreamAdapter osa2(oss2);

		osa2 << s2;

		VASSERT_EQ("2", str, oss2.str());
	}

VMIME_TEST_SUITE_END

