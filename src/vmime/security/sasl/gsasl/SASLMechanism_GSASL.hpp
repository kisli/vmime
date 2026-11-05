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

#ifndef VMIME_SECURITY_SASL_SASLMECHANISM_GSASL_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLMECHANISM_GSASL_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_GSASL


#include "vmime/security/sasl/SASLMechanism.hpp"

#include <gsasl.h>


namespace vmime {
namespace security {
namespace sasl {


class SASLContext;


/** An authentication mechanism provided by the GNU SASL library.
  */
class SASLMechanism_GSASL : public SASLMechanism {

public:

	SASLMechanism_GSASL(const shared_ptr <SASLContext>& ctx, const string& name);
	~SASLMechanism_GSASL();


	/** Test whether an authentication mechanism is provided by
	  * the GNU SASL library.
	  *
	  * @param name mechanism name
	  * @return true if the mechanism is provided by the GNU SASL
	  * library, or false otherwise
	  */
	static bool isMechanismSupported(const string& name);

	/** Return the list of the mechanisms provided by the GNU SASL
	  * library.
	  *
	  * @return list of mechanism names
	  */
	static const std::vector <string> getSupportedMechanisms();

	/** Test whether the mechanisms provided by the GNU SASL library
	  * can use a channel binding type.
	  *
	  * @param type channel binding type, as registered by IANA
	  * @return true if the channel binding type is supported,
	  * or false otherwise
	  */
	static bool isChannelBindingTypeSupported(const string& type);


	const string getName() const;

	bool step(
		const shared_ptr <SASLSession>& sess,
		const byte_t* challenge,
		const size_t challengeLen,
		byte_t** response, size_t* responseLen
	);

	bool isComplete() const;

	bool hasInitialResponse() const;

	void encode(
		const shared_ptr <SASLSession>& sess,
		const byte_t* input,
		const size_t inputLen,
		byte_t** output,
		size_t* outputLen
	);

	void decode(
		const shared_ptr <SASLSession>& sess,
		const byte_t* input,
		const size_t inputLen,
		byte_t** output,
		size_t* outputLen
	);

private:

	SASLMechanism_GSASL(const SASLMechanism_GSASL&);
	SASLMechanism_GSASL& operator=(const SASLMechanism_GSASL&);


	static const string getErrorMessage(const string& fname, const int code);

	static int gsaslCallback(Gsasl* ctx, Gsasl_session* sctx, Gsasl_property prop);


	/** SASL context */
	shared_ptr <SASLContext> m_context;

	/** Mechanism name */
	const string m_name;

	/** Authentication process status. */
	bool m_complete;

	/** Whether the mechanism is client-first, and thus supports an initial response. */
	bool m_initialResponse;

	/** GNU SASL context and session */
	Gsasl* m_gsaslContext;
	Gsasl_session* m_gsaslSession;
};


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_GSASL

#endif // VMIME_SECURITY_SASL_SASLMECHANISM_GSASL_HPP_INCLUDED
