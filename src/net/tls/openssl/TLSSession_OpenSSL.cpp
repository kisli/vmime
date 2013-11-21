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


#include "vmime/net/tls/openssl/TLSSession_OpenSSL.hpp"
#include "vmime/net/tls/openssl/TLSProperties_OpenSSL.hpp"
#include "vmime/net/tls/openssl/OpenSSLInitializer.hpp"

#include "vmime/exception.hpp"

#include <openssl/ssl.h>
#include <openssl/err.h>


namespace vmime {
namespace net {
namespace tls {


static OpenSSLInitializer::autoInitializer openSSLInitializer;


// static
shared_ptr <TLSSession> TLSSession::create(shared_ptr <security::cert::certificateVerifier> cv, shared_ptr <TLSProperties> props)
{
	return make_shared <TLSSession_OpenSSL>(cv, props);
}


TLSSession_OpenSSL::TLSSession_OpenSSL(shared_ptr <vmime::security::cert::certificateVerifier> cv, shared_ptr <TLSProperties> props)
	: m_sslctx(0), m_certVerifier(cv), m_props(props)
{
	m_sslctx = SSL_CTX_new(SSLv23_client_method());
	SSL_CTX_set_options(m_sslctx, SSL_OP_ALL | SSL_OP_NO_SSLv2);
	SSL_CTX_set_mode(m_sslctx, SSL_MODE_AUTO_RETRY);
	SSL_CTX_set_cipher_list(m_sslctx, m_props->getCipherSuite().c_str());
	SSL_CTX_set_session_cache_mode(m_sslctx, SSL_SESS_CACHE_OFF);
}


TLSSession_OpenSSL::TLSSession_OpenSSL(const TLSSession_OpenSSL&)
	: TLSSession()
{
	// Not used
}


TLSSession_OpenSSL::~TLSSession_OpenSSL()
{
	SSL_CTX_free(m_sslctx);
}


shared_ptr <TLSSocket> TLSSession_OpenSSL::getSocket(shared_ptr <socket> sok)
{
	return TLSSocket::wrap(dynamicCast <TLSSession>(shared_from_this()), sok);
}


shared_ptr <security::cert::certificateVerifier> TLSSession_OpenSSL::getCertificateVerifier()
{
	return m_certVerifier;
}


void TLSSession_OpenSSL::usePrivateKeyFile(const vmime::string& keyfile)
{
	if (SSL_CTX_use_PrivateKey_file(m_sslctx, keyfile.c_str(), SSL_FILETYPE_PEM) != 1)
	{
		unsigned long errCode = ERR_get_error();
		char buffer[256];
		ERR_error_string_n(errCode, buffer, sizeof(buffer));
		vmime::string sslErr(buffer);
		std::ostringstream oss;
		oss << "Error loading private key from file " << keyfile;
		oss << " - msg: " << sslErr;
		throw exceptions::certificate_exception(oss.str());
	}
}


void TLSSession_OpenSSL::useCertificateChainFile(const vmime::string& chainFile)
{
	if (SSL_CTX_use_certificate_chain_file(m_sslctx, chainFile.c_str()) != 1)
	{
		unsigned long errCode = ERR_get_error();
		char buffer[256];
		ERR_error_string_n(errCode, buffer, sizeof(buffer));
		vmime::string sslErr(buffer);
		std::ostringstream oss;
		oss << "Error loading certificate from file " << chainFile;
		oss << " - msg: " << sslErr;
		throw exceptions::certificate_exception(oss.str());
	}
}


SSL_CTX* TLSSession_OpenSSL::getContext() const
{
	return m_sslctx;
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

