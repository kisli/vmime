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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/net/tls/TLSSocket.hpp"


namespace vmime {
namespace net {
namespace tls {


// static
bool TLSSocket::isIPAddress(const string& host) {

	// A host name cannot contain ':', so this is an IPv6 address
	if (host.find(':') != string::npos) {
		return true;
	}

	// Check for an IPv4 address in dotted-decimal notation
	int parts = 0;
	string::size_type pos = 0;

	while (true) {

		unsigned int value = 0;
		int digits = 0;

		for ( ; pos < host.length() && host[pos] >= '0' && host[pos] <= '9' ; ++pos, ++digits) {
			value = value * 10 + static_cast <unsigned int>(host[pos] - '0');
		}

		if (digits == 0 || digits > 3 || value > 255) {
			return false;
		}

		if (++parts == 4 || pos == host.length() || host[pos] != '.') {
			break;
		}

		++pos;  // skip '.'
	}

	return parts == 4 && pos == host.length();
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT

