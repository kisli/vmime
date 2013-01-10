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

#ifndef VMIME_PLATFORMS_POSIX_SOCKET_HPP_INCLUDED
#define VMIME_PLATFORMS_POSIX_SOCKET_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/socket.hpp"


namespace vmime {
namespace platforms {
namespace posix {


class posixSocket : public vmime::net::socket
{
public:

	posixSocket(ref <vmime::net::timeoutHandler> th);
	~posixSocket();

	void connect(const vmime::string& address, const vmime::port_t port);
	bool isConnected() const;
	void disconnect();

	void receive(vmime::string& buffer);
	size_type receiveRaw(char* buffer, const size_type count);

	void send(const vmime::string& buffer);
	void sendRaw(const char* buffer, const size_type count);
	size_type sendRawNonBlocking(const char* buffer, const size_type count);

	size_type getBlockSize() const;

	unsigned int getStatus() const;

	const string getPeerName() const;
	const string getPeerAddress() const;

protected:

	static void throwSocketError(const int err);

private:

	ref <vmime::net::timeoutHandler> m_timeoutHandler;

	char m_buffer[65536];
	int m_desc;

	unsigned int m_status;

	string m_serverAddress;
};



class posixSocketFactory : public vmime::net::socketFactory
{
public:

	ref <vmime::net::socket> create();
	ref <vmime::net::socket> create(ref <vmime::net::timeoutHandler> th);
};


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_PLATFORMS_POSIX_SOCKET_HPP_INCLUDED
