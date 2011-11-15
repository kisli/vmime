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
#if GNUTLS_VERSION_NUMBER < 0x030000
#include <gnutls/extra.h>
#endif

#include "vmime/config.hpp"

// Dependency on gcrypt is not needed since GNU TLS version 2.12.
// See here: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=638651
#if GNUTLS_VERSION_NUMBER <= 0x020b00
#	define VMIME_GNUTLS_NEEDS_GCRYPT  1
#endif

#if VMIME_HAVE_PTHREAD
#	include <pthread.h>
#	if VMIME_GNUTLS_NEEDS_GCRYPT
#		include <gcrypt.h>
#	endif
#	include <errno.h>
#endif // VMIME_HAVE_PTHREAD

#include "vmime/net/tls/TLSSession.hpp"

#include "vmime/exception.hpp"


// Enable GnuTLS debugging by defining GNUTLS_DEBUG
//#define GNUTLS_DEBUG 1


#include <sstream>
#include <iomanip>

#if VMIME_DEBUG && GNUTLS_DEBUG
	#include <iostream>
#endif // VMIME_DEBUG && GNUTLS_DEBUG


#if VMIME_HAVE_PTHREAD && VMIME_GNUTLS_NEEDS_GCRYPT && defined(GCRY_THREAD_OPTION_PTHREAD_IMPL)
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
	#if VMIME_GNUTLS_NEEDS_GCRYPT
		gcry_control(GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
	#endif // VMIME_GNUTLS_NEEDS_GCRYPT
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
#if HAVE_GNUTLS_PRIORITY_FUNCS
	gnutls_dh_set_prime_bits(*m_gnutlsSession, 128);

	if ((res = gnutls_priority_set_direct
		(*m_gnutlsSession, "NORMAL:%SSL3_RECORD_VERSION", NULL)) != 0)
	{
		if ((res = gnutls_priority_set_direct
			(*m_gnutlsSession, "NORMAL", NULL)) != 0)
		{
			throwTLSException
				("gnutls_priority_set_direct", res);
		}
	}

#else  // !HAVE_GNUTLS_PRIORITY_FUNCS

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

#endif // !HAVE_GNUTLS_PRIORITY_FUNCS

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
	std::ostringstream msg;

	msg << fname + "() returned code ";
	msg << std::hex << code;
	msg << ": ";
	msg << gnutls_strerror(code);

	throw exceptions::tls_exception(msg.str());
}


} // tls
} // net
} // vmime

