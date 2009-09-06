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

#include "vmime/config.hpp"
#include "vmime/net/service.hpp"

#include "vmime/platform.hpp"

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


service::service(ref <session> sess, const serviceInfos& /* infos */,
                 ref <security::authenticator> auth)
	: m_session(sess), m_auth(auth)
{
	if (!auth)
	{
#if VMIME_HAVE_SASL_SUPPORT
		m_auth = vmime::create
			<security::sasl::defaultSASLAuthenticator>();
#else
		m_auth = vmime::create
			<security::defaultAuthenticator>();
#endif // VMIME_HAVE_SASL_SUPPORT
	}

#if VMIME_HAVE_TLS_SUPPORT
	m_certVerifier = vmime::create <security::cert::defaultCertificateVerifier>();
#endif // VMIME_HAVE_TLS_SUPPORT

	m_socketFactory = platform::getHandler()->getSocketFactory();
}


service::~service()
{
}


ref <const session> service::getSession() const
{
	return (m_session);
}


ref <session> service::getSession()
{
	return (m_session);
}


ref <const security::authenticator> service::getAuthenticator() const
{
	return (m_auth);
}


ref <security::authenticator> service::getAuthenticator()
{
	return (m_auth);
}


void service::setAuthenticator(ref <security::authenticator> auth)
{
	m_auth = auth;
}


#if VMIME_HAVE_TLS_SUPPORT

void service::setCertificateVerifier(ref <security::cert::certificateVerifier> cv)
{
	m_certVerifier = cv;
}


ref <security::cert::certificateVerifier> service::getCertificateVerifier()
{
	return m_certVerifier;
}

#endif // VMIME_HAVE_TLS_SUPPORT


void service::setSocketFactory(ref <socketFactory> sf)
{
	m_socketFactory = sf;
}


ref <socketFactory> service::getSocketFactory()
{
	return m_socketFactory;
}


void service::setTimeoutHandlerFactory(ref <timeoutHandlerFactory> thf)
{
	m_toHandlerFactory = thf;
}


ref <timeoutHandlerFactory> service::getTimeoutHandlerFactory()
{
	return m_toHandlerFactory;
}


} // net
} // vmime
