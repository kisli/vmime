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


#include "vmime/security/sasl/OAuthBearerSASLMechanism.hpp"

#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/exception.hpp"
#include "vmime/parserHelpers.hpp"

#include <algorithm>


namespace vmime {
namespace security {
namespace sasl {


namespace {

// Separator of the key/value pairs (RFC 7628, section 3.1)
const char KVSEP = '\x01';


// RFC 6750, section 2.1:
//    b64token = 1*( ALPHA / DIGIT / "-" / "." / "_" / "~" / "+" / "/" ) *"="
bool isValidBearerToken(const string& token) {

	size_t i = 0;

	while (i < token.length() &&
	       (parserHelpers::isAlpha(token[i]) || parserHelpers::isDigit(token[i]) ||
	        token[i] == '-' || token[i] == '.' || token[i] == '_' ||
	        token[i] == '~' || token[i] == '+' || token[i] == '/')) {

		++i;
	}

	if (i == 0) {
		return false;
	}

	while (i < token.length() && token[i] == '=') {
		++i;
	}

	return i == token.length();
}


// RFC 5801, section 4: in a saslname, "," and "=" are encoded as "=2C" and "=3D"
const string encodeSASLName(const string& name) {

	string res;
	res.reserve(name.length());

	for (string::const_iterator it = name.begin() ; it != name.end() ; ++it) {

		if (*it == ',') {
			res += "=2C";
		} else if (*it == '=') {
			res += "=3D";
		} else {
			res += *it;
		}
	}

	return res;
}

}


OAuthBearerSASLMechanism::OAuthBearerSASLMechanism(
	const shared_ptr <SASLContext>& /* ctx */,
	const string& /* name */
)
	: m_state(STATE_INITIAL) {

}


OAuthBearerSASLMechanism::~OAuthBearerSASLMechanism() {

}


const string OAuthBearerSASLMechanism::getName() const {

	return "OAUTHBEARER";
}


bool OAuthBearerSASLMechanism::step(
	const shared_ptr <SASLSession>& sess,
	const byte_t* /* challenge */,
	const size_t /* challengeLen */,
	byte_t** response,
	size_t* responseLen
) {

	if (m_state == STATE_RESPONSE_SENT) {

		// The server sends a challenge after the initial client response only if
		// authentication failed, with an error message (RFC 7628, section 3.2.2).
		// The client MUST then send a response consisting of a single %x01 character,
		// so that the server finishes the exchange (RFC 7628, section 3.2.3).
		byte_t* res = new byte_t[1];
		res[0] = KVSEP;

		*response = res;
		*responseLen = 1;
		m_state = STATE_ERROR_RECEIVED;

		return false;

	} else if (m_state != STATE_INITIAL) {

		throw exceptions::sasl_exception("OAUTHBEARER: unexpected challenge from the server.");
	}

	// The client sends the first message: if the server sends a challenge first
	// (when the protocol does not allow an initial response), it is ignored. It
	// should be empty, but some servers send a prompt (eg. "OAuthBearer:" with Yahoo)
	string accessToken;

	try {

		accessToken = sess->getAuthenticator()->getAccessToken();

	} catch (exception& e) {

		throw exceptions::sasl_exception("OAUTHBEARER: cannot get the access token.", e);
	}

	if (!isValidBearerToken(accessToken)) {
		throw exceptions::sasl_exception("OAUTHBEARER: invalid access token.");
	}

	// The username is optional
	string authzid;

	try {

		authzid = sess->getAuthenticator()->getUsername();

	} catch (exceptions::no_auth_information&) {

		// Do not send the username
	}

	// RFC 5801, section 4: the NUL character is not allowed in a saslname
	if (authzid.find('\0') != string::npos) {
		throw exceptions::sasl_exception("OAUTHBEARER: invalid username.");
	}

	// The initial client response has the following format (RFC 7628, section 3.1):
	//    client-resp = (gs2-header kvsep *kvpair kvsep) / kvsep
	//
	// The GS2 header (RFC 5801, section 4) indicates that channel binding is not
	// used, and MAY include the username associated with the resource being
	// accessed (authzid). The "auth" key/value pair is REQUIRED: it contains the
	// payload of the HTTP Authorization header (RFC 6750, section 2.1). The "host"
	// and "port" key/value pairs are not sent: they are only required for token
	// types which use keyed message digests.
	//
	// For example: "n,a=user@vmime.org,^Aauth=Bearer vF9dft4qmTc2Nvb3RlckBh^A^A"
	string message;
	message.reserve(32 + authzid.length() + accessToken.length());

	message += "n,";

	if (!authzid.empty()) {
		message += "a=";
		message += encodeSASLName(authzid);
	}

	message += ',';
	message += KVSEP;
	message += "auth=Bearer ";
	message += accessToken;
	message += KVSEP;
	message += KVSEP;

	byte_t* res = new byte_t[message.length()];
	std::copy(message.begin(), message.end(), res);

	*response = res;
	*responseLen = message.length();
	m_state = STATE_RESPONSE_SENT;

	return true;
}


bool OAuthBearerSASLMechanism::isComplete() const {

	return m_state != STATE_INITIAL;
}


bool OAuthBearerSASLMechanism::hasInitialResponse() const {

	// The client sends the first message (RFC 7628, section 3.1)
	return true;
}


bool OAuthBearerSASLMechanism::isPlaintext() const {

	// Clients MUST always use TLS when making requests with bearer
	// tokens (RFC 6750, section 5.3)
	return true;
}


void OAuthBearerSASLMechanism::encode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	// OAUTHBEARER provides no security layer, just copy input bytes
	byte_t* res = new byte_t[inputLen];
	std::copy(input, input + inputLen, res);

	*outputLen = inputLen;
	*output = res;
}


void OAuthBearerSASLMechanism::decode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	// OAUTHBEARER provides no security layer, just copy input bytes
	byte_t* res = new byte_t[inputLen];
	std::copy(input, input + inputLen, res);

	*outputLen = inputLen;
	*output = res;
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
