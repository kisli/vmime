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

#include "messaging/service.hpp"

#include "messaging/defaultAuthenticator.hpp"


namespace vmime {
namespace messaging {


service::service(session* sess, const serviceInfos& infos, authenticator* auth)
	: m_deleteAuth(auth == NULL), m_session(sess), m_auth(auth ? auth :
	  	new defaultAuthenticator(sess->getProperties(), infos.getPropertyPrefix()))
{
}


service::~service()
{
	if (m_deleteAuth)
		delete (m_auth);
}


const session* service::getSession() const
{
	return (m_session);
}


session* service::getSession()
{
	return (m_session);
}


const authenticator* service::getAuthenticator() const
{
	return (m_auth);
}


authenticator* service::getAuthenticator()
{
	return (m_auth);
}


} // messaging
} // vmime
