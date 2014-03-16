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


#include "vmime/net/service.hpp"

#include "vmime/platform.hpp"

#include "vmime/net/defaultTimeoutHandler.hpp"

#if VMIME_HAVE_SASL_SUPPORT
	#include "vmime/security/sasl/defaultSASLAuthenticator.hpp"
#else
	#include "vmime/security/defaultAuthenticator.hpp"
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	#include "vmime/security/cert/defaultCertificateVerifier.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT


namespace vmime {
namespace net {


service::service(shared_ptr <session> sess, const serviceInfos& /* infos */,
                 shared_ptr <security::authenticator> auth)
	: m_session(sess), m_auth(auth)
{
	if (!auth)
	{
#if VMIME_HAVE_SASL_SUPPORT
		m_auth = make_shared
			<security::sasl::defaultSASLAuthenticator>();
#else
		m_auth = make_shared
			<security::defaultAuthenticator>();
#endif // VMIME_HAVE_SASL_SUPPORT
	}

#if VMIME_HAVE_TLS_SUPPORT
	m_certVerifier = make_shared <security::cert::defaultCertificateVerifier>();
#endif // VMIME_HAVE_TLS_SUPPORT

	m_socketFactory = platform::getHandler()->getSocketFactory();

	m_toHandlerFactory = make_shared <defaultTimeoutHandlerFactory>();
}


service::~service()
{
}


shared_ptr <const session> service::getSession() const
{
	return (m_session);
}


shared_ptr <session> service::getSession()
{
	return (m_session);
}


shared_ptr <const security::authenticator> service::getAuthenticator() const
{
	return (m_auth);
}


shared_ptr <security::authenticator> service::getAuthenticator()
{
	return (m_auth);
}


void service::setAuthenticator(shared_ptr <security::authenticator> auth)
{
	m_auth = auth;
}


#if VMIME_HAVE_TLS_SUPPORT

void service::setCertificateVerifier(shared_ptr <security::cert::certificateVerifier> cv)
{
	m_certVerifier = cv;
}


shared_ptr <security::cert::certificateVerifier> service::getCertificateVerifier()
{
	return m_certVerifier;
}

#endif // VMIME_HAVE_TLS_SUPPORT


void service::setSocketFactory(shared_ptr <socketFactory> sf)
{
	m_socketFactory = sf;
}


shared_ptr <socketFactory> service::getSocketFactory()
{
	return m_socketFactory;
}


void service::setTracerFactory(shared_ptr <tracerFactory> tf)
{
	m_tracerFactory = tf;
}


shared_ptr <tracerFactory> service::getTracerFactory()
{
	return m_tracerFactory;
}


void service::setTimeoutHandlerFactory(shared_ptr <timeoutHandlerFactory> thf)
{
	m_toHandlerFactory = thf;
}


shared_ptr <timeoutHandlerFactory> service::getTimeoutHandlerFactory()
{
	return m_toHandlerFactory;
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

