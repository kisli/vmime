//
// VMime library (http://vmime.sourceforge.net)
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

#pragma warning(disable: 4267)

#include "vmime/platforms/windows/windowsSocket.hpp"

#include "vmime/exception.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES

namespace vmime {
namespace platforms {
namespace windows {


//
// posixSocket
//

windowsSocket::windowsSocket(ref <vmime::net::timeoutHandler> th)
	: m_timeoutHandler(th), m_desc(-1)
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);
}


windowsSocket::~windowsSocket()
{
	if (m_desc != -1)
		::closesocket(m_desc);
	WSACleanup();
}


void windowsSocket::connect(const vmime::string& address, const vmime::port_t port)
{
	// Close current connection, if any
	if (m_desc != -1)
	{
		::closesocket(m_desc);
		m_desc = -1;
	}

	// Resolve address
	::sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast <unsigned short>(port));
	addr.sin_addr.s_addr = ::inet_addr(address.c_str());

	if (addr.sin_addr.s_addr == static_cast <int>(-1))
	{
		::hostent* hostInfo = ::gethostbyname(address.c_str());

		if (hostInfo == NULL)
		{
			// Error: cannot resolve address
			throw vmime::exceptions::connection_error("Cannot resolve address.");
		}

		memcpy(reinterpret_cast <char*>(&addr.sin_addr), hostInfo->h_addr, hostInfo->h_length);
	}

	// Get a new socket
	m_desc = ::socket(AF_INET, SOCK_STREAM, 0);

	if (m_desc == -1)
		throw vmime::exceptions::connection_error("Error while creating socket.");

	// Start connection
	if (::connect(m_desc, reinterpret_cast <sockaddr*>(&addr), sizeof(addr)) == -1)
	{
		::closesocket(m_desc);
		m_desc = -1;

		// Error
		throw vmime::exceptions::connection_error("Error while connecting socket.");
	}
}


bool windowsSocket::isConnected() const
{
	return (m_desc != -1);
}


void windowsSocket::disconnect()
{
	if (m_desc != -1)
	{
		::shutdown(m_desc, SD_BOTH);
		::closesocket(m_desc);

		m_desc = -1;
	}
}


windowsSocket::size_type windowsSocket::getBlockSize() const
{
	return 16384;  // 16 KB
}


void windowsSocket::receive(vmime::string& buffer)
{
	int ret = ::recv(m_desc, m_buffer, sizeof(m_buffer), 0);

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


windowsSocket::size_type windowsSocket::receiveRaw(char* buffer, const size_type count)
{
	int ret = ::recv(m_desc, buffer, count, 0);

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


void windowsSocket::send(const vmime::string& buffer)
{
	::send(m_desc, buffer.data(), buffer.length(), 0);
}


void windowsSocket::sendRaw(const char* buffer, const size_type count)
{
	::send(m_desc, buffer, count, 0);
}




//
// posixSocketFactory
//

ref <vmime::net::socket> windowsSocketFactory::create()
{
	ref <vmime::net::timeoutHandler> th = NULL;
	return vmime::create <windowsSocket>(th);
}

ref <vmime::net::socket> windowsSocketFactory::create(ref <vmime::net::timeoutHandler> th)
{
    return vmime::create <windowsSocket>(th);
}

} // posix
} // platforms
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
