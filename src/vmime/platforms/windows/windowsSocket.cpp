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

#include "vmime/config.hpp"


#if VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_MESSAGING_FEATURES


#pragma warning(disable: 4267)

#include "vmime/platforms/windows/windowsSocket.hpp"

#include "vmime/utility/stringUtils.hpp"
#include "vmime/exception.hpp"

#include <ws2tcpip.h>


namespace vmime {
namespace platforms {
namespace windows {


//
// windowsSocket
//

windowsSocket::windowsSocket(shared_ptr <vmime::net::timeoutHandler> th)
	: m_timeoutHandler(th), m_desc(INVALID_SOCKET), m_status(0)
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);
}


windowsSocket::~windowsSocket()
{
	if (m_desc != INVALID_SOCKET)
		::closesocket(m_desc);

	WSACleanup();
}


void windowsSocket::connect(const vmime::string& address, const vmime::port_t port)
{
	// Close current connection, if any
	if (m_desc != INVALID_SOCKET)
	{
		::closesocket(m_desc);
		m_desc = INVALID_SOCKET;
	}

	// Resolve address
	::sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast <unsigned short>(port));
	addr.sin_addr.s_addr = ::inet_addr(address.c_str());

	if (m_tracer)
	{
		std::ostringstream trace;
		trace << "Connecting to " << address << ", port " << port;

		m_tracer->traceSend(trace.str());
	}

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

	m_serverAddress = address;

	// Get a new socket
	m_desc = ::socket(AF_INET, SOCK_STREAM, 0);

	if (m_desc == INVALID_SOCKET)
	{
		try
		{
			int err = WSAGetLastError();
			throwSocketError(err);
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
			int err = WSAGetLastError();
			throwSocketError(err);
		}
		catch (exceptions::socket_exception& e)
		{
			::closesocket(m_desc);
			m_desc = INVALID_SOCKET;

			// Error
			throw vmime::exceptions::connection_error
				("Error while connecting socket.", e);
		}
	}

	// Set socket to non-blocking
	unsigned long non_blocking = 1;
	::ioctlsocket(m_desc, FIONBIO, &non_blocking);
}


bool windowsSocket::isConnected() const
{
	if (m_desc == INVALID_SOCKET)
		return false;

	char buff;

	return ::recv(m_desc, &buff, 1, MSG_PEEK) != 0;
}


void windowsSocket::disconnect()
{
	if (m_desc != INVALID_SOCKET)
	{
		if (m_tracer)
			m_tracer->traceSend("Disconnecting");

		::shutdown(m_desc, SD_BOTH);
		::closesocket(m_desc);

		m_desc = INVALID_SOCKET;
	}
}


static bool isNumericAddress(const char* address)
{
	struct addrinfo hint, *info = NULL;
	memset(&hint, 0, sizeof(hint));

	hint.ai_family = AF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;

	if (getaddrinfo(address, 0, &hint, &info) == 0)
	{
		freeaddrinfo(info);
		return true;
	}
	else
	{
		return false;
	}
}


const string windowsSocket::getPeerAddress() const
{
	// Get address of connected peer
	sockaddr peer;
	socklen_t peerLen = sizeof(peer);

	getpeername(m_desc, reinterpret_cast <sockaddr*>(&peer), &peerLen);

	// Convert to numerical presentation format
	char host[NI_MAXHOST + 1];
	char service[NI_MAXSERV + 1];

	if (getnameinfo(reinterpret_cast <sockaddr *>(&peer), peerLen,
			host, sizeof(host), service, sizeof(service),
			/* flags */ NI_NUMERICHOST) == 0)
	{
		return string(host);
	}

	return "";  // should not happen
}


const string windowsSocket::getPeerName() const
{
	// Get address of connected peer
	sockaddr peer;
	socklen_t peerLen = sizeof(peer);

	getpeername(m_desc, reinterpret_cast <sockaddr*>(&peer), &peerLen);

	// If server address as specified when connecting is a numeric
	// address, try to get a host name for it
	if (isNumericAddress(m_serverAddress.c_str()))
	{
		char host[NI_MAXHOST + 1];
		char service[NI_MAXSERV + 1];

		if (getnameinfo(reinterpret_cast <sockaddr *>(&peer), peerLen,
				host, sizeof(host), service, sizeof(service),
				/* flags */ NI_NAMEREQD) == 0)
		{
			return string(host);
		}
	}

	return m_serverAddress;
}


size_t windowsSocket::getBlockSize() const
{
	return 16384;  // 16 KB
}


void windowsSocket::receive(vmime::string& buffer)
{
	const size_t size = receiveRaw(m_buffer, sizeof(m_buffer));
	buffer = utility::stringUtils::makeStringFromBytes(m_buffer, size);
}


size_t windowsSocket::receiveRaw(byte_t* buffer, const size_t count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	// Check whether data is available
	if (!waitForRead(50 /* msecs */))
	{
		// No data available at this time
		// Check if we are timed out
		if (m_timeoutHandler &&
		    m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
			{
				// Server did not react within timeout delay
				throwSocketError(WSAETIMEDOUT);
			}
			else
			{
				// Reset timeout
				m_timeoutHandler->resetTimeOut();
			}
		}

		// Continue waiting for data
		return 0;
	}

	// Read available data
	int ret = ::recv(m_desc, reinterpret_cast <char*>(buffer), count, 0);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err != WSAEWOULDBLOCK)
			throwSocketError(err);

		m_status |= STATUS_WOULDBLOCK;

		// Error or no data
		return (0);
	}
	else if (ret == 0)
	{
		// Host shutdown
		throwSocketError(WSAENOTCONN);
	}
	else
	{
		// Data received, reset timeout
		if (m_timeoutHandler)
			m_timeoutHandler->resetTimeOut();

		return ret;
	}
}


void windowsSocket::send(const vmime::string& buffer)
{
	sendRaw(reinterpret_cast <const byte_t*>(buffer.data()), buffer.length());
}


void windowsSocket::send(const char* str)
{
	sendRaw(reinterpret_cast <const byte_t*>(str), strlen(str));
}


void windowsSocket::sendRaw(const byte_t* buffer, const size_t count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	size_t size = count;

	while (size > 0)
	{
		const int ret = ::send(m_desc, reinterpret_cast <const char*>(buffer), size, 0);

		if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (err != WSAEWOULDBLOCK)
				throwSocketError(err);

			waitForWrite(50 /* msecs */);
		}
		else
		{
			buffer += ret;
			size -= ret;
		}
	}

	// Reset timeout
	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();
}


size_t windowsSocket::sendRawNonBlocking(const byte_t* buffer, const size_t count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	const int ret = ::send(m_desc, reinterpret_cast <const char*>(buffer), count, 0);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			// Check if we are timed out
			if (m_timeoutHandler &&
			    m_timeoutHandler->isTimeOut())
			{
				if (!m_timeoutHandler->handleTimeOut())
				{
					// Could not send data within timeout delay
					throwSocketError(err);
				}
				else
				{
					// Reset timeout
					m_timeoutHandler->resetTimeOut();
				}
			}

			m_status |= STATUS_WOULDBLOCK;

			// No data can be written at this time
			return 0;
		}
		else
		{
			throwSocketError(err);
		}
	}

	// Reset timeout
	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();

	return ret;
}


unsigned int windowsSocket::getStatus() const
{
	return m_status;
}


void windowsSocket::throwSocketError(const int err)
{
	std::ostringstream oss;
	string msg;

	LPTSTR str;

	if (::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, err, 0, (LPTSTR) &str, 0, NULL) == 0)
	{
		// Failed getting message
		oss << "Unknown socket error (code " << err << ")";
	}
	else
	{
		oss << str;
		::LocalFree(str);
	}

	msg = oss.str();

	throw exceptions::socket_exception(msg);
}


bool windowsSocket::waitForData(const bool read, const bool write, const int msecs)
{
	for (int i = 0 ; i <= msecs / 10 ; ++i)
	{
		// Check whether data is available
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(m_desc, &fds);

		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 10000;  // 10 ms

		int ret = ::select(m_desc + 1, read ? &fds : NULL, write ? &fds : NULL, NULL, &tv);

		if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			throwSocketError(err);
		}
		else if (ret > 0)
		{
			return true;
		}

		// No data available at this time
		// Check if we are timed out
		if (m_timeoutHandler &&
		    m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
			{
				// Server did not react within timeout delay
				throw exceptions::operation_timed_out();
			}
			else
			{
				// Reset timeout
				m_timeoutHandler->resetTimeOut();
			}
		}
	}

	return false;  // time out
}


bool windowsSocket::waitForRead(const int msecs)
{
	return waitForData(/* read */ true, /* write */ false, msecs);
}


bool windowsSocket::waitForWrite(const int msecs)
{
	return waitForData(/* read */ false, /* write */ true, msecs);
}


shared_ptr <net::timeoutHandler> windowsSocket::getTimeoutHandler()
{
	return m_timeoutHandler;
}


void windowsSocket::setTracer(shared_ptr <net::tracer> tracer)
{
	m_tracer = tracer;
}


shared_ptr <net::tracer> windowsSocket::getTracer()
{
	return m_tracer;
}



//
// posixSocketFactory
//

shared_ptr <vmime::net::socket> windowsSocketFactory::create()
{
	shared_ptr <vmime::net::timeoutHandler> th;
	return make_shared <windowsSocket>(th);
}

shared_ptr <vmime::net::socket> windowsSocketFactory::create(shared_ptr <vmime::net::timeoutHandler> th)
{
    return make_shared <windowsSocket>(th);
}

} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_MESSAGING_FEATURES

