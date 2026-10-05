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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/SASLMechanismFactory.hpp"
#include "vmime/security/sasl/defaultSASLAuthenticator.hpp"


class testSASLAuthenticator : public vmime::security::sasl::defaultSASLAuthenticator {

public:

	const vmime::string getUsername() const { return "user"; }
	const vmime::string getPassword() const { return "pass"; }
};


VMIME_TEST_SUITE_BEGIN(SASLContextTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testCreateMechanism)
		VMIME_TEST(testChannelBindingMechanism_NoData)
		VMIME_TEST(testChannelBindingMechanism_WithData)
		VMIME_TEST(testSetChannelBindingData_InvalidType)
		VMIME_TEST(testSetChannelBindingData_EmptyData)
		VMIME_TEST(testChannelBinding_TLSUnique)
		VMIME_TEST(testChannelBinding_TLSExporter)
		VMIME_TEST(testChannelBinding_NotUsedWithoutPlus)
	VMIME_TEST_LIST_END


	static const vmime::byteArray someChannelBindingData() {

		static const vmime::byte_t data[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c };
		return vmime::byteArray(data, data + sizeof(data));
	}

	static bool isMechanismAvailable(const vmime::string& name) {

		return vmime::security::sasl::SASLMechanismFactory::getInstance()->isMechanismSupported(name);
	}

	// Return the first message sent by the client (initial response)
	static const vmime::string getClientFirstMessage(
		const vmime::shared_ptr <vmime::security::sasl::SASLContext>& ctx,
		const vmime::string& mechName
	) {

		vmime::shared_ptr <vmime::security::sasl::SASLSession> sess =
			ctx->createSession("imap", vmime::make_shared <testSASLAuthenticator>(), ctx->createMechanism(mechName));

		sess->init();

		vmime::byte_t* resp = NULL;
		size_t respLen = 0;

		sess->evaluateChallenge(NULL, 0, &resp, &respLen);

		const vmime::string res(resp, resp + respLen);
		delete [] resp;

		return res;
	}


	void testCreateMechanism() {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		VASSERT_EQ("1", "PLAIN", ctx->createMechanism("PLAIN")->getName());
		VASSERT_EQ("2", "PLAIN", ctx->createMechanism("plain")->getName());
	}

	void testChannelBindingMechanism_NoData() {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		VASSERT_FALSE("1", ctx->hasChannelBindingData());

		VASSERT_THROW(
			"2",
			ctx->createMechanism("SCRAM-SHA-256-PLUS"),
			vmime::exceptions::no_such_mechanism
		);

		VASSERT_THROW(
			"3",
			ctx->createMechanism("scram-sha-1-plus"),
			vmime::exceptions::no_such_mechanism
		);
	}

	void testChannelBindingMechanism_WithData() {

		if (!isMechanismAvailable("SCRAM-SHA-256-PLUS")) {
			return;  // not supported by GNU SASL
		}

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		VASSERT_TRUE("1", ctx->setChannelBindingData("tls-unique", someChannelBindingData()));
		VASSERT_TRUE("2", ctx->hasChannelBindingData());

		VASSERT_EQ("3", "SCRAM-SHA-256-PLUS", ctx->createMechanism("SCRAM-SHA-256-PLUS")->getName());
	}

	void testSetChannelBindingData_InvalidType() {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		VASSERT_FALSE("1", ctx->setChannelBindingData("tls-server-end-point", someChannelBindingData()));
		VASSERT_FALSE("2", ctx->hasChannelBindingData());
	}

	void testSetChannelBindingData_EmptyData() {

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		VASSERT_FALSE("1", ctx->setChannelBindingData("tls-unique", vmime::byteArray()));
		VASSERT_FALSE("2", ctx->hasChannelBindingData());
	}

	void testChannelBinding_TLSUnique() {

		if (!isMechanismAvailable("SCRAM-SHA-256-PLUS")) {
			return;  // not supported by GNU SASL
		}

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		ctx->setChannelBindingData("tls-unique", someChannelBindingData());

		const vmime::string msg = getClientFirstMessage(ctx, "SCRAM-SHA-256-PLUS");

		VASSERT_EQ("1", "p=tls-unique,,n=user,r=", msg.substr(0, 23));
	}

	void testChannelBinding_TLSExporter() {

		if (!isMechanismAvailable("SCRAM-SHA-256-PLUS")) {
			return;  // not supported by GNU SASL
		}

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		if (!ctx->setChannelBindingData("tls-exporter", someChannelBindingData())) {
			return;  // not supported by GNU SASL (< 2.1.0)
		}

		const vmime::string msg = getClientFirstMessage(ctx, "SCRAM-SHA-256-PLUS");

		VASSERT_EQ("1", "p=tls-exporter,,n=user,r=", msg.substr(0, 25));
	}

	void testChannelBinding_NotUsedWithoutPlus() {

		if (!isMechanismAvailable("SCRAM-SHA-256")) {
			return;  // not supported by GNU SASL
		}

		vmime::shared_ptr <vmime::security::sasl::SASLContext> ctx =
			vmime::security::sasl::SASLContext::create();

		ctx->setChannelBindingData("tls-unique", someChannelBindingData());

		// Should not send the "y" flag, as the server would reject it if it supports channel binding
		const vmime::string msg = getClientFirstMessage(ctx, "SCRAM-SHA-256");

		VASSERT_EQ("1", "n,,n=user,r=", msg.substr(0, 12));
	}

VMIME_TEST_SUITE_END


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT
