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

#include "vmime/platforms/posix/posixSocket.hpp"

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
}


const bool posixSocket::isConnected() const
{
	return (m_desc != -1);
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
	::ssize_t ret = ::recv(m_desc, m_buffer, sizeof(m_buffer), 0);

	if (ret == -1)
	{
		// Error or no data
		return;
	}
	else if (ret > 0)
	{
		buffer = vmime::string(m_buffer, ret);
	}
}


const int posixSocket::receiveRaw(char* buffer, const int count)
{
	::ssize_t ret = ::recv(m_desc, buffer, count, 0);

	if (ret == -1)
	{
		// Error or no data
		return (0);
	}
	else
	{
		return (ret);
	}
}


void posixSocket::send(const vmime::string& buffer)
{
	if (::send(m_desc, buffer.data(), buffer.length(), 0) == -1)
		throwSocketError(errno);
}


void posixSocket::sendRaw(const char* buffer, const int count)
{
	if (::send(m_desc, buffer, count, 0) == -1)
		throwSocketError(errno);
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
