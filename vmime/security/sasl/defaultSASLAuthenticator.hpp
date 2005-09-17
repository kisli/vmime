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

#ifndef VMIME_SECURITY_SASL_DEFAULTSASLAUTHENTICATOR_HPP_INCLUDED
#define VMIME_SECURITY_SASL_DEFAULTSASLAUTHENTICATOR_HPP_INCLUDED


#include "vmime/security/sasl/SASLAuthenticator.hpp"
#include "vmime/security/defaultAuthenticator.hpp"


namespace vmime {
namespace security {
namespace sasl {


/** An authenticator that is capable of providing information
  * for simple authentication mechanisms (username and password).
  */
class defaultSASLAuthenticator : public SASLAuthenticator
{
public:

	defaultSASLAuthenticator();
	~defaultSASLAuthenticator();

	const std::vector <ref <SASLMechanism> > getAcceptableMechanisms
		(const std::vector <ref <SASLMechanism> >& available,
	         ref <SASLMechanism> suggested) const;

	const string getUsername() const;
	const string getPassword() const;
	const string getHostname() const;
	const string getAnonymousToken() const;
	const string getServiceName() const;

	void setService(ref <net::service> serv);
	weak_ref <net::service> getService() const;

	void setSASLSession(ref <SASLSession> sess);
	ref <SASLSession> getSASLSession() const;

	void setSASLMechanism(ref <SASLMechanism> mech);
	ref <SASLMechanism> getSASLMechanism() const;

private:

	defaultAuthenticator m_default;


	weak_ref <net::service> m_service;

	ref <SASLSession> m_saslSession;
	ref <SASLMechanism> m_saslMech;
};


} // sasl
} // security
} // vmime


#endif // VMIME_SECURITY_SASL_DEFAULTSASLAUTHENTICATOR_HPP_INCLUDED

