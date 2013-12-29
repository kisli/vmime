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

#ifndef VMIME_NET_TLS_TLSSESSION_OPENSSL_HPP_INCLUDED
#define VMIME_NET_TLS_TLSSESSION_OPENSSL_HPP_INCLUDED


#ifndef VMIME_BUILDING_DOC


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL


#include "vmime/types.hpp"

#include "vmime/net/tls/TLSSession.hpp"
#include "vmime/net/tls/TLSSocket.hpp"
#include "vmime/net/tls/TLSProperties.hpp"


#include <openssl/ssl.h>


namespace vmime {
namespace net {
namespace tls {


class TLSSession_OpenSSL : public TLSSession
{
	friend class TLSSocket_OpenSSL;

public:

	TLSSession_OpenSSL(const shared_ptr <security::cert::certificateVerifier> cv, shared_ptr <TLSProperties> props);
	~TLSSession_OpenSSL();


	shared_ptr <TLSSocket> getSocket(shared_ptr <socket> sok);

	shared_ptr <security::cert::certificateVerifier> getCertificateVerifier();


	/** Set the private key to use if server requires a client certificate.
	 *
	 * @param	keyfile         Path to the private key in PEM format
	 * @param passwd_callback If the private key is stored encrypted the
	*/
	void usePrivateKeyFile(const vmime::string& keyfile);

	/** Supply the certificate chain to present if requested by
	 *  server.
	 *
	 * @param chainFile	File in PEM format holding certificate chain
	 */
	void useCertificateChainFile(const vmime::string& chainFile);

	/** Get a pointer to the SSL_CTX used for this session.
	 *
	 * @return the SSL_CTX used for all connections created with this session
	 */
	SSL_CTX* getContext() const;

private:

	TLSSession_OpenSSL(const TLSSession_OpenSSL&);

	SSL_CTX* m_sslctx;

	shared_ptr <security::cert::certificateVerifier> m_certVerifier;
	shared_ptr <TLSProperties> m_props;
};


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

#endif // VMIME_BUILDING_DOC

#endif // VMIME_NET_TLS_TLSSESSION_OPENSSL_HPP_INCLUDED

