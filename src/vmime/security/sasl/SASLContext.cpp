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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/SASLMechanism.hpp"

#if VMIME_HAVE_GSASL
#	include "vmime/security/sasl/gsasl/SASLMechanism_GSASL.hpp"
#endif // VMIME_HAVE_GSASL

#include "vmime/base.hpp"

#include "vmime/utility/encoder/encoderFactory.hpp"

#include "vmime/utility/stream.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/inputStreamByteBufferAdapter.hpp"
#include "vmime/utility/stringUtils.hpp"

#if VMIME_HAVE_TLS_SUPPORT
#	include "vmime/net/tls/TLSSocket.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT


namespace vmime {
namespace security {
namespace sasl {


namespace {

// Mechanisms ordered by preference, from the least to the most secure. This is
// the order in which GNU SASL suggests mechanisms. Unknown mechanisms are less
// preferred than all of these.
const char* const MECHANISMS_BY_PREFERENCE[] = {
	"ANONYMOUS",
	"EXTERNAL",
	"LOGIN",
	"PLAIN",
	"SECURID",
	"NTLM",
	"DIGEST-MD5",
	"CRAM-MD5",
	"SCRAM-SHA-1",
	"SCRAM-SHA-1-PLUS",
	"SCRAM-SHA-256",
	"SCRAM-SHA-256-PLUS",
	"SAML20",
	"OPENID20",
	"GSSAPI",
	"GS2-KRB5"
};


// Return the preference of a mechanism: the greater, the more preferred
size_t getMechanismPreference(const string& name) {

	const size_t count = sizeof(MECHANISMS_BY_PREFERENCE) / sizeof(MECHANISMS_BY_PREFERENCE[0]);

	for (size_t i = 0 ; i < count ; ++i) {

		if (utility::stringUtils::isStringEqualNoCase(name, MECHANISMS_BY_PREFERENCE[i])) {
			return i + 1;
		}
	}

	return 0;
}

}


SASLContext::SASLContext() {

}


SASLContext::~SASLContext() {

}


// static
shared_ptr <SASLContext> SASLContext::create() {

	return shared_ptr <SASLContext>(new SASLContext());
}


shared_ptr <SASLSession> SASLContext::createSession(
	const string& serviceName,
	const shared_ptr <authenticator>& auth,
	const shared_ptr <SASLMechanism>& mech
) {

	return SASLSession::create
		(serviceName, dynamicCast <SASLContext>(shared_from_this()), auth, mech);
}


shared_ptr <SASLMechanism> SASLContext::createMechanism(const string& name) {

	// Mechanisms with channel binding would always fail without channel binding data
	if (isChannelBindingMechanism(name) && !hasChannelBindingData()) {
		throw exceptions::no_such_mechanism(name);
	}

	return SASLMechanismFactory::getInstance()->create(
		dynamicCast <SASLContext>(shared_from_this()), name
	);
}


bool SASLContext::setChannelBindingData(const string& type, const byteArray& data) {

#if VMIME_HAVE_GSASL
	// Only the types that the mechanisms provided by GNU SASL can use
	const bool supported = SASLMechanism_GSASL::isChannelBindingTypeSupported(type);
#else
	const bool supported = (type == "tls-unique" || type == "tls-exporter");
#endif // VMIME_HAVE_GSASL

	if (!supported || data.empty()) {
		return false;
	}

	m_channelBindingType = type;
	m_channelBindingData = data;

	return true;
}


#if VMIME_HAVE_TLS_SUPPORT

void SASLContext::setChannelBindingData(const shared_ptr <net::tls::TLSSocket>& sok) {

	if (!sok) {
		return;
	}

	// Prefer "tls-unique", which is the default for TLS 1.2 and earlier (RFC 5802);
	// it is not defined for TLS 1.3, for which "tls-exporter" is used (RFC 9266)
	byteArray data;

	if (sok->getChannelBindingData(net::tls::TLSSocket::CHANNEL_BINDING_TLS_UNIQUE, data)
	    && setChannelBindingData("tls-unique", data)) {

		return;
	}

	if (sok->getChannelBindingData(net::tls::TLSSocket::CHANNEL_BINDING_TLS_EXPORTER, data)) {
		setChannelBindingData("tls-exporter", data);
	}
}

#endif // VMIME_HAVE_TLS_SUPPORT


bool SASLContext::hasChannelBindingData() const {

	return !m_channelBindingData.empty();
}


const string& SASLContext::getChannelBindingType() const {

	return m_channelBindingType;
}


const byteArray& SASLContext::getChannelBindingData() const {

	return m_channelBindingData;
}


// static
bool SASLContext::isChannelBindingMechanism(const string& name) {

	static const string suffix = "-PLUS";

	return name.length() > suffix.length()
		&& utility::stringUtils::isStringEqualNoCase(
			string(name.end() - suffix.length(), name.end()), suffix
		);
}


shared_ptr <SASLMechanism> SASLContext::suggestMechanism(
	const std::vector <shared_ptr <SASLMechanism> >& mechs
) {

	shared_ptr <SASLMechanism> suggested;
	size_t suggestedPreference = 0;

	for (size_t i = 0 ; i < mechs.size() ; ++i) {

		const size_t preference = getMechanismPreference(mechs[i]->getName());

		if (!suggested || preference > suggestedPreference) {

			suggested = mechs[i];
			suggestedPreference = preference;
		}
	}

	return suggested;
}


void SASLContext::decodeB64(const string& input, byte_t** output, size_t* outputLen) {

	string res;

	utility::inputStreamStringAdapter is(input);
	utility::outputStreamStringAdapter os(res);

	shared_ptr <utility::encoder::encoder> dec =
		utility::encoder::encoderFactory::getInstance()->create("base64");

	dec->decode(is, os);

	byte_t* out = new byte_t[res.length()];

	std::copy(res.begin(), res.end(), out);

	*output = out;
	*outputLen = res.length();
}


const string SASLContext::encodeB64(const byte_t* input, const size_t inputLen) {

	string res;

	utility::inputStreamByteBufferAdapter is(input, inputLen);
	utility::outputStreamStringAdapter os(res);

	shared_ptr <utility::encoder::encoder> enc =
		utility::encoder::encoderFactory::getInstance()->create("base64");

	enc->encode(is, os);

	return res;
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
