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

#include "vmime/messaging/serviceFactory.hpp"
#include "vmime/messaging/service.hpp"

#include "vmime/exception.hpp"
#include "vmime/config.hpp"

#include "src/messaging/builtinServices.inl"


namespace vmime {
namespace messaging {


serviceFactory::serviceFactory()
{
}


serviceFactory::~serviceFactory()
{
	for (std::vector <registeredService*>::const_iterator it = m_services.begin() ;
	     it != m_services.end() ; ++it)
	{
		delete (*it);
	}
}


service* serviceFactory::create
	(session* sess, const string& protocol, authenticator* auth)
{
	return (getServiceByProtocol(protocol)->create(sess, auth));
}


service* serviceFactory::create
	(session* sess, const url& u, authenticator* auth)
{
	service* serv = create(sess, u.getProtocol(), auth);

	sess->getProperties()[serv->getInfos().getPropertyPrefix() + "server.address"] = u.getHost();

	if (u.getPort() != url::UNSPECIFIED_PORT)
		sess->getProperties()[serv->getInfos().getPropertyPrefix() + "server.port"] = u.getPort();

	// Path portion of the URL is used to point a specific folder (empty = root).
	// In maildir, this is used to point to the root of the message repository.
	if (!u.getPath().empty())
		sess->getProperties()[serv->getInfos().getPropertyPrefix() + "server.rootpath"] = u.getPath();

	if (!u.getUsername().empty())
	{
		sess->getProperties()[serv->getInfos().getPropertyPrefix() + "auth.username"] = u.getUsername();
		sess->getProperties()[serv->getInfos().getPropertyPrefix() + "auth.password"] = u.getPassword();
	}

	return (serv);
}


const serviceFactory::registeredService* serviceFactory::getServiceByProtocol(const string& protocol) const
{
	const string name(utility::stringUtils::toLower(protocol));

	for (std::vector <registeredService*>::const_iterator it = m_services.begin() ;
	     it != m_services.end() ; ++it)
	{
		if ((*it)->getName() == name)
			return (*it);
	}

	throw exceptions::no_service_available(name);
}


const int serviceFactory::getServiceCount() const
{
	return (m_services.size());
}


const serviceFactory::registeredService* serviceFactory::getServiceAt(const int pos) const
{
	return (m_services[pos]);
}


const std::vector <const serviceFactory::registeredService*> serviceFactory::getServiceList() const
{
	std::vector <const registeredService*> res;

	for (std::vector <registeredService*>::const_iterator it = m_services.begin() ;
	     it != m_services.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


} // messaging
} // vmime
