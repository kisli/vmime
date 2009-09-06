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

#ifndef VMIME_SECURITY_SASL_SASLSESSION_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLSESSION_HPP_INCLUDED


#include "vmime/types.hpp"

#include "vmime/security/sasl/SASLAuthenticator.hpp"
#include "vmime/security/sasl/SASLMechanism.hpp"
#include "vmime/security/sasl/SASLSocket.hpp"


namespace vmime {
namespace security {
namespace sasl {


class SASLContext;


/** An SASL client session.
  */
class SASLSession : public object
{
	friend class builtinSASLMechanism;
	friend class SASLSocket;

public:

	~SASLSession();

	/** Construct a new SASL session.
	  *
	  * @param serviceName name of the service using this session
	  * @param ctx SASL context
	  * @param auth authenticator to use for this session
	  * @param mech SASL mechanism
	  */
	SASLSession(const string& serviceName, ref <SASLContext> ctx,
	        ref <authenticator> auth, ref <SASLMechanism> mech);

	/** Initialize this SASL session. This must be called before
	  * calling any other method on this object (except accessors).
	  */
	void init();

	/** Return the authenticator used for this session. This is the
	  * authenticator which has been previously set with a call to
	  * setAuthenticator().
	  *
	  * @return authenticator object
	  */
	ref <authenticator> getAuthenticator();

	/** Return the mechanism used for this session.
	  *
	  * @return SASL mechanism
	  */
	ref <SASLMechanism> getMechanism();

	/** Return the SASL context.
	  *
	  * @return SASL context
	  */
	ref <SASLContext> getContext();

	/** Perform one step of SASL authentication. Accept data from the
	  * server (challenge), process it and return data to be returned
	  * in response to the server.
	  *
	  * @param challenge challenge sent from the server
	  * @param challengeLen length of challenge
	  * @param response response to send to the server (allocated by
	  * this function, free with delete[])
	  * @param responseLen length of response buffer
	  * @return true if authentication terminated successfully, or
	  * false if the authentication process should continue
	  * @throw exceptions::sasl_exception if an error occured during
	  * authentication (in this case, the values in 'response' and
	  * 'responseLen' are undetermined)
	  */
	bool evaluateChallenge
		(const byte_t* challenge, const int challengeLen,
		 byte_t** response, int* responseLen);

	/** Return a socket in which transmitted data is integrity
	  * and/or privacy protected, depending on the QOP (Quality of
	  * Protection) negotiated during the SASL authentication.
	  *
	  * @param sok socket to wrap
	  * @return secured socket
	  */
	ref <net::socket> getSecuredSocket(ref <net::socket> sok);

	/** Return the name of the service which is using this
	  * SASL session (eg. "imap"). This value should be returned
	  * by the authenticator when INFO_SERVICE is requested.
	  *
	  * @return service name
	  */
	const string getServiceName() const;

private:

	const string m_serviceName;

	ref <SASLContext> m_context;
	ref <authenticator> m_auth;
	ref <SASLMechanism> m_mech;

#ifdef GSASL_VERSION
	Gsasl* m_gsaslContext;
	Gsasl_session* m_gsaslSession;

	static int gsaslCallback(Gsasl* ctx, Gsasl_session* sctx, Gsasl_property prop);
#else
	void* m_gsaslContext;
	void* m_gsaslSession;

	static int gsaslCallback(void* ctx, void* sctx, int prop);
#endif // GSASL_VERSION

};


} // sasl
} // security
} // vmime


#endif // VMIME_SECURITY_SASL_SASLSESSION_HPP_INCLUDED

