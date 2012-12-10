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

#ifndef VMIME_SECURITY_CERT_X509CERTIFICATE_OPENSSL_HPP_INCLUDED
#define VMIME_SECURITY_CERT_X509CERTIFICATE_OPENSSL_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL


#include "vmime/security/cert/X509Certificate.hpp"

#include <openssl/x509.h>


namespace vmime {
namespace security {
namespace cert {


class X509Certificate_OpenSSL : public X509Certificate
{
	friend class vmime::creator;
	friend class X509Certificate;

protected:

	X509Certificate_OpenSSL();
	X509Certificate_OpenSSL(X509* cert);
	X509Certificate_OpenSSL(const X509Certificate_OpenSSL&);

public:

	~X509Certificate_OpenSSL();


	void write(utility::outputStream& os, const Format format) const;

	const byteArray getSerialNumber() const;

	bool checkIssuer(ref <const X509Certificate> issuer) const;

	bool verify(ref <const X509Certificate> caCert) const;

	bool verifyHostName(const string& hostname) const;

	const datetime getExpirationDate() const;
	const datetime getActivationDate() const;

	const byteArray getFingerprint(const DigestAlgorithm algo) const;


	static ref <X509Certificate> importInternal(X509* cert);


	// Implementation of 'certificate'
	const byteArray getEncoded() const;
	const string getType() const;
	int getVersion() const;
	bool equals(ref <const certificate> other) const;

private:

	/** Internal utility function to convert ASN1_TIME
	 *  structs to vmime::datetime
	 *
	 *  @param pointer to ASN1_TIME struct to convert
	 */
	const datetime convertX509Date(void* time) const;

	struct OpenSSLX509CertificateInternalData* m_data;
};


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

#endif // VMIME_SECURITY_CERT_X509CERTIFICATE_OPENSSL_HPP_INCLUDED

