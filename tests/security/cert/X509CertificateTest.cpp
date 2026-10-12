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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "vmime/security/cert/X509Certificate.hpp"


// Self-signed certificate for CN=mail.vmime.org, with subject alternative
// names DNS:mail.vmime.org, IP:192.0.2.1 and IP:2001:db8::1
static const char testCertificatePEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBvTCCAWSgAwIBAgIUYrH3DcV2gtgaaXI2HoVgAlX5y0UwCgYIKoZIzj0EAwIw\n"
	"GTEXMBUGA1UEAwwObWFpbC52bWltZS5vcmcwIBcNMjYxMDEwMTczMDIyWhgPMjEy\n"
	"NjA5MTYxNzMwMjJaMBkxFzAVBgNVBAMMDm1haWwudm1pbWUub3JnMFkwEwYHKoZI\n"
	"zj0CAQYIKoZIzj0DAQcDQgAEl8CflV/LHMqyY99K6GEj5NULOdEGgEN1q9ZqX8mo\n"
	"FgMnz2NgQcojSfjQZMiiY0TxVjfI19Swy2ZguwkI2UPsHaOBhzCBhDAdBgNVHQ4E\n"
	"FgQUi6ALUpBpLCujEvuanLdVY2rv1V4wHwYDVR0jBBgwFoAUi6ALUpBpLCujEvua\n"
	"nLdVY2rv1V4wDwYDVR0TAQH/BAUwAwEB/zAxBgNVHREEKjAogg5tYWlsLnZtaW1l\n"
	"Lm9yZ4cEwAACAYcQIAENuAAAAAAAAAAAAAAAATAKBggqhkjOPQQDAgNHADBEAiAy\n"
	"3LiV2LL/eSryByB0crI1iB7lPcxisjWcs/J8r/yK8AIgCWeDkeuzkhhRDP71Vcdy\n"
	"Gaor91P2kUwL1MRIKAbkK/w=\n"
	"-----END CERTIFICATE-----\n";



VMIME_TEST_SUITE_BEGIN(X509CertificateTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testVerifyHostName)
		VMIME_TEST(testVerifyHostNameIPv4)
		VMIME_TEST(testVerifyHostNameIPv6)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::security::cert::X509Certificate> importTestCertificate() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert =
			vmime::security::cert::X509Certificate::import(
				reinterpret_cast <const vmime::byte_t*>(testCertificatePEM),
				sizeof(testCertificatePEM) - 1
			);

		VASSERT_NOT_NULL("Import", cert);

		return cert;
	}

	void testVerifyHostName() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert = importTestCertificate();

		VASSERT_TRUE("Match", cert->verifyHostName("mail.vmime.org"));
		VASSERT_FALSE("No match", cert->verifyHostName("www.vmime.org"));
	}

	void testVerifyHostNameIPv4() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert = importTestCertificate();

		VASSERT_TRUE("Match", cert->verifyHostName("192.0.2.1"));
		VASSERT_FALSE("No match", cert->verifyHostName("192.0.2.2"));
	}

	void testVerifyHostNameIPv6() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert = importTestCertificate();

		VASSERT_TRUE("Match", cert->verifyHostName("2001:db8::1"));
		VASSERT_FALSE("No match", cert->verifyHostName("2001:db8::2"));
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT
