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

#ifndef VMIME_NET_SOCKET_HPP_INCLUDED
#define VMIME_NET_SOCKET_HPP_INCLUDED


#include "vmime/base.hpp"


namespace vmime {
namespace net {


/** Interface for connecting to servers.
  */

class socket : public object
{
public:

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
	virtual const bool isConnected() const = 0;

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
	virtual const int receiveRaw(char* buffer, const int count) = 0;

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
	virtual void sendRaw(const char* buffer, const int count) = 0;

protected:

	socket() { }

private:

	socket(const socket&) : object() { }
};


/** A class to create 'socket' objects.
  */

class socketFactory
{
public:

	virtual ~socketFactory() { }

	virtual ref <socket> create() = 0;
};


} // net
} // vmime


#endif // VMIME_NET_SOCKET_HPP_INCLUDED
