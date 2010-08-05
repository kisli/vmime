//
// VMime library (http://www.vmime.org)
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

#ifndef VMIME_NET_SOCKET_HPP_INCLUDED
#define VMIME_NET_SOCKET_HPP_INCLUDED


#include "vmime/base.hpp"

#include "vmime/net/timeoutHandler.hpp"


namespace vmime {
namespace net {


/** Interface for connecting to servers.
  */

class socket : public object
{
public:

	virtual ~socket() { }

	/** Type used for lengths in streams.
	  */
	typedef int size_type;


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

	/** Receive (text) data from the socket.
	  *
	  * @param buffer buffer in which to write received data
	  */
	virtual void receive(string& buffer) = 0;

	/** Receive (raw) data from the socket.
	  *
	  * @param buffer buffer in which to write received data
	  * @param count maximum number of bytes to receive (size of buffer)
	  * @return number of bytes received/written into output buffer
	  */
	virtual int receiveRaw(char* buffer, const size_type count) = 0;

	/** Send (text) data to the socket.
	  *
	  * @param buffer data to send
	  */
	virtual void send(const string& buffer) = 0;

	/** Send (raw) data to the socket.
	  *
	  * @param buffer data to send
	  * @param count number of bytes to send (size of buffer)
	  */
	virtual void sendRaw(const char* buffer, const size_type count) = 0;

	/** Return the preferred maximum block size when reading
	  * from or writing to this stream.
	  *
	  * @return block size, in bytes
	  */
	virtual size_type getBlockSize() const = 0;

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
	virtual ref <socket> create() = 0;

	/** Creates a socket with the specified timeout handler.
	  *
	  * @param th timeout handler
	  * @return a new socket
	  */
	virtual ref <socket> create(ref <timeoutHandler> th) = 0;
};


} // net
} // vmime


#endif // VMIME_NET_SOCKET_HPP_INCLUDED
