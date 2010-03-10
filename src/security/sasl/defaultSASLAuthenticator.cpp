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

#include "vmime/security/sasl/defaultSASLAuthenticator.hpp"

#include "vmime/security/sasl/SASLMechanism.hpp"
#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/net/service.hpp"


namespace vmime {
namespace security {
namespace sasl {


defaultSASLAuthenticator::defaultSASLAuthenticator()
{
}


defaultSASLAuthenticator::~defaultSASLAuthenticator()
{
}


const std::vector <ref <SASLMechanism> >
	defaultSASLAuthenticator::getAcceptableMechanisms
		(const std::vector <ref <SASLMechanism> >& available,
		 ref <SASLMechanism> suggested) const
{
	if (suggested)
	{
		std::vector <ref <SASLMechanism> > res;

		res.push_back(suggested);

		for (unsigned int i = 0 ; i < available.size() ; ++i)
		{
			if (available[i]->getName() != suggested->getName())
				res.push_back(available[i]);
		}

		return res;
	}
	else
	{
		return available;
	}
}


const string defaultSASLAuthenticator::getUsername() const
{
	return m_default.getUsername();
}


const string defaultSASLAuthenticator::getPassword() const
{
	return m_default.getPassword();
}


const string defaultSASLAuthenticator::getHostname() const
{
	return m_default.getHostname();
}


const string defaultSASLAuthenticator::getAnonymousToken() const
{
	return m_default.getAnonymousToken();
}


const string defaultSASLAuthenticator::getServiceName() const
{
	return m_saslSession.acquire()->getServiceName();
}


void defaultSASLAuthenticator::setService(ref <net::service> serv)
{
	m_service = serv;
	m_default.setService(serv);
}


weak_ref <net::service> defaultSASLAuthenticator::getService() const
{
	return m_service;
}


void defaultSASLAuthenticator::setSASLSession(ref <SASLSession> sess)
{
	m_saslSession = sess;
}


ref <SASLSession> defaultSASLAuthenticator::getSASLSession() const
{
	return m_saslSession.acquire().constCast <SASLSession>();
}


void defaultSASLAuthenticator::setSASLMechanism(ref <SASLMechanism> mech)
{
	m_saslMech = mech;
}


ref <SASLMechanism> defaultSASLAuthenticator::getSASLMechanism() const
{
	return m_saslMech;
}


} // sasl
} // security
} // vmime

