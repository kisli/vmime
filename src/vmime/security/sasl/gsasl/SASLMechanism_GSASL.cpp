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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_GSASL


#include "vmime/security/sasl/gsasl/SASLMechanism_GSASL.hpp"

#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/exception.hpp"

#include <stdexcept>
#include <new>
#include <unordered_set>


namespace vmime {
namespace security {
namespace sasl {


namespace {

// Mechanisms in which the client sends data first, and which can therefore
// send an initial response (RFC 4422, section 5). Only the mechanisms supported
// by gsaslCallback() are listed. LOGIN is not, because not all servers accept
// "AUTH LOGIN <username>".
const std::unordered_set <string> CLIENT_FIRST_MECHANISMS {
	"ANONYMOUS",
	"EXTERNAL",
	"GSSAPI",
	"PLAIN",
	"SCRAM-SHA-1",
	"SCRAM-SHA-1-PLUS",
	"SCRAM-SHA-256",
	"SCRAM-SHA-256-PLUS"
};


// Mechanisms which send the credentials (password or passcode) in clear text
const std::unordered_set <string> PLAINTEXT_MECHANISMS {
	"LOGIN",
	"PLAIN",
	"SECURID"
};


// GNU SASL context used to query the mechanisms provided by the library
class gsaslQueryContext {

public:

	gsaslQueryContext() {

		if (gsasl_init(&m_gsaslContext) != GSASL_OK) {
			throw std::bad_alloc();
		}
	}

	~gsaslQueryContext() {

		gsasl_done(m_gsaslContext);
	}

	Gsasl* get() const {

		return m_gsaslContext;
	}

private:

	Gsasl* m_gsaslContext;
};


Gsasl* getQueryContext() {

	static gsaslQueryContext queryContext;
	return queryContext.get();
}

}


SASLMechanism_GSASL::SASLMechanism_GSASL(
	const shared_ptr <SASLContext>& ctx,
	const string& name
)
	: m_context(ctx),
	  m_name(name),
	  m_complete(false),
	  m_initialResponse(CLIENT_FIRST_MECHANISMS.count(name) != 0),
	  m_gsaslContext(NULL),
	  m_gsaslSession(NULL) {

	if (gsasl_init(&m_gsaslContext) != GSASL_OK) {
		throw std::bad_alloc();
	}

	gsasl_client_start(m_gsaslContext, name.c_str(), &m_gsaslSession);

	gsasl_callback_set(m_gsaslContext, gsaslCallback);
}


SASLMechanism_GSASL::~SASLMechanism_GSASL() {

	gsasl_finish(m_gsaslSession);
	gsasl_done(m_gsaslContext);
}


// static
bool SASLMechanism_GSASL::isMechanismSupported(const string& name) {

	return gsasl_client_support_p(getQueryContext(), name.c_str()) != 0;
}


// static
const std::vector <string> SASLMechanism_GSASL::getSupportedMechanisms() {

	std::vector <string> list;
	char* out = 0;

	if (gsasl_client_mechlist(getQueryContext(), &out) == GSASL_OK) {

		// 'out' contains SASL mechanism names, separated by spaces
		for (char *start = out, *p = out ; ; ++p) {

			if (*p == ' ' || !*p) {

				list.push_back(string(start, p));
				start = p + 1;

				// End of string
				if (!*p) break;
			}
		}

		gsasl_free(out);
	}

	return list;
}


// static
bool SASLMechanism_GSASL::isChannelBindingTypeSupported(const string& type) {

	return type == "tls-unique"
#if defined(GSASL_VERSION_NUMBER) && GSASL_VERSION_NUMBER >= 0x020100
		|| type == "tls-exporter"
#endif
		;
}


const string SASLMechanism_GSASL::getName() const {

	return m_name;
}


bool SASLMechanism_GSASL::step(
	const shared_ptr <SASLSession>& sess,
	const byte_t* challenge,
	const size_t challengeLen,
	byte_t** response,
	size_t* responseLen
) {

	char* output = 0;
	size_t outputLen = 0;

	if (nullptr == m_gsaslSession) {
		throw exceptions::sasl_exception("Invalid SASL session");
	}

	// gsaslCallback() gets the authentication properties from the session
	gsasl_callback_hook_set(m_gsaslContext, sess.get());

	const int result = gsasl_step(
		m_gsaslSession,
		reinterpret_cast <const char*>(challenge), challengeLen,
		&output, &outputLen
	);

	gsasl_callback_hook_set(m_gsaslContext, NULL);

	if (result == GSASL_OK || result == GSASL_NEEDS_MORE) {

		byte_t* res = new byte_t[outputLen];

		for (size_t i = 0 ; i < outputLen ; ++i) {
			res[i] = output[i];
		}

		*response = res;
		*responseLen = outputLen;

		gsasl_free(output);

	} else {

		*response = 0;
		*responseLen = 0;
	}

	if (result == GSASL_OK) {

		// Authentication process completed
		m_complete = true;
		return true;

	} else if (result == GSASL_NEEDS_MORE) {

		// Continue authentication process
		return false;

	} else if (result == GSASL_MALLOC_ERROR) {

		throw std::bad_alloc();

	} else {

		throw exceptions::sasl_exception(
			"Error when processing challenge: "
			+ getErrorMessage("gsasl_step", result)
		);
	}
}


bool SASLMechanism_GSASL::isComplete() const {

	return m_complete;
}


bool SASLMechanism_GSASL::hasInitialResponse() const {

	return m_initialResponse;
}


bool SASLMechanism_GSASL::isPlaintext() const {

	return PLAINTEXT_MECHANISMS.count(m_name) != 0;
}


void SASLMechanism_GSASL::encode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	char* coutput = 0;
	size_t coutputLen = 0;

	if (gsasl_encode(m_gsaslSession, reinterpret_cast <const char*>(input), inputLen,
			&coutput, &coutputLen) != GSASL_OK) {

		throw exceptions::sasl_exception("Encoding error.");
	}

	try {

		byte_t* res = new byte_t[coutputLen];

		std::copy(coutput, coutput + coutputLen, res);

		*output = res;
		*outputLen = static_cast <int>(coutputLen);

	} catch (...) {

		gsasl_free(coutput);
		throw;
	}

	gsasl_free(coutput);
}


void SASLMechanism_GSASL::decode(
	const shared_ptr <SASLSession>& /* sess */,
	const byte_t* input,
	const size_t inputLen,
	byte_t** output,
	size_t* outputLen
) {

	char* coutput = 0;
	size_t coutputLen = 0;

	try {

		if (gsasl_decode(m_gsaslSession, reinterpret_cast <const char*>(input), inputLen,
				&coutput, &coutputLen) != GSASL_OK) {

			throw exceptions::sasl_exception("Decoding error.");
		}

		byte_t* res = new byte_t[coutputLen];

		std::copy(coutput, coutput + coutputLen, res);

		*output = res;
		*outputLen = static_cast <int>(coutputLen);

	} catch (...) {

		gsasl_free(coutput);
		throw;
	}

	gsasl_free(coutput);
}


// static
const string SASLMechanism_GSASL::getErrorMessage(const string& fname, const int code) {

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
		ERROR(GSASL_SECURID_SERVER_NEED_ADDITIONAL_PASSCODE)
		ERROR(GSASL_SECURID_SERVER_NEED_NEW_PIN)

		default:

			msg += "unknown error";
			break;
	}

#undef ERROR

	return msg;
}


// static
int SASLMechanism_GSASL::gsaslCallback(
	Gsasl* ctx,
	Gsasl_session* sctx,
	Gsasl_property prop
) {

	SASLSession* sess = reinterpret_cast <SASLSession*>(gsasl_callback_hook_get(ctx));

	if (!sess) {
		return GSASL_AUTHENTICATION_ERROR;
	}

	shared_ptr <authenticator> auth = sess->getAuthenticator();

	try {

		string res;

		switch (prop) {

			case GSASL_AUTHID:

				res = auth->getUsername();
				break;

			case GSASL_PASSWORD:

				res = auth->getPassword();
				break;

			case GSASL_ANONYMOUS_TOKEN:

				res = auth->getAnonymousToken();
				break;

			case GSASL_HOSTNAME:

				res = auth->getHostname();
				break;

			case GSASL_SERVICE:

				res = auth->getServiceName();
				break;

			case GSASL_CB_TLS_UNIQUE:
#if defined(GSASL_VERSION_NUMBER) && GSASL_VERSION_NUMBER >= 0x020100
			case GSASL_CB_TLS_EXPORTER:
#endif
			{
				const string type = (prop == GSASL_CB_TLS_UNIQUE ? "tls-unique" : "tls-exporter");
				const shared_ptr <SASLContext> saslCtx = sess->getContext();

				// Only provide channel binding data to the "-PLUS" mechanisms. With
				// the other SCRAM mechanisms, GNU SASL would tell the server that we
				// support channel binding ("y" flag), and authentication would fail
				// with servers which support it (eg. when falling back after a
				// "-PLUS" mechanism failed)
				if (!SASLContext::isChannelBindingMechanism(sess->getMechanism()->getName())
				    || saslCtx->getChannelBindingType() != type
				    || !saslCtx->hasChannelBindingData()) {

					return GSASL_NO_CALLBACK;
				}

				const byteArray& data = saslCtx->getChannelBindingData();

				res = saslCtx->encodeB64(&data[0], data.size());
				break;
			}

			case GSASL_AUTHZID:
			case GSASL_GSSAPI_DISPLAY_NAME:
			case GSASL_PASSCODE:
			case GSASL_SUGGESTED_PIN:
			case GSASL_PIN:
			case GSASL_REALM:

			default:

				return GSASL_NO_CALLBACK;
		}

		gsasl_property_set(sctx, prop, res.c_str());

		return GSASL_OK;

	} catch (...) {

		return GSASL_NO_CALLBACK;
	}
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_GSASL
