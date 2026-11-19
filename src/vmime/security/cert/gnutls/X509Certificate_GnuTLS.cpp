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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS


#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include <ctime>
#include <utility>
#include <vector>

#include "vmime/security/cert/gnutls/X509Certificate_GnuTLS.hpp"

#include "vmime/security/cert/certificateExpiredException.hpp"
#include "vmime/security/cert/certificateIssuerVerificationException.hpp"
#include "vmime/security/cert/certificateNotTrustedException.hpp"
#include "vmime/security/cert/certificateNotYetValidException.hpp"

#include "vmime/utility/outputStreamByteArrayAdapter.hpp"


namespace vmime {
namespace security {
namespace cert {


#ifndef VMIME_BUILDING_DOC

struct GnuTLSX509CertificateInternalData {

	GnuTLSX509CertificateInternalData() {

		gnutls_x509_crt_init(&cert);
	}

	~GnuTLSX509CertificateInternalData() {

		gnutls_x509_crt_deinit(cert);
	}

	void swap(gnutls_x509_crt_t to) {
		gnutls_x509_crt_deinit(cert);
		cert = to;
	}


	gnutls_x509_crt_t cert;
};

#endif // VMIME_BUILDING_DOC


X509Certificate_GnuTLS::X509Certificate_GnuTLS()
	: m_data(new GnuTLSX509CertificateInternalData) {

}


X509Certificate_GnuTLS::X509Certificate_GnuTLS(const X509Certificate&)
	: X509Certificate(), m_data(NULL) {

	// Not used
}


X509Certificate_GnuTLS::~X509Certificate_GnuTLS() {

	delete m_data;
}


void* X509Certificate_GnuTLS::getInternalData() {

	return &m_data->cert;
}


// static
shared_ptr <X509Certificate> X509Certificate::import(
	utility::inputStream& is
) {

	byteArray bytes;
	byte_t chunk[4096];

	while (!is.eof()) {
		const size_t len = is.read(chunk, sizeof(chunk));
		bytes.insert(bytes.end(), chunk, chunk + len);
	}

	return import(&bytes[0], bytes.size());
}


// static
shared_ptr <X509Certificate> X509Certificate::import(
	const byte_t* data,
	const size_t length
) {

	gnutls_datum_t buffer;
	buffer.data = const_cast <byte_t*>(data);
	buffer.size = static_cast <unsigned int>(length);

	// Try DER format
	shared_ptr <X509Certificate_GnuTLS> derCert = make_shared <X509Certificate_GnuTLS>();

	if (gnutls_x509_crt_import(derCert->m_data->cert, &buffer, GNUTLS_X509_FMT_DER) >= 0) {
		return derCert;
	}

	// Try PEM format
	shared_ptr <X509Certificate_GnuTLS> pemCert = make_shared <X509Certificate_GnuTLS>();

	if (gnutls_x509_crt_import(pemCert->m_data->cert, &buffer, GNUTLS_X509_FMT_PEM) >= 0) {
		return pemCert;
	}

	return null;
}


// static
void X509Certificate::import(
	utility::inputStream& is,
	std::vector <shared_ptr <X509Certificate> >& certs
) {

	byteArray bytes;
	byte_t chunk[4096];

	while (!is.eof()) {
		const size_t len = is.read(chunk, sizeof(chunk));
		bytes.insert(bytes.end(), chunk, chunk + len);
	}

	import(&bytes[0], bytes.size(), certs);
}


// static
void X509Certificate::import(
	const byte_t* data,
	const size_t length,
	std::vector <shared_ptr <X509Certificate> >& certs
) {

	gnutls_datum_t buffer;
	buffer.data = const_cast <byte_t*>(data);
	buffer.size = static_cast <unsigned int>(length);

	unsigned int size = 1024;
	gnutls_x509_crt_t x509[1024];

	// Try DER format
	if (gnutls_x509_crt_list_import(x509, &size, &buffer, GNUTLS_X509_FMT_DER, 0) < 0) {

		// Try PEM format
		if (gnutls_x509_crt_list_import(x509, &size, &buffer, GNUTLS_X509_FMT_PEM, 0) < 0) {
			return;
		}
	}

	for (unsigned int i = 0 ; i < size ; i += 1) {

		auto c = make_shared <X509Certificate_GnuTLS>();
		c->m_data->swap(x509[i]);
		certs.push_back(c);
	}
}


void X509Certificate_GnuTLS::write(
	utility::outputStream& os,
	const Format format
) const {

	size_t dataSize = 0;
	gnutls_x509_crt_fmt_t fmt = GNUTLS_X509_FMT_DER;

	switch (format) {
		case FORMAT_DER: fmt = GNUTLS_X509_FMT_DER; break;
		case FORMAT_PEM: fmt = GNUTLS_X509_FMT_PEM; break;
	}

	gnutls_x509_crt_export(m_data->cert, fmt, NULL, &dataSize);

	std::vector <byte_t> data(dataSize);

	gnutls_x509_crt_export(m_data->cert, fmt, &data[0], &dataSize);

	os.write(reinterpret_cast <byte_t*>(&data[0]), dataSize);
}


byteArray X509Certificate_GnuTLS::getSerialNumber() const {

	char serial[64];
	size_t serialSize = sizeof(serial);

	gnutls_x509_crt_get_serial(m_data->cert, serial, &serialSize);

	return byteArray(serial, serial + serialSize);
}


#ifndef VMIME_BUILDING_DOC

// Output of the verification of each certificate, collected for
// X509Certificate::verifyChain() by verifyOutputCallback()
static thread_local std::vector <std::pair <gnutls_x509_crt_t, unsigned int> >* t_verifyOutput = NULL;


static int verifyOutputCallback(
	gnutls_x509_crt_t cert,
	gnutls_x509_crt_t /* issuer */,
	gnutls_x509_crl_t /* crl */,
	unsigned int verificationOutput
) {

	if (t_verifyOutput) {
		t_verifyOutput->push_back(std::make_pair(cert, verificationOutput));
	}

	return 0;
}


// Returns the reasons for the given verification status
static string getStatusReasons(const unsigned int status) {

	static const struct {
		unsigned int flag;
		const char* reason;
	} REASONS[] = {
		{ GNUTLS_CERT_SIGNATURE_FAILURE, "certificate signature failure" },
		{ GNUTLS_CERT_SIGNER_NOT_CA, "invalid CA certificate" },
		{ GNUTLS_CERT_SIGNER_CONSTRAINTS_FAILURE, "issuer constraints violated" },
		{ GNUTLS_CERT_INSECURE_ALGORITHM, "insecure algorithm" },
		{ GNUTLS_CERT_SIGNER_NOT_FOUND, "unknown issuer" },
		{ GNUTLS_CERT_PURPOSE_MISMATCH, "unsuitable certificate purpose" },
		{ GNUTLS_CERT_REVOKED, "certificate revoked" },
	};

	string reasons;

	for (size_t i = 0 ; i < sizeof(REASONS) / sizeof(REASONS[0]) ; ++i) {

		if (status & REASONS[i].flag) {

			if (!reasons.empty()) {
				reasons += ", ";
			}

			reasons += REASONS[i].reason;
		}
	}

	// Other reasons: use the description from GnuTLS
	if (reasons.empty()) {

		gnutls_datum_t desc;

		if (gnutls_certificate_verification_status_print(status, GNUTLS_CRT_X509, &desc, 0) == 0) {

			reasons = reinterpret_cast <const char*>(desc.data);
			gnutls_free(desc.data);

			while (!reasons.empty() && reasons[reasons.length() - 1] == ' ') {
				reasons.erase(reasons.length() - 1);
			}
		}
	}

	return reasons;
}


template <typename E>
static void throwCertificateException(E ex, const shared_ptr <certificate>& cert) {

	ex.setCertificate(cert);
	throw ex;
}

#endif // VMIME_BUILDING_DOC


// static
void X509Certificate::verifyChain(
	const std::vector <shared_ptr <X509Certificate> >& chain,
	const std::vector <shared_ptr <X509Certificate> >& rootCAs
) {

	if (chain.empty()) {
		throw certificateNotTrustedException();
	}

	std::vector <gnutls_x509_crt_t> chainCerts;
	std::vector <gnutls_x509_crt_t> caCerts;

	// Native certificates, to find the certificate on which a problem occurred
	std::vector <std::pair <gnutls_x509_crt_t, shared_ptr <X509Certificate> > > certs;

	for (const shared_ptr <X509Certificate>& cert : chain) {

		gnutls_x509_crt_t crt = dynamicCast <X509Certificate_GnuTLS>(cert)->m_data->cert;

		chainCerts.push_back(crt);
		certs.push_back(std::make_pair(crt, cert));
	}

	for (const shared_ptr <X509Certificate>& cert : rootCAs) {

		gnutls_x509_crt_t crt = dynamicCast <X509Certificate_GnuTLS>(cert)->m_data->cert;

		caCerts.push_back(crt);
		certs.push_back(std::make_pair(crt, cert));
	}

	gnutls_x509_trust_list_t trustList;

	if (gnutls_x509_trust_list_init(&trustList, 0) < 0) {
		throw certificateNotTrustedException();
	}

	// GnuTLS accepts a path which ends with any trusted CA, even if it is
	// not self-signed (RFC 5280, section 6.1.1 (d))
	if (!caCerts.empty() &&
	    gnutls_x509_trust_list_add_cas(
	        trustList, &caCerts[0], static_cast <unsigned int>(caCerts.size()), 0
	    ) < 0) {

		gnutls_x509_trust_list_deinit(trustList, 0);
		throw certificateNotTrustedException();
	}

	// The extended key usage of the server certificate, if any, must allow
	// TLS server authentication (RFC 5280, section 4.2.1.12)
	gnutls_typed_vdata_st purpose;
	purpose.type = GNUTLS_DT_KEY_PURPOSE_OID;
	purpose.data = reinterpret_cast <unsigned char*>(const_cast <char*>(GNUTLS_KP_TLS_WWW_SERVER));
	purpose.size = 0;

	std::vector <std::pair <gnutls_x509_crt_t, unsigned int> > output;
	unsigned int status = 0;

	t_verifyOutput = &output;

	const int res = gnutls_x509_trust_list_verify_crt2(
		trustList, &chainCerts[0], static_cast <unsigned int>(chainCerts.size()),
		&purpose, 1, 0, &status, verifyOutputCallback
	);

	t_verifyOutput = NULL;

	// Do not deinitialize the certificates, which are still ours
	gnutls_x509_trust_list_deinit(trustList, 0);

	if (res < 0) {
		throw certificateNotTrustedException(gnutls_strerror(res));
	}

	if (status == 0) {
		return;
	}

	const unsigned int ISSUER_ERRORS =
		GNUTLS_CERT_SIGNATURE_FAILURE |
		GNUTLS_CERT_SIGNER_NOT_CA |
		GNUTLS_CERT_SIGNER_CONSTRAINTS_FAILURE |
		GNUTLS_CERT_INSECURE_ALGORITHM;

	unsigned int errors = status & ~GNUTLS_CERT_INVALID;

	if (status & ISSUER_ERRORS) {
		errors = status & ISSUER_ERRORS;
	} else if (status & GNUTLS_CERT_EXPIRED) {
		errors = GNUTLS_CERT_EXPIRED;
	} else if (status & GNUTLS_CERT_NOT_ACTIVATED) {
		errors = GNUTLS_CERT_NOT_ACTIVATED;
	}

	// Find the certificate on which the problem occurred (or else, report
	// the server certificate)
	shared_ptr <X509Certificate> cert = chain[0];
	gnutls_x509_crt_t errorCert = NULL;

	for (size_t i = 0 ; !errorCert && i < output.size() ; ++i) {

		if (output[i].second & errors) {
			errorCert = output[i].first;
		}
	}

	for (size_t i = 0 ; errorCert && i < certs.size() ; ++i) {

		if (certs[i].first == errorCert) {
			cert = certs[i].second;
			break;
		}
	}

	if (errors & ISSUER_ERRORS) {
		throwCertificateException(certificateIssuerVerificationException(getStatusReasons(errors)), cert);
	} else if (errors == GNUTLS_CERT_EXPIRED) {
		throwCertificateException(certificateExpiredException(), cert);
	} else if (errors == GNUTLS_CERT_NOT_ACTIVATED) {
		throwCertificateException(certificateNotYetValidException(), cert);
	} else {
		// No path to a trusted CA, unsuitable purpose, etc.
		throwCertificateException(certificateNotTrustedException(getStatusReasons(status)), cert);
	}
}


bool X509Certificate_GnuTLS::verifyHostName(
	const string& hostname,
	std::vector <std::string>* nonMatchingNames
) const {

	if (gnutls_x509_crt_check_hostname(m_data->cert, hostname.c_str()) != 0) {
		return true;
	}

	if (nonMatchingNames) {

		const int MAX_CN = 256;
		const char* OID_X520_COMMON_NAME = "2.5.4.3";

		char dnsName[MAX_CN];
		size_t dnsNameLength;

		dnsNameLength = sizeof(dnsName);

		if (gnutls_x509_crt_get_dn_by_oid(m_data->cert, OID_X520_COMMON_NAME, 0, 0, dnsName, &dnsNameLength) >= 0) {
			nonMatchingNames->push_back(dnsName);
		}

		for (int i = 0, ret = 0 ; ret >= 0 ; ++i) {

			dnsNameLength = sizeof(dnsName);
			ret = gnutls_x509_crt_get_subject_alt_name(m_data->cert, i, dnsName, &dnsNameLength, NULL);

			if (ret == GNUTLS_SAN_DNSNAME) {
				nonMatchingNames->push_back(dnsName);
			}
		}
	}

	return false;
}


datetime X509Certificate_GnuTLS::getActivationDate() const {

	const time_t t = gnutls_x509_crt_get_activation_time(m_data->cert);
	return datetime(t);
}


datetime X509Certificate_GnuTLS::getExpirationDate() const {

	const time_t t = gnutls_x509_crt_get_expiration_time(m_data->cert);
	return datetime(t);
}


byteArray X509Certificate_GnuTLS::getFingerprint(const DigestAlgorithm algo) const {

	gnutls_digest_algorithm_t galgo;

	switch (algo) {

		case DIGEST_MD5:

			galgo = GNUTLS_DIG_MD5;
			break;

		case DIGEST_SHA256:

			galgo = GNUTLS_DIG_SHA256;
			break;

		default:
		case DIGEST_SHA1:

			galgo = GNUTLS_DIG_SHA1;
			break;
	}

	size_t bufferSize = 0;
	gnutls_x509_crt_get_fingerprint(m_data->cert, galgo, NULL, &bufferSize);

	std::vector <byte_t> buffer(bufferSize);

	if (gnutls_x509_crt_get_fingerprint(m_data->cert, galgo, &buffer[0], &bufferSize) == 0) {

		byteArray res;
		res.insert(res.end(), &buffer[0], &buffer[0] + bufferSize);

		return res;
	}

	return byteArray();
}


byteArray X509Certificate_GnuTLS::getEncoded() const {

	byteArray bytes;
	utility::outputStreamByteArrayAdapter os(bytes);

	write(os, FORMAT_DER);

	return bytes;
}


string X509Certificate_GnuTLS::getIssuerString() const {

	char buffer[4096];
	size_t bufferSize = sizeof(buffer);

	if (gnutls_x509_crt_get_issuer_dn(m_data->cert, buffer, &bufferSize) != GNUTLS_E_SUCCESS) {
		return "";
	}

	return buffer;
}


string X509Certificate_GnuTLS::getType() const {

	return "X.509";
}


int X509Certificate_GnuTLS::getVersion() const {

	return gnutls_x509_crt_get_version(m_data->cert);
}


bool X509Certificate_GnuTLS::equals(const shared_ptr <const certificate>& other) const {

	shared_ptr <const X509Certificate_GnuTLS> otherX509 =
		dynamicCast <const X509Certificate_GnuTLS>(other);

	if (!otherX509) {
		return false;
	}

	const byteArray fp1 = getFingerprint(DIGEST_MD5);
	const byteArray fp2 = otherX509->getFingerprint(DIGEST_MD5);

	return fp1 == fp2;
}


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS
