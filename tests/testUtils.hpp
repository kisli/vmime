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

#include <ostream>
#include <iostream>
#include <sstream>
#include <vector>


// VMime
#include "vmime/vmime.hpp"


// CppUnit
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#define VASSERT(msg, cond) \
	CPPUNIT_ASSERT_MESSAGE(std::string(msg), cond)

#define VASSERT_TRUE(msg, cond) \
	VASSERT(msg, cond)
#define VASSERT_FALSE(msg, cond) \
	VASSERT(!(msg, cond))

#define VASSERT_EQ(msg, expected, actual) \
	CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string(msg), expected, actual)

#define VASSERT_THROW(msg, expression, exceptionType) \
	CPPUNIT_ASSERT_THROW(expression, exceptionType)
#define VASSERT_NO_THROW(msg, expression) \
	CPPUNIT_ASSERT_NO_THROW(expression)

#define VMIME_TEST_SUITE_BEGIN \
	class VMIME_TEST_SUITE : public CppUnit::TestFixture { public:
#define VMIME_TEST_SUITE_END \
	}; \
	\
	static CppUnit::AutoRegisterSuite <VMIME_TEST_SUITE>(autoRegisterRegistry1); \
	static CppUnit::AutoRegisterSuite <VMIME_TEST_SUITE>(autoRegisterRegistry2)(VMIME_TEST_SUITE_MODULE);

#define VMIME_TEST_LIST_BEGIN       CPPUNIT_TEST_SUITE(VMIME_TEST_SUITE);
#define VMIME_TEST_LIST_END         CPPUNIT_TEST_SUITE_END();
#define VMIME_TEST(name)            CPPUNIT_TEST(name);


namespace CppUnit
{
	// Work-around for comparing 'std::string' against 'char*'
	inline void assertEquals(const char* expected, const std::string actual,
	                         SourceLine sourceLine, const std::string &message)
	{
		assertEquals(std::string(expected), actual, sourceLine, message);
	}

	template <typename X, typename Y>
	void assertEquals(const X expected, const Y actual,
	                  SourceLine sourceLine, const std::string &message)
	{
		assertEquals(static_cast <Y>(expected), actual, sourceLine, message);
	}
}


namespace std
{


inline std::ostream& operator<<(std::ostream& os, const vmime::charset& ch)
{
	os << "[charset: " << ch.getName() << "]";
	return (os);
}


inline std::ostream& operator<<(std::ostream& os, const vmime::text& txt)
{
	os << "[text: [";

	for (int i = 0 ; i < txt.getWordCount() ; ++i)
	{
		const vmime::word& w = *txt.getWordAt(i);

		if (i != 0)
			os << ",";

		os << "[word: charset=" << w.getCharset().getName() << ", buffer=" << w.getBuffer() << "]";
	}

	os << "]]";

	return (os);
}


inline std::ostream& operator<<(std::ostream& os, const vmime::mailbox& mbox)
{
	os << "[mailbox: name=" << mbox.getName() << ", email=" << mbox.getEmail() << "]";

	return (os);
}


inline std::ostream& operator<<(std::ostream& os, const vmime::mailboxGroup& group)
{
	os << "[mailbox-group: name=" << group.getName() << ", list=[";

	for (int i = 0 ; i < group.getMailboxCount() ; ++i)
	{
		if (i != 0)
			os << ",";

		os << *group.getMailboxAt(i);
	}

	os << "]]";

	return (os);
}


inline std::ostream& operator<<(std::ostream& os, const vmime::addressList& list)
{
	os << "[address-list: [";

	for (int i = 0 ; i < list.getAddressCount() ; ++i)
	{
		const vmime::address& addr = *list.getAddressAt(i);

		if (i != 0)
			os << ",";

		if (addr.isGroup())
		{
			const vmime::mailboxGroup& group =
				dynamic_cast <const vmime::mailboxGroup&>(addr);

			os << group;
		}
		else
		{
			const vmime::mailbox& mbox =
				dynamic_cast <const vmime::mailbox&>(addr);

			os << mbox;
		}
	}

	os << "]]";

	return (os);
}


inline std::ostream& operator<<(std::ostream& os, const vmime::datetime& d)
{
	os << "[datetime: " << d.getYear() << "/" << d.getMonth() << "/" << d.getDay();
	os << " " << d.getHour() << ":" << d.getMinute() << ":" << d.getSecond();
	os << " #" << d.getZone() << "]";

	return (os);
}


}

