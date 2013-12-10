//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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


#include <sstream>

#include <gsasl.h>

#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/SASLMechanism.hpp"

#include "vmime/base.hpp"

#include "vmime/utility/encoder/encoderFactory.hpp"

#include "vmime/utility/stream.hpp"
#include "vmime/utility/outputStreamStringAdapter.hpp"
#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/inputStreamByteBufferAdapter.hpp"


namespace vmime {
namespace security {
namespace sasl {


SASLContext::SASLContext()
{
	if (gsasl_init(&m_gsaslContext) != GSASL_OK)
		throw std::bad_alloc();
}


SASLContext::~SASLContext()
{
	gsasl_done(m_gsaslContext);
}


shared_ptr <SASLSession> SASLContext::createSession
	(const string& serviceName,
	 shared_ptr <authenticator> auth, shared_ptr <SASLMechanism> mech)
{
	return make_shared <SASLSession>
		(serviceName, dynamicCast <SASLContext>(shared_from_this()), auth, mech);
}


shared_ptr <SASLMechanism> SASLContext::createMechanism(const string& name)
{
	return SASLMechanismFactory::getInstance()->create
		(dynamicCast <SASLContext>(shared_from_this()), name);
}


shared_ptr <SASLMechanism> SASLContext::suggestMechanism
	(const std::vector <shared_ptr <SASLMechanism> >& mechs)
{
	if (mechs.empty())
		return null;

	std::ostringstream oss;

	for (unsigned int i = 0 ; i < mechs.size() ; ++i)
		oss << mechs[i]->getName() << " ";

	const string mechList = oss.str();
	const char* suggested = gsasl_client_suggest_mechanism
		(m_gsaslContext, mechList.c_str());

	if (suggested)
	{
		for (unsigned int i = 0 ; i < mechs.size() ; ++i)
		{
			if (mechs[i]->getName() == suggested)
				return mechs[i];
		}
	}

	return null;
}


void SASLContext::decodeB64(const string& input, byte_t** output, size_t* outputLen)
{
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


const string SASLContext::encodeB64(const byte_t* input, const size_t inputLen)
{
	string res;

	utility::inputStreamByteBufferAdapter is(input, inputLen);
	utility::outputStreamStringAdapter os(res);

	shared_ptr <utility::encoder::encoder> enc =
		utility::encoder::encoderFactory::getInstance()->create("base64");

	enc->encode(is, os);

	return res;
}


const string SASLContext::getErrorMessage(const string& fname, const int code)
{
	string msg = fname + "() returned ";

#define ERROR(x) \
	case x: msg += #x; break;

	switch (code)
	{
	ERROR(GSASL_NEEDS_MORE)
	ERROR(GSASL_UNKNOWN_MECHANISM)
	ERROR(GSASL_MECHANISM_CALLED_TOO_MANY_TIMES)
	ERROR(GSASL_MALLOC_ERROR)
	ERROR(GSASL_BASE64_ERROR)
	ERROR(GSASL_CRYPTO_ERROR)
	ERROR(GSASL_SASLPREP_ERROR)
	ERROR(GSASL_MECHANISM_PARSE_ERROR)
	ERROR(GSASL_AUTHENTICATION_ERROR)
	ERROR(GSASL_INTEGRITY_ERROR)
	ERROR(GSASL_NO_CLIENT_CODE)
	ERROR(GSASL_NO_SERVER_CODE)
	ERROR(GSASL_NO_CALLBACK)
	ERROR(GSASL_NO_ANONYMOUS_TOKEN)
	ERROR(GSASL_NO_AUTHID)
	ERROR(GSASL_NO_AUTHZID)
	ERROR(GSASL_NO_PASSWORD)
	ERROR(GSASL_NO_PASSCODE)
	ERROR(GSASL_NO_PIN)
	ERROR(GSASL_NO_SERVICE)
	ERROR(GSASL_NO_HOSTNAME)
	ERROR(GSASL_GSSAPI_RELEASE_BUFFER_ERROR)
	ERROR(GSASL_GSSAPI_IMPORT_NAME_ERROR)
	ERROR(GSASL_GSSAPI_INIT_SEC_CONTEXT_ERROR)
	ERROR(GSASL_GSSAPI_ACCEPT_SEC_CONTEXT_ERROR)
	ERROR(GSASL_GSSAPI_UNWRAP_ERROR)
	ERROR(GSASL_GSSAPI_WRAP_ERROR)
	ERROR(GSASL_GSSAPI_ACQUIRE_CRED_ERROR)
	ERROR(GSASL_GSSAPI_DISPLAY_NAME_ERROR)
	ERROR(GSASL_GSSAPI_UNSUPPORTED_PROTECTION_ERROR)
	ERROR(GSASL_KERBEROS_V5_INIT_ERROR)
	ERROR(GSASL_KERBEROS_V5_INTERNAL_ERROR)
	ERROR(GSASL_SECURID_SERVER_NEED_ADDITIONAL_PASSCODE)
	ERROR(GSASL_SECURID_SERVER_NEED_NEW_PIN)

	default:

		msg += "unknown error";
		break;
	}

#undef ERROR

	return msg;
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

