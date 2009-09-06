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


void defaultCertificateVerifier::verify(ref <certificateChain> chain)
{
	if (chain->getCount() == 0)
		return;

	const string type = chain->getAt(0)->getType();

	if (type == "X.509")
		verifyX509(chain);
	else
		throw exceptions::unsupported_certificate_type(type);
}


void defaultCertificateVerifier::verifyX509(ref <certificateChain> chain)
{
	// For every certificate in the chain, verify that the certificate
	// has been issued by the next certificate in the chain
	if (chain->getCount() >= 2)
	{
		for (unsigned int i = 0 ; i < chain->getCount() - 1 ; ++i)
		{
			ref <X509Certificate> cert =
				chain->getAt(i).dynamicCast <X509Certificate>();

			ref <X509Certificate> next =
				chain->getAt(i + 1).dynamicCast <X509Certificate>();

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
		ref <X509Certificate> cert =
			chain->getAt(i).dynamicCast <X509Certificate>();

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
	ref <X509Certificate> lastCert =
		chain->getAt(chain->getCount() - 1).dynamicCast <X509Certificate>();

	bool trusted = false;

	for (unsigned int i = 0 ; !trusted && i < m_x509RootCAs.size() ; ++i)
	{
		ref <X509Certificate> rootCa = m_x509RootCAs[i];

		if (lastCert->verify(rootCa))
			trusted = true;
	}

	// -- Next, if the issuer certificate cannot be verified against
	// -- root CAs, compare the subject's certificate against the
	// -- trusted certificates
	ref <X509Certificate> firstCert =
		chain->getAt(0).dynamicCast <X509Certificate>();

	for (unsigned int i = 0 ; !trusted && i < m_x509TrustedCerts.size() ; ++i)
	{
		ref <X509Certificate> cert = m_x509TrustedCerts[i];

		if (firstCert->equals(cert))
			trusted = true;
	}

	if (!trusted)
	{
		throw exceptions::certificate_verification_exception
			("Cannot verify certificate against trusted certificates.");
	}
}


void defaultCertificateVerifier::setX509RootCAs
	(const std::vector <ref <X509Certificate> >& caCerts)
{
	m_x509RootCAs = caCerts;
}


void defaultCertificateVerifier::setX509TrustedCerts
	(const std::vector <ref <X509Certificate> >& trustedCerts)
{
	m_x509TrustedCerts = trustedCerts;
}


} // cert
} // security
} // vmime

