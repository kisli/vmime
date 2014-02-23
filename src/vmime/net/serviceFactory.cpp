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


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/serviceFactory.hpp"
#include "vmime/net/service.hpp"

#include "vmime/exception.hpp"

#include "vmime/net/builtinServices.inl"


namespace vmime {
namespace net {


serviceFactory::serviceFactory()
{
}


serviceFactory::~serviceFactory()
{
}


shared_ptr <serviceFactory> serviceFactory::getInstance()
{
	static serviceFactory instance;
	return shared_ptr <serviceFactory>(&instance, noop_shared_ptr_deleter <serviceFactory>());
}


shared_ptr <service> serviceFactory::create
	(shared_ptr <session> sess, const string& protocol,
	 shared_ptr <security::authenticator> auth)
{
	shared_ptr <const registeredService> rserv = getServiceByProtocol(protocol);

	if (!rserv)
		throw exceptions::no_factory_available("No service is registered for protocol '" + protocol + "'.");

	return rserv->create(sess, auth);
}


shared_ptr <service> serviceFactory::create
	(shared_ptr <session> sess, const utility::url& u,
	 shared_ptr <security::authenticator> auth)
{
	shared_ptr <service> serv = create(sess, u.getProtocol(), auth);

	sess->getProperties()[serv->getInfos().getPropertyPrefix() + "server.address"] = u.getHost();

	if (u.getPort() != utility::url::UNSPECIFIED_PORT)
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


shared_ptr <const serviceFactory::registeredService> serviceFactory::getServiceByProtocol(const string& protocol) const
{
	const string name(utility::stringUtils::toLower(protocol));

	for (std::vector <shared_ptr <registeredService> >::const_iterator it = m_services.begin() ;
	     it != m_services.end() ; ++it)
	{
		if ((*it)->getName() == name)
			return (*it);
	}

	return null;
}


size_t serviceFactory::getServiceCount() const
{
	return (m_services.size());
}


shared_ptr <const serviceFactory::registeredService> serviceFactory::getServiceAt(const size_t pos) const
{
	return (m_services[pos]);
}


const std::vector <shared_ptr <const serviceFactory::registeredService> > serviceFactory::getServiceList() const
{
	std::vector <shared_ptr <const registeredService> > res;

	for (std::vector <shared_ptr <registeredService> >::const_iterator it = m_services.begin() ;
	     it != m_services.end() ; ++it)
	{
		res.push_back(*it);
	}

	return (res);
}


void serviceFactory::registerService(shared_ptr <registeredService> reg)
{
	m_services.push_back(reg);
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

