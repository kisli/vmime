//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include <stdexcept>
#include <new>

#include <gsasl.h>

#include "vmime/security/sasl/SASLMechanismFactory.hpp"
#include "vmime/security/sasl/builtinSASLMechanism.hpp"
#include "vmime/security/sasl/SASLContext.hpp"

#include "vmime/utility/stringUtils.hpp"

#include "vmime/base.hpp"
#include "vmime/exception.hpp"


namespace vmime {
namespace security {
namespace sasl {


SASLMechanismFactory::SASLMechanismFactory()
{
	if (gsasl_init(&m_gsaslContext) != GSASL_OK)
		throw std::bad_alloc();
}


SASLMechanismFactory::~SASLMechanismFactory()
{
	gsasl_done(m_gsaslContext);
}


// static
SASLMechanismFactory* SASLMechanismFactory::getInstance()
{
	static SASLMechanismFactory instance;
	return &instance;
}


ref <SASLMechanism> SASLMechanismFactory::create
	(ref <SASLContext> ctx, const string& name_)
{
	const string name(utility::stringUtils::toUpper(name_));

	// Check for built-in mechanisms
	if (isMechanismSupported(name))
	{
		return vmime::create <builtinSASLMechanism>(ctx, name);
	}
	// Check for registered mechanisms
	else
	{
		MapType::iterator it = m_mechs.find(name);

		if (it != m_mechs.end())
			return (*it).second->create(ctx, name);
	}

	throw exceptions::no_such_mechanism(name);
	return 0;
}


const std::vector <string> SASLMechanismFactory::getSupportedMechanisms() const
{
	std::vector <string> list;

	// Registered mechanisms
	for (MapType::const_iterator it = m_mechs.begin() ;
	     it != m_mechs.end() ; ++it)
	{
		list.push_back((*it).first);
	}

	// Built-in mechanisms
	char* out = 0;

	if (gsasl_client_mechlist(m_gsaslContext, &out) == GSASL_OK)
	{
		// 'out' contains SASL mechanism names, separated by spaces
		for (char *start = out, *p = out ; ; ++p)
		{
			if (*p == ' ' || !*p)
			{
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


bool SASLMechanismFactory::isMechanismSupported(const string& name) const
{
	return (gsasl_client_support_p(m_gsaslContext, name.c_str()) != 0 ||
		m_mechs.find(name) != m_mechs.end());
}


} // sasl
} // security
} // vmime

