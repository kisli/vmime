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


#include "vmime/security/sasl/OAuth2SASLAuthenticator.hpp"
#include "vmime/security/sasl/SASLMechanism.hpp"


class oauth2TestSASLMechanism : public vmime::security::sasl::SASLMechanism {

public:

	oauth2TestSASLMechanism(const vmime::string& name)
		: m_name(name) {

	}

	const vmime::string getName() const { return m_name; }

	bool step(
		const vmime::shared_ptr <vmime::security::sasl::SASLSession>& /* sess */,
		const vmime::byte_t* /* challenge */,
		const size_t /* challengeLen */,
		vmime::byte_t** /* response */,
		size_t* /* responseLen */
	) {

		return true;
	}

	bool isComplete() const { return true; }

	bool hasInitialResponse() const { return false; }

	bool isPlaintext() const { return false; }

	void encode(
		const vmime::shared_ptr <vmime::security::sasl::SASLSession>& /* sess */,
		const vmime::byte_t* /* input */,
		const size_t /* inputLen */,
		vmime::byte_t** /* output */,
		size_t* /* outputLen */
	) {

	}

	void decode(
		const vmime::shared_ptr <vmime::security::sasl::SASLSession>& /* sess */,
		const vmime::byte_t* /* input */,
		const size_t /* inputLen */,
		vmime::byte_t** /* output */,
		size_t* /* outputLen */
	) {

	}

private:

	const vmime::string m_name;
};


VMIME_TEST_SUITE_BEGIN(OAuth2SASLAuthenticatorTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testExclusive)
		VMIME_TEST(testExclusive_OnlyXOAuth2)
		VMIME_TEST(testExclusive_OnlyOAuthBearer)
		VMIME_TEST(testExclusive_NoOAuth2)
		VMIME_TEST(testSuggest)
		VMIME_TEST(testSuggest_XOAuth2Suggested)
		VMIME_TEST(testSuggest_NoOAuth2)
	VMIME_TEST_LIST_END


	// Return the names of the acceptable mechanisms, separated by spaces. The
	// first available mechanism is the suggested one.
	static const vmime::string getAcceptableMechanisms(
		const vmime::security::sasl::OAuth2SASLAuthenticator::Mode mode,
		const std::vector <vmime::string>& names
	) {

		std::vector <vmime::shared_ptr <vmime::security::sasl::SASLMechanism> > available;

		for (size_t i = 0 ; i < names.size() ; ++i) {
			available.push_back(vmime::make_shared <oauth2TestSASLMechanism>(names[i]));
		}

		const std::vector <vmime::shared_ptr <vmime::security::sasl::SASLMechanism> > mechs =
			vmime::security::sasl::OAuth2SASLAuthenticator(mode).getAcceptableMechanisms(
				available, available.empty() ? nullptr : available[0]
			);

		vmime::string res;

		for (size_t i = 0 ; i < mechs.size() ; ++i) {

			if (i != 0) {
				res += ' ';
			}

			res += mechs[i]->getName();
		}

		return res;
	}


	// OAUTHBEARER is preferred, and XOAUTH2 is not tried after it: it would send
	// the same access token
	void testExclusive() {

		const vmime::security::sasl::OAuth2SASLAuthenticator::Mode mode =
			vmime::security::sasl::OAuth2SASLAuthenticator::MODE_EXCLUSIVE;

		VASSERT_EQ("1", "OAUTHBEARER", getAcceptableMechanisms(mode, { "PLAIN", "XOAUTH2", "OAUTHBEARER", "LOGIN" }));
		VASSERT_EQ("2", "OAUTHBEARER", getAcceptableMechanisms(mode, { "OAUTHBEARER", "XOAUTH2" }));
		VASSERT_EQ("3", "OAUTHBEARER", getAcceptableMechanisms(mode, { "XOAUTH2", "OAUTHBEARER" }));
	}

	// Some servers only support XOAUTH2
	void testExclusive_OnlyXOAuth2() {

		const vmime::security::sasl::OAuth2SASLAuthenticator::Mode mode =
			vmime::security::sasl::OAuth2SASLAuthenticator::MODE_EXCLUSIVE;

		VASSERT_EQ("1", "XOAUTH2", getAcceptableMechanisms(mode, { "PLAIN", "XOAUTH2", "LOGIN" }));
	}

	void testExclusive_OnlyOAuthBearer() {

		const vmime::security::sasl::OAuth2SASLAuthenticator::Mode mode =
			vmime::security::sasl::OAuth2SASLAuthenticator::MODE_EXCLUSIVE;

		VASSERT_EQ("1", "OAUTHBEARER", getAcceptableMechanisms(mode, { "PLAIN", "OAUTHBEARER" }));
	}

	void testExclusive_NoOAuth2() {

		const vmime::security::sasl::OAuth2SASLAuthenticator::Mode mode =
			vmime::security::sasl::OAuth2SASLAuthenticator::MODE_EXCLUSIVE;

		VASSERT_EQ("1", "", getAcceptableMechanisms(mode, { "PLAIN", "LOGIN" }));
		VASSERT_EQ("2", "", getAcceptableMechanisms(mode, {}));
	}

	// The other mechanisms are tried after OAUTHBEARER or XOAUTH2, in the default order
	void testSuggest() {

		const vmime::security::sasl::OAuth2SASLAuthenticator::Mode mode =
			vmime::security::sasl::OAuth2SASLAuthenticator::MODE_SUGGEST;

		VASSERT_EQ(
			"1",
			"OAUTHBEARER PLAIN LOGIN",
			getAcceptableMechanisms(mode, { "PLAIN", "XOAUTH2", "LOGIN", "OAUTHBEARER" })
		);

		VASSERT_EQ(
			"2",
			"XOAUTH2 PLAIN LOGIN",
			getAcceptableMechanisms(mode, { "PLAIN", "LOGIN", "XOAUTH2" })
		);
	}

	// XOAUTH2 is not tried with the other mechanisms either, even if it is the
	// suggested one
	void testSuggest_XOAuth2Suggested() {

		const vmime::security::sasl::OAuth2SASLAuthenticator::Mode mode =
			vmime::security::sasl::OAuth2SASLAuthenticator::MODE_SUGGEST;

		VASSERT_EQ(
			"1",
			"OAUTHBEARER PLAIN LOGIN",
			getAcceptableMechanisms(mode, { "XOAUTH2", "PLAIN", "OAUTHBEARER", "LOGIN" })
		);
	}

	void testSuggest_NoOAuth2() {

		const vmime::security::sasl::OAuth2SASLAuthenticator::Mode mode =
			vmime::security::sasl::OAuth2SASLAuthenticator::MODE_SUGGEST;

		VASSERT_EQ("1", "PLAIN LOGIN", getAcceptableMechanisms(mode, { "PLAIN", "LOGIN" }));
		VASSERT_EQ("2", "", getAcceptableMechanisms(mode, {}));
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES
