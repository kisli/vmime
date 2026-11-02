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

#include "tests/testUtils.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS


#include "vmime/net/tls/TLSSession.hpp"
#include "vmime/net/tls/TLSProperties.hpp"
#include "vmime/security/cert/defaultCertificateVerifier.hpp"


using vmime::net::tls::TLSSession;
using vmime::net::tls::TLSProperties;


VMIME_TEST_SUITE_BEGIN(TLSSessionTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testInvalidCipherSuite)
	VMIME_TEST_LIST_END


	// The GnuTLS session allocated by the constructor must be released
	// when the cipher suite is rejected (leak is reported by LeakSanitizer)
	void testInvalidCipherSuite() {

		vmime::shared_ptr <TLSProperties> props = vmime::make_shared <TLSProperties>();
		props->setCipherSuite("NOT-A-VALID-PRIORITY-STRING");

		VASSERT_THROW(
			"1",
			TLSSession::create(
				vmime::make_shared <vmime::security::cert::defaultCertificateVerifier>(),
				props
			),
			vmime::exceptions::tls_exception
		);
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS
