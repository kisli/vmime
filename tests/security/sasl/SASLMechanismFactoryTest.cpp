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
#include "vmime/security/sasl/SASLMechanismFactory.hpp"
#include "vmime/security/sasl/plainSASLMechanism.hpp"
#include "vmime/security/sasl/loginSASLMechanism.hpp"

#include <algorithm>


VMIME_TEST_SUITE_BEGIN(SASLMechanismFactoryTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testIsBuiltinMechanism)
		VMIME_TEST(testIsMechanismSupported)
		VMIME_TEST(testGetSupportedMechanisms)
		VMIME_TEST(testCreate_Builtin)
		VMIME_TEST(testCreate_Unknown)
	VMIME_TEST_LIST_END


	static vmime::security::sasl::SASLMechanismFactory* getFactory() {

		return vmime::security::sasl::SASLMechanismFactory::getInstance();
	}


	void testIsBuiltinMechanism() {

		VASSERT_TRUE("PLAIN", getFactory()->isBuiltinMechanism("PLAIN"));
		VASSERT_TRUE("LOGIN", getFactory()->isBuiltinMechanism("LOGIN"));

		// Provided by GNU SASL, if available
		VASSERT_FALSE("SCRAM-SHA-256", getFactory()->isBuiltinMechanism("SCRAM-SHA-256"));

		// Implemented by VMime, but must be registered
		VASSERT_FALSE("XOAUTH2", getFactory()->isBuiltinMechanism("XOAUTH2"));

		VASSERT_FALSE("X-UNKNOWN", getFactory()->isBuiltinMechanism("X-UNKNOWN"));
	}

	void testIsMechanismSupported() {

		VASSERT_TRUE("PLAIN", getFactory()->isMechanismSupported("PLAIN"));
		VASSERT_TRUE("LOGIN", getFactory()->isMechanismSupported("LOGIN"));
		VASSERT_FALSE("X-UNKNOWN", getFactory()->isMechanismSupported("X-UNKNOWN"));
	}

	void testGetSupportedMechanisms() {

		const std::vector <vmime::string> mechs = getFactory()->getSupportedMechanisms();

		// Provided both by VMime and GNU SASL: must be listed only once
		VASSERT_EQ("PLAIN", 1, static_cast <int>(std::count(mechs.begin(), mechs.end(), "PLAIN")));
		VASSERT_EQ("LOGIN", 1, static_cast <int>(std::count(mechs.begin(), mechs.end(), "LOGIN")));

		for (size_t i = 0 ; i < mechs.size() ; ++i) {
			VASSERT_TRUE(mechs[i], getFactory()->isMechanismSupported(mechs[i]));
		}
	}

	// Mechanisms built into VMime take precedence over the ones provided by GNU SASL
	void testCreate_Builtin() {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		VASSERT_TRUE(
			"PLAIN",
			vmime::dynamicCast <vmime::security::sasl::plainSASLMechanism>(
				getFactory()->create(ctx, "PLAIN")
			) != nullptr
		);

		VASSERT_TRUE(
			"plain",
			vmime::dynamicCast <vmime::security::sasl::plainSASLMechanism>(
				getFactory()->create(ctx, "plain")
			) != nullptr
		);

		VASSERT_TRUE(
			"LOGIN",
			vmime::dynamicCast <vmime::security::sasl::loginSASLMechanism>(
				getFactory()->create(ctx, "LOGIN")
			) != nullptr
		);
	}

	void testCreate_Unknown() {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		VASSERT_THROW(
			"1",
			getFactory()->create(ctx, "X-UNKNOWN"),
			vmime::exceptions::no_such_mechanism
		);
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES
