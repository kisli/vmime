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


#include <cstring>


using namespace vmime::security::cert;


// Test PKI (ECDSA P-256 keys, certificates valid until 2126):
//
//   rootCA          self-signed root "VMime Test newroot"
//   oldRootCA       self-signed root "VMime Test oldroot"
//   crossSignedRootCA
//                   "VMime Test newroot" (same key as rootCA), cross-signed
//                   by oldRootCA
//   intermediateCA  "VMime Test intermediate", issued by rootCA
//   leaf            mail.vmime.org, issued by intermediateCA
//   forgedLeaf      mail.vmime.org, names intermediateCA as its issuer (also
//                   in the authority key identifier), but is signed by
//                   another key
//   subLeaf         smtp.vmime.org, issued (and signed) by leaf, which is
//                   not a CA
//   noCertSignCA    "VMime Test intermediate without keyCertSign", issued by
//                   rootCA: a CA, but whose key usage does not allow signing
//                   certificates
//   noCertSignLeaf  mail.vmime.org, issued (and signed) by noCertSignCA
//   expiredLeaf     mail.vmime.org, issued by intermediateCA, expired in 2021
//   notYetValidLeaf mail.vmime.org, issued by intermediateCA, valid from 2120
//   clientLeaf      mail.vmime.org, issued by intermediateCA, with an
//                   extended key usage for TLS client authentication only
//   expiredCA       "VMime Test expired intermediate", issued by rootCA,
//                   expired in 2021
//   expiredCALeaf   mail.vmime.org, issued by expiredCA
//   pathLenCA       "VMime Test intermediate with pathLen 0", issued by
//                   rootCA: it must not issue CA certificates
//   subCA           "VMime Test sub-intermediate", issued by pathLenCA
//   subCALeaf       mail.vmime.org, issued by subCA
//   nameConstraintsCA
//                   "VMime Test intermediate for example.org", issued by
//                   rootCA: it may only issue certificates for example.org
//   nameConstraintsLeaf
//                   mail.vmime.org, issued by nameConstraintsCA

static const char rootCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBojCCAUegAwIBAgIUAuwmSE8Loy0bAtEnidDapn1aiKQwCgYIKoZIzj0EAwIw\n"
	"HTEbMBkGA1UEAwwSVk1pbWUgVGVzdCBuZXdyb290MCAXDTI2MTExODEwMzY0OFoY\n"
	"DzIxMjYxMDI1MTAzNjQ4WjAdMRswGQYDVQQDDBJWTWltZSBUZXN0IG5ld3Jvb3Qw\n"
	"WTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAR5vGwSvLjuVxcufkTLgoAC4vWQZAwW\n"
	"47goSEf72cV11bqlbgmo3HYyG3fr2+H+VbAGE/d8iDi8SFgG4m1QhO3yo2MwYTAd\n"
	"BgNVHQ4EFgQU2IBcGFdTU8kzeE8j7S8PQjMBNpAwHwYDVR0jBBgwFoAU2IBcGFdT\n"
	"U8kzeE8j7S8PQjMBNpAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw\n"
	"CgYIKoZIzj0EAwIDSQAwRgIhAO1Qa29+w7jBywEsofcPr8f2mJ90bWUfoGNehLmT\n"
	"b3ElAiEAzBwjNtEfq+1l46mworz9zrDCiCH9wRALFLo669xD4yE=\n"
	"-----END CERTIFICATE-----\n";

static const char oldRootCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBoDCCAUegAwIBAgIUbDOiWBcEfX6hgoRBNutXsgG8hCUwCgYIKoZIzj0EAwIw\n"
	"HTEbMBkGA1UEAwwSVk1pbWUgVGVzdCBvbGRyb290MCAXDTI2MTExODEwMzY0OFoY\n"
	"DzIxMjYxMDI1MTAzNjQ4WjAdMRswGQYDVQQDDBJWTWltZSBUZXN0IG9sZHJvb3Qw\n"
	"WTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAASsPKdld/p3/RtGh7X/LICBrwyzRQD8\n"
	"obSOWLGPSPwM2Yss5oxVbhb0GawnoRKsPFNYBYjg60KB347KGN0G4vRpo2MwYTAd\n"
	"BgNVHQ4EFgQUGBQXldoJCTqMqxPEaf0CF2qhoA0wHwYDVR0jBBgwFoAUGBQXldoJ\n"
	"CTqMqxPEaf0CF2qhoA0wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw\n"
	"CgYIKoZIzj0EAwIDRwAwRAIgVz9AiAcrbHoKjER/OqRG2Kf8YjIMh9amHoyos2Ch\n"
	"GGECIAozo70xzpzkscUzy+yeYFSIKqJRTYfRVEyfoh7lxt9J\n"
	"-----END CERTIFICATE-----\n";

static const char crossSignedRootCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBoTCCAUegAwIBAgIUYVh/ECrrLMCGUaoc8GjKpeSLWCswCgYIKoZIzj0EAwIw\n"
	"HTEbMBkGA1UEAwwSVk1pbWUgVGVzdCBvbGRyb290MCAXDTI2MTExODEwMzY0OFoY\n"
	"DzIxMjYxMDI1MTAzNjQ4WjAdMRswGQYDVQQDDBJWTWltZSBUZXN0IG5ld3Jvb3Qw\n"
	"WTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAR5vGwSvLjuVxcufkTLgoAC4vWQZAwW\n"
	"47goSEf72cV11bqlbgmo3HYyG3fr2+H+VbAGE/d8iDi8SFgG4m1QhO3yo2MwYTAP\n"
	"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNVHQ4EFgQU2IBcGFdT\n"
	"U8kzeE8j7S8PQjMBNpAwHwYDVR0jBBgwFoAUGBQXldoJCTqMqxPEaf0CF2qhoA0w\n"
	"CgYIKoZIzj0EAwIDSAAwRQIhALJRdsZUcEg50jDQyAz09rnuIIdiKFK0W2ZRIh7P\n"
	"oDl1AiAuFAvBwXUPrX/YULjXy+Qr+dnAnSyJVquEbdz9J2m1aA==\n"
	"-----END CERTIFICATE-----\n";

static const char intermediateCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBpzCCAUygAwIBAgIUHaoVaJ20gPNgaG4QZY6gIb7wKnMwCgYIKoZIzj0EAwIw\n"
	"HTEbMBkGA1UEAwwSVk1pbWUgVGVzdCBuZXdyb290MCAXDTI2MTExODEwMzY0OVoY\n"
	"DzIxMjYxMDI1MTAzNjQ5WjAiMSAwHgYDVQQDDBdWTWltZSBUZXN0IGludGVybWVk\n"
	"aWF0ZTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABOpIEvaJD8e5ALwWnzdIf7Yb\n"
	"Yo1/o5wZnPxyiBfb0YZ8h/2OheFgZjwkNEeN+Wm7osaY/227erfWSHJf5APA5Dej\n"
	"YzBhMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgEGMB0GA1UdDgQWBBTo\n"
	"3EqKGvCGBDJ+clGWG3tb1rWK9DAfBgNVHSMEGDAWgBTYgFwYV1NTyTN4TyPtLw9C\n"
	"MwE2kDAKBggqhkjOPQQDAgNJADBGAiEArK7tvqThI1mzhw8LwZdKB99LxK4N7LST\n"
	"LIcO9FRxH14CIQCQYHIfQUQU/WN5NuQjmHOOg1gpZwaG58Bu9SHgRPROVA==\n"
	"-----END CERTIFICATE-----\n";

static const char leafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBuzCCAWCgAwIBAgIUfmtuZDSsLVp4ePJwFDsiUOWL18owCgYIKoZIzj0EAwIw\n"
	"IjEgMB4GA1UEAwwXVk1pbWUgVGVzdCBpbnRlcm1lZGlhdGUwIBcNMjYxMTE4MTAz\n"
	"NjQ5WhgPMjEyNjEwMjUxMDM2NDlaMBkxFzAVBgNVBAMMDm1haWwudm1pbWUub3Jn\n"
	"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEB2WF5j7GdidrWBVc2q2t4MPAl/3D\n"
	"zI6E4sBXODBJq/ZnhLUKBy/E6XO0hksGhn7ISlrr7ptcsqX3++9TsBpFxKN7MHkw\n"
	"DAYDVR0TAQH/BAIwADAOBgNVHQ8BAf8EBAMCB4AwGQYDVR0RBBIwEIIObWFpbC52\n"
	"bWltZS5vcmcwHQYDVR0OBBYEFEaAQo0CkL8xDVWFOO/6ACDPYmyOMB8GA1UdIwQY\n"
	"MBaAFOjcSooa8IYEMn5yUZYbe1vWtYr0MAoGCCqGSM49BAMCA0kAMEYCIQCrvz0z\n"
	"berKfLRjsVkTsszqm3Ayqvfrl41Os9iPXUlSgwIhAMpAR4y6cLqkwSi7ATWa7q6L\n"
	"TZ6fexwkBj+BleE/nZDG\n"
	"-----END CERTIFICATE-----\n";

static const char forgedLeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBujCCAWCgAwIBAgIUHiMsqWljC+ADKdrtGyaE9h36CJkwCgYIKoZIzj0EAwIw\n"
	"IjEgMB4GA1UEAwwXVk1pbWUgVGVzdCBpbnRlcm1lZGlhdGUwIBcNMjYxMTE4MTAz\n"
	"NjU4WhgPMjEyNjEwMjUxMDM2NThaMBkxFzAVBgNVBAMMDm1haWwudm1pbWUub3Jn\n"
	"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAErvPUXl9e3mAiaaBfoZm6oMOGRLYP\n"
	"MshOtgM7obNExDN8iiIlNwAO4hPCzs6ObWossy0x7RogmRrNNoW8xhtfpaN7MHkw\n"
	"DAYDVR0TAQH/BAIwADAOBgNVHQ8BAf8EBAMCB4AwGQYDVR0RBBIwEIIObWFpbC52\n"
	"bWltZS5vcmcwHQYDVR0OBBYEFMmkaPA4eyCHt+S5vwWkVX746+3cMB8GA1UdIwQY\n"
	"MBaAFOjcSooa8IYEMn5yUZYbe1vWtYr0MAoGCCqGSM49BAMCA0gAMEUCIFXAR+jU\n"
	"Qx7W6gI8Nk5mUAG+AL66jpM8MzMAJlJwt/hlAiEA+G/IedCaoNylJZKKIKZcrJyt\n"
	"wxnFm/NWhpl1p4E7bzY=\n"
	"-----END CERTIFICATE-----\n";

static const char subLeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBsDCCAVegAwIBAgIUNHDXpP9ltH5nHllXDtrZS5ejyV4wCgYIKoZIzj0EAwIw\n"
	"GTEXMBUGA1UEAwwObWFpbC52bWltZS5vcmcwIBcNMjYxMTE4MTIzNDI1WhgPMjEy\n"
	"NjEwMjUxMjM0MjVaMBkxFzAVBgNVBAMMDnNtdHAudm1pbWUub3JnMFkwEwYHKoZI\n"
	"zj0CAQYIKoZIzj0DAQcDQgAETO0E4LvcppCNjJpmZYG4BlVCyriPqumN9as8LhnA\n"
	"jQhdUlAp9dgTB3VnieM45K4F3JU1uPn0PQk9rBvuZhooIaN7MHkwDAYDVR0TAQH/\n"
	"BAIwADAOBgNVHQ8BAf8EBAMCB4AwGQYDVR0RBBIwEIIOc210cC52bWltZS5vcmcw\n"
	"HwYDVR0jBBgwFoAURoBCjQKQvzENVYU47/oAIM9ibI4wHQYDVR0OBBYEFKjUcheh\n"
	"4aqZsfflNGb7fmHtHK4hMAoGCCqGSM49BAMCA0cAMEQCID9XVPD/OBOyM2x1IOhe\n"
	"b/LcCe2pW0ezyc1GTaQ26UVeAiA6jB3iKHf+guwxKovZDWowqAjW+4GJOS9E4HxQ\n"
	"cuip1A==\n"
	"-----END CERTIFICATE-----\n";

static const char noCertSignCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBujCCAWCgAwIBAgIUc+oKe4tVquEbThT4hsfY9kjDWykwCgYIKoZIzj0EAwIw\n"
	"HTEbMBkGA1UEAwwSVk1pbWUgVGVzdCBuZXdyb290MCAXDTI2MTExODEyMzQyNVoY\n"
	"DzIxMjYxMDI1MTIzNDI1WjA2MTQwMgYDVQQDDCtWTWltZSBUZXN0IGludGVybWVk\n"
	"aWF0ZSB3aXRob3V0IGtleUNlcnRTaWduMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcD\n"
	"QgAEjls3l1EVwuwkJmSE+74Nix2JfJrexu4RA2IkBYIRnX5b8sJdWxjxE34TtKfn\n"
	"B15o9tQoNO5VrCg7bNBdcgG5gKNjMGEwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8B\n"
	"Af8EBAMCB4AwHQYDVR0OBBYEFJ0Ypwgkm679RHG7yearBj4kjxFfMB8GA1UdIwQY\n"
	"MBaAFNiAXBhXU1PJM3hPI+0vD0IzATaQMAoGCCqGSM49BAMCA0gAMEUCIQCZLKpY\n"
	"i27kRjOasNC19R9/qqKfHeRlrxJmEHrIjMzDIQIgQK60m/KMfbnBz2kc8pl6m2/E\n"
	"fIjohReG3ymntO0GLB4=\n"
	"-----END CERTIFICATE-----\n";

static const char noCertSignLeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBzTCCAXSgAwIBAgIUU195T1Ycr4oP1xJylj5X4YafakAwCgYIKoZIzj0EAwIw\n"
	"NjE0MDIGA1UEAwwrVk1pbWUgVGVzdCBpbnRlcm1lZGlhdGUgd2l0aG91dCBrZXlD\n"
	"ZXJ0U2lnbjAgFw0yNjExMTgxMjM0MjVaGA8yMTI2MTAyNTEyMzQyNVowGTEXMBUG\n"
	"A1UEAwwObWFpbC52bWltZS5vcmcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATp\n"
	"RYHS8XnFR+PFpe9aaBePA9Rkd5/rlHZFgGTaSPFmMebH23KJvFhB3PpEGWy8xCzC\n"
	"uuQSZvWi+HTU4jUGg9lDo3sweTAMBgNVHRMBAf8EAjAAMA4GA1UdDwEB/wQEAwIH\n"
	"gDAZBgNVHREEEjAQgg5tYWlsLnZtaW1lLm9yZzAfBgNVHSMEGDAWgBSdGKcIJJuu\n"
	"/URxu8nmqwY+JI8RXzAdBgNVHQ4EFgQUbKP7QVieye938cmrJNRC8cvRGXgwCgYI\n"
	"KoZIzj0EAwIDRwAwRAIgfFf/BeJmu4MtNXuHZ0E4Y1WFaSxV4NX5s1shcCpR+5AC\n"
	"IGTkGzykcbl0UAGGQKdiCsc4LJ4pYwWav/LrG0/iITsV\n"
	"-----END CERTIFICATE-----\n";

static const char expiredLeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBuDCCAV6gAwIBAgIUMbJQFJ6ut0ECbY1Mi4RPXIIf74AwCgYIKoZIzj0EAwIw\n"
	"IjEgMB4GA1UEAwwXVk1pbWUgVGVzdCBpbnRlcm1lZGlhdGUwHhcNMjAwMTAxMDAw\n"
	"MDAwWhcNMjEwMTAxMDAwMDAwWjAZMRcwFQYDVQQDDA5tYWlsLnZtaW1lLm9yZzBZ\n"
	"MBMGByqGSM49AgEGCCqGSM49AwEHA0IABL7vrESqzZbK+tFwVU4Fwjxi+CjOaU3W\n"
	"G14Yk9bwBuv54qmVi4LX+tLRlGEG9gxYYef7o6HtbciThEiwlvYcjkyjezB5MAwG\n"
	"A1UdEwEB/wQCMAAwHQYDVR0OBBYEFOkaLQjmOHE0QcU8nXl8wq8yIzpvMB8GA1Ud\n"
	"IwQYMBaAFOjcSooa8IYEMn5yUZYbe1vWtYr0MA4GA1UdDwEB/wQEAwIHgDAZBgNV\n"
	"HREEEjAQgg5tYWlsLnZtaW1lLm9yZzAKBggqhkjOPQQDAgNIADBFAiAh/ImLJVna\n"
	"HR7PGCzUk+L1Mdyw6xc+/sQF4lpxasJhNgIhAKzaSvBalRkTKGKdJYJJsaHa1ene\n"
	"wyaawgIyHu5d1ihb\n"
	"-----END CERTIFICATE-----\n";

static const char notYetValidLeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBvDCCAWKgAwIBAgIUBxDUBovKvusNxQjKvq+9KdVFeg4wCgYIKoZIzj0EAwIw\n"
	"IjEgMB4GA1UEAwwXVk1pbWUgVGVzdCBpbnRlcm1lZGlhdGUwIhgPMjEyMDAxMDEw\n"
	"MDAwMDBaGA8yMTI1MDEwMTAwMDAwMFowGTEXMBUGA1UEAwwObWFpbC52bWltZS5v\n"
	"cmcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARYOQMYioIGR32snTTlxg+n/vov\n"
	"S59xEPI5/ZoVYPzj5Df1wxC4H01oHixcOwDZYP0GHqEMFB7Hlj1KBIntxUnxo3sw\n"
	"eTAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBTSFlcPJmpBbbHmdeitedC6wQZ4MTAf\n"
	"BgNVHSMEGDAWgBTo3EqKGvCGBDJ+clGWG3tb1rWK9DAOBgNVHQ8BAf8EBAMCB4Aw\n"
	"GQYDVR0RBBIwEIIObWFpbC52bWltZS5vcmcwCgYIKoZIzj0EAwIDSAAwRQIhAKfc\n"
	"fc1O5OiPQJcYAqR+7gtUam2sGW3VQxieHJdM0/1mAiBERSHoLmR0q6UHlGp4smbi\n"
	"uEY0BtFl2xjmsmey+vfS7A==\n"
	"-----END CERTIFICATE-----\n";

static const char clientLeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIB0TCCAXegAwIBAgIUf3j7jbtFbgbCYqcQ1TQVWfjd6kQwCgYIKoZIzj0EAwIw\n"
	"IjEgMB4GA1UEAwwXVk1pbWUgVGVzdCBpbnRlcm1lZGlhdGUwIBcNMjYxMTE4MDAw\n"
	"MDAwWhgPMjEyNjEwMjUwMDAwMDBaMBkxFzAVBgNVBAMMDm1haWwudm1pbWUub3Jn\n"
	"MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEIVNU0bDc4hRMT7JM5mX6mPcvnNqZ\n"
	"oqqVSQDjkUPGj6XqZhtu8wVKWZZ5vw4F17Db43dCHz9xymnSNK5TLO/ZoKOBkTCB\n"
	"jjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBSqy6mBZG4+NFABJFA5+GqVZoAngDAf\n"
	"BgNVHSMEGDAWgBTo3EqKGvCGBDJ+clGWG3tb1rWK9DAOBgNVHQ8BAf8EBAMCB4Aw\n"
	"GQYDVR0RBBIwEIIObWFpbC52bWltZS5vcmcwEwYDVR0lBAwwCgYIKwYBBQUHAwIw\n"
	"CgYIKoZIzj0EAwIDSAAwRQIgIds2cNJ6MeHXsFmFOMawJ6xoi6ejCw1CSBo0Ey9U\n"
	"+eQCIQCIQN8tMdInwViA6oOuxEcC/2Vkr5LzbKbUBBHxf1kP0Q==\n"
	"-----END CERTIFICATE-----\n";

static const char expiredCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBrDCCAVKgAwIBAgIUIlaKhRUHZ0LmB/mIVuEIrNUgYn8wCgYIKoZIzj0EAwIw\n"
	"HTEbMBkGA1UEAwwSVk1pbWUgVGVzdCBuZXdyb290MB4XDTIwMDEwMTAwMDAwMFoX\n"
	"DTIxMDEwMTAwMDAwMFowKjEoMCYGA1UEAwwfVk1pbWUgVGVzdCBleHBpcmVkIGlu\n"
	"dGVybWVkaWF0ZTBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABL0xHXWjsie/YarC\n"
	"VrzFJIdcZHlX8JKEsif74QqYkAD9E6qfqz7dM/X8y7pLHPme2Mmg1dPqrutehb4b\n"
	"6RFjbvWjYzBhMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFBhiw0iiJEUiUwC1\n"
	"ZSXtArh2fMYuMB8GA1UdIwQYMBaAFNiAXBhXU1PJM3hPI+0vD0IzATaQMA4GA1Ud\n"
	"DwEB/wQEAwIBBjAKBggqhkjOPQQDAgNIADBFAiAm1hf1JoRhncYfZf7StYtTkiky\n"
	"r48WP6ZYZS7161S85gIhAIQ/5Q7K8AO61xXWIj7V/rqTgVEkJEejWG44Jg3q3WEF\n"
	"-----END CERTIFICATE-----\n";

static const char expiredCALeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBwjCCAWigAwIBAgIURIwFCOhikmgbHoW9y0KvU7ksdjQwCgYIKoZIzj0EAwIw\n"
	"KjEoMCYGA1UEAwwfVk1pbWUgVGVzdCBleHBpcmVkIGludGVybWVkaWF0ZTAgFw0y\n"
	"NjExMTgwMDAwMDBaGA8yMTI2MTAyNTAwMDAwMFowGTEXMBUGA1UEAwwObWFpbC52\n"
	"bWltZS5vcmcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATduXFOmxIfXi0i1ekA\n"
	"21bTkoBecXfDby7Pez4y3lMdEwcTm+UdozzMjvU5k1JzOoa7mq+G6g0p4PPX5Ir2\n"
	"Zrh0o3sweTAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBQQhxX5bQmnGojpFdQSxNDm\n"
	"PuHkyjAfBgNVHSMEGDAWgBQYYsNIoiRFIlMAtWUl7QK4dnzGLjAOBgNVHQ8BAf8E\n"
	"BAMCB4AwGQYDVR0RBBIwEIIObWFpbC52bWltZS5vcmcwCgYIKoZIzj0EAwIDSAAw\n"
	"RQIhAIZOdxaY2qZswOLzGHRmEX+0vak0BhuwLClb7SmP0RIvAiBje3lnmzmDBtsP\n"
	"777UJmmk0bnUgM2BK2ArwKA6C4EbEQ==\n"
	"-----END CERTIFICATE-----\n";

static const char pathLenCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBuDCCAV6gAwIBAgIUdpwyhmYPkJdW9bOmIDnmLiVQOaMwCgYIKoZIzj0EAwIw\n"
	"HTEbMBkGA1UEAwwSVk1pbWUgVGVzdCBuZXdyb290MCAXDTI2MTExODAwMDAwMFoY\n"
	"DzIxMjYxMDI1MDAwMDAwWjAxMS8wLQYDVQQDDCZWTWltZSBUZXN0IGludGVybWVk\n"
	"aWF0ZSB3aXRoIHBhdGhMZW4gMDBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABG3M\n"
	"3EJk3R5+ulJxFp4GR2zUt5+fsrQqLDeEhxbSLFGvgz/tQt1RYYxJyIM8ywwZRJ2J\n"
	"xHK7cx+fYlG2LNues4qjZjBkMBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYDVR0OBBYE\n"
	"FDIujBZlDjQy87N+oQ2x+Fs5O+oMMB8GA1UdIwQYMBaAFNiAXBhXU1PJM3hPI+0v\n"
	"D0IzATaQMA4GA1UdDwEB/wQEAwIBBjAKBggqhkjOPQQDAgNIADBFAiBVvCk069cC\n"
	"iA/UKh6u6TGG0zRQfxLpurjgZr4PhAieiwIhALGexL6hDAJzbgAjPmgktG07T+lT\n"
	"T2ZvVJ336AGM9QIk\n"
	"-----END CERTIFICATE-----\n";

static const char subCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBvjCCAWSgAwIBAgIUIUhHUNTvda+nXKkZ3BsvD9RAjcgwCgYIKoZIzj0EAwIw\n"
	"MTEvMC0GA1UEAwwmVk1pbWUgVGVzdCBpbnRlcm1lZGlhdGUgd2l0aCBwYXRoTGVu\n"
	"IDAwIBcNMjYxMTE4MDAwMDAwWhgPMjEyNjEwMjUwMDAwMDBaMCYxJDAiBgNVBAMM\n"
	"G1ZNaW1lIFRlc3Qgc3ViLWludGVybWVkaWF0ZTBZMBMGByqGSM49AgEGCCqGSM49\n"
	"AwEHA0IABEsftqVQax1VZRhp2F/3Pg5LvtnxZy13njGFcA1sT59YfZh+nhAPChM9\n"
	"ne+hOc1xBBYlcQOhgTH13rH2Ngdqiw+jYzBhMA8GA1UdEwEB/wQFMAMBAf8wHQYD\n"
	"VR0OBBYEFOyG8Rf77CQrjvRmyCWztkQL1AEYMB8GA1UdIwQYMBaAFDIujBZlDjQy\n"
	"87N+oQ2x+Fs5O+oMMA4GA1UdDwEB/wQEAwIBBjAKBggqhkjOPQQDAgNIADBFAiEA\n"
	"wfPPoOQ67ggPMTQdOSDFoy2y9C0vilhx6gqPdQrTok4CIA1W5hbfaHfTlgmB7CZo\n"
	"+V2qo4ODtXwti//rXT1xyHVv\n"
	"-----END CERTIFICATE-----\n";

static const char subCALeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBvjCCAWSgAwIBAgIUOLTAped7PYdG3ha/jacp1BO1xWwwCgYIKoZIzj0EAwIw\n"
	"JjEkMCIGA1UEAwwbVk1pbWUgVGVzdCBzdWItaW50ZXJtZWRpYXRlMCAXDTI2MTEx\n"
	"ODAwMDAwMFoYDzIxMjYxMDI1MDAwMDAwWjAZMRcwFQYDVQQDDA5tYWlsLnZtaW1l\n"
	"Lm9yZzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABKF5upgoE6zKl2GsLprkYoli\n"
	"NfQxWWvvrvgCTfjOw+vSAcCNy2JXLYCm4LufiR9SXyZ4lgicQwPLUCcf/weHl+ij\n"
	"ezB5MAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFLLq9SB2dG1ELi1bYmmlaEukKWVK\n"
	"MB8GA1UdIwQYMBaAFOyG8Rf77CQrjvRmyCWztkQL1AEYMA4GA1UdDwEB/wQEAwIH\n"
	"gDAZBgNVHREEEjAQgg5tYWlsLnZtaW1lLm9yZzAKBggqhkjOPQQDAgNIADBFAiEA\n"
	"zWwApeOd374KlLV0/rvZS6NO03duHSUDumv4QBXCRbICIBgwM3cN9jeYz/gUKm4U\n"
	"yMnil0OMc6/zwf0q25GIXgmw\n"
	"-----END CERTIFICATE-----\n";

static const char nameConstraintsCAPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIB2DCCAX2gAwIBAgIUfcofxRcCOwOIW14//RUkySxWeEcwCgYIKoZIzj0EAwIw\n"
	"HTEbMBkGA1UEAwwSVk1pbWUgVGVzdCBuZXdyb290MCAXDTI2MTExODAwMDAwMFoY\n"
	"DzIxMjYxMDI1MDAwMDAwWjAyMTAwLgYDVQQDDCdWTWltZSBUZXN0IGludGVybWVk\n"
	"aWF0ZSBmb3IgZXhhbXBsZS5vcmcwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATY\n"
	"9S2eObDW7CefsoIJcjrQOvZS+bcJfTQ5rOCpQl5JfGQndYKyJaV/dsfwSaFtVoo1\n"
	"VIK6u7X728N/9qaL8bYqo4GDMIGAMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYE\n"
	"FMb0MJphsJ6BgWdSG/NyPB4DCaF3MB8GA1UdIwQYMBaAFNiAXBhXU1PJM3hPI+0v\n"
	"D0IzATaQMA4GA1UdDwEB/wQEAwIBBjAdBgNVHR4BAf8EEzARoA8wDYILZXhhbXBs\n"
	"ZS5vcmcwCgYIKoZIzj0EAwIDSQAwRgIhAKCFh7Vhz26t+r/dzVdBXFstaGtW+8NZ\n"
	"ld/Z+051xs+1AiEA9CHCQLOXe0IkPqlKXUIxafzgSs0SWYgWYImErYOwhrA=\n"
	"-----END CERTIFICATE-----\n";

static const char nameConstraintsLeafPEM[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIByzCCAXCgAwIBAgIUeebaVcBs9jXhWCrPsKauVJLTpK8wCgYIKoZIzj0EAwIw\n"
	"MjEwMC4GA1UEAwwnVk1pbWUgVGVzdCBpbnRlcm1lZGlhdGUgZm9yIGV4YW1wbGUu\n"
	"b3JnMCAXDTI2MTExODAwMDAwMFoYDzIxMjYxMDI1MDAwMDAwWjAZMRcwFQYDVQQD\n"
	"DA5tYWlsLnZtaW1lLm9yZzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABCCs6htr\n"
	"p1qZ+KJNVDaVGlS6KG84Ptyt776B2SvxCf170F6Z8tMnZHr8bmqVTcHUzzFrEqyN\n"
	"/1VItRbStZlLnq6jezB5MAwGA1UdEwEB/wQCMAAwHQYDVR0OBBYEFL9xWdX4SkTh\n"
	"b+jzW/Kedc7YRzOQMB8GA1UdIwQYMBaAFMb0MJphsJ6BgWdSG/NyPB4DCaF3MA4G\n"
	"A1UdDwEB/wQEAwIHgDAZBgNVHREEEjAQgg5tYWlsLnZtaW1lLm9yZzAKBggqhkjO\n"
	"PQQDAgNJADBGAiEAyTn7vzxn+tFb+TLiDfux4vqKo8wLmwh9FnM2xAV5LgMCIQDB\n"
	"cVapUJCdKkyj7/qMYR3nGcLT+VRkKitpyn8LSGxsNw==\n"
	"-----END CERTIFICATE-----\n";



VMIME_TEST_SUITE_BEGIN(defaultCertificateVerifierTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testValidChain)
		VMIME_TEST(testValidChainWithRoot)
		VMIME_TEST(testMisorderedChain)
		VMIME_TEST(testCrossSignedRoot)
		VMIME_TEST(testCrossSignedRootWithOldRoot)
		VMIME_TEST(testExtraneousCertificate)
		VMIME_TEST(testExtraneousCertificateInTheMiddle)
		VMIME_TEST(testForgedSignature)
		VMIME_TEST(testIssuerNotCA)
		VMIME_TEST(testIssuerNotAllowedToSign)
		VMIME_TEST(testPathLengthConstraint)
		VMIME_TEST(testNameConstraints)
		VMIME_TEST(testExpiredCertificate)
		VMIME_TEST(testExpiredIntermediate)
		VMIME_TEST(testNotYetValidCertificate)
		VMIME_TEST(testUnsuitablePurpose)
		VMIME_TEST(testUntrustedRoot)
		VMIME_TEST(testMissingIntermediate)
		VMIME_TEST(testTrustedCertificate)
		VMIME_TEST(testExpiredTrustedCertificate)
		VMIME_TEST(testHostNameMismatch)
		VMIME_TEST(testEmptyChain)
	VMIME_TEST_LIST_END


	typedef std::vector <const char*> PEMList;


	static std::vector <vmime::shared_ptr <X509Certificate> > importCertificates(const PEMList& pems) {

		std::vector <vmime::shared_ptr <X509Certificate> > certs;

		for (const char* pem : pems) {

			vmime::shared_ptr <X509Certificate> cert = X509Certificate::import(
				reinterpret_cast <const vmime::byte_t*>(pem), strlen(pem)
			);

			VASSERT_NOT_NULL("Import", cert);

			certs.push_back(cert);
		}

		return certs;
	}

	static void verify(
		const PEMList& chain,
		const PEMList& rootCAs,
		const vmime::string& hostname,
		const PEMList& trustedCerts = PEMList()
	) {

		vmime::shared_ptr <defaultCertificateVerifier> verifier =
			vmime::make_shared <defaultCertificateVerifier>();

		verifier->setX509RootCAs(importCertificates(rootCAs));
		verifier->setX509TrustedCerts(importCertificates(trustedCerts));

		const std::vector <vmime::shared_ptr <X509Certificate> > x509Certs = importCertificates(chain);
		std::vector <vmime::shared_ptr <certificate> > certs(x509Certs.begin(), x509Certs.end());

		verifier->verify(vmime::make_shared <certificateChain>(certs), hostname);
	}

	// Verifies a chain, which must be refused with the given exception, on
	// the given certificate (if not NULL); returns the exception message
	template <typename E>
	static vmime::string verifyError(
		const PEMList& chain,
		const PEMList& rootCAs,
		const vmime::string& hostname,
		const char* certPEM
	) {

		try {

			verify(chain, rootCAs, hostname);

		} catch (E& e) {

			if (certPEM) {

				VASSERT_NOT_NULL("Certificate", e.getCertificate());
				VASSERT_TRUE("Certificate", e.getCertificate()->equals(importCertificates({ certPEM })[0]));
			}

			return e.what();
		}

		VASSERT("Exception expected", false);
		return "";
	}

	void testValidChain() {

		VASSERT_NO_THROW(
			"Verify",
			verify({ leafPEM, intermediateCAPEM }, { rootCAPEM }, "mail.vmime.org")
		);
	}

	void testValidChainWithRoot() {

		VASSERT_NO_THROW(
			"Verify",
			verify({ leafPEM, intermediateCAPEM, rootCAPEM }, { rootCAPEM }, "mail.vmime.org")
		);
	}

	void testMisorderedChain() {

		VASSERT_NO_THROW(
			"Verify",
			verify({ leafPEM, rootCAPEM, intermediateCAPEM }, { rootCAPEM }, "mail.vmime.org")
		);
	}

	void testCrossSignedRoot() {

		// The old root is not trusted, but the path can end with the
		// self-signed version of the cross-signed root
		VASSERT_NO_THROW(
			"Verify",
			verify({ leafPEM, intermediateCAPEM, crossSignedRootCAPEM }, { rootCAPEM }, "mail.vmime.org")
		);

		verifyError <serverIdentityException>(
			{ leafPEM, intermediateCAPEM, crossSignedRootCAPEM }, { rootCAPEM }, "www.vmime.org", leafPEM
		);
	}

	void testCrossSignedRootWithOldRoot() {

		VASSERT_NO_THROW(
			"Verify",
			verify({ leafPEM, intermediateCAPEM, crossSignedRootCAPEM }, { oldRootCAPEM }, "mail.vmime.org")
		);
	}

	void testExtraneousCertificate() {

		VASSERT_NO_THROW(
			"Verify",
			verify({ leafPEM, intermediateCAPEM, noCertSignCAPEM }, { rootCAPEM }, "mail.vmime.org")
		);
	}

	void testExtraneousCertificateInTheMiddle() {

		VASSERT_NO_THROW(
			"Verify",
			verify({ leafPEM, noCertSignCAPEM, intermediateCAPEM }, { rootCAPEM }, "mail.vmime.org")
		);
	}

	void testForgedSignature() {

		const vmime::string what = verifyError <certificateIssuerVerificationException>(
			{ forgedLeafPEM, intermediateCAPEM }, { rootCAPEM }, "mail.vmime.org", forgedLeafPEM
		);

		// The precise reason is given by the TLS library
		VASSERT_NEQ("Reason", vmime::string(certificateIssuerVerificationException().what()), what);
	}

	void testIssuerNotCA() {

		verifyError <certificateIssuerVerificationException>(
			{ subLeafPEM, leafPEM, intermediateCAPEM }, { rootCAPEM }, "smtp.vmime.org", subLeafPEM
		);
	}

	void testIssuerNotAllowedToSign() {

		verifyError <certificateIssuerVerificationException>(
			{ noCertSignLeafPEM, noCertSignCAPEM }, { rootCAPEM }, "mail.vmime.org", noCertSignLeafPEM
		);
	}

	void testPathLengthConstraint() {

		// OpenSSL and GnuTLS do not report the same certificate
		verifyError <certificateIssuerVerificationException>(
			{ subCALeafPEM, subCAPEM, pathLenCAPEM }, { rootCAPEM }, "mail.vmime.org", NULL
		);
	}

	void testNameConstraints() {

		verifyError <certificateIssuerVerificationException>(
			{ nameConstraintsLeafPEM, nameConstraintsCAPEM }, { rootCAPEM }, "mail.vmime.org",
			nameConstraintsLeafPEM
		);
	}

	void testExpiredCertificate() {

		verifyError <certificateExpiredException>(
			{ expiredLeafPEM, intermediateCAPEM }, { rootCAPEM }, "mail.vmime.org", expiredLeafPEM
		);
	}

	void testExpiredIntermediate() {

		verifyError <certificateExpiredException>(
			{ expiredCALeafPEM, expiredCAPEM }, { rootCAPEM }, "mail.vmime.org", expiredCAPEM
		);
	}

	void testNotYetValidCertificate() {

		verifyError <certificateNotYetValidException>(
			{ notYetValidLeafPEM, intermediateCAPEM }, { rootCAPEM }, "mail.vmime.org", notYetValidLeafPEM
		);
	}

	void testUnsuitablePurpose() {

		const vmime::string what = verifyError <certificateNotTrustedException>(
			{ clientLeafPEM, intermediateCAPEM }, { rootCAPEM }, "mail.vmime.org", clientLeafPEM
		);

		VASSERT_NEQ("Reason", vmime::string(certificateNotTrustedException().what()), what);
	}

	void testUntrustedRoot() {

		const vmime::string what = verifyError <certificateNotTrustedException>(
			{ leafPEM, intermediateCAPEM }, { oldRootCAPEM }, "mail.vmime.org", intermediateCAPEM
		);

		VASSERT_NEQ("Reason", vmime::string(certificateNotTrustedException().what()), what);

		verifyError <certificateNotTrustedException>(
			{ leafPEM, intermediateCAPEM }, { }, "mail.vmime.org", intermediateCAPEM
		);
	}

	void testMissingIntermediate() {

		verifyError <certificateNotTrustedException>(
			{ leafPEM }, { rootCAPEM }, "mail.vmime.org", leafPEM
		);
	}

	void testTrustedCertificate() {

		VASSERT_NO_THROW(
			"Verify",
			verify({ leafPEM }, { }, "mail.vmime.org", { leafPEM })
		);
	}

	void testExpiredTrustedCertificate() {

		VASSERT_THROW(
			"Verify",
			verify({ expiredLeafPEM }, { }, "mail.vmime.org", { expiredLeafPEM }),
			certificateExpiredException
		);
	}

	void testHostNameMismatch() {

		verifyError <serverIdentityException>(
			{ leafPEM, intermediateCAPEM }, { rootCAPEM }, "www.vmime.org", leafPEM
		);
	}

	void testEmptyChain() {

		VASSERT_THROW(
			"Verify",
			verify({ }, { rootCAPEM }, "mail.vmime.org"),
			certificateNotTrustedException
		);
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT
