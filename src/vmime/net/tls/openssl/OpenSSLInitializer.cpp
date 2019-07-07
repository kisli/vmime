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


#include "vmime/net/tls/openssl/OpenSSLInitializer.hpp"

#include "vmime/platform.hpp"

#include <openssl/opensslv.h>

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#	define OPENSSL_API_COMPAT 0x10100000L
#endif

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#if OPENSSL_VERSION_NUMBER >= 0x0907000L
#	include <openssl/conf.h>
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#	include "vmime/utility/sync/autoLock.hpp"
#	include "vmime/utility/sync/criticalSection.hpp"
#endif


// OpenSSL locking callbacks for multithreading support (< v1.1 only)
#if OPENSSL_VERSION_NUMBER < 0x10100000L

namespace {

vmime::shared_ptr <vmime::utility::sync::criticalSection >* g_openSSLMutexes = NULL;

extern "C" void VMime_OpenSSLCallback_lock(int mode, int n, const char* /* file */, int /* line */) {

	if (mode & CRYPTO_LOCK) {
		g_openSSLMutexes[n]->lock();
	} else {
		g_openSSLMutexes[n]->unlock();
	}
}

extern "C" unsigned long VMime_OpenSSLCallback_id() {

	return vmime::platform::getHandler()->getThreadId();
}

}

#endif


namespace vmime {
namespace net {
namespace tls {


OpenSSLInitializer::autoInitializer::autoInitializer() {

	// The construction of this unique 'oneTimeInitializer' object will be triggered
	// by the 'autoInitializer' objects from the other translation units
	static OpenSSLInitializer::oneTimeInitializer oneTimeInitializer;
}


OpenSSLInitializer::autoInitializer::~autoInitializer() {

}


OpenSSLInitializer::oneTimeInitializer::oneTimeInitializer() {

	initialize();
}


OpenSSLInitializer::oneTimeInitializer::~oneTimeInitializer() {

	uninitialize();
}


// static
void OpenSSLInitializer::initialize() {

#if OPENSSL_VERSION_NUMBER >= 0x0907000L && OPENSSL_VERSION_NUMBER < 0x10100000L
	OPENSSL_config(NULL);
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();

	int numMutexes = CRYPTO_num_locks();
	g_openSSLMutexes = new shared_ptr <vmime::utility::sync::criticalSection>[numMutexes];

	for (int i = 0 ; i < numMutexes ; ++i) {
		g_openSSLMutexes[i] = vmime::platform::getHandler()->createCriticalSection();
	}

	CRYPTO_set_locking_callback(OpenSSLCallback_lock);
	CRYPTO_set_id_callback(OpenSSLCallback_id);
#endif

	// Seed the RNG, in case /dev/urandom is not available. Explicitely calling
	// RAND_seed() even though /dev/urandom is available is harmless.
	enum {
		SEEDSIZE = 256
	};

	unsigned char seed[SEEDSIZE];
	vmime::platform::getHandler()->generateRandomBytes(seed, SEEDSIZE);
	RAND_seed(seed, SEEDSIZE);
}


// static
void OpenSSLInitializer::uninitialize() {

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	EVP_cleanup();
	ERR_free_strings();

	CRYPTO_set_locking_callback(NULL);
	CRYPTO_set_id_callback(NULL);

	delete [] g_openSSLMutexes;
	g_openSSLMutexes = NULL;
#endif

}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

