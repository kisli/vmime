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


#include "vmime/security/sasl/plainSASLMechanism.hpp"

#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/exception.hpp"

#include <algorithm>


namespace vmime {
namespace security {
namespace sasl {


namespace {

// RFC 4616, section 2: authcid and passwd are made of at least one character,
// and the NUL character, which is used as a delimiter, MUST NOT appear in them
bool isValidProduction(const string& value) {

	return !value.empty() && value.find('\0') == string::npos;
}

}


plainSASLMechanism::plainSASLMechanism(
	const shared_ptr <SASLContext>& /* ctx */,
	const string& /* name */
)
	: m_complete(false) {

}


plainSASLMechanism::~plainSASLMechanism() {

}


const string plainSASLMechanism::getName() const {

	return "PLAIN";
}


bool plainSASLMechanism::step(
	const shared_ptr <SASLSession>& sess,
	const byte_t* /* challenge */,
	const size_t /* challengeLen */,
	byte_t** response,
	size_t* responseLen
) {

	// The mechanism consists of a single message from the client: the server
	// sends no data (RFC 4616, section 2), so the challenge is ignored
	if (m_complete) {
		throw exceptions::sasl_exception("PLAIN: unexpected challenge from the server.");
	}

	string authcid, passwd;

	try {

		authcid = sess->getAuthenticator()->getUsername();
		passwd = sess->getAuthenticator()->getPassword();

	} catch (exception& e) {

		throw exceptions::sasl_exception("PLAIN: cannot get the username or password.", e);
	}

	if (!isValidProduction(authcid) || !isValidProduction(passwd)) {
		throw exceptions::sasl_exception("PLAIN: invalid username or password.");
	}

	// The message has the following format (RFC 4616, section 2):
	//    message = [authzid] UTF8NUL authcid UTF8NUL passwd
	//
	// No authorization identity is sent, so that the server derives it
	// from the authentication identity.
	string message;
	message.reserve(1 + authcid.length() + 1 + passwd.length());

	message += '\0';
	message += authcid;
	message += '\0';
	message += passwd;

	byte_t* res = new byte_t[message.length()];
	std::copy(message.begin(), message.end(), res);

	*response = res;
	*responseLen = message.length();
	m_complete = true;

	return true;
}


bool plainSASLMechanism::isComplete() const {

	return m_complete;
}


bool plainSASLMechanism::hasInitialResponse() const {

	return true;
}


void plainSASLMechanism::encode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	// PLAIN provides no security layer, just copy input bytes
	byte_t* res = new byte_t[inputLen];
	std::copy(input, input + inputLen, res);

	*outputLen = inputLen;
	*output = res;
}


void plainSASLMechanism::decode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	// PLAIN provides no security layer, just copy input bytes
	byte_t* res = new byte_t[inputLen];
	std::copy(input, input + inputLen, res);

	*outputLen = inputLen;
	*output = res;
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
