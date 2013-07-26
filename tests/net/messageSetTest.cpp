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

#include "vmime/net/messageSet.hpp"


VMIME_TEST_SUITE_BEGIN(messageSetTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testNumberSet_Single)
		VMIME_TEST(testNumberSet_Range)
		VMIME_TEST(testNumberSet_InfiniteRange)
		VMIME_TEST(testNumberSet_Multiple)
		VMIME_TEST(testUIDSet_Single)
		VMIME_TEST(testUIDSet_Range)
		VMIME_TEST(testUIDSet_InfiniteRange)
		VMIME_TEST(testUIDSet_MultipleNumeric)
		VMIME_TEST(testUIDSet_MultipleNonNumeric)
		VMIME_TEST(testIsNumberSet)
		VMIME_TEST(testIsUIDSet)
	VMIME_TEST_LIST_END


	class messageSetStringEnumerator : public vmime::net::messageSetEnumerator
	{
	public:

		messageSetStringEnumerator()
			: m_first(true)
		{
		}

		void enumerateNumberMessageRange(const vmime::net::numberMessageRange& range)
		{
			if (!m_first)
				m_oss << ",";

			if (range.getFirst() == range.getLast())
				m_oss << range.getFirst();
			else
				m_oss << range.getFirst() << ":" << range.getLast();

			m_first = false;
		}

		void enumerateUIDMessageRange(const vmime::net::UIDMessageRange& range)
		{
			if (!m_first)
				m_oss << ",";

			if (range.getFirst() == range.getLast())
				m_oss << range.getFirst();
			else
				m_oss << range.getFirst() << ":" << range.getLast();

			m_first = false;
		}

		const std::string str() const
		{
			return m_oss.str();
		}

	private:

		std::ostringstream m_oss;
		bool m_first;
	};


	const std::string enumerateAsString(const vmime::net::messageSet& set)
	{
		messageSetStringEnumerator en;
		set.enumerate(en);

		return en.str();
	}


	void testNumberSet_Single()
	{
		VASSERT_EQ("str", "42", enumerateAsString(vmime::net::messageSet::byNumber(42)));
	}

	void testNumberSet_Range()
	{
		VASSERT_EQ("str", "42:100", enumerateAsString(vmime::net::messageSet::byNumber(42, 100)));
	}

	void testNumberSet_InfiniteRange()
	{
		VASSERT_EQ("str", "42:-1", enumerateAsString(vmime::net::messageSet::byNumber(42, -1)));
	}

	void testNumberSet_Multiple()
	{
		std::vector <int> numbers;
		numbers.push_back(1);    // test grouping 1:3
		numbers.push_back(89);   // test sorting
		numbers.push_back(2);
		numbers.push_back(3);
		numbers.push_back(42);
		numbers.push_back(53);   // test grouping 53:57
		numbers.push_back(54);
		numbers.push_back(55);
		numbers.push_back(56);
		numbers.push_back(56);   // test duplicates
		numbers.push_back(57);
		numbers.push_back(99);

		VASSERT_EQ("str", "1:3,42,53:57,89,99", enumerateAsString(vmime::net::messageSet::byNumber(numbers)));
	}


	void testUIDSet_Single()
	{
		VASSERT_EQ("str", "abcdef", enumerateAsString(vmime::net::messageSet::byUID("abcdef")));
	}

	void testUIDSet_Range()
	{
		VASSERT_EQ("str", "abc:def", enumerateAsString(vmime::net::messageSet::byUID("abc:def")));
	}

	void testUIDSet_InfiniteRange()
	{
		VASSERT_EQ("str", "abc:*", enumerateAsString(vmime::net::messageSet::byUID("abc", "*")));
	}

	void testUIDSet_MultipleNumeric()
	{
		std::vector <vmime::net::message::uid> uids;
		uids.push_back("1");    // test grouping 1:3
		uids.push_back("89");   // test sorting
		uids.push_back("2");
		uids.push_back("3");
		uids.push_back("42");
		uids.push_back("53");   // test grouping 53:57
		uids.push_back("54");
		uids.push_back("55");
		uids.push_back("56");
		uids.push_back("56");   // test duplicates
		uids.push_back("57");
		uids.push_back("99");

		VASSERT_EQ("str", "1:3,42,53:57,89,99", enumerateAsString(vmime::net::messageSet::byUID(uids)));
	}

	void testUIDSet_MultipleNonNumeric()
	{
		std::vector <vmime::net::message::uid> uids;
		uids.push_back("12");
		uids.push_back("34");
		uids.push_back("ab56");
		uids.push_back("78cd");

		VASSERT_EQ("str", "12,34,ab56,78cd", enumerateAsString(vmime::net::messageSet::byUID(uids)));
	}

	void testIsNumberSet()
	{
		VASSERT_TRUE("number1", vmime::net::messageSet::byNumber(42).isNumberSet());
		VASSERT_FALSE("uid1", vmime::net::messageSet::byUID("42").isNumberSet());

		VASSERT_TRUE("number2", vmime::net::messageSet::byNumber(42, -1).isNumberSet());
		VASSERT_FALSE("uid2", vmime::net::messageSet::byUID("42", "*").isNumberSet());
	}

	void testIsUIDSet()
	{
		VASSERT_FALSE("number1", vmime::net::messageSet::byNumber(42).isUIDSet());
		VASSERT_TRUE("uid1", vmime::net::messageSet::byUID("42").isUIDSet());

		VASSERT_FALSE("number2", vmime::net::messageSet::byNumber(42, -1).isUIDSet());
		VASSERT_TRUE("uid2", vmime::net::messageSet::byUID("42", "*").isUIDSet());
	}

VMIME_TEST_SUITE_END
