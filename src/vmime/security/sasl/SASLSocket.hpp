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

#ifndef VMIME_SECURITY_SASL_SASLSOCKET_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLSOCKET_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/types.hpp"

#include "vmime/net/socket.hpp"


namespace vmime {
namespace security {
namespace sasl {


class SASLSession;


/** A socket which provides data integrity and/or privacy protection.
  */
class VMIME_EXPORT SASLSocket : public net::socket
{
public:

	SASLSocket(shared_ptr <SASLSession> sess, shared_ptr <net::socket> wrapped);
	~SASLSocket();

	void connect(const string& address, const port_t port);
	void disconnect();

	bool isConnected() const;

	bool waitForRead(const int msecs = 30000);
	bool waitForWrite(const int msecs = 30000);

	void receive(string& buffer);
	size_t receiveRaw(byte_t* buffer, const size_t count);

	void send(const string& buffer);
	void send(const char* str);
	void sendRaw(const byte_t* buffer, const size_t count);
	size_t sendRawNonBlocking(const byte_t* buffer, const size_t count);

	size_t getBlockSize() const;

	unsigned int getStatus() const;

	const string getPeerName() const;
	const string getPeerAddress() const;

	shared_ptr <net::timeoutHandler> getTimeoutHandler();

	void setTracer(shared_ptr <net::tracer> tracer);
	shared_ptr <net::tracer> getTracer();

private:

	shared_ptr <SASLSession> m_session;
	shared_ptr <net::socket> m_wrapped;

	byte_t* m_pendingBuffer;
	size_t m_pendingPos;
	size_t m_pendingLen;

	byte_t m_recvBuffer[65536];
};


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

#endif // VMIME_SECURITY_SASL_SASLSOCKET_HPP_INCLUDED

