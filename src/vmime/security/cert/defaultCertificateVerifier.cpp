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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/security/cert/defaultCertificateVerifier.hpp"

#include "vmime/security/cert/X509Certificate.hpp"

#include "vmime/exception.hpp"


namespace vmime {
namespace security {
namespace cert {


defaultCertificateVerifier::defaultCertificateVerifier() {

}


defaultCertificateVerifier::~defaultCertificateVerifier() {

}


defaultCertificateVerifier::defaultCertificateVerifier(const defaultCertificateVerifier&)
	: certificateVerifier() {

	// Not used
}


void defaultCertificateVerifier::verify(
	const shared_ptr <certificateChain>& chain,
	const string& hostname
) {

	// Nothing to trust
	if (chain->getCount() == 0) {
		throw certificateNotTrustedException();
	}

	const string type = chain->getAt(0)->getType();

	if (type == "X.509") {
		verifyX509(chain, hostname);
	} else {
		throw unsupportedCertificateTypeException(type);
	}
}


void defaultCertificateVerifier::verifyX509(
	const shared_ptr <certificateChain>& chain,
	const string& hostname
) {

	std::vector <shared_ptr <X509Certificate> > certs;

	for (size_t i = 0 ; i < chain->getCount() ; ++i) {

		shared_ptr <X509Certificate> cert =
			dynamicCast <X509Certificate>(chain->getAt(i));

		if (!cert) {
			throw unsupportedCertificateTypeException(chain->getAt(i)->getType());
		}

		certs.push_back(cert);
	}

	shared_ptr <X509Certificate> firstCert = certs[0];

	// The user may have decided to trust the server certificate: then, it
	// only has to be valid at the current time
	bool trusted = false;

	for (size_t i = 0 ; !trusted && i < m_x509TrustedCerts.size() ; ++i) {

		if (firstCert->equals(m_x509TrustedCerts[i])) {

			firstCert->checkValidity();
			trusted = true;
		}
	}

	// Otherwise, let the TLS library build and validate a certification
	// path to one of the root CAs. It accepts the certificates in any order,
	// and ignores extraneous ones (for example, a root certificate cross-
	// signed by an older root which is not trusted any more), as RFC 8446,
	// section 4.4.2 recommends.
	if (!trusted) {
		X509Certificate::verifyChain(certs, m_x509RootCAs);
	}

	// Ensure the first certificate's subject name matches server hostname
	if (!firstCert->verifyHostName(hostname)) {

		serverIdentityException ex;
		ex.setCertificate(firstCert);

		throw ex;
	}
}


void defaultCertificateVerifier::setX509RootCAs(
	const std::vector <shared_ptr <X509Certificate> >& caCerts
) {

	m_x509RootCAs = caCerts;
}


void defaultCertificateVerifier::setX509TrustedCerts(
	const std::vector <shared_ptr <X509Certificate> >& trustedCerts
) {

	m_x509TrustedCerts = trustedCerts;
}


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT
