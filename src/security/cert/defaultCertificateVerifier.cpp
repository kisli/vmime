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

#if VMIME_HAVE_TLS_SUPPORT

#include "vmime/security/cert/defaultCertificateVerifier.hpp"

#include "vmime/security/cert/X509Certificate.hpp"

#include "vmime/exception.hpp"


namespace vmime {
namespace security {
namespace cert {


defaultCertificateVerifier::defaultCertificateVerifier()
{
}


defaultCertificateVerifier::~defaultCertificateVerifier()
{
}


defaultCertificateVerifier::defaultCertificateVerifier(const defaultCertificateVerifier&)
	: certificateVerifier()
{
	// Not used
}


void defaultCertificateVerifier::verify
	(shared_ptr <certificateChain> chain, const string& hostname)
{
	if (chain->getCount() == 0)
		return;

	const string type = chain->getAt(0)->getType();

	if (type == "X.509")
		verifyX509(chain, hostname);
	else
		throw exceptions::unsupported_certificate_type(type);
}


void defaultCertificateVerifier::verifyX509
	(shared_ptr <certificateChain> chain, const string& hostname)
{
	// For every certificate in the chain, verify that the certificate
	// has been issued by the next certificate in the chain
	if (chain->getCount() >= 2)
	{
		for (unsigned int i = 0 ; i < chain->getCount() - 1 ; ++i)
		{
			shared_ptr <X509Certificate> cert =
				dynamicCast <X509Certificate>(chain->getAt(i));

			shared_ptr <X509Certificate> next =
				dynamicCast <X509Certificate>(chain->getAt(i + 1));

			if (!cert->checkIssuer(next))
			{
				throw exceptions::certificate_verification_exception
					("Subject/issuer verification failed.");
			}
		}
	}

	// For every certificate in the chain, verify that the certificate
	// is valid at the current time
	const datetime now = datetime::now();

	for (unsigned int i = 0 ; i < chain->getCount() ; ++i)
	{
		shared_ptr <X509Certificate> cert =
			dynamicCast <X509Certificate>(chain->getAt(i));

		const datetime begin = cert->getActivationDate();
		const datetime end = cert->getExpirationDate();

		if (now < begin || now > end)
		{
			throw exceptions::certificate_verification_exception
				("Validity date check failed.");
		}
	}

	// Check whether the certificate can be trusted

	// -- First, verify that the the last certificate in the chain was
	// -- issued by a third-party that we trust
	shared_ptr <X509Certificate> lastCert =
		dynamicCast <X509Certificate>(chain->getAt(chain->getCount() - 1));

	bool trusted = false;

	for (unsigned int i = 0 ; !trusted && i < m_x509RootCAs.size() ; ++i)
	{
		shared_ptr <X509Certificate> rootCa = m_x509RootCAs[i];

		if (lastCert->verify(rootCa))
			trusted = true;
	}

	// -- Next, if the issuer certificate cannot be verified against
	// -- root CAs, compare the subject's certificate against the
	// -- trusted certificates
	shared_ptr <X509Certificate> firstCert =
		dynamicCast <X509Certificate>(chain->getAt(0));

	for (unsigned int i = 0 ; !trusted && i < m_x509TrustedCerts.size() ; ++i)
	{
		shared_ptr <X509Certificate> cert = m_x509TrustedCerts[i];

		if (firstCert->equals(cert))
			trusted = true;
	}

	if (!trusted)
	{
		throw exceptions::certificate_verification_exception
			("Cannot verify certificate against trusted certificates.");
	}

	// Ensure the first certificate's subject name matches server hostname
	if (!firstCert->verifyHostName(hostname))
	{
		throw exceptions::certificate_verification_exception
			("Server identity cannot be verified.");
	}
}


void defaultCertificateVerifier::setX509RootCAs
	(const std::vector <shared_ptr <X509Certificate> >& caCerts)
{
	m_x509RootCAs = caCerts;
}


void defaultCertificateVerifier::setX509TrustedCerts
	(const std::vector <shared_ptr <X509Certificate> >& trustedCerts)
{
	m_x509TrustedCerts = trustedCerts;
}


} // cert
} // security
} // vmime

#endif
