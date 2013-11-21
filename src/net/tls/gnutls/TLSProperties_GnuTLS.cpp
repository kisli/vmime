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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS


#include "vmime/base.hpp"
#include "vmime/net/tls/gnutls/TLSProperties_GnuTLS.hpp"

#include <gnutls/gnutls.h>
#if GNUTLS_VERSION_NUMBER < 0x030000
#include <gnutls/extra.h>
#endif


namespace vmime {
namespace net {
namespace tls {


TLSProperties::TLSProperties()
	: m_data(make_shared <TLSProperties_GnuTLS>())
{
	setCipherSuite(CIPHERSUITE_DEFAULT);
}


TLSProperties::TLSProperties(const TLSProperties& props)
	: object(),
	  m_data(make_shared <TLSProperties_GnuTLS>())
{
	*dynamicCast <TLSProperties_GnuTLS>(m_data) = *dynamicCast <TLSProperties_GnuTLS>(props.m_data);
}


void TLSProperties::setCipherSuite(const GenericCipherSuite cipherSuite)
{
	switch (cipherSuite)
	{
	case CIPHERSUITE_HIGH:

		setCipherSuite("SECURE256:%SSL3_RECORD_VERSION");
		break;

	case CIPHERSUITE_MEDIUM:

		setCipherSuite("SECURE128:%SSL3_RECORD_VERSION");
		break;

	case CIPHERSUITE_LOW:

		setCipherSuite("NORMAL:%SSL3_RECORD_VERSION");
		break;

	default:
	case CIPHERSUITE_DEFAULT:

		setCipherSuite("NORMAL:%SSL3_RECORD_VERSION");
		break;
	}
}


void TLSProperties::setCipherSuite(const string& cipherSuite)
{
	dynamicCast <TLSProperties_GnuTLS>(m_data)->cipherSuite = cipherSuite;
}


const string TLSProperties::getCipherSuite() const
{
	return dynamicCast <TLSProperties_GnuTLS>(m_data)->cipherSuite;
}



TLSProperties_GnuTLS& TLSProperties_GnuTLS::operator=(const TLSProperties_GnuTLS& other)
{
	cipherSuite = other.cipherSuite;

	return *this;
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS
