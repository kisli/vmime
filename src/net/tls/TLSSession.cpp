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
#include <gnutls/extra.h>

#include "vmime/config.hpp"

#if VMIME_HAVE_PTHREAD
#	include <pthread.h>
#	include <gcrypt.h>
#	include <errno.h>
#endif // VMIME_HAVE_PTHREAD

#include "vmime/net/tls/TLSSession.hpp"

#include "vmime/exception.hpp"


// Enable GnuTLS debugging by defining GNUTLS_DEBUG
//#define GNUTLS_DEBUG 1


#if VMIME_DEBUG && GNUTLS_DEBUG
	#include <iostream>
#endif // VMIME_DEBUG && GNUTLS_DEBUG


#if VMIME_HAVE_PTHREAD && defined(GCRY_THREAD_OPTION_PTHREAD_IMPL)
extern "C"
{
	GCRY_THREAD_OPTION_PTHREAD_IMPL;
}
#endif // VMIME_HAVE_PTHREAD && defined(GCRY_THREAD_OPTION_PTHREAD_IMPL


namespace vmime {
namespace net {
namespace tls {


#ifndef VMIME_BUILDING_DOC

// Initialize GNU TLS library
struct TLSGlobal
{
	TLSGlobal()
	{
#if VMIME_HAVE_PTHREAD && defined(GCRY_THREAD_OPTION_PTHREAD_IMPL)
		gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
#endif // VMIME_HAVE_PTHREAD && defined(GCRY_THREAD_OPTION_PTHREAD_IMPL

		gnutls_global_init();
		//gnutls_global_init_extra();

#if VMIME_DEBUG && GNUTLS_DEBUG
		gnutls_global_set_log_function(TLSLogFunc);
		gnutls_global_set_log_level(10);
#endif // VMIME_DEBUG && GNUTLS_DEBUG

		gnutls_anon_allocate_client_credentials(&anonCred);
		gnutls_certificate_allocate_credentials(&certCred);
	}

	~TLSGlobal()
	{
		gnutls_anon_free_client_credentials(anonCred);
		gnutls_certificate_free_credentials(certCred);

		gnutls_global_deinit();
	}

#if VMIME_DEBUG && GNUTLS_DEBUG

	static void TLSLogFunc(int level, const char *str)
	{
		std::cerr << "GNUTLS: [" << level << "] " << str << std::endl;
	}

#endif // VMIME_DEBUG && GNUTLS_DEBUG


	gnutls_anon_client_credentials anonCred;
	gnutls_certificate_credentials certCred;
};

static TLSGlobal g_gnutlsGlobal;


#endif // VMIME_BUILDING_DOC



TLSSession::TLSSession(ref <security::cert::certificateVerifier> cv)
	: m_certVerifier(cv)
{
	int res;

	m_gnutlsSession = new gnutls_session;

	if (gnutls_init(m_gnutlsSession, GNUTLS_CLIENT) != 0)
		throw std::bad_alloc();

	// Sets some default priority on the ciphers, key exchange methods,
	// macs and compression methods.
	gnutls_set_default_priority(*m_gnutlsSession);

	// Sets the priority on the certificate types supported by gnutls.
	// Priority is higher for types specified before others. After
	// specifying the types you want, you must append a 0.
	const int certTypePriority[] = { GNUTLS_CRT_X509, 0 };

	res = gnutls_certificate_type_set_priority
		(*m_gnutlsSession, certTypePriority);

	if (res < 0)
	{
		throwTLSException
			("gnutls_certificate_type_set_priority", res);
	}

	// Sets the priority on the protocol types
	const int protoPriority[] = { GNUTLS_TLS1, GNUTLS_SSL3, 0 };

	res = gnutls_protocol_set_priority(*m_gnutlsSession, protoPriority);

	if (res < 0)
	{
		throwTLSException
			("gnutls_certificate_type_set_priority", res);
	}

	// Priority on the ciphers
	const int cipherPriority[] =
	{
		GNUTLS_CIPHER_ARCFOUR_128,
		GNUTLS_CIPHER_3DES_CBC,
		GNUTLS_CIPHER_AES_128_CBC,
		GNUTLS_CIPHER_AES_256_CBC,
		GNUTLS_CIPHER_ARCFOUR_40,
		GNUTLS_CIPHER_RC2_40_CBC,
		GNUTLS_CIPHER_DES_CBC,
		0
	};

	gnutls_cipher_set_priority(*m_gnutlsSession, cipherPriority);

	// Priority on MACs
	const int macPriority[] = { GNUTLS_MAC_SHA, GNUTLS_MAC_MD5, 0};

	gnutls_mac_set_priority(*m_gnutlsSession, macPriority);

	// Priority on key exchange methods
	const int kxPriority[] =
	{
		GNUTLS_KX_RSA,
		GNUTLS_KX_DHE_DSS,
		GNUTLS_KX_DHE_RSA,
		GNUTLS_KX_ANON_DH,
		GNUTLS_KX_SRP,
		GNUTLS_KX_RSA_EXPORT,
		GNUTLS_KX_SRP_RSA,
		GNUTLS_KX_SRP_DSS,
		0
	};

	gnutls_kx_set_priority(*m_gnutlsSession, kxPriority);

	// Priority on compression methods
	const int compressionPriority[] =
	{
		GNUTLS_COMP_ZLIB,
		//GNUTLS_COMP_LZO,
		GNUTLS_COMP_NULL,
		0
	};

	gnutls_compression_set_priority(*m_gnutlsSession, compressionPriority);

	// Initialize credentials
	gnutls_credentials_set(*m_gnutlsSession,
		GNUTLS_CRD_ANON, g_gnutlsGlobal.anonCred);

	gnutls_credentials_set(*m_gnutlsSession,
		GNUTLS_CRD_CERTIFICATE, g_gnutlsGlobal.certCred);
}


TLSSession::TLSSession(const TLSSession&)
	: object()
{
	// Not used
}


TLSSession::~TLSSession()
{
	if (m_gnutlsSession)
	{
		gnutls_deinit(*m_gnutlsSession);

		delete m_gnutlsSession;
		m_gnutlsSession = NULL;
	}
}


ref <TLSSocket> TLSSession::getSocket(ref <socket> sok)
{
	return vmime::create <TLSSocket>
		(thisRef().dynamicCast <TLSSession>(), sok);
}


ref <security::cert::certificateVerifier> TLSSession::getCertificateVerifier()
{
	return m_certVerifier;
}


void TLSSession::throwTLSException(const string& fname, const int code)
{
	string msg = fname + "() returned ";

#define ERROR(x) \
	case x: msg += #x; break;

	switch (code)
	{
	ERROR(GNUTLS_E_SUCCESS)
	ERROR(GNUTLS_E_UNKNOWN_COMPRESSION_ALGORITHM)
	ERROR(GNUTLS_E_UNKNOWN_CIPHER_TYPE)
	ERROR(GNUTLS_E_LARGE_PACKET)
	ERROR(GNUTLS_E_UNSUPPORTED_VERSION_PACKET)
	ERROR(GNUTLS_E_UNEXPECTED_PACKET_LENGTH)
	ERROR(GNUTLS_E_INVALID_SESSION)
	ERROR(GNUTLS_E_FATAL_ALERT_RECEIVED)
	ERROR(GNUTLS_E_UNEXPECTED_PACKET)
	ERROR(GNUTLS_E_WARNING_ALERT_RECEIVED)
	ERROR(GNUTLS_E_ERROR_IN_FINISHED_PACKET)
	ERROR(GNUTLS_E_UNEXPECTED_HANDSHAKE_PACKET)
	ERROR(GNUTLS_E_UNKNOWN_CIPHER_SUITE)
	ERROR(GNUTLS_E_UNWANTED_ALGORITHM)
	ERROR(GNUTLS_E_MPI_SCAN_FAILED)
	ERROR(GNUTLS_E_DECRYPTION_FAILED)
	ERROR(GNUTLS_E_MEMORY_ERROR)
	ERROR(GNUTLS_E_DECOMPRESSION_FAILED)
	ERROR(GNUTLS_E_COMPRESSION_FAILED)
	ERROR(GNUTLS_E_AGAIN)
	ERROR(GNUTLS_E_EXPIRED)
	ERROR(GNUTLS_E_DB_ERROR)
	ERROR(GNUTLS_E_SRP_PWD_ERROR)
	ERROR(GNUTLS_E_INSUFFICIENT_CREDENTIALS)
	ERROR(GNUTLS_E_HASH_FAILED)
	ERROR(GNUTLS_E_BASE64_DECODING_ERROR)
	ERROR(GNUTLS_E_MPI_PRINT_FAILED)
	ERROR(GNUTLS_E_REHANDSHAKE)
	ERROR(GNUTLS_E_GOT_APPLICATION_DATA)
	ERROR(GNUTLS_E_RECORD_LIMIT_REACHED)
	ERROR(GNUTLS_E_ENCRYPTION_FAILED)
	ERROR(GNUTLS_E_PK_ENCRYPTION_FAILED)
	ERROR(GNUTLS_E_PK_DECRYPTION_FAILED)
	ERROR(GNUTLS_E_PK_SIGN_FAILED)
	ERROR(GNUTLS_E_X509_UNSUPPORTED_CRITICAL_EXTENSION)
	ERROR(GNUTLS_E_KEY_USAGE_VIOLATION)
	ERROR(GNUTLS_E_NO_CERTIFICATE_FOUND)
	ERROR(GNUTLS_E_INVALID_REQUEST)
	ERROR(GNUTLS_E_SHORT_MEMORY_BUFFER)
	ERROR(GNUTLS_E_INTERRUPTED)
	ERROR(GNUTLS_E_PUSH_ERROR)
	ERROR(GNUTLS_E_PULL_ERROR)
	ERROR(GNUTLS_E_RECEIVED_ILLEGAL_PARAMETER)
	ERROR(GNUTLS_E_REQUESTED_DATA_NOT_AVAILABLE)
	ERROR(GNUTLS_E_PKCS1_WRONG_PAD)
	ERROR(GNUTLS_E_RECEIVED_ILLEGAL_EXTENSION)
	ERROR(GNUTLS_E_INTERNAL_ERROR)
	ERROR(GNUTLS_E_DH_PRIME_UNACCEPTABLE)
	ERROR(GNUTLS_E_FILE_ERROR)
	ERROR(GNUTLS_E_TOO_MANY_EMPTY_PACKETS)
	ERROR(GNUTLS_E_UNKNOWN_PK_ALGORITHM)
	ERROR(GNUTLS_E_INIT_LIBEXTRA)
	ERROR(GNUTLS_E_LIBRARY_VERSION_MISMATCH)
	ERROR(GNUTLS_E_NO_TEMPORARY_RSA_PARAMS)
	ERROR(GNUTLS_E_LZO_INIT_FAILED)
	ERROR(GNUTLS_E_NO_COMPRESSION_ALGORITHMS)
	ERROR(GNUTLS_E_NO_CIPHER_SUITES)
	ERROR(GNUTLS_E_OPENPGP_GETKEY_FAILED)
	ERROR(GNUTLS_E_PK_SIG_VERIFY_FAILED)
	ERROR(GNUTLS_E_ILLEGAL_SRP_USERNAME)
	ERROR(GNUTLS_E_SRP_PWD_PARSING_ERROR)
	ERROR(GNUTLS_E_NO_TEMPORARY_DH_PARAMS)
	ERROR(GNUTLS_E_ASN1_ELEMENT_NOT_FOUND)
	ERROR(GNUTLS_E_ASN1_IDENTIFIER_NOT_FOUND)
	ERROR(GNUTLS_E_ASN1_DER_ERROR)
	ERROR(GNUTLS_E_ASN1_VALUE_NOT_FOUND)
	ERROR(GNUTLS_E_ASN1_GENERIC_ERROR)
	ERROR(GNUTLS_E_ASN1_VALUE_NOT_VALID)
	ERROR(GNUTLS_E_ASN1_TAG_ERROR)
	ERROR(GNUTLS_E_ASN1_TAG_IMPLICIT)
	ERROR(GNUTLS_E_ASN1_TYPE_ANY_ERROR)
	ERROR(GNUTLS_E_ASN1_SYNTAX_ERROR)
	ERROR(GNUTLS_E_ASN1_DER_OVERFLOW)
	//ERROR(GNUTLS_E_OPENPGP_TRUSTDB_VERSION_UNSUPPORTED)
	ERROR(GNUTLS_E_OPENPGP_UID_REVOKED)
	ERROR(GNUTLS_E_CERTIFICATE_ERROR)
	//ERROR(GNUTLS_E_X509_CERTIFICATE_ERROR)
	ERROR(GNUTLS_E_CERTIFICATE_KEY_MISMATCH)
	ERROR(GNUTLS_E_UNSUPPORTED_CERTIFICATE_TYPE)
	ERROR(GNUTLS_E_X509_UNKNOWN_SAN)
	ERROR(GNUTLS_E_OPENPGP_FINGERPRINT_UNSUPPORTED)
	ERROR(GNUTLS_E_X509_UNSUPPORTED_ATTRIBUTE)
	ERROR(GNUTLS_E_UNKNOWN_HASH_ALGORITHM)
	ERROR(GNUTLS_E_UNKNOWN_PKCS_CONTENT_TYPE)
	ERROR(GNUTLS_E_UNKNOWN_PKCS_BAG_TYPE)
	ERROR(GNUTLS_E_INVALID_PASSWORD)
	ERROR(GNUTLS_E_MAC_VERIFY_FAILED)
	ERROR(GNUTLS_E_CONSTRAINT_ERROR)
	ERROR(GNUTLS_E_BASE64_ENCODING_ERROR)
	ERROR(GNUTLS_E_INCOMPATIBLE_GCRYPT_LIBRARY)
	//ERROR(GNUTLS_E_INCOMPATIBLE_CRYPTO_LIBRARY)
	ERROR(GNUTLS_E_INCOMPATIBLE_LIBTASN1_LIBRARY)
	ERROR(GNUTLS_E_OPENPGP_KEYRING_ERROR)
	ERROR(GNUTLS_E_X509_UNSUPPORTED_OID)
	//ERROR(GNUTLS_E_RANDOM_FAILED)
	ERROR(GNUTLS_E_UNIMPLEMENTED_FEATURE)

	default:

		msg += "unknown error";
		break;
	}

#undef ERROR

	throw exceptions::tls_exception(msg);
}


} // tls
} // net
} // vmime

