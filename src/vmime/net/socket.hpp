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

#ifndef VMIME_NET_SOCKET_HPP_INCLUDED
#define VMIME_NET_SOCKET_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/base.hpp"

#include "vmime/net/timeoutHandler.hpp"
#include "vmime/net/tracer.hpp"


namespace vmime {
namespace net {


/** Interface for connecting to servers.
  */

class VMIME_EXPORT socket : public object
{
public:

	enum Status
	{
		STATUS_WOULDBLOCK = 0xf,    /**< The operation would block. Retry later. */
		STATUS_WANT_READ = 0x1,     /**< The socket wants to read data, retry when data is available. */
		STATUS_WANT_WRITE = 0x2     /**< The socket wants to write data, retry when data can be written. */
	};


	virtual ~socket() { }


	/** Connect to the specified address and port.
	  *
	  * @param address server address (this can be a full qualified domain name
	  * or an IP address, doesn't matter)
	  * @param port server port
	  */
	virtual void connect(const string& address, const port_t port) = 0;

	/** Disconnect from the server.
	  */
	virtual void disconnect() = 0;

	/** Test whether this socket is connected.
	  *
	  * @return true if the socket is connected, false otherwise
	  */
	virtual bool isConnected() const = 0;

	/** Block until new data is available for reading. The function will
	  * timeout after msecs milliseconds.
	  *
	  * @param timeout maximum wait time, in milliseconds (default is 30000);
	  * resolution is 10ms
	  * @return true if data is available, or false if the operation timed out
	  */
	virtual bool waitForRead(const int msecs = 30000) = 0;

	/** Block until pending data has been written and new data can be written.
	  * The function will timeout after msecs milliseconds.
	  *
	  * @param timeout maximum wait time, in milliseconds (default is 30000);
	  * resolution is 10ms
	  * @return true if new data can be written immediately, or false if the
	  * operation timed out
	  */
	virtual bool waitForWrite(const int msecs = 30000) = 0;

	/** Receive text data from the socket.
	  *
	  * @param buffer buffer in which to write received data
	  */
	virtual void receive(string& buffer) = 0;

	/** Receive raw data from the socket.
	  *
	  * @param buffer buffer in which to write received data
	  * @param count maximum number of bytes to receive (size of buffer)
	  * @return number of bytes received/written into output buffer
	  */
	virtual size_t receiveRaw(byte_t* buffer, const size_t count) = 0;

	/** Send text data to the socket.
	  *
	  * @param buffer data to send
	  */
	virtual void send(const string& buffer) = 0;

	/** Send text data to the socket.
	  *
	  * @param str null-terminated string
	  */
	virtual void send(const char* str) = 0;

	/** Send raw data to the socket.
	  *
	  * @param buffer data to send
	  * @param count number of bytes to send (size of buffer)
	  */
	virtual void sendRaw(const byte_t* buffer, const size_t count) = 0;

	/** Send raw data to the socket.
	  * Function may returns before all data is sent.
	  *
	  * @param buffer data to send
	  * @param count number of bytes to send (size of buffer)
	  * @return number of bytes sent
	  */
	virtual size_t sendRawNonBlocking(const byte_t* buffer, const size_t count) = 0;

	/** Return the preferred maximum block size when reading
	  * from or writing to this stream.
	  *
	  * @return block size, in bytes
	  */
	virtual size_t getBlockSize() const = 0;

	/** Return the current status of this socket.
	  *
	  * @return status flags for this socket
	  */
	virtual unsigned int getStatus() const = 0;

	/** Return the hostname of peer this socket is connected to.
	  *
	  * @return name of the peer, or numeric address if it cannot be found
	  */
	virtual const string getPeerName() const = 0;

	/** Return the address of peer this socket is connected to.
	  *
	  * @return numeric address of the peer
	  */
	virtual const string getPeerAddress() const = 0;

	/** Return the timeout handler associated with this socket.
	  *
	  * @return timeout handler, or NULL if none is set
	  */
	virtual shared_ptr <timeoutHandler> getTimeoutHandler() = 0;

	/** Set the tracer used by this socket. Tracer will only be used
	  * to report socket-specific events such as connection (not when
	  * sending/receiving data).
	  *
	  * @param tracer tracer to use
	  */
	virtual void setTracer(shared_ptr <tracer> tracer) = 0;

	/** Return the tracer used by this socket.
	  *
	  * @return tracer, or NULL if none is set
	  */
	virtual shared_ptr <tracer> getTracer() = 0;

protected:

	socket() { }

private:

	socket(const socket&) : object() { }
};


/** A class to create 'socket' objects.
  */

class socketFactory : public object
{
public:

	virtual ~socketFactory() { }

	/** Creates a socket without timeout handler.
	  *
	  * @return a new socket
	  */
	virtual shared_ptr <socket> create() = 0;

	/** Creates a socket with the specified timeout handler.
	  *
	  * @param th timeout handler
	  * @return a new socket
	  */
	virtual shared_ptr <socket> create(shared_ptr <timeoutHandler> th) = 0;
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_NET_SOCKET_HPP_INCLUDED
