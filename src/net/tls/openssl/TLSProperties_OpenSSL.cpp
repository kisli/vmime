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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL


#include "vmime/base.hpp"
#include "vmime/net/tls/openssl/TLSProperties_OpenSSL.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>


namespace vmime {
namespace net {
namespace tls {


TLSProperties::TLSProperties()
	: m_data(make_shared <TLSProperties_OpenSSL>())
{
	setCipherSuite(CIPHERSUITE_DEFAULT);
}


TLSProperties::TLSProperties(const TLSProperties& props)
	: object(),
	  m_data(make_shared <TLSProperties_OpenSSL>())
{
	*dynamicCast <TLSProperties_OpenSSL>(m_data) = *dynamicCast <TLSProperties_OpenSSL>(props.m_data);
}


void TLSProperties::setCipherSuite(const GenericCipherSuite cipherSuite)
{
	switch (cipherSuite)
	{
	case CIPHERSUITE_HIGH:

		setCipherSuite("HIGH:!ADH:@STRENGTH");
		break;

	case CIPHERSUITE_MEDIUM:

		setCipherSuite("MEDIUM:!ADH:@STRENGTH");
		break;

	case CIPHERSUITE_LOW:

		setCipherSuite("LOW:!ADH:@STRENGTH");
		break;

	default:
	case CIPHERSUITE_DEFAULT:

		setCipherSuite("DEFAULT:!ADH:@STRENGTH");
		break;
	}
}


void TLSProperties::setCipherSuite(const string& cipherSuite)
{
	dynamicCast <TLSProperties_OpenSSL>(m_data)->cipherSuite = cipherSuite;
}


const string TLSProperties::getCipherSuite() const
{
	return dynamicCast <TLSProperties_OpenSSL>(m_data)->cipherSuite;
}



TLSProperties_OpenSSL& TLSProperties_OpenSSL::operator=(const TLSProperties_OpenSSL& other)
{
	cipherSuite = other.cipherSuite;

	return *this;
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

