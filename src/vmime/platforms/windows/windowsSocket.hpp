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

#ifndef VMIME_PLATFORMS_WINDOWS_SOCKET_HPP_INCLUDED
#define VMIME_PLATFORMS_WINDOWS_SOCKET_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_MESSAGING_FEATURES


#include <winsock2.h>
#include "vmime/net/socket.hpp"


namespace vmime {
namespace platforms {
namespace windows {


class windowsSocket : public vmime::net::socket
{
public:

	windowsSocket();
	windowsSocket(shared_ptr <vmime::net::timeoutHandler> th);
	~windowsSocket();

public:

	void connect(const vmime::string& address, const vmime::port_t port);
	bool isConnected() const;
	void disconnect();

	bool waitForRead(const int msecs = 30000);
	bool waitForWrite(const int msecs = 30000);

	void receive(vmime::string& buffer);
	size_t receiveRaw(byte_t* buffer, const size_t count);
	
	void send(const vmime::string& buffer);
	void send(const char* str);
	void sendRaw(const byte_t* buffer, const size_t count);
	size_t sendRawNonBlocking(const byte_t* buffer, const size_t count);

	size_t getBlockSize() const;

	unsigned int getStatus() const;

	const string getPeerName() const;
	const string getPeerAddress() const;

	shared_ptr <net::timeoutHandler> getTimeoutHandler();
	shared_ptr <net::tracer> m_tracer;

	void setTracer(shared_ptr <net::tracer> tracer);
	shared_ptr <net::tracer> getTracer();

protected:

	void throwSocketError(const int err);

	bool waitForData(const bool read, const bool write, const int msecs);

private:

	shared_ptr <vmime::net::timeoutHandler> m_timeoutHandler;

	byte_t m_buffer[65536];
	SOCKET m_desc;

	unsigned int m_status;

	string m_serverAddress;
};



class windowsSocketFactory : public vmime::net::socketFactory
{
public:

	shared_ptr <vmime::net::socket> create();
	shared_ptr <vmime::net::socket> create(shared_ptr <vmime::net::timeoutHandler> th);
};


} // windows
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_PLATFORMS_WINDOWS_SOCKET_HPP_INCLUDED
