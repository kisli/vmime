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

#ifndef VMIME_SECURITY_SASL_SASLSOCKET_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLSOCKET_HPP_INCLUDED


#include "vmime/types.hpp"

#include "vmime/net/socket.hpp"


namespace vmime {
namespace security {
namespace sasl {


class SASLSession;


/** A socket which provides data integrity and/or privacy protection.
  */
class SASLSocket : public net::socket
{
public:

	SASLSocket(ref <SASLSession> sess, ref <net::socket> wrapped);
	~SASLSocket();

	void connect(const string& address, const port_t port);
	void disconnect();

	bool isConnected() const;

	void receive(string& buffer);
	size_type receiveRaw(char* buffer, const size_type count);

	void send(const string& buffer);
	void sendRaw(const char* buffer, const size_type count);

	size_type getBlockSize() const;

private:

	ref <SASLSession> m_session;
	ref <net::socket> m_wrapped;

	byte_t* m_pendingBuffer;
	int m_pendingPos;
	int m_pendingLen;

	char m_recvBuffer[65536];
};


} // sasl
} // security
} // vmime


#endif // VMIME_SECURITY_SASL_SASLSOCKET_HPP_INCLUDED

