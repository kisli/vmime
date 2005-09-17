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

#include <gsasl.h>

#include "vmime/security/sasl/builtinSASLMechanism.hpp"

#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/SASLSession.hpp"

#include "vmime/exception.hpp"

#include <stdexcept>
#include <new>


namespace vmime {
namespace security {
namespace sasl {


builtinSASLMechanism::builtinSASLMechanism(ref <SASLContext> ctx, const string& name)
	: m_context(ctx), m_name(name), m_complete(false)
{
}


builtinSASLMechanism::~builtinSASLMechanism()
{
}


const string builtinSASLMechanism::getName() const
{
	return m_name;
}


const bool builtinSASLMechanism::step
	(ref <SASLSession> sess, const byte* challenge, const int challengeLen,
	 byte** response, int* responseLen)
{
	char* output = 0;
	size_t outputLen = 0;

	const int result = gsasl_step(sess->m_gsaslSession,
		reinterpret_cast <const char*>(challenge), challengeLen,
		&output, &outputLen);

	if (result == GSASL_OK || result == GSASL_NEEDS_MORE)
	{
		byte* res = new byte[outputLen];

		for (size_t i = 0 ; i < outputLen ; ++i)
			res[i] = output[i];

		*response = res;
		*responseLen = outputLen;

		free(output);
	}
	else
	{
		*response = 0;
		*responseLen = 0;
	}

	if (result == GSASL_OK)
	{
		// Authentication process completed
		m_complete = true;
		return true;
	}
	else if (result == GSASL_NEEDS_MORE)
	{
		// Continue authentication process
		return false;
	}
	else if (result == GSASL_MALLOC_ERROR)
	{
		throw std::bad_alloc();
	}
	else
	{
		throw exceptions::sasl_exception("Error when processing challenge: "
			+ SASLContext::getErrorMessage("gsasl_step", result));
	}
}


const bool builtinSASLMechanism::isComplete() const
{
	return m_complete;
}


void builtinSASLMechanism::encode
	(ref <SASLSession> sess, const byte* input, const int inputLen,
	 byte** output, int* outputLen)
{
	char* coutput = 0;
	size_t coutputLen = 0;

	if (gsasl_encode(sess->m_gsaslSession,
		reinterpret_cast <const char*>(input), inputLen,
		&coutput, &coutputLen) != GSASL_OK)
	{
		throw exceptions::sasl_exception("Encoding error.");
	}

	try
	{
		byte* res = new byte[coutputLen];

		std::copy(coutput, coutput + coutputLen, res);

		*output = res;
		*outputLen = static_cast <int>(coutputLen);
	}
	catch (...)
	{
		free(coutput);
		throw;
	}

	free(coutput);
}


void builtinSASLMechanism::decode
	(ref <SASLSession> sess, const byte* input, const int inputLen,
	 byte** output, int* outputLen)
{
	char* coutput = 0;
	size_t coutputLen = 0;

	try
	{
		if (gsasl_decode(sess->m_gsaslSession,
			reinterpret_cast <const char*>(input), inputLen,
			&coutput, &coutputLen) != GSASL_OK)
		{
			throw exceptions::sasl_exception("Decoding error.");
		}

		byte* res = new byte[coutputLen];

		std::copy(coutput, coutput + coutputLen, res);

		*output = res;
		*outputLen = static_cast <int>(coutputLen);
	}
	catch (...)
	{
		free(coutput);
		throw;
	}

	free(coutput);
}


} // sasl
} // security
} // vmime

