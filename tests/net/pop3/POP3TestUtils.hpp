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

#include "vmime/net/pop3/POP3Connection.hpp"
#include "vmime/net/pop3/POP3Store.hpp"


class POP3TestStore : public vmime::net::pop3::POP3Store
{
public:

	POP3TestStore()
		: POP3Store(vmime::make_shared <vmime::net::session>(),
		            vmime::shared_ptr <vmime::security::authenticator>())
	{
	}
};


class POP3ConnectionTest : public vmime::net::pop3::POP3Connection
{
public:

	POP3ConnectionTest(vmime::shared_ptr <vmime::net::socket> socket,
	                   vmime::shared_ptr <vmime::net::timeoutHandler> timeoutHandler)
		: POP3Connection(vmime::make_shared <POP3TestStore>(),
		                 vmime::shared_ptr <vmime::security::authenticator>()),
		  m_socket(socket), m_timeoutHandler(timeoutHandler)
	{
	}

	vmime::shared_ptr <vmime::net::socket> getSocket()
	{
		return m_socket;
	}

	vmime::shared_ptr <vmime::net::timeoutHandler> getTimeoutHandler()
	{
		return m_timeoutHandler;
	}

private:

	vmime::shared_ptr <vmime::net::socket> m_socket;
	vmime::shared_ptr <vmime::net::timeoutHandler> m_timeoutHandler;
};
