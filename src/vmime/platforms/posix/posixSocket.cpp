//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE  // for getaddrinfo_a() in <netdb.h>
#endif

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
#include <stdio.h>
#include <poll.h>

#include "vmime/utility/stringUtils.hpp"

#include "vmime/exception.hpp"


#if defined(EWOULDBLOCK)
#   define IS_EAGAIN(x)  ((x) == EAGAIN || (x) == EWOULDBLOCK || (x) == EINTR || (x) == EINPROGRESS)
#else
#   define IS_EAGAIN(x)  ((x) == EAGAIN || (x) == EINTR || (x) == EINPROGRESS)
#endif


// Workaround for detection of strerror_r variants
#if VMIME_HAVE_STRERROR_R

namespace {

char* vmime_strerror_r_result(int /* res */, char* buf) {

	// XSI-compliant prototype:
	// int strerror_r(int errnum, char *buf, size_t buflen);
	return buf;
}

char* vmime_strerror_r_result(char* res, char* /* buf */) {

	// GNU-specific prototype:
	// char *strerror_r(int errnum, char *buf, size_t buflen);
	return res;
}

}

#endif // VMIME_HAVE_STRERROR_R



namespace vmime {
namespace platforms {
namespace posix {


//
// posixSocket
//

posixSocket::posixSocket(shared_ptr <vmime::net::timeoutHandler> th)
	: m_timeoutHandler(th),
	  m_desc(-1),
	  m_status(0) {

}


posixSocket::~posixSocket() {

	if (m_desc != -1) {
		::close(m_desc);
	}
}


void posixSocket::connect(const vmime::string& address, const vmime::port_t port) {

	// Close current connection, if any
	if (m_desc != -1) {
		::close(m_desc);
		m_desc = -1;
	}

	if (m_tracer) {

		std::ostringstream trace;
		trace << "Connecting to " << address << ", port " << port;

		m_tracer->traceSend(trace.str());
	}

#if VMIME_HAVE_GETADDRINFO  // use thread-safe and IPv6-aware getaddrinfo() if available

	// Resolve address, if needed
	m_serverAddress = address;

	struct ::addrinfo* addrInfo = NULL;  // resolved addresses
	resolve(&addrInfo, address, port);

	// Connect to host
	int sock = -1;
	int connectErrno = 0;

	if (m_timeoutHandler != NULL) {
		m_timeoutHandler->resetTimeOut();
	}

	for (struct ::addrinfo* curAddrInfo = addrInfo ;
	     sock == -1 && curAddrInfo != NULL ;
	     curAddrInfo = curAddrInfo->ai_next, connectErrno = ETIMEDOUT) {

		if (curAddrInfo->ai_family != AF_INET && curAddrInfo->ai_family != AF_INET6) {
			continue;
		}

		sock = ::socket(curAddrInfo->ai_family, curAddrInfo->ai_socktype, curAddrInfo->ai_protocol);

		if (sock < 0) {
			connectErrno = errno;
			continue;  // try next
		}

#if VMIME_HAVE_SO_KEEPALIVE

		// Enable TCP Keepalive
		int keepAlive_optval = 1;
		socklen_t keepAlive_optlen = sizeof(keepAlive_optval);

		::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive_optval, keepAlive_optlen);

#endif // VMIME_HAVE_SO_KEEPALIVE

#if VMIME_HAVE_SO_NOSIGPIPE

		// Return EPIPE instead of generating SIGPIPE
		int nosigpipe_optval = 1;
		socklen_t nosigpipe_optlen = sizeof(nosigpipe_optval);

		::setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe_optval, nosigpipe_optlen);

#endif // VMIME_HAVE_SO_NOSIGPIPE


		if (m_timeoutHandler) {

			::fcntl(sock, F_SETFL, ::fcntl(sock, F_GETFL) | O_NONBLOCK);

			if (::connect(sock, curAddrInfo->ai_addr, curAddrInfo->ai_addrlen) < 0) {

				switch (errno) {

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

				const int pollTimeout = 1000;   // poll() timeout (ms)
				const int tryNextTimeout = 5000;  // maximum time before trying next (ms)

				timeval startTime = { 0, 0 };
				gettimeofday(&startTime, /* timezone */ NULL);

				do {

					pollfd fds[1];
					fds[0].fd = sock;
					fds[0].events = POLLIN | POLLOUT;

					const int ret = ::poll(fds, sizeof(fds) / sizeof(fds[0]), pollTimeout);

					// Success
					if (ret > 0) {

						if (fds[0].revents & (POLLIN | POLLOUT)) {

							int error = 0;
							socklen_t len = sizeof(error);

							if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {

								connectErrno = errno;

							} else {

								if (error != 0) {
									connectErrno = error;
								} else {
									connected = true;
								}
							}
						}

						break;

					// Error
					} else if (ret < -1) {

						if (errno != EAGAIN && errno != EINTR) {

							// Cancel connection
							connectErrno = errno;
							break;
						}
					}

					// Check for timeout
					if (m_timeoutHandler->isTimeOut()) {

						if (!m_timeoutHandler->handleTimeOut()) {

							// Cancel connection
							connectErrno = ETIMEDOUT;
							break;

						} else {

							// Reset timeout and keep waiting for connection
							m_timeoutHandler->resetTimeOut();
						}

					} else {

						// Keep waiting for connection
					}

					timeval curTime = { 0, 0 };
					gettimeofday(&curTime, /* timezone */ NULL);

					if (curAddrInfo->ai_next != NULL &&
						curTime.tv_usec - startTime.tv_usec >= tryNextTimeout * 1000) {

						connectErrno = ETIMEDOUT;
						break;
					}

				} while (true);

				if (!connected) {

					::close(sock);
					sock = -1;
					continue;  // try next
				}

				break;

			} else {

				// Connection successful
				break;
			}

		} else {

			if (::connect(sock, curAddrInfo->ai_addr, curAddrInfo->ai_addrlen) < 0) {

				connectErrno = errno;
				::close(sock);
				sock = -1;
				continue;  // try next
			}
		}
	}

	::freeaddrinfo(addrInfo);

	if (sock == -1) {

		try {
			throwSocketError(connectErrno);
		} catch (exceptions::socket_exception& e) {  // wrap
			throw vmime::exceptions::connection_error("Error while connecting socket.", e);
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

	if (addr.sin_addr.s_addr == static_cast <in_addr_t>(-1)) {

		::hostent* hostInfo = ::gethostbyname(address.c_str());

		if (hostInfo == NULL) {
			// Error: cannot resolve address
			throw vmime::exceptions::connection_error("Cannot resolve address.");
		}

		::memcpy(reinterpret_cast <char*>(&addr.sin_addr), hostInfo->h_addr, hostInfo->h_length);
	}

	m_serverAddress = address;

	// Get a new socket
	m_desc = ::socket(AF_INET, SOCK_STREAM, 0);

	if (m_desc == -1) {

		try {
			throwSocketError(errno);
		} catch (exceptions::socket_exception& e) {  // wrap
			throw vmime::exceptions::connection_error("Error while creating socket.", e);
		}
	}

	// Start connection
	if (::connect(m_desc, reinterpret_cast <sockaddr*>(&addr), sizeof(addr)) == -1) {

		try {

			throwSocketError(errno);

		} catch (exceptions::socket_exception& e) {  // wrap

			::close(m_desc);
			m_desc = -1;

			// Error
			throw vmime::exceptions::connection_error("Error while connecting socket.", e);
		}
	}

#endif // VMIME_HAVE_GETADDRINFO

	::fcntl(m_desc, F_SETFL, ::fcntl(m_desc, F_GETFL) | O_NONBLOCK);
}


void posixSocket::resolve(
	struct ::addrinfo** addrInfo,
	const vmime::string& address,
	const vmime::port_t port
) {

	char portStr[16];
	snprintf(portStr, sizeof(portStr), "%u", static_cast <unsigned int>(port));


	struct ::addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	hints.ai_flags = AI_CANONNAME | AI_NUMERICSERV;
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

#if VMIME_HAVE_GETADDRINFO_A

	// If getaddrinfo_a() is available, use asynchronous resolving to allow
	// the timeout handler to cancel the operation

	struct ::gaicb gaiRequest;
	memset(&gaiRequest, 0, sizeof(gaiRequest));

	gaiRequest.ar_name = address.c_str();
	gaiRequest.ar_service = portStr;
	gaiRequest.ar_request = &hints;

	struct ::gaicb* gaiRequests = &gaiRequest;
	int gaiError;

	if ((gaiError = getaddrinfo_a(GAI_NOWAIT, &gaiRequests, 1, NULL)) != 0) {

		throw vmime::exceptions::connection_error(
			"getaddrinfo_a() failed: " + std::string(gai_strerror(gaiError))
		);
	}

	if (m_timeoutHandler) {
		m_timeoutHandler->resetTimeOut();
	}

	while (true) {

		struct timespec gaiTimeout;
		gaiTimeout.tv_sec = 1;  // query timeout handler every second
		gaiTimeout.tv_nsec = 0;

		gaiError = gai_suspend(&gaiRequests, 1, &gaiTimeout);

		if (gaiError == 0 || gaiError == EAI_ALLDONE) {

			const int ret = gai_error(&gaiRequest);

			if (ret != 0) {

				throw vmime::exceptions::connection_error(
					"getaddrinfo_a() request failed: " + std::string(gai_strerror(ret))
				);

			} else {

				*addrInfo = gaiRequest.ar_result;
				break;
			}

		} else if (gaiError != EAI_AGAIN) {

			if (gaiError == EAI_SYSTEM) {

				const int ret = gai_error(&gaiRequest);

				if (ret != EAI_INPROGRESS && errno != 0) {

					try {
						throwSocketError(errno);
					} catch (exceptions::socket_exception& e) {  // wrap
						throw vmime::exceptions::connection_error("Error while connecting socket.", e);
					}
				}

			} else {

				throw vmime::exceptions::connection_error(
					"gai_suspend() failed: " + std::string(gai_strerror(gaiError))
				);
			}
		}

		// Check for timeout
		if (m_timeoutHandler && m_timeoutHandler->isTimeOut()) {

			if (!m_timeoutHandler->handleTimeOut()) {

				throw exceptions::operation_timed_out();

			} else {

				// Reset timeout and keep waiting for connection
				m_timeoutHandler->resetTimeOut();
			}
		}
	}

#else  // !VMIME_HAVE_GETADDRINFO_A

	if (::getaddrinfo(address.c_str(), portStr, &hints, addrInfo) != 0) {

		// Error: cannot resolve address
		throw vmime::exceptions::connection_error("Cannot resolve address.");
	}

#endif  // VMIME_HAVE_GETADDRINFO_A

}


bool posixSocket::isConnected() const {

	if (m_desc == -1) {
		return false;
	}

	char buff;

	return ::recv(m_desc, &buff, 1, MSG_PEEK) != 0;
}


void posixSocket::disconnect() {

	if (m_desc != -1) {

		if (m_tracer) {
			m_tracer->traceSend("Disconnecting");
		}

		::shutdown(m_desc, SHUT_RDWR);
		::close(m_desc);

		m_desc = -1;
	}
}


static bool isNumericAddress(const char* address) {

#if VMIME_HAVE_GETADDRINFO

	struct addrinfo hint, *info = NULL;
	memset(&hint, 0, sizeof(hint));

	hint.ai_family = AF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;

	if (getaddrinfo(address, 0, &hint, &info) == 0) {

		freeaddrinfo(info);
		return true;

	} else {

		return false;
	}

#else

	return inet_addr(address) != INADDR_NONE;

#endif

}


const string posixSocket::getPeerAddress() const {

	// Get address of connected peer
	sockaddr peer;
	socklen_t peerLen = sizeof(peer);

	if (getpeername(m_desc, &peer, &peerLen) != 0) {
		throwSocketError(errno);
	}

	// Convert to numerical presentation format
	char buf[INET6_ADDRSTRLEN];

	if (!inet_ntop(peer.sa_family, &(reinterpret_cast <struct sockaddr_in *>(&peer))->sin_addr, buf, sizeof(buf))) {
		throwSocketError(errno);
	}

	return string(buf);
}


const string posixSocket::getPeerName() const {

	// Get address of connected peer
	sockaddr peer;
	socklen_t peerLen = sizeof(peer);

	if (getpeername(m_desc, &peer, &peerLen) != 0) {
		throwSocketError(errno);
	}

	// If server address as specified when connecting is a numeric
	// address, try to get a host name for it
	if (isNumericAddress(m_serverAddress.c_str())) {

#if VMIME_HAVE_GETNAMEINFO

		char host[NI_MAXHOST + 1];
		char service[NI_MAXSERV + 1];

		if (getnameinfo(reinterpret_cast <sockaddr *>(&peer), peerLen,
				host, sizeof(host), service, sizeof(service),
				/* flags */ NI_NAMEREQD) == 0) {

			return string(host);
		}

#else

		struct hostent *hp;

		if ((hp = gethostbyaddr(reinterpret_cast <const void *>(&peer),
				sizeof(peer), peer.sa_family)) != NULL) {

			return string(hp->h_name);
		}

#endif

	}

	return m_serverAddress;
}


size_t posixSocket::getBlockSize() const {

	return 16384;  // 16 KB
}


bool posixSocket::waitForData(const bool read, const bool write, const int msecs) {

	for (int i = 0 ; i <= msecs / 10 ; ++i) {

		// Check whether data is available
		pollfd fds[1];
		fds[0].fd = m_desc;
		fds[0].events = 0;

		if (read) {
			fds[0].events |= POLLIN;
		}

		if (write) {
			fds[0].events |= POLLOUT;
		}

		const int ret = ::poll(fds, sizeof(fds) / sizeof(fds[0]), 10 /* ms */);

		if (ret < 0) {

			if (errno != EAGAIN && errno != EINTR) {
				throwSocketError(errno);
			}

		} else if (ret > 0) {

			if (fds[0].revents & (POLLIN | POLLOUT)) {
				return true;
			}
		}

		// No data available at this time
		// Check if we are timed out
		if (m_timeoutHandler &&
		    m_timeoutHandler->isTimeOut()) {

			if (!m_timeoutHandler->handleTimeOut()) {

				// Server did not react within timeout delay
				throw exceptions::operation_timed_out();

			} else {

				// Reset timeout
				m_timeoutHandler->resetTimeOut();
			}
		}
	}

	return false;  // time out
}


bool posixSocket::waitForRead(const int msecs) {

	return waitForData(/* read */ true, /* write */ false, msecs);
}


bool posixSocket::waitForWrite(const int msecs) {

	return waitForData(/* read */ false, /* write */ true, msecs);
}


void posixSocket::receive(vmime::string& buffer) {

	const size_t size = receiveRaw(m_buffer, sizeof(m_buffer));
	buffer = utility::stringUtils::makeStringFromBytes(m_buffer, size);
}


size_t posixSocket::receiveRaw(byte_t* buffer, const size_t count) {

	m_status &= ~STATUS_WOULDBLOCK;

	// Check whether data is available
	if (!waitForRead(50 /* msecs */)) {

		m_status |= STATUS_WOULDBLOCK;

		// Continue waiting for data
		return 0;
	}

	// Read available data
	ssize_t ret = ::recv(m_desc, buffer, count, 0);

	if (ret < 0) {

		if (!IS_EAGAIN(errno)) {
			throwSocketError(errno);
		}

		// Check if we are timed out
		if (m_timeoutHandler &&
		    m_timeoutHandler->isTimeOut()) {

			if (!m_timeoutHandler->handleTimeOut()) {

				// Server did not react within timeout delay
				throwSocketError(errno);

			} else {

				// Reset timeout
				m_timeoutHandler->resetTimeOut();
			}
		}

		m_status |= STATUS_WOULDBLOCK;

		// No data available at this time
		return 0;

	} else if (ret == 0) {

		// Host shutdown
		throwSocketError(ENOTCONN);

	} else {

		// Data received, reset timeout
		if (m_timeoutHandler) {
			m_timeoutHandler->resetTimeOut();
		}
	}

	return ret;
}


void posixSocket::send(const vmime::string& buffer) {

	sendRaw(reinterpret_cast <const byte_t*>(buffer.data()), buffer.length());
}


void posixSocket::send(const char* str) {

	sendRaw(reinterpret_cast <const byte_t*>(str), ::strlen(str));
}


void posixSocket::sendRaw(const byte_t* buffer, const size_t count) {

	m_status &= ~STATUS_WOULDBLOCK;

	size_t size = count;

	while (size > 0) {

#if VMIME_HAVE_MSG_NOSIGNAL
		const ssize_t ret = ::send(m_desc, buffer, size, MSG_NOSIGNAL);
#else
		const ssize_t ret = ::send(m_desc, buffer, size, 0);
#endif

		if (ret <= 0) {

			if (ret < 0 && !IS_EAGAIN(errno)) {
				throwSocketError(errno);
			}

			waitForWrite(50 /* msecs */);

		} else {

			buffer += ret;
			size -= ret;
		}
	}

	// Reset timeout
	if (m_timeoutHandler) {
		m_timeoutHandler->resetTimeOut();
	}
}


size_t posixSocket::sendRawNonBlocking(const byte_t* buffer, const size_t count) {

	m_status &= ~STATUS_WOULDBLOCK;

#if VMIME_HAVE_MSG_NOSIGNAL
	const ssize_t ret = ::send(m_desc, buffer, count, MSG_NOSIGNAL);
#else
	const ssize_t ret = ::send(m_desc, buffer, count, 0);
#endif

	if (ret <= 0) {

		if (ret < 0 && !IS_EAGAIN(errno)) {
			throwSocketError(errno);
		}

		// Check if we are timed out
		if (m_timeoutHandler &&
		    m_timeoutHandler->isTimeOut()) {

			if (!m_timeoutHandler->handleTimeOut()) {

				// Could not send data within timeout delay
				throw exceptions::operation_timed_out();

			} else {

				// Reset timeout
				m_timeoutHandler->resetTimeOut();
			}
		}

		m_status |= STATUS_WOULDBLOCK;

		// No data can be written at this time
		return 0;
	}

	// Reset timeout
	if (m_timeoutHandler) {
		m_timeoutHandler->resetTimeOut();
	}

	return ret;
}


void posixSocket::throwSocketError(const int err) {

	const char* msg = NULL;

	switch (err) {

		case EACCES:          msg = "EACCES: permission denied"; break;
		case EAFNOSUPPORT:    msg = "EAFNOSUPPORT: address family not supported"; break;
		case EMFILE:          msg = "EMFILE: process file table overflow"; break;
		case ENFILE:          msg = "ENFILE: system limit reached"; break;
		case EPROTONOSUPPORT: msg = "EPROTONOSUPPORT: protocol not supported"; break;
		case EAGAIN:          msg = "EGAIN: blocking operation"; break;
		case EBADF:           msg = "EBADF: invalid descriptor"; break;
		case ECONNRESET:      msg = "ECONNRESET: connection reset by peer"; break;
		case EFAULT:          msg = "EFAULT: bad user space address"; break;
		case EINTR:           msg = "EINTR: signal occurred before transmission"; break;
		case EINVAL:          msg = "EINVAL: invalid argument"; break;
		case EMSGSIZE:        msg = "EMSGSIZE: message cannot be sent atomically"; break;
		case ENOBUFS:         msg = "ENOBUFS: output queue is full"; break;
		case ENOMEM:          msg = "ENOMEM: out of memory"; break;
		case EPIPE:           msg = "EPIPE: broken pipe"; break;
		case ENOTCONN:        msg = "ENOTCONN: not connected"; break;
		case ECONNREFUSED:    msg = "ECONNREFUSED: connection refused"; break;
	}

	if (msg) {

		throw exceptions::socket_exception(msg);

	} else {

		// Use strerror() to get string describing error number

#if VMIME_HAVE_STRERROR_R

		char errbuf[512];

		throw exceptions::socket_exception(
			vmime_strerror_r_result(
				strerror_r(err, errbuf, sizeof(errbuf)),
				errbuf
			)
		);

#else  // !VMIME_HAVE_STRERROR_R

		const std::string strmsg(strerror(err));
		throw exceptions::socket_exception(strmsg);

#endif  // VMIME_HAVE_STRERROR_R

	}
}


unsigned int posixSocket::getStatus() const {

	return m_status;
}


shared_ptr <net::timeoutHandler> posixSocket::getTimeoutHandler() {

	return m_timeoutHandler;
}


void posixSocket::setTracer(const shared_ptr <net::tracer>& tracer) {

	m_tracer = tracer;
}


shared_ptr <net::tracer> posixSocket::getTracer() {

	return m_tracer;
}



//
// posixSocketFactory
//

shared_ptr <vmime::net::socket> posixSocketFactory::create() {

	shared_ptr <vmime::net::timeoutHandler> th;
	return make_shared <posixSocket>(th);
}


shared_ptr <vmime::net::socket> posixSocketFactory::create(const shared_ptr <vmime::net::timeoutHandler>& th) {

	return make_shared <posixSocket>(th);
}


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_MESSAGING_FEATURES
