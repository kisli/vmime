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

#ifndef VMIME_SECURITY_SASL_SASLCONTEXT_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLCONTEXT_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/types.hpp"

#include "vmime/security/sasl/SASLSession.hpp"
#include "vmime/security/sasl/SASLMechanismFactory.hpp"


namespace vmime {
namespace security {
namespace sasl {


/** An SASL client context.
  */
class VMIME_EXPORT SASLContext : public object
{
	friend class SASLSession;
	friend class builtinSASLMechanism;

public:

	~SASLContext();

	/** Construct and initialize a new SASL context.
	  */
	SASLContext();

	/** Create and initialize a new SASL session.
	  *
	  * @param serviceName name of the service which will use the session
	  * @param auth authenticator object to use during the session
	  * @param mech SASL mechanism
	  * @return a new SASL session
	  */
	shared_ptr <SASLSession> createSession
		(const string& serviceName,
		 shared_ptr <authenticator> auth, shared_ptr <SASLMechanism> mech);

	/** Create an instance of an SASL mechanism.
	  *
	  * @param name mechanism name
	  * @return a new instance of the specified SASL mechanism
	  * @throw exceptions::no_such_mechanism if no mechanism is
	  * registered for the specified name
	  */
	shared_ptr <SASLMechanism> createMechanism(const string& name);

	/** Suggests an SASL mechanism among a set of mechanisms
	  * supported by the server.
	  *
	  * @param mechs list of mechanisms
	  * @return suggested mechanism (usually the safest mechanism
	  * supported by both the client and the server)
	  */
	shared_ptr <SASLMechanism> suggestMechanism
		(const std::vector <shared_ptr <SASLMechanism> >& mechs);

	/** Helper function for decoding Base64-encoded challenge.
	  *
	  * @param input input buffer
	  * @param output output buffer
	  * @param outputLen length of output buffer
	  */
	void decodeB64(const string& input, byte_t** output, size_t* outputLen);

	/** Helper function for encoding challenge in Base64.
	  *
	  * @param input input buffer
	  * @param inputLen length of input buffer
	  * @return Base64-encoded challenge
	  */
	const string encodeB64(const byte_t* input, const size_t inputLen);

private:

	static const string getErrorMessage(const string& fname, const int code);


#ifdef GSASL_VERSION
	Gsasl* m_gsaslContext;
#else
	void* m_gsaslContext;
#endif // GSASL_VERSION

};


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

#endif // VMIME_SECURITY_SASL_SASLCONTEXT_HPP_INCLUDED

