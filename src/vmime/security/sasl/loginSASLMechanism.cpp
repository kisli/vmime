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


#include "vmime/security/sasl/loginSASLMechanism.hpp"

#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/exception.hpp"

#include <algorithm>


namespace vmime {
namespace security {
namespace sasl {


loginSASLMechanism::loginSASLMechanism(
	const shared_ptr <SASLContext>& /* ctx */,
	const string& /* name */
)
	: m_usernameSent(false),
	  m_complete(false) {

}


loginSASLMechanism::~loginSASLMechanism() {

}


const string loginSASLMechanism::getName() const {

	return "LOGIN";
}


bool loginSASLMechanism::step(
	const shared_ptr <SASLSession>& sess,
	const byte_t* /* challenge */,
	const size_t /* challengeLen */,
	byte_t** response,
	size_t* responseLen
) {

	// The server sends two challenges: the client responds to the first one
	// with the username, and to the second one with the password. The contents
	// of the challenges SHOULD be ignored (draft-murchison-sasl-login).
	if (m_complete) {
		throw exceptions::sasl_exception("LOGIN: unexpected challenge from the server.");
	}

	string value;

	try {

		if (m_usernameSent) {
			value = sess->getAuthenticator()->getPassword();
		} else {
			value = sess->getAuthenticator()->getUsername();
		}

	} catch (exception& e) {

		throw exceptions::sasl_exception("LOGIN: cannot get the username or password.", e);
	}

	byte_t* res = new byte_t[value.length()];
	std::copy(value.begin(), value.end(), res);

	*response = res;
	*responseLen = value.length();

	if (!m_usernameSent) {

		m_usernameSent = true;
		return false;
	}

	m_complete = true;
	return true;
}


bool loginSASLMechanism::isComplete() const {

	return m_complete;
}


bool loginSASLMechanism::hasInitialResponse() const {

	// The server sends the first challenge (draft-murchison-sasl-login)
	return false;
}


void loginSASLMechanism::encode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	// LOGIN provides no security layer, just copy input bytes
	byte_t* res = new byte_t[inputLen];
	std::copy(input, input + inputLen, res);

	*outputLen = inputLen;
	*output = res;
}


void loginSASLMechanism::decode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	// LOGIN provides no security layer, just copy input bytes
	byte_t* res = new byte_t[inputLen];
	std::copy(input, input + inputLen, res);

	*outputLen = inputLen;
	*output = res;
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
