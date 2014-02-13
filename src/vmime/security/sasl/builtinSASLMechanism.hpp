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

#ifndef VMIME_SECURITY_SASL_BUILTINSASLMECHANISM_HPP_INCLUDED
#define VMIME_SECURITY_SASL_BUILTINSASLMECHANISM_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/security/sasl/SASLMechanism.hpp"


namespace vmime {
namespace security {
namespace sasl {


class SASLContext;


/** A built-in authentication mechanism that relies on
  * the GNU SASL library.
  */
class VMIME_EXPORT builtinSASLMechanism : public SASLMechanism
{
public:

	builtinSASLMechanism(shared_ptr <SASLContext> ctx, const string& name);
	~builtinSASLMechanism();


	const string getName() const;

	bool step(shared_ptr <SASLSession> sess,
		 const byte_t* challenge, const size_t challengeLen,
		 byte_t** response, size_t* responseLen);

	bool isComplete() const;

	bool hasInitialResponse() const;

	void encode(shared_ptr <SASLSession> sess,
		const byte_t* input, const size_t inputLen,
		byte_t** output, size_t* outputLen);

	void decode(shared_ptr <SASLSession> sess,
		const byte_t* input, const size_t inputLen,
		byte_t** output, size_t* outputLen);

private:

	/** SASL context */
	shared_ptr <SASLContext> m_context;

	/** Mechanism name */
	const string m_name;

	/** Authentication process status. */
	bool m_complete;
};


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

#endif // VMIME_SECURITY_SASL_BUILTINSASLMECHANISM_HPP_INCLUDED

