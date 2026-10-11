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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/net/tls/TLSSocket.hpp"


using vmime::net::tls::TLSSocket;


VMIME_TEST_SUITE_BEGIN(TLSSocketTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testIsIPAddress_IPv4)
		VMIME_TEST(testIsIPAddress_IPv6)
		VMIME_TEST(testIsIPAddress_HostName)
		VMIME_TEST(testIsIPAddress_Invalid)
	VMIME_TEST_LIST_END


	void testIsIPAddress_IPv4() {

		VASSERT_TRUE("1", TLSSocket::isIPAddress("192.0.2.1"));
		VASSERT_TRUE("2", TLSSocket::isIPAddress("0.0.0.0"));
		VASSERT_TRUE("3", TLSSocket::isIPAddress("255.255.255.255"));
	}

	void testIsIPAddress_IPv6() {

		VASSERT_TRUE("1", TLSSocket::isIPAddress("2001:db8::1"));
		VASSERT_TRUE("2", TLSSocket::isIPAddress("::1"));
		VASSERT_TRUE("3", TLSSocket::isIPAddress("::ffff:192.0.2.1"));
	}

	void testIsIPAddress_HostName() {

		VASSERT_FALSE("1", TLSSocket::isIPAddress("mail.vmime.org"));
		VASSERT_FALSE("2", TLSSocket::isIPAddress("localhost"));
		VASSERT_FALSE("3", TLSSocket::isIPAddress("192.vmime.org"));
		VASSERT_FALSE("4", TLSSocket::isIPAddress("192.0.2.1.vmime.org"));
	}

	void testIsIPAddress_Invalid() {

		VASSERT_FALSE("1", TLSSocket::isIPAddress(""));
		VASSERT_FALSE("2", TLSSocket::isIPAddress("192.0.2"));
		VASSERT_FALSE("3", TLSSocket::isIPAddress("192.0.2.1.5"));
		VASSERT_FALSE("4", TLSSocket::isIPAddress("192.0.2.1."));
		VASSERT_FALSE("5", TLSSocket::isIPAddress("192..2.1"));
		VASSERT_FALSE("6", TLSSocket::isIPAddress("192.0.2.256"));
		VASSERT_FALSE("7", TLSSocket::isIPAddress("192.0.2.0001"));
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT
