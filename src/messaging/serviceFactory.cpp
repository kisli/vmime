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

#include "serviceFactory.hpp"
#include "service.hpp"

#include "../exception.hpp"
#include "../config.hpp"

#include "builtinServices.inl"


namespace vmime {
namespace messaging {


serviceFactory::serviceFactory()
{
}


serviceFactory::~serviceFactory()
{
	for (ProtoMap::iterator it = m_protoMap.begin() ; it != m_protoMap.end() ; ++it)
		delete ((*it).second);
}


service* serviceFactory::create
	(session& sess, const string& protocol, authenticator* auth)
{
	ProtoMap::const_iterator pos = m_protoMap.find(toLower(protocol));

	if (pos != m_protoMap.end())
	{
		return ((*pos).second)->create(sess, auth);
	}
	else
	{
		throw exceptions::no_service_available();
		return (NULL);
	}
}


service* serviceFactory::create
	(session& sess, const url& u, authenticator* auth)
{
	service* serv = create(sess, u.protocol(), auth);

	sess.properties()[serv->infos().propertyPrefix() + "server.address"] = u.host();

	if (u.port() != url::UNSPECIFIED_PORT)
		sess.properties()[serv->infos().propertyPrefix() + "server.port"] = u.port();

	if (!u.path().empty())
		sess.properties()[serv->infos().propertyPrefix() + "server.path"] = u.path();

	if (!u.username().empty())
	{
		sess.properties()[serv->infos().propertyPrefix() + "auth.username"] = u.username();
		sess.properties()[serv->infos().propertyPrefix() + "auth.password"] = u.password();
	}

	return (serv);
}


const serviceFactory::registeredService& serviceFactory::operator[]
	(const string& protocol) const
{
	ProtoMap::const_iterator pos = m_protoMap.find(toLower(protocol));

	if (pos != m_protoMap.end())
	{
		return *((*pos).second);
	}
	else
	{
		throw exceptions::no_service_available();
	}
}


} // messaging
} // vmime
