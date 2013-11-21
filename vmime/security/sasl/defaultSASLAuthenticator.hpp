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

#ifndef VMIME_SECURITY_SASL_DEFAULTSASLAUTHENTICATOR_HPP_INCLUDED
#define VMIME_SECURITY_SASL_DEFAULTSASLAUTHENTICATOR_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/security/sasl/SASLAuthenticator.hpp"
#include "vmime/security/defaultAuthenticator.hpp"


namespace vmime {
namespace security {
namespace sasl {


/** An authenticator that is capable of providing information
  * for simple authentication mechanisms (username and password).
  */
class VMIME_EXPORT defaultSASLAuthenticator : public SASLAuthenticator
{
public:

	defaultSASLAuthenticator();
	~defaultSASLAuthenticator();

	const std::vector <shared_ptr <SASLMechanism> > getAcceptableMechanisms
		(const std::vector <shared_ptr <SASLMechanism> >& available,
	         shared_ptr <SASLMechanism> suggested) const;

	const string getUsername() const;
	const string getPassword() const;
	const string getHostname() const;
	const string getAnonymousToken() const;
	const string getServiceName() const;

	void setService(shared_ptr <net::service> serv);
	weak_ptr <net::service> getService() const;

	void setSASLSession(shared_ptr <SASLSession> sess);
	shared_ptr <SASLSession> getSASLSession() const;

	void setSASLMechanism(shared_ptr <SASLMechanism> mech);
	shared_ptr <SASLMechanism> getSASLMechanism() const;

private:

	defaultAuthenticator m_default;

	weak_ptr <net::service> m_service;
	weak_ptr <SASLSession> m_saslSession;
	shared_ptr <SASLMechanism> m_saslMech;
};


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

#endif // VMIME_SECURITY_SASL_DEFAULTSASLAUTHENTICATOR_HPP_INCLUDED

