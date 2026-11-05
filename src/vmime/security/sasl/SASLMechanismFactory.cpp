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


#include <algorithm>

#include "vmime/security/sasl/SASLMechanismFactory.hpp"
#include "vmime/security/sasl/SASLContext.hpp"
#include "vmime/security/sasl/plainSASLMechanism.hpp"
#include "vmime/security/sasl/loginSASLMechanism.hpp"

#if VMIME_HAVE_GSASL
#	include "vmime/security/sasl/gsasl/SASLMechanism_GSASL.hpp"
#endif // VMIME_HAVE_GSASL

#include "vmime/utility/stringUtils.hpp"

#include "vmime/base.hpp"
#include "vmime/exception.hpp"


namespace vmime {
namespace security {
namespace sasl {


SASLMechanismFactory::SASLMechanismFactory() {

	// Mechanisms built into VMime
	m_builtinMechs.insert(
		MapType::value_type(
			"PLAIN",
			vmime::make_shared <registeredMechanismImpl <plainSASLMechanism> >()
		)
	);

	m_builtinMechs.insert(
		MapType::value_type(
			"LOGIN",
			vmime::make_shared <registeredMechanismImpl <loginSASLMechanism> >()
		)
	);
}


SASLMechanismFactory::~SASLMechanismFactory() {

}


// static
SASLMechanismFactory* SASLMechanismFactory::getInstance() {

	static SASLMechanismFactory instance;
	return &instance;
}


shared_ptr <SASLMechanism> SASLMechanismFactory::create(
	const shared_ptr <SASLContext>& ctx,
	const string& name_
) {

	const string name(utility::stringUtils::toUpper(name_));

	// Check for registered mechanisms
	MapType::iterator it = m_mechs.find(name);

	if (it != m_mechs.end()) {
		return (*it).second->create(ctx, name);
	}

	// Check for built-in mechanisms
	it = m_builtinMechs.find(name);

	if (it != m_builtinMechs.end()) {
		return (*it).second->create(ctx, name);
	}

#if VMIME_HAVE_GSASL
	// Check for mechanisms provided by the GNU SASL library
	if (SASLMechanism_GSASL::isMechanismSupported(name)) {
		return make_shared <SASLMechanism_GSASL>(ctx, name);
	}
#endif // VMIME_HAVE_GSASL

	throw exceptions::no_such_mechanism(name);
	return null;
}


const std::vector <string> SASLMechanismFactory::getSupportedMechanisms() const {

	std::vector <string> list;

	// Registered mechanisms
	for (MapType::const_iterator it = m_mechs.begin() ;
	     it != m_mechs.end() ; ++it) {

		list.push_back((*it).first);
	}

	// Built-in mechanisms
	for (MapType::const_iterator it = m_builtinMechs.begin() ;
	     it != m_builtinMechs.end() ; ++it) {

		if (std::find(list.begin(), list.end(), (*it).first) == list.end()) {
			list.push_back((*it).first);
		}
	}

#if VMIME_HAVE_GSASL
	// Mechanisms provided by the GNU SASL library
	const std::vector <string> gsaslMechs = SASLMechanism_GSASL::getSupportedMechanisms();

	for (size_t i = 0 ; i < gsaslMechs.size() ; ++i) {

		if (std::find(list.begin(), list.end(), gsaslMechs[i]) == list.end()) {
			list.push_back(gsaslMechs[i]);
		}
	}
#endif // VMIME_HAVE_GSASL

	return list;
}


bool SASLMechanismFactory::isMechanismSupported(const string& name) const {

	if (m_mechs.find(name) != m_mechs.end() || isBuiltinMechanism(name)) {
		return true;
	}

#if VMIME_HAVE_GSASL
	return SASLMechanism_GSASL::isMechanismSupported(name);
#else
	return false;
#endif // VMIME_HAVE_GSASL
}


bool SASLMechanismFactory::isBuiltinMechanism(const string& name) const {

	return m_builtinMechs.find(name) != m_builtinMechs.end();
}


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
