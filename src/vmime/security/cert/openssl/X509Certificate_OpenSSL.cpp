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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL


#include <cstdio>
#include <ctime>
#include <map>
#include <algorithm>

#include "vmime/security/cert/openssl/X509Certificate_OpenSSL.hpp"

#include "vmime/net/tls/openssl/OpenSSLInitializer.hpp"

#include "vmime/utility/outputStreamByteArrayAdapter.hpp"

#include "vmime/exception.hpp"

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

class monthMap
{
public:

	monthMap()
	{
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

	int getMonth(vmime::string mstr)
	{
		std::transform(mstr.begin(), mstr.end(), mstr.begin(), ::tolower);

		std::map <vmime::string, vmime::datetime::Months>::const_iterator
			c_it = m_monthMap.find(mstr);

		if (c_it != m_monthMap.end())
			return c_it->second;

		return -1;
	}

private:

	std::map<vmime::string, vmime::datetime::Months> m_monthMap;
};

static monthMap sg_monthMap;



struct OpenSSLX509CertificateInternalData
{
	OpenSSLX509CertificateInternalData()
	{
		cert = 0;
	}

	~OpenSSLX509CertificateInternalData()
	{
		if (cert)
			X509_free(cert);
	}

	X509* cert;
};

#endif // VMIME_BUILDING_DOC


X509Certificate_OpenSSL::X509Certificate_OpenSSL()
	: m_data(new OpenSSLX509CertificateInternalData)
{
}


X509Certificate_OpenSSL::X509Certificate_OpenSSL(X509* cert)
	: m_data(new OpenSSLX509CertificateInternalData)
{
	m_data->cert = X509_dup(cert);
}


X509Certificate_OpenSSL::X509Certificate_OpenSSL(const X509Certificate_OpenSSL&)
	: X509Certificate(), m_data(NULL)
{
	// Not used
}


X509Certificate_OpenSSL::~X509Certificate_OpenSSL()
{
	delete m_data;
}


void* X509Certificate_OpenSSL::getInternalData()
{
	return &m_data->cert;
}


// static
shared_ptr <X509Certificate> X509Certificate_OpenSSL::importInternal(X509* cert)
{
	if (cert)
		return make_shared <X509Certificate_OpenSSL>(reinterpret_cast <X509 *>(cert));

	return null;
}


// static
shared_ptr <X509Certificate> X509Certificate::import(utility::inputStream& is)
{
	byteArray bytes;
	byte_t chunk[4096];

	while (!is.eof())
	{
		const size_t len = is.read(chunk, sizeof(chunk));
		bytes.insert(bytes.end(), chunk, chunk + len);
	}

	return import(&bytes[0], bytes.size());
}


// static
shared_ptr <X509Certificate> X509Certificate::import
	(const byte_t* data, const size_t length)
{
	shared_ptr <X509Certificate_OpenSSL> cert = make_shared <X509Certificate_OpenSSL>();

	BIO* membio = BIO_new_mem_buf(const_cast <byte_t*>(data), static_cast <int>(length));

	if (!PEM_read_bio_X509(membio, &(cert->m_data->cert), 0, 0))
	{
		BIO_vfree(membio);
		return null;
	}

	BIO_vfree(membio);

	return cert;
}


void X509Certificate_OpenSSL::write
	(utility::outputStream& os, const Format format) const
{
	BIO* membio = 0;
	long dataSize = 0;
	unsigned char* out = 0;

	if (format == FORMAT_DER)
	{
		if ((dataSize = i2d_X509(m_data->cert, &out)) < 0)
			goto err;

		os.write(reinterpret_cast <byte_t*>(out), dataSize);
		os.flush();
		OPENSSL_free(out);
	}
	else if (format == FORMAT_PEM)
	{
		membio = BIO_new(BIO_s_mem());
		BIO_set_close(membio, BIO_CLOSE);

		if (!PEM_write_bio_X509(membio, m_data->cert))
			goto pem_err;

		dataSize = BIO_get_mem_data(membio, &out);
		os.write(reinterpret_cast <byte_t*>(out), dataSize);
		os.flush();
		BIO_vfree(membio);
	}
	else
	{
		throw vmime::exceptions::unsupported_certificate_type("Unknown cert type");
	}

	return; // #### Early Return ####

pem_err:
	{
		if (membio)
			BIO_vfree(membio);
	}

err:
	{
		char errstr[256];
		long ec = ERR_get_error();
		ERR_error_string(ec, errstr);
		throw vmime::exceptions::certificate_exception(
			"OpenSSLX509Certificate_OpenSSL::write exception - " + string(errstr));
	}
}


const byteArray X509Certificate_OpenSSL::getSerialNumber() const
{
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


bool X509Certificate_OpenSSL::checkIssuer(shared_ptr <const X509Certificate> cert_) const
{
	shared_ptr <const X509Certificate_OpenSSL> cert =
		dynamicCast <const X509Certificate_OpenSSL>(cert_);

	// Get issuer for this cert
	BIO *out;
	unsigned char *issuer;

	out = BIO_new(BIO_s_mem());
	X509_NAME_print_ex(out, X509_get_issuer_name(m_data->cert), 0, XN_FLAG_RFC2253);
	long n = BIO_get_mem_data(out, &issuer);
	vmime::string thisIssuerName((char*)issuer, n);
	BIO_free(out);

	// Get subject of issuer
	unsigned char *subject;
	out = BIO_new(BIO_s_mem());
	X509_NAME_print_ex(out, X509_get_subject_name(cert->m_data->cert), 0, XN_FLAG_RFC2253);
	n = BIO_get_mem_data(out, &subject);
	vmime::string subjOfIssuer((char*)subject, n);
	BIO_free(out);

	return subjOfIssuer == thisIssuerName;
}


bool X509Certificate_OpenSSL::verify(shared_ptr <const X509Certificate> caCert_) const
{
	shared_ptr <const X509Certificate_OpenSSL> caCert =
		dynamicCast <const X509Certificate_OpenSSL>(caCert_);


	bool verified = false;
	bool error = true;

	X509_STORE *store = X509_STORE_new();

	if (store)
	{
		X509_STORE_CTX *verifyCtx = X509_STORE_CTX_new();

		if (verifyCtx)
		{
			if (X509_STORE_add_cert(store, caCert->m_data->cert))
			{
				X509_STORE_CTX_init(verifyCtx, store, m_data->cert, NULL);

				int ret = X509_verify_cert(verifyCtx);

				if (ret == 1)
				{
					verified = true;
					error = false;
				}
				else if (ret == 0)
				{
					verified = false;
					error = false;
				}

				//X509_verify_cert_error_string(vrfy_ctx->error)

				X509_STORE_CTX_free(verifyCtx);
			}
		}

		X509_STORE_free(store);
	}

	return verified && !error;
}


// static
bool X509Certificate_OpenSSL::cnMatch(const char* cnBuf, const char* host)
{
	// Right-to-left match, looking for a '*' wildcard
	const bool hasWildcard = (strlen(cnBuf) > 1 && cnBuf[0] == '*' && cnBuf[1] == '.');
	const char* cnBufReverseEndPtr = (cnBuf + (hasWildcard ? 2 : 0));
	const char* hostPtr = host + strlen(host);
	const char* cnPtr = cnBuf + strlen(cnBuf);

	bool matches = true;

	while (matches && --hostPtr >= host && --cnPtr >= cnBufReverseEndPtr)
		matches = (toupper(*hostPtr) == toupper(*cnPtr));

	return matches;
}


bool X509Certificate_OpenSSL::verifyHostName
	(const string& hostname, std::vector <std::string>* nonMatchingNames) const
{
	// First, check subject common name against hostname
	char CNBuffer[1024];
	CNBuffer[sizeof(CNBuffer) - 1] = '\0';

	X509_NAME* xname = X509_get_subject_name(m_data->cert);

	if (X509_NAME_get_text_by_NID(xname, NID_commonName, CNBuffer, sizeof(CNBuffer)) != -1)
	{
		if (cnMatch(CNBuffer, hostname.c_str()))
			return true;

		if (nonMatchingNames)
			nonMatchingNames->push_back(CNBuffer);
	}

	// Now, look in subject alternative names
	for (int i = 0, extCount = X509_get_ext_count(m_data->cert) ; i < extCount ; ++i)
	{
		X509_EXTENSION* ext = X509_get_ext(m_data->cert, i);
		const char* extStr = OBJ_nid2sn(OBJ_obj2nid(X509_EXTENSION_get_object(ext)));

		if (strcmp(extStr, "subjectAltName") == 0)
		{
			const X509V3_EXT_METHOD* method;

			if ((method = X509V3_EXT_get(ext)) != NULL)
			{
				const unsigned char* extVal = ext->value->data;
				void *extValStr;

				if (method->it)
				{
					extValStr = ASN1_item_d2i
						(NULL, &extVal, ext->value->length, ASN1_ITEM_ptr(method->it));
				}
				else
				{
					extValStr = method->d2i
						(NULL, &extVal, ext->value->length);
				}

				if (extValStr && method->i2v)
				{
					STACK_OF(CONF_VALUE)* val = method->i2v(method, extValStr, NULL);

					for (int j = 0 ; j < sk_CONF_VALUE_num(val) ; ++j)
					{
						CONF_VALUE* cnf = sk_CONF_VALUE_value(val, j);

						if ((strcasecmp(cnf->name, "DNS") == 0 &&
							 strcasecmp(cnf->value, hostname.c_str()) == 0)
							 ||
							(strncasecmp(cnf->name, "IP", 2) == 0 &&
							 strcasecmp(cnf->value, hostname.c_str()) == 0))
						{
							return true;
						}

						if (nonMatchingNames)
							nonMatchingNames->push_back(cnf->value);
					}
				}
			}
		}
	}

	return false;
}


const datetime X509Certificate_OpenSSL::convertX509Date(void* time) const
{
	char* buffer;
	BIO* out = BIO_new(BIO_s_mem());
	BIO_set_close(out, BIO_CLOSE);

	ASN1_TIME* asn1_time = reinterpret_cast<ASN1_TIME*>(time);
	ASN1_TIME_print(out, asn1_time);

	int sz = BIO_get_mem_data(out, &buffer);
	char* dest = new char[sz + 1];
	dest[sz] = 0;
	memcpy(dest, buffer, sz);
	vmime::string t(dest);

	BIO_free(out);
	delete dest;

	if (t.size() > 0)
	{
		char month[4] = {0};
		char zone[4] = {0};
		int day, hour, minute, second, year;
		int nrconv = sscanf(t.c_str(), "%s %2d %02d:%02d:%02d %d%s", month, &day, &hour, &minute, &second,&year,zone);

		if (nrconv >= 6)
			return datetime(year, sg_monthMap.getMonth(vmime::string(month)), day, hour, minute, second);
	}

	// let datetime try and parse it
	return datetime(t);
}


const datetime X509Certificate_OpenSSL::getActivationDate() const
{
	return convertX509Date(X509_get_notBefore(m_data->cert));
}


const datetime X509Certificate_OpenSSL::getExpirationDate() const
{
	return convertX509Date(X509_get_notAfter(m_data->cert));
}


const byteArray X509Certificate_OpenSSL::getFingerprint(const DigestAlgorithm algo) const
{
	BIO *out;
	int j;
	unsigned int n;
	const EVP_MD *digest;
	unsigned char * fingerprint, *result;
	unsigned char md[EVP_MAX_MD_SIZE];

	switch (algo)
	{
	case DIGEST_MD5:

		digest = EVP_md5();
		break;

	default:
	case DIGEST_SHA1:

		digest = EVP_sha1();
		break;
	}

	out = BIO_new(BIO_s_mem());
	BIO_set_close(out, BIO_CLOSE);

	if (X509_digest(m_data->cert, digest, md, &n))
	{
		for (j=0; j<(int)n; j++)
		{
			BIO_printf (out, "%02X",md[j]);
			if (j+1 != (int)n) BIO_printf(out, ":");
		}
	}

	n = BIO_get_mem_data(out, &fingerprint);
	result = new unsigned char[n];
	memcpy (result, fingerprint, n);
	BIO_free(out);

	byteArray res;
	res.insert(res.end(), &result[0], &result[0] + n);

	delete [] result;

	return res;
}


const byteArray X509Certificate_OpenSSL::getEncoded() const
{
	byteArray bytes;
	utility::outputStreamByteArrayAdapter os(bytes);

	write(os, FORMAT_DER);

	return bytes;
}


const string X509Certificate_OpenSSL::getIssuerString() const
{
	// Get issuer for this cert
	BIO* out = BIO_new(BIO_s_mem());
	X509_NAME_print_ex(out, X509_get_issuer_name(m_data->cert), 0, XN_FLAG_RFC2253);

	unsigned char* issuer;
	const int n = BIO_get_mem_data(out, &issuer);

	vmime::string name(reinterpret_cast <char*>(issuer), n);
	BIO_free(out);

	return name;
}


const string X509Certificate_OpenSSL::getType() const
{
	return "X.509";
}


int X509Certificate_OpenSSL::getVersion() const
{
	return (int)X509_get_version(m_data->cert);
}


bool X509Certificate_OpenSSL::equals(shared_ptr <const certificate> other) const
{
	shared_ptr <const X509Certificate_OpenSSL> otherX509 =
		dynamicCast <const X509Certificate_OpenSSL>(other);

	if (!otherX509)
		return false;

	const byteArray fp1 = getFingerprint(DIGEST_MD5);
	const byteArray fp2 = otherX509->getFingerprint(DIGEST_MD5);

	return fp1 == fp2;
}


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

