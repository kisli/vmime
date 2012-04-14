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

#include <gnutls/gnutls.h>
#include <gnutls/x509.h>

#include <ctime>

#include "vmime/security/cert/X509Certificate.hpp"

#include "vmime/utility/outputStreamByteArrayAdapter.hpp"


namespace vmime {
namespace security {
namespace cert {


#ifndef VMIME_BUILDING_DOC

struct X509CertificateInternalData
{
	X509CertificateInternalData()
	{
		gnutls_x509_crt_init(&cert);
	}

	~X509CertificateInternalData()
	{
		gnutls_x509_crt_deinit(cert);
	}


	gnutls_x509_crt cert;
};

#endif // VMIME_BUILDING_DOC


X509Certificate::X509Certificate()
	: m_data(new X509CertificateInternalData)
{
}


X509Certificate::X509Certificate(const X509Certificate&)
	: certificate(), m_data(NULL)
{
	// Not used
}


X509Certificate::~X509Certificate()
{
	delete m_data;
}


// static
ref <X509Certificate> X509Certificate::import(utility::inputStream& is)
{
	byteArray bytes;
	utility::stream::value_type chunk[4096];

	while (!is.eof())
	{
		const int len = is.read(chunk, sizeof(chunk));
		bytes.insert(bytes.end(), chunk, chunk + len);
	}

	return import(&bytes[0], bytes.size());
}


// static
ref <X509Certificate> X509Certificate::import
	(const byte_t* data, const unsigned int length)
{
	gnutls_datum buffer;
	buffer.data = const_cast <byte_t*>(data);
	buffer.size = length;

	// Try DER format
	ref <X509Certificate> derCert = vmime::create <X509Certificate>();

	if (gnutls_x509_crt_import(derCert->m_data->cert, &buffer, GNUTLS_X509_FMT_DER) >= 0)
		return derCert;

	// Try PEM format
	ref <X509Certificate> pemCert = vmime::create <X509Certificate>();

	if (gnutls_x509_crt_import(pemCert->m_data->cert, &buffer, GNUTLS_X509_FMT_PEM) >= 0)
		return pemCert;

	return NULL;
}


void X509Certificate::write
	(utility::outputStream& os, const Format format) const
{
	size_t dataSize = 0;
	gnutls_x509_crt_fmt fmt = GNUTLS_X509_FMT_DER;

	switch (format)
	{
	case FORMAT_DER: fmt = GNUTLS_X509_FMT_DER; break;
	case FORMAT_PEM: fmt = GNUTLS_X509_FMT_PEM; break;
	}

	gnutls_x509_crt_export(m_data->cert, fmt, NULL, &dataSize);

	std::vector <byte_t> data(dataSize);

	gnutls_x509_crt_export(m_data->cert, fmt, &data[0], &dataSize);

	os.write(reinterpret_cast <utility::stream::value_type*>(&data[0]), dataSize);
}


const byteArray X509Certificate::getSerialNumber() const
{
	char serial[64];
	size_t serialSize = sizeof(serial);

	gnutls_x509_crt_get_serial(m_data->cert, serial, &serialSize);

	return byteArray(serial, serial + serialSize);
}


bool X509Certificate::checkIssuer(ref <const X509Certificate> issuer) const
{
	return (gnutls_x509_crt_check_issuer
			(m_data->cert, issuer->m_data->cert) >= 1);
}


bool X509Certificate::verify(ref <const X509Certificate> caCert) const
{
	unsigned int verify = 0;

	const int res = gnutls_x509_crt_verify
		(m_data->cert, &(caCert->m_data->cert), 1,
		 GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT,
		 &verify);

	return (res == 0 && verify == 0);
}


const datetime X509Certificate::getActivationDate() const
{
	const time_t t = gnutls_x509_crt_get_activation_time(m_data->cert);
	return datetime(t);
}


const datetime X509Certificate::getExpirationDate() const
{
	const time_t t = gnutls_x509_crt_get_expiration_time(m_data->cert);
	return datetime(t);
}


const byteArray X509Certificate::getFingerprint(const DigestAlgorithm algo) const
{
	gnutls_digest_algorithm galgo;

	switch (algo)
	{
	case DIGEST_MD5:

		galgo = GNUTLS_DIG_MD5;
		break;

	default:
	case DIGEST_SHA1:

		galgo = GNUTLS_DIG_SHA;
		break;
	}

	size_t bufferSize = 0;
	gnutls_x509_crt_get_fingerprint
		(m_data->cert, galgo, NULL, &bufferSize);

	std::vector <byte_t> buffer(bufferSize);

	if (gnutls_x509_crt_get_fingerprint
		(m_data->cert, galgo, &buffer[0], &bufferSize) == 0)
	{
		byteArray res;
		res.insert(res.end(), &buffer[0], &buffer[0] + bufferSize);

		return res;
	}

	return byteArray();
}


const byteArray X509Certificate::getEncoded() const
{
	byteArray bytes;
	utility::outputStreamByteArrayAdapter os(bytes);

	write(os, FORMAT_DER);

	return bytes;
}


const string X509Certificate::getType() const
{
	return "X.509";
}


int X509Certificate::getVersion() const
{
	return gnutls_x509_crt_get_version(m_data->cert);
}


bool X509Certificate::equals(ref <const certificate> other) const
{
	ref <const X509Certificate> otherX509 =
		other.dynamicCast <const X509Certificate>();

	if (!otherX509)
		return false;

	const byteArray fp1 = getFingerprint(DIGEST_MD5);
	const byteArray fp2 = otherX509->getFingerprint(DIGEST_MD5);

	return fp1 == fp2;
}


} // cert
} // security
} // vmime

