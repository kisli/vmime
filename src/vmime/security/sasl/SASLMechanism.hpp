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

#ifndef VMIME_SECURITY_SASL_SASLMECHANISM_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLMECHANISM_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/types.hpp"


namespace vmime {
namespace security {
namespace sasl {


class SASLSession;


/** An SASL mechanism.
  */
class VMIME_EXPORT SASLMechanism : public object
{
public:

	/** Return the name of this mechanism.
	  *
	  * @return mechanism name
	  */
	virtual const string getName() const = 0;

	/** Perform one step of SASL authentication. Accept data from the
	  * server (challenge), process it and return data to be returned
	  * in response to the server.
	  *
	  * If the challenge is empty (challengeLen == 0), the initial
	  * response is returned, if this mechanism has one.
	  *
	  * @param sess SASL session
	  * @param challenge challenge sent from the server
	  * @param challengeLen length of challenge
	  * @param response response to send to the server (allocated by
	  * this function, free with delete[])
	  * @param responseLen length of response buffer
	  * @return true if authentication terminated successfully, or
	  * false if the authentication process should continue
	  * @throw exceptions::sasl_exception if an error occured during
	  * authentication (in this case, the values in 'response' and
	  * 'responseLen' are undetermined)
	  */
	virtual bool step
		(shared_ptr <SASLSession> sess,
		 const byte_t* challenge, const size_t challengeLen,
		 byte_t** response, size_t* responseLen) = 0;

	/** Check whether authentication has completed. If false, more
	  * calls to evaluateChallenge() are needed to complete the
	  * authentication process).
	  *
	  * @return true if the authentication has finished, or false
	  * otherwise
	  */
	virtual bool isComplete() const = 0;

	/** Determine if this mechanism has an optional initial response.
	  * If true, caller should call step() with an empty challenge to
	  * get the initial response.
	  *
	  * @return true if this mechanism has an initial response, or
	  * false otherwise
	  */
	virtual bool hasInitialResponse() const = 0;

	/** Encode data according to negotiated SASL mechanism. This
	  * might mean that data is integrity or privacy protected.
	  *
	  * @param sess SASL session
	  * @param input input buffer
	  * @param inputLen length of input buffer
	  * @param output output buffer (allocated bu the function,
	  * free with delete[])
	  * @param outputLen length of output buffer
	  * @throw exceptions::sasl_exception if an error occured during
	  * the encoding of data (in this case, the values in 'output' and
	  * 'outputLen' are undetermined)
	  */
	virtual void encode(shared_ptr <SASLSession> sess,
		const byte_t* input, const size_t inputLen,
		byte_t** output, size_t* outputLen) = 0;

	/** Decode data according to negotiated SASL mechanism. This
	  * might mean that data is integrity or privacy protected.
	  *
	  * @param sess SASL session
	  * @param input input buffer
	  * @param inputLen length of input buffer
	  * @param output output buffer (allocated bu the function,
	  * free with delete[])
	  * @param outputLen length of output buffer
	  * @throw exceptions::sasl_exception if an error occured during
	  * the encoding of data (in this case, the values in 'output' and
	  * 'outputLen' are undetermined)
	  */
	virtual void decode(shared_ptr <SASLSession> sess,
		const byte_t* input, const size_t inputLen,
		byte_t** output, size_t* outputLen) = 0;
};


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

#endif // VMIME_SECURITY_SASL_SASLMECHANISM_HPP_INCLUDED

