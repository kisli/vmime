//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include <ostream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>


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
	static CppUnit::AutoRegisterSuite <VMIME_TEST_SUITE>(autoRegisterRegistry2)(VMIME_TEST_SUITE_MODULE); \
	extern void registerTestModule(const char* name); \
	template <typename T> \
	struct AutoRegisterModule { \
		AutoRegisterModule() { \
			registerTestModule(VMIME_TEST_SUITE_MODULE); \
		} \
	}; \
	static AutoRegisterModule <VMIME_TEST_SUITE> autoRegisterModule;

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



// Used to test network features.
//
// This works like a local pipe: client reads and writes data using receive()
// and send(). Server reads incoming data with localReceive() and sends data
// to client with localSend().

class testSocket : public vmime::net::socket
{
public:

	void connect(const vmime::string& address, const vmime::port_t port);
	void disconnect();

	bool isConnected() const;

	void receive(vmime::string& buffer);
	void send(const vmime::string& buffer);

	int receiveRaw(char* buffer, const int count);
	void sendRaw(const char* buffer, const int count);

	size_type getBlockSize() const;

	/** Send data to client.
	  *
	  * @buffer data to send
	  */
	void localSend(const vmime::string& buffer);

	/** Receive data from client.
	  *
	  * @buffer buffer in which to store received data
	  */
	void localReceive(vmime::string& buffer);

protected:

	/** Called when the client has sent some data.
	  */
	virtual void onDataReceived();

	/** Called when the client has connected.
	  */
	virtual void onConnected();

private:

	vmime::string m_address;
	vmime::port_t m_port;
	bool m_connected;

	vmime::string m_inBuffer;
	vmime::string m_outBuffer;
};


template <typename T>
class testSocketFactory : public vmime::net::socketFactory
{
public:

	vmime::ref <vmime::net::socket> create()
	{
		return vmime::create <T>();
	}

	vmime::ref <vmime::net::socket> create(vmime::ref <vmime::net::timeoutHandler> /* th */)
	{
		return vmime::create <T>();
	}
};


class lineBasedTestSocket : public testSocket
{
public:

	void onDataReceived();

	const vmime::string getNextLine();
	bool haveMoreLines() const;

	virtual void processCommand() = 0;

private:

	std::vector <vmime::string> m_lines;
	std::string m_buffer;
};


class testTimeoutHandler : public vmime::net::timeoutHandler
{
public:

	testTimeoutHandler(const unsigned int delay = 3);

	bool isTimeOut();
	void resetTimeOut();
	bool handleTimeOut();

private:

	unsigned int m_delay;
	unsigned int m_start;
};


class testTimeoutHandlerFactory : public vmime::net::timeoutHandlerFactory
{
public:

	vmime::ref <vmime::net::timeoutHandler> create();
};


// Exception helper
std::ostream& operator<<(std::ostream& os, const vmime::exception& e);

