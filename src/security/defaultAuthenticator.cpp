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

#include "vmime/security/defaultAuthenticator.hpp"

#include "vmime/net/service.hpp"

#include "vmime/platformDependant.hpp"


namespace vmime {
namespace security {


defaultAuthenticator::defaultAuthenticator()
{
}


defaultAuthenticator::~defaultAuthenticator()
{
}


const string defaultAuthenticator::getUsername() const
{
	const string& prefix = m_service->getInfos().getPropertyPrefix();
	const propertySet& props = m_service->getSession()->getProperties();

	if (props.hasProperty(prefix + net::serviceInfos::property::AUTH_USERNAME.getName()))
		return props[prefix + net::serviceInfos::property::AUTH_USERNAME.getName()];

	throw exceptions::no_auth_information();
}


const string defaultAuthenticator::getPassword() const
{
	const string& prefix = m_service->getInfos().getPropertyPrefix();
	const propertySet& props = m_service->getSession()->getProperties();

	if (props.hasProperty(prefix + net::serviceInfos::property::AUTH_PASSWORD.getName()))
		return props[prefix + net::serviceInfos::property::AUTH_PASSWORD.getName()];

	throw exceptions::no_auth_information();
}


const string defaultAuthenticator::getHostname() const
{
	return platformDependant::getHandler()->getHostName();
}


const string defaultAuthenticator::getAnonymousToken() const
{
	return "anonymous@" + platformDependant::getHandler()->getHostName();
}


const string defaultAuthenticator::getServiceName() const
{
	// Information cannot be provided
	throw exceptions::no_auth_information();
}


void defaultAuthenticator::setService(ref <net::service> serv)
{
	m_service = serv;
}


weak_ref <net::service> defaultAuthenticator::getService() const
{
	return m_service;
}


} // security
} // vmime

