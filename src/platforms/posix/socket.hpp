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

#ifndef VMIME_PLATFORMS_POSIX_SOCKET_HPP_INCLUDED
#define VMIME_PLATFORMS_POSIX_SOCKET_HPP_INCLUDED


#include "../../messaging/socket.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


namespace vmime {
namespace platforms {
namespace posix {


class posixSocket : public vmime::messaging::socket
{
public:

	posixSocket();
	~posixSocket();

	void connect(const vmime::string& address, const vmime::port_t port);
	const bool isConnected() const;
	void disconnect();

	void receive(vmime::string& buffer);
	const int receiveRaw(char* buffer, const int count);

	void send(const vmime::string& buffer);
	void sendRaw(const char* buffer, const int count);

private:

	char m_buffer[65536];
	int m_desc;
};



class posixSocketFactory : public vmime::messaging::socketFactory
{
public:

	vmime::messaging::socket* create();
};


} // posix
} // platforms
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_PLATFORMS_POSIX_SOCKET_HPP_INCLUDED
