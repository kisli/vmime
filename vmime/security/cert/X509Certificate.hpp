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

#ifndef VMIME_SECURITY_CERT_X509CERTIFICATE_HPP_INCLUDED
#define VMIME_SECURITY_CERT_X509CERTIFICATE_HPP_INCLUDED


#include "vmime/security/cert/certificate.hpp"

#include "vmime/utility/stream.hpp"

#include "vmime/base.hpp"
#include "vmime/dateTime.hpp"


namespace vmime {
namespace security {
namespace cert {


/** Identity certificate based on X.509 standard.
  */
class X509Certificate : public certificate
{
	friend class vmime::creator;

protected:

	X509Certificate();
	X509Certificate(const X509Certificate&);

public:

	~X509Certificate();

	/** Supported encodings for X.509 certificates. */
	enum Format
	{
		FORMAT_DER,   /**< DER encoding */
		FORMAT_PEM    /**< PEM encoding */
	};

	/** Supported digest algorithms (used for fingerprint). */
	enum DigestAlgorithm
	{
		DIGEST_MD5,   /**< MD5 digest */
		DIGEST_SHA1   /**< SHA1 digest */
	};


	/** Imports a DER or PEM encoded X.509 certificate.
	  *
	  * @param is input stream to read data from
	  * @return a X.509 certificate, or NULL if the given data does not
	  * represent a valid certificate
	  */
	static ref <X509Certificate> import(utility::inputStream& is);

	/** Imports a DER or PEM encoded X.509 certificate.
	  *
	  * @param data points to raw data
	  * @param length size of data
	  * @return a X.509 certificate, or NULL if the given data does not
	  * represent a valid certificate
	  */
	static ref <X509Certificate> import(const byte_t* data, const unsigned int length);

	/** Exports this X.509 certificate to the specified format.
	  *
	  * @param os output stream into which write data
	  * @param format output format
	  */
	void write(utility::outputStream& os, const Format format) const;

	/** Returns the X.509 certificate's serial number. This is obtained
	  * by the X.509 Certificate 'serialNumber' field. Serial is not
	  * always a 32 or 64bit number. Some CAs use large serial numbers,
	  * thus it may be wise to handle it as something opaque.
	  *
	  * @return serial number of this certificate
	  */
	const byteArray getSerialNumber() const;

	/** Checks if this certificate has the given issuer.
	  *
	  * @param issuer certificate of a possible issuer
	  * @return true if this certificate was issued by the given issuer,
	  * false otherwise
	  */
	bool checkIssuer(ref <const X509Certificate> issuer) const;

	/** Verifies this certificate against a given trusted one.
	  *
	  * @param caCert a certificate that is considered to be trusted one
	  * @return true if the verification succeeded, false otherwise
	  */
	bool verify(ref <const X509Certificate> caCert) const;

	/** Gets the expiration date of this certificate. This is the date
	  * at which this certificate will not be valid anymore.
	  *
	  * @return expiration date of this certificate
	  */
	const datetime getExpirationDate() const;

	/** Gets the activation date of this certificate. This is the date
	  * at which this certificate will be valid.
	  *
	  * @return activation date of this certificate
	  */
	const datetime getActivationDate() const;

	/** Returns the fingerprint of this certificate.
	  *
	  * @return the fingerprint of this certificate
	  */
	const byteArray getFingerprint(const DigestAlgorithm algo) const;


	// Implementation of 'certificate'
	const byteArray getEncoded() const;
	const string getType() const;
	int getVersion() const;
	bool equals(ref <const certificate> other) const;

private:

	class X509CertificateInternalData* m_data;
};


} // cert
} // security
} // vmime


#endif // VMIME_SECURITY_CERT_X509CERTIFICATE_HPP_INCLUDED

