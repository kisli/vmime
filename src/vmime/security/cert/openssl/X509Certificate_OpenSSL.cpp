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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL


#include <cstdio>
#include <ctime>
#include <map>
#include <algorithm>

#include "vmime/security/cert/openssl/X509Certificate_OpenSSL.hpp"

#include "vmime/net/tls/openssl/OpenSSLInitializer.hpp"

#include "vmime/utility/outputStreamByteArrayAdapter.hpp"

#include "vmime/security/cert/certificateException.hpp"
#include "vmime/security/cert/certificateExpiredException.hpp"
#include "vmime/security/cert/certificateIssuerVerificationException.hpp"
#include "vmime/security/cert/certificateNotTrustedException.hpp"
#include "vmime/security/cert/certificateNotYetValidException.hpp"
#include "vmime/security/cert/unsupportedCertificateTypeException.hpp"

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/conf.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>


#ifdef _WIN32
#	define strcasecmp _stricmp
#	define strncasecmp _strnicmp
#endif


namespace vmime {
namespace security {
namespace cert {


static net::tls::OpenSSLInitializer::autoInitializer openSSLInitializer;


#ifndef VMIME_BUILDING_DOC

class monthMap {

public:

	monthMap() {

		m_monthMap["jan"] = vmime::datetime::JAN;
		m_monthMap["feb"] = vmime::datetime::FEB;
		m_monthMap["mar"] = vmime::datetime::MAR;
		m_monthMap["apr"] = vmime::datetime::APR;
		m_monthMap["may"] = vmime::datetime::MAY;
		m_monthMap["jun"] = vmime::datetime::JUN;
		m_monthMap["jul"] = vmime::datetime::JUL;
		m_monthMap["aug"] = vmime::datetime::AUG;
		m_monthMap["sep"] = vmime::datetime::SEP;
		m_monthMap["oct"] = vmime::datetime::OCT;
		m_monthMap["nov"] = vmime::datetime::NOV;
		m_monthMap["dec"] = vmime::datetime::DEC;
	}

	int getMonth(vmime::string mstr) {

		std::transform(mstr.begin(), mstr.end(), mstr.begin(), ::tolower);

		std::map <vmime::string, vmime::datetime::Months>::const_iterator
			c_it = m_monthMap.find(mstr);

		if (c_it != m_monthMap.end()) {
			return c_it->second;
		}

		return -1;
	}

private:

	std::map<vmime::string, vmime::datetime::Months> m_monthMap;
};

static monthMap sg_monthMap;



struct OpenSSLX509CertificateInternalData {

	OpenSSLX509CertificateInternalData() {

		cert = 0;
	}

	~OpenSSLX509CertificateInternalData() {

		if (cert) {
			X509_free(cert);
		}
	}

	X509* cert;
};


// Workaround for i2v() taking either a const or a non-const 'method' on some platforms
STACK_OF(CONF_VALUE)* call_i2v(const X509V3_EXT_METHOD* m, void* p1, STACK_OF(CONF_VALUE)* p2) {

	return m->i2v(m, p1, p2);
}


STACK_OF(CONF_VALUE)* call_i2v(X509V3_EXT_METHOD* m, void* p1, STACK_OF(CONF_VALUE)* p2) {

	return m->i2v(m, p1, p2);
}


#endif // VMIME_BUILDING_DOC


X509Certificate_OpenSSL::X509Certificate_OpenSSL()
	: m_data(new OpenSSLX509CertificateInternalData) {

}


X509Certificate_OpenSSL::X509Certificate_OpenSSL(X509* cert)
	: m_data(new OpenSSLX509CertificateInternalData) {

	m_data->cert = X509_dup(cert);
}


X509Certificate_OpenSSL::X509Certificate_OpenSSL(const X509Certificate_OpenSSL&)
	: X509Certificate(), m_data(NULL) {

	// Not used
}


X509Certificate_OpenSSL::~X509Certificate_OpenSSL() {

	delete m_data;
}


void* X509Certificate_OpenSSL::getInternalData() {

	return &m_data->cert;
}


// static
shared_ptr <X509Certificate> X509Certificate_OpenSSL::importInternal(X509* cert) {

	if (cert) {
		return make_shared <X509Certificate_OpenSSL>(reinterpret_cast <X509 *>(cert));
	}

	return null;
}


// static
shared_ptr <X509Certificate> X509Certificate::import(utility::inputStream& is) {

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

	shared_ptr <X509Certificate_OpenSSL> cert = make_shared <X509Certificate_OpenSSL>();

	BIO* membio = BIO_new_mem_buf(const_cast <byte_t*>(data), static_cast <int>(length));

	if (!PEM_read_bio_X509(membio, &(cert->m_data->cert), 0, 0)) {
		BIO_vfree(membio);
		return null;
	}

	BIO_vfree(membio);

	return cert;
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

	BIO* membio = BIO_new_mem_buf(const_cast <byte_t*>(data), static_cast <int>(length));
	shared_ptr <X509Certificate_OpenSSL> cert = null;

	while (true) {

		cert = make_shared <X509Certificate_OpenSSL>();

		if (!PEM_read_bio_X509(membio, &(cert->m_data->cert), 0, 0)) {
			break;
		}

		certs.push_back(cert);
	}

	BIO_vfree(membio);
}


void X509Certificate_OpenSSL::write(
	utility::outputStream& os,
	const Format format
) const {

	BIO* membio = 0;
	long dataSize = 0;
	unsigned char* out = 0;

	if (format == FORMAT_DER) {

		if ((dataSize = i2d_X509(m_data->cert, &out)) < 0) {
			goto err;
		}

		os.write(reinterpret_cast <byte_t*>(out), dataSize);
		os.flush();
		OPENSSL_free(out);

	} else if (format == FORMAT_PEM) {

		membio = BIO_new(BIO_s_mem());
		BIO_set_close(membio, BIO_CLOSE);

		if (!PEM_write_bio_X509(membio, m_data->cert)) {
			goto pem_err;
		}

		dataSize = BIO_get_mem_data(membio, &out);
		os.write(reinterpret_cast <byte_t*>(out), dataSize);
		os.flush();
		BIO_vfree(membio);

	} else {

		throw unsupportedCertificateTypeException("Unknown format");
	}

	return; // #### Early Return ####

pem_err:
	{
		if (membio) {
			BIO_vfree(membio);
		}
	}

err:
	{
		char errstr[256];
		long ec = ERR_get_error();
		ERR_error_string(ec, errstr);
		throw certificateException("OpenSSLX509Certificate_OpenSSL::write exception - " + string(errstr));
	}
}


byteArray X509Certificate_OpenSSL::getSerialNumber() const {

	ASN1_INTEGER *serial = X509_get_serialNumber(m_data->cert);
	BIGNUM *bnser = ASN1_INTEGER_to_BN(serial, NULL);
	int n = BN_num_bytes(bnser);
	byte_t* outbuf = new byte_t[n];
	BN_bn2bin(bnser, outbuf);
	byteArray ser(outbuf, outbuf + n);
	delete [] outbuf;
	BN_free(bnser);
	return ser;
}


#ifndef VMIME_BUILDING_DOC

// Resources used for verifying a certificate chain
struct OpenSSLChainVerification {

	OpenSSLChainVerification()
		: store(X509_STORE_new()),
		  untrusted(sk_X509_new_null()),
		  ctx(X509_STORE_CTX_new()),
		  path(NULL) {

	}

	~OpenSSLChainVerification() {

		if (path) {
			sk_X509_pop_free(path, X509_free);
		}

		if (ctx) {
			X509_STORE_CTX_free(ctx);
		}

		if (untrusted) {
			sk_X509_free(untrusted);
		}

		if (store) {
			X509_STORE_free(store);
		}

		// A failed verification leaves errors in the queue
		ERR_clear_error();
	}

	X509_STORE* store;
	STACK_OF(X509)* untrusted;
	X509_STORE_CTX* ctx;
	STACK_OF(X509)* path;
};


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

	OpenSSLChainVerification verif;

	if (!verif.store || !verif.untrusted || !verif.ctx) {
		throw certificateNotTrustedException();
	}

	// Native certificates, to find the certificate on which a problem occurred
	std::vector <std::pair <X509*, shared_ptr <X509Certificate> > > certs;

	for (const shared_ptr <X509Certificate>& rootCA : rootCAs) {

		X509* cert = dynamicCast <X509Certificate_OpenSSL>(rootCA)->m_data->cert;

		// Fails if the certificate is already in the store, which is not a problem
		X509_STORE_add_cert(verif.store, cert);

		certs.push_back(std::make_pair(cert, rootCA));
	}

#ifdef X509_V_FLAG_PARTIAL_CHAIN
	// The path can end with any trusted CA, even if it is not self-signed
	// (RFC 5280, section 6.1.1 (d))
	X509_STORE_set_flags(verif.store, X509_V_FLAG_PARTIAL_CHAIN);
#endif

	for (size_t i = 0 ; i < chain.size() ; ++i) {

		X509* cert = dynamicCast <X509Certificate_OpenSSL>(chain[i])->m_data->cert;

		if (i != 0) {
			sk_X509_push(verif.untrusted, cert);
		}

		certs.push_back(std::make_pair(cert, chain[i]));
	}

	X509* serverCert = dynamicCast <X509Certificate_OpenSSL>(chain[0])->m_data->cert;

	if (X509_STORE_CTX_init(verif.ctx, verif.store, serverCert, verif.untrusted) != 1) {
		throw certificateNotTrustedException();
	}

	// The extended key usage of the server certificate, if any, must allow
	// TLS server authentication (RFC 5280, section 4.2.1.12)
	X509_STORE_CTX_set_purpose(verif.ctx, X509_PURPOSE_SSL_SERVER);

	if (X509_verify_cert(verif.ctx) == 1) {
		return;
	}

	const int error = X509_STORE_CTX_get_error(verif.ctx);
	const int depth = X509_STORE_CTX_get_error_depth(verif.ctx);

	X509* errorCert = X509_STORE_CTX_get_current_cert(verif.ctx);

	// For these errors, OpenSSL reports the issuer: report the certificate
	// which cannot be verified against it instead (as GnuTLS does)
	if ((error == X509_V_ERR_INVALID_CA ||
	     error == X509_V_ERR_KEYUSAGE_NO_CERTSIGN ||
	     error == X509_V_ERR_PATH_LENGTH_EXCEEDED) && depth >= 1) {

		verif.path = X509_STORE_CTX_get1_chain(verif.ctx);

		if (verif.path && depth - 1 < sk_X509_num(verif.path)) {
			errorCert = sk_X509_value(verif.path, depth - 1);
		}
	}

	shared_ptr <X509Certificate> cert = chain[0];

	if (errorCert) {

		cert = null;

		for (size_t i = 0 ; !cert && i < certs.size() ; ++i) {

			if (certs[i].first == errorCert) {
				cert = certs[i].second;
			}
		}

		if (!cert) {
			cert = X509Certificate_OpenSSL::importInternal(errorCert);
		}
	}

	const string reason = X509_verify_cert_error_string(error);

	switch (error) {

		case X509_V_ERR_CERT_HAS_EXPIRED:

			throwCertificateException(certificateExpiredException(), cert);
			break;

		case X509_V_ERR_CERT_NOT_YET_VALID:

			throwCertificateException(certificateNotYetValidException(), cert);
			break;

		case X509_V_ERR_CERT_SIGNATURE_FAILURE:
		case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
		case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
		case X509_V_ERR_INVALID_CA:
		case X509_V_ERR_KEYUSAGE_NO_CERTSIGN:
		case X509_V_ERR_PATH_LENGTH_EXCEEDED:
		case X509_V_ERR_PERMITTED_VIOLATION:
		case X509_V_ERR_EXCLUDED_VIOLATION:
		case X509_V_ERR_SUBTREE_MINMAX:
		case X509_V_ERR_UNSUPPORTED_CONSTRAINT_TYPE:
		case X509_V_ERR_UNSUPPORTED_CONSTRAINT_SYNTAX:
		case X509_V_ERR_UNSUPPORTED_NAME_SYNTAX:

			throwCertificateException(certificateIssuerVerificationException(reason), cert);
			break;

		default:

			// No path to a trusted CA, unsuitable purpose, etc.
			throwCertificateException(certificateNotTrustedException(reason), cert);
			break;
	}
}


bool X509Certificate_OpenSSL::verifyHostName(
	const string& hostname,
	std::vector <std::string>* nonMatchingNames
) const {

	// An IP address must match an IP address in subject alternative names
	// (X509_check_ip_asc() returns -2 if hostname is not an IP address)
	const int ipMatch = X509_check_ip_asc(m_data->cert, hostname.c_str(), 0);

	if (ipMatch != -2) {
		return ipMatch == 1;
	}

	// DNS names of the subject alternative names
	std::vector <string> dnsNames;

	STACK_OF(GENERAL_NAME)* altNames = static_cast <GENERAL_NAMES*>
		(X509_get_ext_d2i(m_data->cert, NID_subject_alt_name, NULL, NULL));

	if (altNames) {

		for (int i = 0, n = sk_GENERAL_NAME_num(altNames) ; i < n ; ++i) {

			const GENERAL_NAME* currentName = sk_GENERAL_NAME_value(altNames, i);

			if (currentName->type == GEN_DNS) {

				dnsNames.push_back(string(
					reinterpret_cast <const char*>(ASN1_STRING_get0_data(currentName->d.dNSName)),
					ASN1_STRING_length(currentName->d.dNSName)
				));
			}
		}

		sk_GENERAL_NAME_pop_free(altNames, GENERAL_NAME_free);
	}

	// Match the host name against the DNS names of the subject alternative
	// names. Names must match entirely (labels are not compared partially),
	// and a wildcard only matches the whole left-most label (RFC 9525,
	// section 6.3). If there are no DNS names, the common name is used, as
	// RFC 6125, section 6.4.4 allowed: RFC 9525 does not allow it any more,
	// but it is kept for compatibility (eg. self-signed certificates with a
	// common name only), like GnuTLS does.
	//
	// The use of the common name is always requested explicitly: by default,
	// OpenSSL uses it if there are no DNS names before OpenSSL 4.1, and never
	// since OpenSSL 4.1.
	const unsigned int flags =
		X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS |
		(dnsNames.empty() ? X509_CHECK_FLAG_ALWAYS_CHECK_SUBJECT : X509_CHECK_FLAG_NEVER_CHECK_SUBJECT);

	// X509_check_host() takes a name starting with a dot as "any subdomain":
	// do not accept such a name.
	if (!hostname.empty() && hostname[0] != '.' &&
	    X509_check_host(m_data->cert, hostname.data(), hostname.length(), flags, NULL) == 1) {

		return true;
	}

	if (nonMatchingNames) {

		char CNBuffer[1024];
		CNBuffer[sizeof(CNBuffer) - 1] = '\0';

		auto xname = X509_get_subject_name(m_data->cert);

		if (X509_NAME_get_text_by_NID(xname, NID_commonName, CNBuffer, sizeof(CNBuffer)) != -1) {
			nonMatchingNames->push_back(CNBuffer);
		}

		nonMatchingNames->insert(nonMatchingNames->end(), dnsNames.begin(), dnsNames.end());
	}

	return false;
}


datetime X509Certificate_OpenSSL::convertX509Date(void* time) const {

	char* buffer;
	BIO* out = BIO_new(BIO_s_mem());
	BIO_set_close(out, BIO_CLOSE);

	ASN1_TIME* asn1_time = reinterpret_cast<ASN1_TIME*>(time);
	ASN1_TIME_print(out, asn1_time);

	const long sz = BIO_get_mem_data(out, &buffer);
	char* dest = new char[sz + 1];
	dest[sz] = 0;
	memcpy(dest, buffer, sz);
	vmime::string t(dest);

	BIO_free(out);
	delete [] dest;

	if (t.size() > 0) {

		char month[4] = {0};
		char zone[4] = {0};
		int day, hour, minute, second, year;
		int nrconv = sscanf(t.c_str(), "%s %2d %02d:%02d:%02d %d%s", month, &day, &hour, &minute, &second,&year,zone);

		if (nrconv >= 6) {
			return datetime(year, sg_monthMap.getMonth(vmime::string(month)), day, hour, minute, second);
		}
	}

	// let datetime try and parse it
	return datetime(t);
}


datetime X509Certificate_OpenSSL::getActivationDate() const {

	return convertX509Date(X509_get_notBefore(m_data->cert));
}


datetime X509Certificate_OpenSSL::getExpirationDate() const {

	return convertX509Date(X509_get_notAfter(m_data->cert));
}


byteArray X509Certificate_OpenSSL::getFingerprint(const DigestAlgorithm algo) const {

	BIO *out;
	int j;
	unsigned int n;
	const EVP_MD *digest;
	unsigned char * fingerprint;
	unsigned char md[EVP_MAX_MD_SIZE];

	switch (algo) {

		case DIGEST_MD5:

			digest = EVP_md5();
			break;

		case DIGEST_SHA256:

			digest = EVP_sha256();
			break;

		default:
		case DIGEST_SHA1:

			digest = EVP_sha1();
			break;
	}

	out = BIO_new(BIO_s_mem());
	BIO_set_close(out, BIO_CLOSE);

	if (X509_digest(m_data->cert, digest, md, &n)) {

		for (j = 0 ; j < (int) n ; j++) {
			BIO_printf(out, "%02X", md[j]);
			if (j + 1 != (int) n) BIO_printf(out, ":");
		}
	}

	const long resultLen = BIO_get_mem_data(out, &fingerprint);
	unsigned char* result = new unsigned char[resultLen];
	memcpy(result, fingerprint, resultLen);
	BIO_free(out);

	byteArray res;
	res.insert(res.end(), &result[0], &result[0] + resultLen);

	delete [] result;

	return res;
}


byteArray X509Certificate_OpenSSL::getEncoded() const {

	byteArray bytes;
	utility::outputStreamByteArrayAdapter os(bytes);

	write(os, FORMAT_DER);

	return bytes;
}


string X509Certificate_OpenSSL::getIssuerString() const {

	// Get issuer for this cert
	BIO* out = BIO_new(BIO_s_mem());
	X509_NAME_print_ex(out, X509_get_issuer_name(m_data->cert), 0, XN_FLAG_RFC2253);

	unsigned char* issuer;
	const long n = BIO_get_mem_data(out, &issuer);

	vmime::string name(reinterpret_cast <char*>(issuer), n);
	BIO_free(out);

	return name;
}


string X509Certificate_OpenSSL::getType() const {

	return "X.509";
}


int X509Certificate_OpenSSL::getVersion() const {

	return (int) X509_get_version(m_data->cert);
}


bool X509Certificate_OpenSSL::equals(const shared_ptr <const certificate>& other) const {

	shared_ptr <const X509Certificate_OpenSSL> otherX509 =
		dynamicCast <const X509Certificate_OpenSSL>(other);

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


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

