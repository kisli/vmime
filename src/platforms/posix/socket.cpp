//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/platforms/posix/socket.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

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
	addr.sin_port = htons((unsigned short) port);
	addr.sin_addr.s_addr = ::inet_addr(address.c_str());

	if (addr.sin_addr.s_addr == (::in_addr_t) -1)
	{
		::hostent* hostInfo = (hostent*) ::gethostbyname(address.c_str());

		if (hostInfo == NULL)
		{
			// Error: cannot resolve address
			throw vmime::exceptions::connection_error();
		}

		bcopy(hostInfo->h_addr, (char*) &addr.sin_addr, hostInfo->h_length);
	}
	else
	{
		// Error: cannot resolve address
		throw vmime::exceptions::connection_error();
	}

	// Get a new socket
	m_desc = ::socket(AF_INET, SOCK_STREAM, 0);

	if (m_desc == -1)
		throw vmime::exceptions::connection_error();

	// Start connection
	if (::connect(m_desc, (sockaddr*) &addr, sizeof(addr)) == -1)
	{
		::close(m_desc);
		m_desc = -1;

		// Error
		throw vmime::exceptions::connection_error();
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
	::send(m_desc, buffer.data(), buffer.length(), 0);
}


void posixSocket::sendRaw(const char* buffer, const int count)
{
	::send(m_desc, buffer, count, 0);
}




//
// posixSocketFactory
//

vmime::messaging::socket* posixSocketFactory::create()
{
	return new posixSocket();
}


} // posix
} // platforms
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
