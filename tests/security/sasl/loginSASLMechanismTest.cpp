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
#include "vmime/security/sasl/loginSASLMechanism.hpp"
#include "vmime/security/sasl/defaultSASLAuthenticator.hpp"


class loginTestAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	const vmime::string getUsername() const { return "user"; }
	const vmime::string getPassword() const { return "pass"; }
};


class loginNoPasswordTestAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	const vmime::string getUsername() const { return "user"; }
	const vmime::string getPassword() const { throw vmime::exceptions::no_auth_information(); }
};


VMIME_TEST_SUITE_BEGIN(loginSASLMechanismTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testName)
		VMIME_TEST(testHasInitialResponse)
		VMIME_TEST(testIsPlaintext)
		VMIME_TEST(testResponses)
		VMIME_TEST(testChallengesIgnored)
		VMIME_TEST(testUnexpectedChallenge)
		VMIME_TEST(testNoAuthInformation)
		VMIME_TEST(testNoSecurityLayer)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::security::sasl::SASLSession> createSession(
		const vmime::shared_ptr <vmime::security::authenticator>& auth
			= vmime::make_shared <loginTestAuthenticator>()
	) {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = ctx->createSession(
			"smtp", auth, vmime::make_shared <vmime::security::sasl::loginSASLMechanism>(ctx, "LOGIN")
		);

		sess->init();

		return sess;
	}

	static const vmime::string evaluateChallenge(
		const vmime::shared_ptr <vmime::security::sasl::SASLSession>& sess,
		const vmime::string& challenge,
		bool* complete = NULL
	) {

		vmime::byte_t* resp = NULL;
		size_t respLen = 0;

		const bool res = sess->evaluateChallenge(
			reinterpret_cast <const vmime::byte_t*>(challenge.data()), challenge.length(),
			&resp, &respLen
		);

		if (complete) {
			*complete = res;
		}

		const vmime::string response(resp, resp + respLen);
		delete [] resp;

		return response;
	}


	void testName() {

		VASSERT_EQ("1", "LOGIN", createSession()->getMechanism()->getName());
	}

	// The server sends the first challenge
	void testHasInitialResponse() {

		VASSERT_FALSE("1", createSession()->getMechanism()->hasInitialResponse());
	}

	// LOGIN MUST NOT be used without adequate security protection
	// (draft-murchison-sasl-login)
	void testIsPlaintext() {

		VASSERT_TRUE("1", createSession()->getMechanism()->isPlaintext());
	}

	void testResponses() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		bool complete = true;

		VASSERT_EQ("1", "user", evaluateChallenge(sess, "Username:", &complete));
		VASSERT_FALSE("2", complete);
		VASSERT_FALSE("3", sess->getMechanism()->isComplete());

		VASSERT_EQ("4", "pass", evaluateChallenge(sess, "Password:", &complete));
		VASSERT_TRUE("5", complete);
		VASSERT_TRUE("6", sess->getMechanism()->isComplete());
	}

	// The contents of the challenges SHOULD be ignored (draft-murchison-sasl-login)
	void testChallengesIgnored() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		VASSERT_EQ("1", "user", evaluateChallenge(sess, "Password:"));
		VASSERT_EQ("2", "pass", evaluateChallenge(sess, ""));
	}

	void testUnexpectedChallenge() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		evaluateChallenge(sess, "Username:");
		evaluateChallenge(sess, "Password:");

		VASSERT_THROW("1", evaluateChallenge(sess, "Password:"), vmime::exceptions::sasl_exception);
	}

	void testNoAuthInformation() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			createSession(vmime::make_shared <loginNoPasswordTestAuthenticator>());

		evaluateChallenge(sess, "Username:");

		VASSERT_THROW("1", evaluateChallenge(sess, "Password:"), vmime::exceptions::sasl_exception);
	}

	void testNoSecurityLayer() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession();

		evaluateChallenge(sess, "Username:");
		evaluateChallenge(sess, "Password:");

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
