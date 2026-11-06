//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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


#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/SASLSocket.hpp"
#include "vmime/security/sasl/SASLAuthenticator.hpp"


namespace vmime {
namespace security {
namespace sasl {


SASLSession::SASLSession(
	const string& serviceName,
	const shared_ptr <SASLContext>& ctx,
	const shared_ptr <authenticator>& auth,
	const shared_ptr <SASLMechanism>& mech
)
	: m_serviceName(serviceName),
	  m_context(ctx),
	  m_auth(auth),
	  m_mech(mech) {

}


SASLSession::~SASLSession() {

}


// static
shared_ptr <SASLSession> SASLSession::create(
	const string& serviceName,
	const shared_ptr <SASLContext>& ctx,
	const shared_ptr <authenticator>& auth,
	const shared_ptr <SASLMechanism>& mech
) {

	return shared_ptr <SASLSession>(new SASLSession(serviceName, ctx, auth, mech));
}


void SASLSession::init() {

	shared_ptr <SASLAuthenticator> saslAuth = dynamicCast <SASLAuthenticator>(m_auth);

	if (saslAuth) {
		saslAuth->setSASLMechanism(m_mech);
		saslAuth->setSASLSession(dynamicCast <SASLSession>(shared_from_this()));
	}
}


shared_ptr <authenticator> SASLSession::getAuthenticator() {

	return m_auth;
}


shared_ptr <SASLMechanism> SASLSession::getMechanism() {

	return m_mech;
}


shared_ptr <SASLContext> SASLSession::getContext() {

	return m_context;
}


bool SASLSession::evaluateChallenge(
	const byte_t* challenge,
	const size_t challengeLen,
	byte_t** response,
	size_t* responseLen
) {

	return m_mech->step(
		dynamicCast <SASLSession>(shared_from_this()),
		challenge, challengeLen, response, responseLen
	);
}


shared_ptr <net::socket> SASLSession::getSecuredSocket(const shared_ptr <net::socket>& sok) {

	return make_shared <SASLSocket>(dynamicCast <SASLSession>(shared_from_this()), sok);
}


const string SASLSession::getServiceName() const {

	return m_serviceName;
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
