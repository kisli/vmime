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
#include "vmime/security/sasl/XOAuth2SASLMechanism.hpp"
#include "vmime/security/sasl/defaultSASLAuthenticator.hpp"


class xoauth2TestAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	const vmime::string getUsername() const { return "user@vmime.org"; }
	const vmime::string getAccessToken() const { return "ya29.vF9dft4qmTc2Nvb3RlckBhdHRhdmlzdGEuY29tCg"; }
};


VMIME_TEST_SUITE_BEGIN(XOAuth2SASLMechanismTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testName)
		VMIME_TEST(testHasInitialResponse)
		VMIME_TEST(testIsPlaintext)
		VMIME_TEST(testResponse)
		VMIME_TEST(testResponse_Prompt)
		VMIME_TEST(testErrorChallenge)
		VMIME_TEST(testUnexpectedChallenge)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::security::sasl::SASLSession> createSession() {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = ctx->createSession(
			"imap",
			vmime::make_shared <xoauth2TestAuthenticator>(),
			vmime::make_shared <vmime::security::sasl::XOAuth2SASLMechanism>(ctx, "XOAUTH2")
		);

		sess->init();

		return sess;
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

		VASSERT_EQ("1", "XOAUTH2", createSession()->getMechanism()->getName());
	}

	void testHasInitialResponse() {

		VASSERT_TRUE("1", createSession()->getMechanism()->hasInitialResponse());
	}

	// Clients MUST always use TLS when making requests with bearer tokens
	// (RFC 6750, section 5.3)
	void testIsPlaintext() {

		VASSERT_TRUE("1", createSession()->getMechanism()->isPlaintext());
	}

	// base64("user=" {User} "^Aauth=Bearer " {Access Token} "^A^A")
	void testResponse() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		VASSERT_FALSE("1", sess->getMechanism()->isComplete());

		bool complete = false;

		VASSERT_EQ(
			"2",
			"user=user@vmime.org\x01" "auth=Bearer ya29.vF9dft4qmTc2Nvb3RlckBhdHRhdmlzdGEuY29tCg\x01\x01",
			evaluateChallenge(sess, "", &complete)
		);

		VASSERT_TRUE("3", complete);
		VASSERT_TRUE("4", sess->getMechanism()->isComplete());
	}

	// When the initial response is not sent with the command, Yahoo SMTP sends
	// a prompt ("XOauth2:") instead of an empty challenge: it is ignored
	void testResponse_Prompt() {

		VASSERT_EQ(
			"1",
			"user=user@vmime.org\x01" "auth=Bearer ya29.vF9dft4qmTc2Nvb3RlckBhdHRhdmlzdGEuY29tCg\x01\x01",
			evaluateChallenge(createSession(), "XOauth2:")
		);
	}

	// On error, the server sends a challenge with an error message, and the
	// client must send an empty response (example from Google's documentation)
	void testErrorChallenge() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		evaluateChallenge(sess);

		bool complete = true;

		VASSERT_EQ(
			"1",
			"",
			evaluateChallenge(
				sess,
				"{\"status\":\"401\",\"schemes\":\"bearer\",\"scope\":\"https://mail.google.com/\"}",
				&complete
			)
		);

		VASSERT_FALSE("2", complete);
		VASSERT_TRUE("3", sess->getMechanism()->isComplete());
	}

	// The server finishes the exchange after the error
	void testUnexpectedChallenge() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		evaluateChallenge(sess);
		evaluateChallenge(sess, "{\"status\":\"401\"}");

		VASSERT_THROW("1", evaluateChallenge(sess, "{\"status\":\"401\"}"), vmime::exceptions::sasl_exception);
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES
