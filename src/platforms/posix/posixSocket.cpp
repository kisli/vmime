//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2008 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "vmime/platforms/posix/posixSocket.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "vmime/exception.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


namespace vmime {
namespace platforms {
namespace posix {


//
// posixSocket
//

posixSocket::posixSocket()
	: m_desc(-1)
{
}


posixSocket::~posixSocket()
{
	if (m_desc != -1)
		::close(m_desc);
}


void posixSocket::connect(const vmime::string& address, const vmime::port_t port)
{
	// Close current connection, if any
	if (m_desc != -1)
	{
		::close(m_desc);
		m_desc = -1;
	}

#if VMIME_HAVE_GETADDRINFO  // use thread-safe and IPv6-aware getaddrinfo() if available

	// Resolve address, if needed
	struct ::addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	std::ostringstream portStr;
	portStr.imbue(std::locale::classic());

	portStr << port;

	struct ::addrinfo* res0;

	if (::getaddrinfo(address.c_str(), portStr.str().c_str(), &hints, &res0) != 0)
	{
		// Error: cannot resolve address
		throw vmime::exceptions::connection_error("Cannot resolve address.");
	}

	// Connect to host
	int sock = -1;
	struct ::addrinfo* res = res0;

	for ( ; sock == -1 && res != NULL ; res = res->ai_next)
	{
		sock = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if (sock < 0)
			continue;  // try next

		if (::connect(sock, res->ai_addr, res->ai_addrlen) < 0)
		{
			::close(sock);
			sock = -1;
			continue;  // try next
		}
	}

	::freeaddrinfo(res0);

	if (sock == -1)
	{
		try
		{
			throwSocketError(errno);
		}
		catch (exceptions::socket_exception& e)
		{
			throw vmime::exceptions::connection_error
				("Error while connecting socket.", e);
		}
	}

	m_desc = sock;

#else // !VMIME_HAVE_GETADDRINFO

	// Resolve address
	::sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast <unsigned short>(port));
	addr.sin_addr.s_addr = ::inet_addr(address.c_str());

	if (addr.sin_addr.s_addr == static_cast <in_addr_t>(-1))
	{
		::hostent* hostInfo = ::gethostbyname(address.c_str());

		if (hostInfo == NULL)
		{
			// Error: cannot resolve address
			throw vmime::exceptions::connection_error("Cannot resolve address.");
		}

		::memcpy(reinterpret_cast <char*>(&addr.sin_addr), hostInfo->h_addr, hostInfo->h_length);
	}

	// Get a new socket
	m_desc = ::socket(AF_INET, SOCK_STREAM, 0);

	if (m_desc == -1)
	{
		try
		{
			throwSocketError(errno);
		}
		catch (exceptions::socket_exception& e)
		{
			throw vmime::exceptions::connection_error
				("Error while creating socket.", e);
		}
	}

	// Start connection
	if (::connect(m_desc, reinterpret_cast <sockaddr*>(&addr), sizeof(addr)) == -1)
	{
		try
		{
			throwSocketError(errno);
		}
		catch (exceptions::socket_exception& e)
		{
			::close(m_desc);
			m_desc = -1;

			// Error
			throw vmime::exceptions::connection_error
				("Error while connecting socket.", e);
		}
	}

#endif // VMIME_HAVE_GETADDRINFO

	::fcntl(m_desc, F_SETFL, ::fcntl(m_desc, F_GETFL) | O_NONBLOCK);
}


const bool posixSocket::isConnected() const
{
	if (m_desc == -1)
		return false;

	char buff;

	return ::recv(m_desc, &buff, 1, MSG_PEEK) != 0;
}


void posixSocket::disconnect()
{
	if (m_desc != -1)
	{
		::shutdown(m_desc, SHUT_RDWR);
		::close(m_desc);

		m_desc = -1;
	}
}


void posixSocket::receive(vmime::string& buffer)
{
	const int size = receiveRaw(m_buffer, sizeof(m_buffer));
	buffer = vmime::string(m_buffer, size);
}


const int posixSocket::receiveRaw(char* buffer, const int count)
{
	const int ret = ::recv(m_desc, buffer, count, 0);

	if (ret < 0)
	{
		if (errno != EAGAIN)
			throwSocketError(errno);

		// No data available at this time
		return 0;
	}
	else if (ret == 0)
	{
		// Host shutdown
		throwSocketError(ENOTCONN);
	}

	return ret;
}


void posixSocket::send(const vmime::string& buffer)
{
	sendRaw(buffer.data(), buffer.length());
}


void posixSocket::sendRaw(const char* buffer, const int count)
{
	int size = count;

	while (size > 0)
	{
		const int ret = ::send(m_desc, buffer, size, 0);

		if (ret < 0)
		{
			if (errno != EAGAIN)
				throwSocketError(errno);

			platform::getHandler()->wait();
		}
		else
		{
			buffer += ret;
			size -= ret;
		}
	}
}


void posixSocket::throwSocketError(const int err)
{
	string msg;

	switch (err)
	{
	case EACCES:          msg = "EACCES: permission denied"; break;
	case EAFNOSUPPORT:    msg = "EAFNOSUPPORT: address family not supported"; break;
	case EMFILE:          msg = "EMFILE: process file table overflow"; break;
	case ENFILE:          msg = "ENFILE: system limit reached"; break;
	case EPROTONOSUPPORT: msg = "EPROTONOSUPPORT: protocol not supported"; break;
	case EAGAIN:          msg = "EGAIN: blocking operation"; break;
	case EBADF:           msg = "EBADF: invalid descriptor"; break;
	case ECONNRESET:      msg = "ECONNRESET: connection reset by peer"; break;
	case EFAULT:          msg = "EFAULT: bad user space address"; break;
	case EINTR:           msg = "EINTR: signal occured before transmission"; break;
	case EINVAL:          msg = "EINVAL: invalid argument"; break;
	case EMSGSIZE:        msg = "EMSGSIZE: message cannot be sent atomically"; break;
	case ENOBUFS:         msg = "ENOBUFS: output queue is full"; break;
	case ENOMEM:          msg = "ENOMEM: out of memory"; break;
	case EPIPE:
	case ENOTCONN:        msg = "ENOTCONN: not connected"; break;
	case ECONNREFUSED:    msg = "ECONNREFUSED: connection refused"; break;
	default:

		std::ostringstream oss;
		oss << ::strerror(err);

		msg = oss.str();
		break;
	}

	throw exceptions::socket_exception(msg);
}



//
// posixSocketFactory
//

ref <vmime::net::socket> posixSocketFactory::create()
{
	return vmime::create <posixSocket>();
}


} // posix
} // platforms
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
