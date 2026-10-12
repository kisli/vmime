//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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


#if VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_MESSAGING_FEATURES


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>


// Socket listening on an ephemeral port of a loopback address.
// Connections are completed by the kernel and never accepted.
class loopbackServer {

public:

	explicit loopbackServer(const char* address)
		: m_address(address),
		  m_desc(-1),
		  m_port(0),
		  m_addrLen(sizeof(m_addr)) {

		struct addrinfo hint, *info = NULL;
		memset(&hint, 0, sizeof(hint));

		hint.ai_family = AF_UNSPEC;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_flags = AI_NUMERICHOST | AI_PASSIVE;

		if (getaddrinfo(address, "0", &hint, &info) != 0) {
			return;
		}

		m_desc = ::socket(info->ai_family, info->ai_socktype, info->ai_protocol);

		if (m_desc != -1 &&
		    (::bind(m_desc, info->ai_addr, info->ai_addrlen) != 0 ||
		     ::listen(m_desc, 1) != 0 ||
		     ::getsockname(m_desc, reinterpret_cast <sockaddr*>(&m_addr), &m_addrLen) != 0)) {

			::close(m_desc);
			m_desc = -1;
		}

		freeaddrinfo(info);

		if (m_desc == -1) {
			return;
		}

		if (m_addr.ss_family == AF_INET6) {
			m_port = ntohs(reinterpret_cast <sockaddr_in6*>(&m_addr)->sin6_port);
		} else {
			m_port = ntohs(reinterpret_cast <sockaddr_in*>(&m_addr)->sin_port);
		}
	}

	~loopbackServer() {

		if (m_desc != -1) {
			::close(m_desc);
		}
	}

	bool isListening() const {

		return m_desc != -1;
	}

	const vmime::string& getAddress() const {

		return m_address;
	}

	vmime::port_t getPort() const {

		return m_port;
	}

	// Host name the resolver gives for the listening address, or the
	// address itself if it has none
	const vmime::string getHostName() const {

		char host[NI_MAXHOST];

		if (getnameinfo(reinterpret_cast <const sockaddr*>(&m_addr), m_addrLen,
				host, sizeof(host), NULL, 0, NI_NAMEREQD) == 0) {

			return host;
		}

		return m_address;
	}

private:

	const vmime::string m_address;
	int m_desc;
	vmime::port_t m_port;
	sockaddr_storage m_addr;
	socklen_t m_addrLen;
};



VMIME_TEST_SUITE_BEGIN(posixSocketTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testGetPeerAddressIPv4)
		VMIME_TEST(testGetPeerAddressIPv6)
		VMIME_TEST(testGetPeerNameIPv4)
		VMIME_TEST(testGetPeerNameIPv6)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::net::socket> connectTo(const loopbackServer& server) {

		vmime::shared_ptr <vmime::net::socket> sock =
			vmime::platform::getHandler()->getSocketFactory()->create();

		sock->connect(server.getAddress(), server.getPort());

		return sock;
	}

	void testGetPeerAddressIPv4() {

		loopbackServer server("127.0.0.1");
		VASSERT_TRUE("Listen", server.isListening());

		VASSERT_EQ("Address", "127.0.0.1", connectTo(server)->getPeerAddress());
	}

	void testGetPeerAddressIPv6() {

		loopbackServer server("::1");

		if (!server.isListening()) {
			return;  // IPv6 not available on this host
		}

		VASSERT_EQ("Address", "::1", connectTo(server)->getPeerAddress());
	}

	void testGetPeerNameIPv4() {

		loopbackServer server("127.0.0.1");
		VASSERT_TRUE("Listen", server.isListening());

		VASSERT_EQ("Name", server.getHostName(), connectTo(server)->getPeerName());
	}

	void testGetPeerNameIPv6() {

		loopbackServer server("::1");

		if (!server.isListening()) {
			return;  // IPv6 not available on this host
		}

		VASSERT_EQ("Name", server.getHostName(), connectTo(server)->getPeerName());
	}

VMIME_TEST_SUITE_END


#endif // VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_MESSAGING_FEATURES
