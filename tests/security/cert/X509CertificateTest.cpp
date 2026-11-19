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

#include <cstring>


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

// Self-signed certificate for *.vmime.org (common name and subject
// alternative name)
static const char wildcardCertificatePEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBSTCB8KADAgECAhQHjRzftcHILtJRA3QlxMpsXJWtvDAKBggqhkjOPQQDAjAW\n"
	"MRQwEgYDVQQDDAsqLnZtaW1lLm9yZzAgFw0yNjExMTkwMDAwMDBaGA8yMTI2MTAy\n"
	"NjAwMDAwMFowFjEUMBIGA1UEAwwLKi52bWltZS5vcmcwWTATBgcqhkjOPQIBBggq\n"
	"hkjOPQMBBwNCAAQ6v1lmkhuZQQI3PJgWg99Es/j2nweN+eBMCA1zpcbBgNW1V5rV\n"
	"cs0Sr6BzFf/S40t/s/XKx2llbNCJyt0tKAMUoxowGDAWBgNVHREEDzANggsqLnZt\n"
	"aW1lLm9yZzAKBggqhkjOPQQDAgNIADBFAiEA5hNED3t0/Syq27W77lFP/8XBRl2I\n"
	"714dkl9l3iEqyEcCIHUix5jdhzB5Xr3W3dyRbgky2K/k+JFLFsqVfSy9rFHx\n"
	"-----END CERTIFICATE-----\n";

// Self-signed certificate for CN=mail.vmime.org, with the subject
// alternative name DNS:smtp.vmime.org
static const char subjectAltNameCertificatePEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBUTCB+aADAgECAhR5YRJW6C6r8wTO4w7i5ubZUFqbsjAKBggqhkjOPQQDAjAZ\n"
	"MRcwFQYDVQQDDA5tYWlsLnZtaW1lLm9yZzAgFw0yNjExMTkwMDAwMDBaGA8yMTI2\n"
	"MTAyNjAwMDAwMFowGTEXMBUGA1UEAwwObWFpbC52bWltZS5vcmcwWTATBgcqhkjO\n"
	"PQIBBggqhkjOPQMBBwNCAAQGsDb9T9cte1aHsCUfhtmSCk6xfa11Lp8H2NqfHwMx\n"
	"njnKEOzFW23nvVbxWn5PZycb/qstW9GeS4iOlM2+yEXsox0wGzAZBgNVHREEEjAQ\n"
	"gg5zbXRwLnZtaW1lLm9yZzAKBggqhkjOPQQDAgNHADBEAiB8uua2aVKv3fGS1USs\n"
	"6P44my6tkpQ45DX22X4CR44MxQIgUv9etOn6gTdMpz0qu4xLw4BwOWkOQTS329et\n"
	"iqfSuGg=\n"
	"-----END CERTIFICATE-----\n";

// Self-signed certificate for CN=mail.vmime.org, without subject
// alternative names
static const char commonNameCertificatePEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBMzCB2qADAgECAhQd8YQSnEnwsCKji9Lo7XI5r62zAjAKBggqhkjOPQQDAjAZ\n"
	"MRcwFQYDVQQDDA5tYWlsLnZtaW1lLm9yZzAgFw0yNjExMTkwMDAwMDBaGA8yMTI2\n"
	"MTAyNjAwMDAwMFowGTEXMBUGA1UEAwwObWFpbC52bWltZS5vcmcwWTATBgcqhkjO\n"
	"PQIBBggqhkjOPQMBBwNCAAT6mvctqsJVhZQ1ncjTCiQR9cyF5CAUe+k7ih+DQmbe\n"
	"8Cziu+7DuiSxapHZtEPjUm6AQuaTF2bgKP1a4yQu5m+qMAoGCCqGSM49BAMCA0gA\n"
	"MEUCIBopM92d1KZEBbi66j1+9GxwDHcQTt5DanwBfkM0uWoJAiEAvokgbimTIyqR\n"
	"Grm9jkJvPRubvMtr9P1mhdRMZ2r4Cxw=\n"
	"-----END CERTIFICATE-----\n";

// Self-signed certificate for CN=mail.vmime.org\0.other.vmime.org (with
// a NUL character), without subject alternative names
static const char nulCommonNameCertificatePEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBVjCB/KADAgECAhQiAsLPnjFdOukIj4g4lBhAETCZdjAKBggqhkjOPQQDAjAq\n"
	"MSgwJgYDVQQDDB9tYWlsLnZtaW1lLm9yZwAub3RoZXIudm1pbWUub3JnMCAXDTI2\n"
	"MTExOTAwMDAwMFoYDzIxMjYxMDI2MDAwMDAwWjAqMSgwJgYDVQQDDB9tYWlsLnZt\n"
	"aW1lLm9yZwAub3RoZXIudm1pbWUub3JnMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcD\n"
	"QgAEmZARuq/g4QUuVt+K32g3+RrQrQhP2T9WkJV2mOTtgezQyS5FzGaIp84m03mg\n"
	"00n3YumaWMDz/SjSJDAb/MgCITAKBggqhkjOPQQDAgNJADBGAiEA+cdusS+a30WG\n"
	"3v6bwuJROrRz0IJ6tWtfHAMCsggQ7h8CIQDRBFne1JxH9+v1+wSPZVHWYmh62KUM\n"
	"enkFiC8ghP/NLg==\n"
	"-----END CERTIFICATE-----\n";



VMIME_TEST_SUITE_BEGIN(X509CertificateTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testVerifyHostName)
		VMIME_TEST(testVerifyHostNamePartialLabels)
		VMIME_TEST(testVerifyHostNameWildcard)
		VMIME_TEST(testVerifyHostNameSubjectAltName)
		VMIME_TEST(testVerifyHostNameCommonName)
		VMIME_TEST(testVerifyHostNameNulInCommonName)
		VMIME_TEST(testVerifyHostNameIPv4)
		VMIME_TEST(testVerifyHostNameIPv6)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::security::cert::X509Certificate>
		importTestCertificate(const char* pem = testCertificatePEM) {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert =
			vmime::security::cert::X509Certificate::import(
				reinterpret_cast <const vmime::byte_t*>(pem), strlen(pem)
			);

		VASSERT_NOT_NULL("Import", cert);

		return cert;
	}

	void testVerifyHostName() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert = importTestCertificate();

		VASSERT_TRUE("Match", cert->verifyHostName("mail.vmime.org"));
		VASSERT_TRUE("Case", cert->verifyHostName("MAIL.Vmime.Org"));
		VASSERT_FALSE("No match", cert->verifyHostName("www.vmime.org"));
	}

	void testVerifyHostNamePartialLabels() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert = importTestCertificate();

		// Names must match entirely, not only their end
		VASSERT_FALSE("Suffix of the name", cert->verifyHostName("ail.vmime.org"));
		VASSERT_FALSE("Name as suffix", cert->verifyHostName("xmail.vmime.org"));
		VASSERT_FALSE("Name as suffix 2", cert->verifyHostName("smtp.mail.vmime.org"));
		VASSERT_FALSE("Parent domain", cert->verifyHostName("vmime.org"));
		VASSERT_FALSE("Leading dot", cert->verifyHostName(".vmime.org"));
		VASSERT_FALSE("Empty", cert->verifyHostName(""));
	}

	void testVerifyHostNameWildcard() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert =
			importTestCertificate(wildcardCertificatePEM);

		VASSERT_TRUE("Match", cert->verifyHostName("smtp.vmime.org"));

		// The wildcard only matches the whole left-most label (RFC 9525,
		// section 6.3)
		VASSERT_FALSE("Domain", cert->verifyHostName("vmime.org"));
		VASSERT_FALSE("Two labels", cert->verifyHostName("a.b.vmime.org"));
		VASSERT_FALSE("Partial label", cert->verifyHostName("evilvmime.org"));
	}

	void testVerifyHostNameSubjectAltName() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert =
			importTestCertificate(subjectAltNameCertificatePEM);

		// The common name is not used if there are DNS names in subject
		// alternative names (RFC 6125, section 6.4.4)
		VASSERT_TRUE("Subject alternative name", cert->verifyHostName("smtp.vmime.org"));
		VASSERT_FALSE("Common name", cert->verifyHostName("mail.vmime.org"));
	}

	void testVerifyHostNameCommonName() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert =
			importTestCertificate(commonNameCertificatePEM);

		// Without DNS names in subject alternative names, the common name is
		// used (RFC 6125, section 6.4.4): RFC 9525 does not allow it any more,
		// but it is kept for compatibility
		VASSERT_TRUE("Match", cert->verifyHostName("mail.vmime.org"));
		VASSERT_FALSE("Suffix of the name", cert->verifyHostName("ail.vmime.org"));
	}

	void testVerifyHostNameNulInCommonName() {

		vmime::shared_ptr <vmime::security::cert::X509Certificate> cert =
			importTestCertificate(nulCommonNameCertificatePEM);

		VASSERT_FALSE("Before NUL", cert->verifyHostName("mail.vmime.org"));
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
