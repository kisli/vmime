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


#if VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/platforms/posix/posixSocket.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "vmime/utility/stringUtils.hpp"

#include "vmime/exception.hpp"


#if defined(EWOULDBLOCK)
#   define IS_EAGAIN(x)  ((x) == EAGAIN || (x) == EWOULDBLOCK || (x) == EINTR || (x) == EINPROGRESS)
#else
#   define IS_EAGAIN(x)  ((x) == EAGAIN || (x) == EINTR || (x) == EINPROGRESS)
#endif


namespace vmime {
namespace platforms {
namespace posix {


//
// posixSocket
//

posixSocket::posixSocket(shared_ptr <vmime::net::timeoutHandler> th)
	: m_timeoutHandler(th), m_desc(-1), m_status(0)
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

	if (m_tracer)
	{
		std::ostringstream trace;
		trace << "Connecting to " << address << ", port " << port;

		m_tracer->traceSend(trace.str());
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

	m_serverAddress = address;

	// Connect to host
	int sock = -1;
	struct ::addrinfo* res = res0;
	int connectErrno = 0;

	if (m_timeoutHandler != NULL)
		m_timeoutHandler->resetTimeOut();

	for ( ; sock == -1 && res != NULL ; res = res->ai_next, connectErrno = ETIMEDOUT)
	{
		if (res->ai_family != AF_INET && res->ai_family != AF_INET6)
			continue;

		sock = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if (sock < 0)
		{
			connectErrno = errno;
			continue;  // try next
		}

		if (m_timeoutHandler != NULL)
		{
			::fcntl(sock, F_SETFL, ::fcntl(sock, F_GETFL) | O_NONBLOCK);

			if (::connect(sock, res->ai_addr, res->ai_addrlen) < 0)
			{
				switch (errno)
				{
				case 0:
				case EINPROGRESS:
				case EINTR:
#if defined(EAGAIN)
				case EAGAIN:
#endif // EAGAIN
#if defined(EWOULDBLOCK) && (!defined(EAGAIN) || (EWOULDBLOCK != EAGAIN))
				case EWOULDBLOCK:
#endif // EWOULDBLOCK

					// Connection in progress
					break;

				default:

					connectErrno = errno;
					::close(sock);
					sock = -1;
					continue;  // try next
				}

				// Wait for socket to be connected.
				bool connected = false;

				const int selectTimeout = 1000;   // select() timeout (ms)
				const int tryNextTimeout = 5000;  // maximum time before trying next (ms)

				timeval startTime = { 0, 0 };
				gettimeofday(&startTime, /* timezone */ NULL);

				do
				{
					struct timeval tm;
					tm.tv_sec = selectTimeout / 1000;
					tm.tv_usec = selectTimeout % 1000;

					fd_set fds;
					FD_ZERO(&fds);
					FD_SET(sock, &fds);

					const int ret = select(sock + 1, NULL, &fds, NULL, &tm);

					// Success
					if (ret > 0)
					{
						int error = 0;
						socklen_t len = sizeof(error);

						if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
						{
							connectErrno = errno;
						}
						else
						{
							if (error != 0)
								connectErrno = error;
							else
								connected = true;
						}

						break;
					}
					// Error
					else if (ret < -1)
					{
						if (errno != EINTR)
						{
							// Cancel connection
							connectErrno = errno;
							break;
						}
					}
					// Check for timeout
					else if (ret == 0)
					{
						if (m_timeoutHandler->isTimeOut())
						{
							if (!m_timeoutHandler->handleTimeOut())
							{
								// Cancel connection
								connectErrno = ETIMEDOUT;
								break;
							}
							else
							{
								// Reset timeout and keep waiting for connection
								m_timeoutHandler->resetTimeOut();
							}
						}
						else
						{
							// Keep waiting for connection
						}
					}

					timeval curTime = { 0, 0 };
					gettimeofday(&curTime, /* timezone */ NULL);

					if (res->ai_next != NULL &&
						curTime.tv_usec - startTime.tv_usec >= tryNextTimeout * 1000)
					{
						connectErrno = ETIMEDOUT;
						break;
					}

				} while (true);

				if (!connected)
				{
					::close(sock);
					sock = -1;
					continue;  // try next
				}

				break;
			}
			else
			{
				// Connection successful
				break;
			}
		}
		else
		{
			if (::connect(sock, res->ai_addr, res->ai_addrlen) < 0)
			{
				connectErrno = errno;
				::close(sock);
				sock = -1;
				continue;  // try next
			}
		}
	}

	::freeaddrinfo(res0);

	if (sock == -1)
	{
		try
		{
			throwSocketError(connectErrno);
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

	m_serverAddress = address;

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


bool posixSocket::isConnected() const
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
		if (m_tracer)
			m_tracer->traceSend("Disconnecting");

		::shutdown(m_desc, SHUT_RDWR);
		::close(m_desc);

		m_desc = -1;
	}
}


static bool isNumericAddress(const char* address)
{

#if VMIME_HAVE_GETADDRINFO

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

#else

	return inet_addr(address) != INADDR_NONE;

#endif

}


const string posixSocket::getPeerAddress() const
{
	// Get address of connected peer
	sockaddr peer;
	socklen_t peerLen = sizeof(peer);

	getpeername(m_desc, reinterpret_cast <sockaddr*>(&peer), &peerLen);

	// Convert to numerical presentation format
	char numericAddress[1024];

	if (inet_ntop(peer.sa_family, &peer, numericAddress, sizeof(numericAddress)) != NULL)
		return string(numericAddress);

	return "";  // should not happen
}


const string posixSocket::getPeerName() const
{
	// Get address of connected peer
	sockaddr peer;
	socklen_t peerLen = sizeof(peer);

	getpeername(m_desc, reinterpret_cast <sockaddr*>(&peer), &peerLen);

	// If server address as specified when connecting is a numeric
	// address, try to get a host name for it
	if (isNumericAddress(m_serverAddress.c_str()))
	{

#if VMIME_HAVE_GETNAMEINFO

		char host[NI_MAXHOST + 1];
		char service[NI_MAXSERV + 1];

		if (getnameinfo(reinterpret_cast <sockaddr *>(&peer), peerLen,
				host, sizeof(host), service, sizeof(service),
				/* flags */ NI_NAMEREQD) == 0)
		{
			return string(host);
		}

#else

		struct hostent *hp;

		if ((hp = gethostbyaddr(reinterpret_cast <const void *>(&peer),
				sizeof(peer), peer.sa_family)) != NULL)
		{
			return string(hp->h_name);
		}

#endif

	}

	return m_serverAddress;
}


size_t posixSocket::getBlockSize() const
{
	return 16384;  // 16 KB
}


bool posixSocket::waitForData(const bool read, const bool write, const int msecs)
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

		ssize_t ret = ::select(m_desc + 1, read ? &fds : NULL, write ? &fds : NULL, NULL, &tv);

		if (ret <= 0)
		{
			if (ret < 0 && !IS_EAGAIN(errno))
				throwSocketError(errno);

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
		else if (ret > 0)
		{
			return true;
		}
	}

	return false;  // time out
}


bool posixSocket::waitForRead(const int msecs)
{
	return waitForData(/* read */ true, /* write */ false, msecs);
}


bool posixSocket::waitForWrite(const int msecs)
{
	return waitForData(/* read */ false, /* write */ true, msecs);
}


void posixSocket::receive(vmime::string& buffer)
{
	const size_t size = receiveRaw(m_buffer, sizeof(m_buffer));
	buffer = utility::stringUtils::makeStringFromBytes(m_buffer, size);
}


size_t posixSocket::receiveRaw(byte_t* buffer, const size_t count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	// Check whether data is available
	if (!waitForRead(50 /* msecs */))
	{
		m_status |= STATUS_WOULDBLOCK;

		// Continue waiting for data
		return 0;
	}

	// Read available data
	ssize_t ret = ::recv(m_desc, buffer, count, 0);

	if (ret < 0)
	{
		if (!IS_EAGAIN(errno))
			throwSocketError(errno);

		// Check if we are timed out
		if (m_timeoutHandler &&
		    m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
			{
				// Server did not react within timeout delay
				throwSocketError(errno);
			}
			else
			{
				// Reset timeout
				m_timeoutHandler->resetTimeOut();
			}
		}

		m_status |= STATUS_WOULDBLOCK;

		// No data available at this time
		return 0;
	}
	else if (ret == 0)
	{
		// Host shutdown
		throwSocketError(ENOTCONN);
	}
	else
	{
		// Data received, reset timeout
		if (m_timeoutHandler)
			m_timeoutHandler->resetTimeOut();
	}

	return ret;
}


void posixSocket::send(const vmime::string& buffer)
{
	sendRaw(reinterpret_cast <const byte_t*>(buffer.data()), buffer.length());
}


void posixSocket::send(const char* str)
{
	sendRaw(reinterpret_cast <const byte_t*>(str), ::strlen(str));
}


void posixSocket::sendRaw(const byte_t* buffer, const size_t count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	size_t size = count;

	while (size > 0)
	{
		const ssize_t ret = ::send(m_desc, buffer, size, 0);

		if (ret <= 0)
		{
			if (ret < 0 && !IS_EAGAIN(errno))
				throwSocketError(errno);

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


size_t posixSocket::sendRawNonBlocking(const byte_t* buffer, const size_t count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	const ssize_t ret = ::send(m_desc, buffer, count, 0);

	if (ret <= 0)
	{
		if (ret < 0 && !IS_EAGAIN(errno))
			throwSocketError(errno);

		// Check if we are timed out
		if (m_timeoutHandler &&
		    m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
			{
				// Could not send data within timeout delay
				throw exceptions::operation_timed_out();
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

	// Reset timeout
	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();

	return ret;
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


unsigned int posixSocket::getStatus() const
{
	return m_status;
}


shared_ptr <net::timeoutHandler> posixSocket::getTimeoutHandler()
{
	return m_timeoutHandler;
}


void posixSocket::setTracer(shared_ptr <net::tracer> tracer)
{
	m_tracer = tracer;
}


shared_ptr <net::tracer> posixSocket::getTracer()
{
	return m_tracer;
}



//
// posixSocketFactory
//

shared_ptr <vmime::net::socket> posixSocketFactory::create()
{
	shared_ptr <vmime::net::timeoutHandler> th;
	return make_shared <posixSocket>(th);
}


shared_ptr <vmime::net::socket> posixSocketFactory::create(shared_ptr <vmime::net::timeoutHandler> th)
{
	return make_shared <posixSocket>(th);
}


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_MESSAGING_FEATURES
