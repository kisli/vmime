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


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/OAuthBearerSASLMechanism.hpp"
#include "vmime/security/sasl/defaultSASLAuthenticator.hpp"


// Access token from the examples of RFC 7628, section 4
static const vmime::string TEST_ACCESS_TOKEN = "vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg==";


class oauthBearerTestAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	oauthBearerTestAuthenticator(const vmime::string& username, const vmime::string& accessToken)
		: m_username(username),
		  m_accessToken(accessToken) {

	}

	const vmime::string getUsername() const { return m_username; }
	const vmime::string getAccessToken() const { return m_accessToken; }

private:

	const vmime::string m_username;
	const vmime::string m_accessToken;
};


class oauthBearerNoUsernameTestAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	const vmime::string getUsername() const { throw vmime::exceptions::no_auth_information(); }
	const vmime::string getAccessToken() const { return TEST_ACCESS_TOKEN; }
};


class oauthBearerNoAccessTokenTestAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	const vmime::string getUsername() const { return "user@vmime.org"; }
	const vmime::string getAccessToken() const { throw vmime::exceptions::no_auth_information(); }
};


VMIME_TEST_SUITE_BEGIN(OAuthBearerSASLMechanismTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testName)
		VMIME_TEST(testHasInitialResponse)
		VMIME_TEST(testIsPlaintext)
		VMIME_TEST(testResponse)
		VMIME_TEST(testResponse_EmptyChallenge)
		VMIME_TEST(testResponse_NoUsername)
		VMIME_TEST(testResponse_EmptyUsername)
		VMIME_TEST(testResponse_UsernameEncoding)
		VMIME_TEST(testNULInUsername)
		VMIME_TEST(testNoAccessToken)
		VMIME_TEST(testEmptyAccessToken)
		VMIME_TEST(testInvalidAccessToken)
		VMIME_TEST(testErrorChallenge)
		VMIME_TEST(testUnexpectedChallenge)
		VMIME_TEST(testNoSecurityLayer)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::security::sasl::SASLSession> createSession(
		const vmime::shared_ptr <vmime::security::authenticator>& auth
	) {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = ctx->createSession(
			"imap", auth, vmime::make_shared <vmime::security::sasl::OAuthBearerSASLMechanism>(ctx, "OAUTHBEARER")
		);

		sess->init();

		return sess;
	}

	static vmime::shared_ptr <vmime::security::sasl::SASLSession> createSession(
		const vmime::string& username = "user@vmime.org",
		const vmime::string& accessToken = TEST_ACCESS_TOKEN
	) {

		return createSession(vmime::make_shared <oauthBearerTestAuthenticator>(username, accessToken));
	}

	static const vmime::string evaluateChallenge(
		const vmime::shared_ptr <vmime::security::sasl::SASLSession>& sess,
		const vmime::string& challenge = "",
		bool* complete = NULL
	) {

		vmime::byte_t* resp = NULL;
		size_t respLen = 0;

		const bool res = sess->evaluateChallenge(
			challenge.empty() ? NULL : reinterpret_cast <const vmime::byte_t*>(challenge.data()),
			challenge.length(), &resp, &respLen
		);

		if (complete) {
			*complete = res;
		}

		const vmime::string response(resp, resp + respLen);
		delete [] resp;

		return response;
	}


	void testName() {

		VASSERT_EQ("1", "OAUTHBEARER", createSession()->getMechanism()->getName());
	}

	// The client sends the first message (RFC 7628, section 3.1)
	void testHasInitialResponse() {

		VASSERT_TRUE("1", createSession()->getMechanism()->hasInitialResponse());
	}

	// Clients MUST always use TLS when making requests with bearer tokens
	// (RFC 6750, section 5.3)
	void testIsPlaintext() {

		VASSERT_TRUE("1", createSession()->getMechanism()->isPlaintext());
	}

	// RFC 7628, section 3.1: client-resp = (gs2-header kvsep *kvpair kvsep) / kvsep
	void testResponse() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		VASSERT_FALSE("1", sess->getMechanism()->isComplete());

		bool complete = false;

		VASSERT_EQ(
			"2",
			"n,a=user@vmime.org,\x01" "auth=Bearer vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg==\x01\x01",
			evaluateChallenge(sess, "", &complete)
		);

		VASSERT_TRUE("3", complete);
		VASSERT_TRUE("4", sess->getMechanism()->isComplete());
	}

	// When the protocol does not allow an initial response (eg. IMAP without
	// SASL-IR), the server first sends an empty challenge
	void testResponse_EmptyChallenge() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		vmime::byte_t* resp = NULL;
		size_t respLen = 0;

		const vmime::byte_t emptyChallenge[] = { 0 };

		sess->evaluateChallenge(emptyChallenge, 0, &resp, &respLen);

		const vmime::string response(resp, resp + respLen);
		delete [] resp;

		VASSERT_EQ(
			"1",
			"n,a=user@vmime.org,\x01" "auth=Bearer vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg==\x01\x01",
			response
		);
	}

	// RFC 7628, section 3.1: the GS2 header MAY include the username (authzid)
	void testResponse_NoUsername() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			createSession(vmime::make_shared <oauthBearerNoUsernameTestAuthenticator>());

		VASSERT_EQ(
			"1",
			"n,,\x01" "auth=Bearer vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg==\x01\x01",
			evaluateChallenge(sess)
		);
	}

	// RFC 5801, section 4: saslname = 1*(UTF8-char-safe / "=2C" / "=3D")
	void testResponse_EmptyUsername() {

		VASSERT_EQ(
			"1",
			"n,,\x01" "auth=Bearer vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg==\x01\x01",
			evaluateChallenge(createSession(""))
		);
	}

	// RFC 5801, section 4: "," and "=" are encoded in a saslname
	void testResponse_UsernameEncoding() {

		VASSERT_EQ(
			"1",
			"n,a=us=2Cer=3D@vmime.org,\x01" "auth=Bearer vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg==\x01\x01",
			evaluateChallenge(createSession("us,er=@vmime.org"))
		);

		VASSERT_EQ(
			"2",
			"n,a=j\xc3\xa9r\xc3\xb4me@vmime.org,\x01" "auth=Bearer vF9dft4qmTc2Nvb3RlckBhbHRhdmlzdGEuY29tCg==\x01\x01",
			evaluateChallenge(createSession("j\xc3\xa9r\xc3\xb4me@vmime.org"))
		);
	}

	// RFC 5801, section 4: the NUL character is not allowed in a saslname
	void testNULInUsername() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			createSession(vmime::string("us\0er", 5));

		VASSERT_THROW("1", evaluateChallenge(sess), vmime::exceptions::sasl_exception);
		VASSERT_FALSE("2", sess->getMechanism()->isComplete());
	}

	// RFC 7628, section 3.1: the "auth" key/value pair is REQUIRED
	void testNoAccessToken() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			createSession(vmime::make_shared <oauthBearerNoAccessTokenTestAuthenticator>());

		VASSERT_THROW("1", evaluateChallenge(sess), vmime::exceptions::sasl_exception);
		VASSERT_FALSE("2", sess->getMechanism()->isComplete());
	}

	// RFC 6750, section 2.1: b64token = 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" / "/" ) *"="
	void testEmptyAccessToken() {

		VASSERT_THROW("1", evaluateChallenge(createSession("user@vmime.org", "")), vmime::exceptions::sasl_exception);
	}

	void testInvalidAccessToken() {

		VASSERT_THROW("1", evaluateChallenge(createSession("user@vmime.org", "tok en")), vmime::exceptions::sasl_exception);
		VASSERT_THROW("2", evaluateChallenge(createSession("user@vmime.org", "tok\x01" "en")), vmime::exceptions::sasl_exception);
		VASSERT_THROW("3", evaluateChallenge(createSession("user@vmime.org", "token\r\n")), vmime::exceptions::sasl_exception);
		VASSERT_THROW("4", evaluateChallenge(createSession("user@vmime.org", "==")), vmime::exceptions::sasl_exception);
		VASSERT_THROW("5", evaluateChallenge(createSession("user@vmime.org", "tok=en")), vmime::exceptions::sasl_exception);

		// All the allowed characters
		VASSERT_EQ(
			"6",
			"n,a=user@vmime.org,\x01" "auth=Bearer azAZ09-._~+/==\x01\x01",
			evaluateChallenge(createSession("user@vmime.org", "azAZ09-._~+/=="))
		);
	}

	// RFC 7628, section 3.2.3: on error, the client MUST send a response
	// consisting of a single %x01 character (example from section 4.3)
	void testErrorChallenge() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		evaluateChallenge(sess);

		bool complete = true;

		VASSERT_EQ(
			"1",
			"\x01",
			evaluateChallenge(
				sess,
				"{\"status\":\"invalid_token\",\"scope\":\"example_scope\","
				"\"openid-configuration\":\"https://example.com/.well-known/openid-configuration\"}",
				&complete
			)
		);

		VASSERT_FALSE("2", complete);
	}

	// The server finishes the exchange after the error
	void testUnexpectedChallenge() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		evaluateChallenge(sess);
		evaluateChallenge(sess, "{\"status\":\"invalid_token\"}");

		VASSERT_THROW("1", evaluateChallenge(sess, "{\"status\":\"invalid_token\"}"), vmime::exceptions::sasl_exception);
	}

	void testNoSecurityLayer() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		evaluateChallenge(sess);

		const vmime::byte_t data[] = { 'd', 'a', 't', 'a', 0x00, 0xff };

		vmime::byte_t* output = NULL;
		size_t outputLen = 0;

		sess->getMechanism()->encode(sess, data, sizeof(data), &output, &outputLen);

		VASSERT_EQ("encode", vmime::string(data, data + sizeof(data)), vmime::string(output, output + outputLen));
		delete [] output;

		sess->getMechanism()->decode(sess, data, sizeof(data), &output, &outputLen);

		VASSERT_EQ("decode", vmime::string(data, data + sizeof(data)), vmime::string(output, output + outputLen));
		delete [] output;
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES
