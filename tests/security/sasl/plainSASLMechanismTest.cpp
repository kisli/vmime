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
#include "vmime/security/sasl/plainSASLMechanism.hpp"
#include "vmime/security/sasl/defaultSASLAuthenticator.hpp"


class plainTestAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	plainTestAuthenticator(const vmime::string& username, const vmime::string& password)
		: m_username(username),
		  m_password(password) {

	}

	const vmime::string getUsername() const { return m_username; }
	const vmime::string getPassword() const { return m_password; }

private:

	const vmime::string m_username;
	const vmime::string m_password;
};


class noPasswordTestAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	const vmime::string getUsername() const { return "user"; }
	const vmime::string getPassword() const { throw vmime::exceptions::no_auth_information(); }
};


VMIME_TEST_SUITE_BEGIN(plainSASLMechanismTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testName)
		VMIME_TEST(testHasInitialResponse)
		VMIME_TEST(testResponse)
		VMIME_TEST(testResponse_UTF8)
		VMIME_TEST(testUnexpectedChallenge)
		VMIME_TEST(testEmptyUsername)
		VMIME_TEST(testEmptyPassword)
		VMIME_TEST(testNULInUsername)
		VMIME_TEST(testNULInPassword)
		VMIME_TEST(testNoAuthInformation)
		VMIME_TEST(testNoSecurityLayer)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::security::sasl::SASLSession> createSession(
		const vmime::shared_ptr <vmime::security::authenticator>& auth
	) {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = ctx->createSession(
			"smtp", auth, vmime::make_shared <vmime::security::sasl::plainSASLMechanism>(ctx, "PLAIN")
		);

		sess->init();

		return sess;
	}

	static vmime::shared_ptr <vmime::security::sasl::SASLSession> createSession(
		const vmime::string& username,
		const vmime::string& password
	) {

		return createSession(vmime::make_shared <plainTestAuthenticator>(username, password));
	}

	static const vmime::string evaluateChallenge(
		const vmime::shared_ptr <vmime::security::sasl::SASLSession>& sess
	) {

		vmime::byte_t* resp = NULL;
		size_t respLen = 0;

		sess->evaluateChallenge(NULL, 0, &resp, &respLen);

		const vmime::string res(resp, resp + respLen);
		delete [] resp;

		return res;
	}


	void testName() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession("user", "pass");

		VASSERT_EQ("1", "PLAIN", sess->getMechanism()->getName());
	}

	void testHasInitialResponse() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession("user", "pass");

		VASSERT_TRUE("1", sess->getMechanism()->hasInitialResponse());
	}

	// RFC 4616, section 2: message = [authzid] UTF8NUL authcid UTF8NUL passwd
	void testResponse() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession("user", "pass");

		VASSERT_FALSE("1", sess->getMechanism()->isComplete());
		VASSERT_EQ("2", vmime::string("\0user\0pass", 10), evaluateChallenge(sess));
		VASSERT_TRUE("3", sess->getMechanism()->isComplete());
	}

	void testResponse_UTF8() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			createSession("j\xc3\xa9r\xc3\xb4me", "p\xc3\xa0ss");

		VASSERT_EQ("1", vmime::string("\0j\xc3\xa9r\xc3\xb4me\0p\xc3\xa0ss", 15), evaluateChallenge(sess));
	}

	// The mechanism consists of a single message from the client
	void testUnexpectedChallenge() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession("user", "pass");

		evaluateChallenge(sess);

		VASSERT_THROW("1", evaluateChallenge(sess), vmime::exceptions::sasl_exception);
	}

	// RFC 4616, section 2: authcid = 1*SAFE
	void testEmptyUsername() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession("", "pass");

		VASSERT_THROW("1", evaluateChallenge(sess), vmime::exceptions::sasl_exception);
		VASSERT_FALSE("2", sess->getMechanism()->isComplete());
	}

	// RFC 4616, section 2: passwd = 1*SAFE
	void testEmptyPassword() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession("user", "");

		VASSERT_THROW("1", evaluateChallenge(sess), vmime::exceptions::sasl_exception);
	}

	// RFC 4616, section 2: the NUL character MUST NOT appear in authcid
	void testNULInUsername() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			createSession(vmime::string("us\0er", 5), "pass");

		VASSERT_THROW("1", evaluateChallenge(sess), vmime::exceptions::sasl_exception);
	}

	// RFC 4616, section 2: the NUL character MUST NOT appear in passwd
	void testNULInPassword() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			createSession("user", vmime::string("pa\0ss", 5));

		VASSERT_THROW("1", evaluateChallenge(sess), vmime::exceptions::sasl_exception);
	}

	void testNoAuthInformation() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			createSession(vmime::make_shared <noPasswordTestAuthenticator>());

		VASSERT_THROW("1", evaluateChallenge(sess), vmime::exceptions::sasl_exception);
	}

	void testNoSecurityLayer() {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess = createSession("user", "pass");

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
