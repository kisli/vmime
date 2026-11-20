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

#ifndef VMIME_SECURITY_CERT_X509CERTIFICATE_HPP_INCLUDED
#define VMIME_SECURITY_CERT_X509CERTIFICATE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/security/cert/certificate.hpp"

#include "vmime/utility/stream.hpp"

#include "vmime/base.hpp"
#include "vmime/types.hpp"
#include "vmime/dateTime.hpp"


namespace vmime {
namespace security {
namespace cert {


/** Identity certificate based on X.509 standard.
  */
class VMIME_EXPORT X509Certificate : public certificate {

public:

	~X509Certificate();

	/** Supported encodings for X.509 certificates. */
	enum Format {
		FORMAT_DER,   /**< DER encoding */
		FORMAT_PEM    /**< PEM encoding */
	};

	/** Supported digest algorithms (used for fingerprint). */
	enum DigestAlgorithm {
		DIGEST_MD5,
		DIGEST_SHA1,
		DIGEST_SHA256
	};


	/** Imports a DER or PEM encoded X.509 certificate.
	  *
	  * @param is input stream to read data from
	  * @return a X.509 certificate, or NULL if the given data does not
	  * represent a valid certificate
	  */
	static shared_ptr <X509Certificate> import(utility::inputStream& is);

	/** Imports a DER or PEM encoded X.509 certificate.
	  *
	  * @param data points to raw data
	  * @param length size of data
	  * @return a X.509 certificate, or NULL if the given data does not
	  * represent a valid certificate
	  */
	static shared_ptr <X509Certificate> import(const byte_t* data, const size_t length);

	/** Import several DER or PEM encoded X.509 certificates.
	  *
	  * @param is input stream to read data from
	  * @param certs the resulting list of certificates
	  */
	static void import(
		utility::inputStream& is,
		std::vector <shared_ptr <X509Certificate> >& certs
	);

	/** Import several DER or PEM encoded X.509 certificates.
	 *
	 * @param data points to raw data
	 * @param length size of data
	 * @param certs the resulting list of certificates
	 */
	static void import(
		const byte_t* data,
		const size_t length,
		std::vector <shared_ptr <X509Certificate> >& certs
	);

	/** Exports this X.509 certificate to the specified format.
	  *
	  * @param os output stream into which write data
	  * @param format output format
	  */
	virtual void write(utility::outputStream& os, const Format format) const = 0;

	/** Returns the X.509 certificate's serial number. This is obtained
	  * by the X.509 Certificate 'serialNumber' field. Serial is not
	  * always a 32 or 64bit number. Some CAs use large serial numbers,
	  * thus it may be wise to handle it as something opaque.
	  *
	  * @return serial number of this certificate
	  */
	virtual byteArray getSerialNumber() const = 0;

	/** Returns the distinguished name of the issuer of this certificate.
	  * Eg. "C=US,O=VeriSign\, Inc.,OU=Class 1 Public Primary Certification Authority"
	  *
	  * @return distinguished name of the certificate issuer, as a string
	  */
	virtual string getIssuerString() const = 0;

	/** Verifies a certificate chain sent by a TLS server, using the TLS
	  * library. A certification path is built from the first certificate
	  * of the chain (the server certificate) to one of the root CAs, with
	  * the other certificates of the chain, in any order (extraneous
	  * certificates are ignored). The path is then validated as specified
	  * in RFC 5280, at the current time: signatures, CA certificates
	  * allowed to sign certificates, path length and name constraints,
	  * and validity dates. If the server certificate has an extended key
	  * usage extension, it must allow TLS server authentication.
	  *
	  * The host name is not verified (see verifyHostName()), and the
	  * revocation status of the certificates is not checked.
	  *
	  * @param chain certificate chain, starting with the server certificate
	  * @param rootCAs trusted CA certificates: the path can end with any of
	  * them, even if it is not self-signed
	  * @throw certificateExpiredException if a certificate of the path
	  * has expired
	  * @throw certificateNotYetValidException if a certificate of the path
	  * is not yet valid
	  * @throw certificateIssuerVerificationException if a certificate of
	  * the path cannot be verified against its issuer
	  * @throw certificateNotTrustedException if no path to a root CA can be
	  * built, or if the server certificate cannot be used by a TLS server
	  */
	static void verifyChain(
		const std::vector <shared_ptr <X509Certificate> >& chain,
		const std::vector <shared_ptr <X509Certificate> >& rootCAs
	);

	/** Verify certificate's subject name against the given hostname.
	  *
	  * @param hostname DNS name of the server
	  * @param nonMatchingNames if not NULL, will contain the names that do
	  * not match the identities in the certificate
	  * @return true if the match is successful, false otherwise
	  */
	virtual bool verifyHostName(
		const string& hostname,
		std::vector <std::string>* nonMatchingNames = NULL
	) const = 0;

	/** Gets the expiration date of this certificate. This is the date
	  * at which this certificate will not be valid anymore.
	  *
	  * @return expiration date of this certificate
	  */
	virtual datetime getExpirationDate() const = 0;

	/** Gets the activation date of this certificate. This is the date
	  * at which this certificate will be valid.
	  *
	  * @return activation date of this certificate
	  */
	virtual datetime getActivationDate() const = 0;

	/** Returns the fingerprint of this certificate.
	  *
	  * @return the fingerprint of this certificate
	  */
	virtual byteArray getFingerprint(const DigestAlgorithm algo) const = 0;

	/** Checks that the certificate is currently valid. For the certificate
	  * to be valid, the current date and time must be in the validity period
	  * specified in the certificate.
	  *
	  * @throw certificateExpiredException if the certificate has expired
	  * @throw certificateNotYetValidException if the certificate is not yet valid
	  */
	virtual void checkValidity();
};


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT

#endif // VMIME_SECURITY_CERT_X509CERTIFICATE_HPP_INCLUDED

