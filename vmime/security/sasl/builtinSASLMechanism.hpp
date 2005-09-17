//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef VMIME_SECURITY_SASL_BUILTINSASLMECHANISM_HPP_INCLUDED
#define VMIME_SECURITY_SASL_BUILTINSASLMECHANISM_HPP_INCLUDED


#include "vmime/security/sasl/SASLMechanism.hpp"


namespace vmime {
namespace security {
namespace sasl {


class SASLContext;


/** A built-in authentication mechanism that relies on
  * the GNU SASL library.
  */
class builtinSASLMechanism : public SASLMechanism
{
public:

	builtinSASLMechanism(ref <SASLContext> ctx, const string& name);
	~builtinSASLMechanism();


	const string getName() const;

	const bool step
		(ref <SASLSession> sess,
		 const byte* challenge, const int challengeLen,
		 byte** response, int* responseLen);

	const bool isComplete() const;

	void encode(ref <SASLSession> sess,
		const byte* input, const int inputLen,
		byte** output, int* outputLen);

	void decode(ref <SASLSession> sess,
		const byte* input, const int inputLen,
		byte** output, int* outputLen);

private:

	/** SASL context */
	ref <SASLContext> m_context;

	/** Mechanism name */
	const string m_name;

	/** Authentication process status. */
	bool m_complete;
};


} // sasl
} // security
} // vmime


#endif // VMIME_SECURITY_SASL_BUILTINSASLMECHANISM_HPP_INCLUDED

