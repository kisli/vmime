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

#ifndef VMIME_SECURITY_CERT_X509CERTIFICATE_GNUTLS_HPP_INCLUDED
#define VMIME_SECURITY_CERT_X509CERTIFICATE_GNUTLS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS


#include "vmime/security/cert/X509Certificate.hpp"


namespace vmime {
namespace security {
namespace cert {


class X509Certificate_GnuTLS : public X509Certificate
{
	friend class X509Certificate;

	X509Certificate_GnuTLS(const X509Certificate&);

public:

	X509Certificate_GnuTLS();

	~X509Certificate_GnuTLS();


	void write(utility::outputStream& os, const Format format) const;

	const byteArray getSerialNumber() const;

	const string getIssuerString() const;
	bool checkIssuer(shared_ptr <const X509Certificate> issuer) const;

	bool verify(shared_ptr <const X509Certificate> caCert) const;

	bool verifyHostName
		(const string& hostname,
		 std::vector <std::string>* nonMatchingNames = NULL) const;

	const datetime getExpirationDate() const;
	const datetime getActivationDate() const;

	const byteArray getFingerprint(const DigestAlgorithm algo) const;


	// Implementation of 'certificate'
	const byteArray getEncoded() const;
	const string getType() const;
	int getVersion() const;
	bool equals(shared_ptr <const certificate> other) const;
	void* getInternalData();

private:

	struct GnuTLSX509CertificateInternalData* m_data;
};


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS

#endif // VMIME_SECURITY_CERT_X509CERTIFICATE_GNUTLS_HPP_INCLUDED

