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


#include "vmime/net/tls/openssl/OpenSSLInitializer.hpp"

#include "vmime/utility/sync/autoLock.hpp"
#include "vmime/utility/sync/criticalSection.hpp"

#include "vmime/platform.hpp"

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

#if OPENSSL_VERSION_NUMBER >= 0x0907000L
#	include <openssl/conf.h>
#endif


namespace vmime {
namespace net {
namespace tls {


shared_ptr <vmime::utility::sync::criticalSection >* OpenSSLInitializer::sm_mutexes;


OpenSSLInitializer::autoInitializer::autoInitializer()
{
	// The construction of this unique 'oneTimeInitializer' object will be triggered
	// by the 'autoInitializer' objects from the other translation units
	static OpenSSLInitializer::oneTimeInitializer oneTimeInitializer;
}


OpenSSLInitializer::autoInitializer::~autoInitializer()
{
}


OpenSSLInitializer::oneTimeInitializer::oneTimeInitializer()
{
	initialize();
}


OpenSSLInitializer::oneTimeInitializer::~oneTimeInitializer()
{
	uninitialize();
}


// static
void OpenSSLInitializer::initialize()
{
#if OPENSSL_VERSION_NUMBER >= 0x0907000L
	OPENSSL_config(NULL);
#endif

	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();

	unsigned char seed[SEEDSIZE];
	vmime::platform::getHandler()->generateRandomBytes(seed, SEEDSIZE);
	RAND_seed(seed, SEEDSIZE);

	int numMutexes = CRYPTO_num_locks();
	sm_mutexes = new shared_ptr <vmime::utility::sync::criticalSection>[numMutexes];

	for (int i = 0 ; i < numMutexes ; ++i)
		sm_mutexes[i] = vmime::platform::getHandler()->createCriticalSection();

	CRYPTO_set_locking_callback(&OpenSSLInitializer::lock);
	CRYPTO_set_id_callback(&OpenSSLInitializer::id);
}


// static
void OpenSSLInitializer::uninitialize()
{
	EVP_cleanup();
	ERR_free_strings();

	CRYPTO_set_locking_callback(NULL);
	CRYPTO_set_id_callback(NULL);

	delete [] sm_mutexes;
}


// static
void OpenSSLInitializer::lock(int mode, int n, const char* /* file */, int /* line */)
{
	if (mode & CRYPTO_LOCK)
		sm_mutexes[n]->lock();
	else
		sm_mutexes[n]->unlock();
}


// static
unsigned long OpenSSLInitializer::id()
{
	return vmime::platform::getHandler()->getThreadId();
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

