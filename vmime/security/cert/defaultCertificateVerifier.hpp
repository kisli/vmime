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

#ifndef VMIME_SECURITY_CERT_DEFAULTCERTIFICATEVERIFIER_HPP_INCLUDED
#define VMIME_SECURITY_CERT_DEFAULTCERTIFICATEVERIFIER_HPP_INCLUDED


#include "vmime/security/cert/certificateVerifier.hpp"


namespace vmime {
namespace security {
namespace cert {


class X509Certificate;


/** Default implementation for certificate verification.
  */
class VMIME_EXPORT defaultCertificateVerifier : public certificateVerifier
{
private:

	defaultCertificateVerifier(const defaultCertificateVerifier&);

public:

	defaultCertificateVerifier();
	~defaultCertificateVerifier();

	/** Sets a list of X.509 certificates that are trusted.
	  *
	  * @param trustedCerts list of trusted certificates
	  */
	void setX509TrustedCerts(const std::vector <shared_ptr <X509Certificate> >& trustedCerts);

	/** Sets the X.509 root CAs used for certificate verification.
	  *
	  * @param caCerts list of root CAs
	  */
	void setX509RootCAs(const std::vector <shared_ptr <X509Certificate> >& caCerts);


	// Implementation of 'certificateVerifier'
	void verify(shared_ptr <certificateChain> chain, const string& hostname);

private:

	/** Verify a chain of X.509 certificates.
	  *
	  * @param chain list of X.509 certificates
	  * @param hostname server hostname
	  */
	void verifyX509(shared_ptr <certificateChain> chain, const string& hostname);


	std::vector <shared_ptr <X509Certificate> > m_x509RootCAs;
	std::vector <shared_ptr <X509Certificate> > m_x509TrustedCerts;
};


} // cert
} // security
} // vmime


#endif // VMIME_SECURITY_CERT_DEFAULTCERTIFICATEVERIFIER_HPP_INCLUDED

