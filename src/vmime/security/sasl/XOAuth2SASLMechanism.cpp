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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/security/sasl/XOAuth2SASLMechanism.hpp"

#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/exception.hpp"

#include <sstream>


namespace vmime {
namespace security {
namespace sasl {


XOAuth2SASLMechanism::XOAuth2SASLMechanism(
	const shared_ptr <SASLContext>& ctx,
	const string& /* name */
)
	: m_context(ctx),
	  m_complete(false) {

}


XOAuth2SASLMechanism::~XOAuth2SASLMechanism() {

}


const string XOAuth2SASLMechanism::getName() const {

	return "XOAUTH2";
}


bool XOAuth2SASLMechanism::step(
	const shared_ptr <SASLSession>& sess,
	const byte_t* /* challenge */,
	const size_t /* challengeLen */,
	byte_t** response,
	size_t* responseLen
) {

	// Build initial response
	//
	// The SASL XOAUTH2 initial client response has the following format:
	//    base64("user=" {User} "^Aauth=Bearer " {Access Token} "^A^A")

	const std::string user(sess->getAuthenticator()->getUsername());
	const std::string accessToken(sess->getAuthenticator()->getAccessToken());

	std::ostringstream initRespBytes;
	initRespBytes.write("user=", 5);
	initRespBytes.write(user.c_str(), user.length());
	initRespBytes.write("\x01", 1);
	initRespBytes.write("auth=Bearer ", 12);
	initRespBytes.write(accessToken.c_str(), accessToken.length());
	initRespBytes.write("\x01\x01", 2);

	const std::string initResp = initRespBytes.str();

	// Set initial response
	byte_t* res = new byte_t[initResp.length()];
	std::copy(initResp.c_str(), initResp.c_str() + initResp.length(), res);

	*response = res;
	*responseLen = initResp.length();
	m_complete = true;

	return true;
}


bool XOAuth2SASLMechanism::isComplete() const {

	return m_complete;
}


bool XOAuth2SASLMechanism::hasInitialResponse() const {

	return true;
}


void XOAuth2SASLMechanism::encode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	// No encoding performed, just copy input bytes
	byte_t* res = new byte_t[inputLen];
	std::copy(input, input + inputLen, res);

	*outputLen = inputLen;
	*output = res;
}


void XOAuth2SASLMechanism::decode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	// No decoding performed, just copy input bytes
	byte_t* res = new byte_t[inputLen];
	std::copy(input, input + inputLen, res);

	*outputLen = inputLen;
	*output = res;
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT
