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

#ifndef VMIME_SECURITY_SASL_SASLMECHANISMFACTORY_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLMECHANISMFACTORY_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/types.hpp"
#include "vmime/base.hpp"

#include "vmime/security/sasl/SASLMechanism.hpp"

#include "vmime/utility/stringUtils.hpp"

#include <map>


namespace vmime {
namespace security {
namespace sasl {


class SASLContext;


/** Constructs SASL mechanism objects.
  *
  * Mechanism names are case-insensitive.
  */
class VMIME_EXPORT SASLMechanismFactory : public object {

private:

	SASLMechanismFactory();
	~SASLMechanismFactory();


	struct registeredMechanism : public object {

		virtual shared_ptr <SASLMechanism> create(
			const shared_ptr <SASLContext>& ctx, const string& name
		) = 0;
	};

	template <typename T>
	struct registeredMechanismImpl : public registeredMechanism {

		shared_ptr <SASLMechanism> create(
			const shared_ptr <SASLContext>& ctx,
			const string& name
		) {

			return vmime::make_shared <T>(ctx, name);
		}
	};

	typedef std::map <string, shared_ptr <registeredMechanism> > MapType;

	/** Mechanisms registered using registerMechanism() */
	MapType m_mechs;

	/** Mechanisms built into VMime */
	MapType m_builtinMechs;

public:

	static SASLMechanismFactory* getInstance();

	/** Register a mechanism into this factory, so that subsequent
	  * calls to create return a valid object for this mechanism.
	  *
	  * @param name mechanism name
	  */
	template <typename MECH_CLASS>
	void registerMechanism(const string& name) {

		m_mechs.insert(
			MapType::value_type(
				utility::stringUtils::toUpper(name),
				vmime::make_shared <registeredMechanismImpl <MECH_CLASS> >()
			)
		);
	}

	/** Create a mechanism object given its name.
	  *
	  * Mechanisms registered using registerMechanism() take precedence
	  * over the ones built into VMime, which take precedence over the
	  * ones provided by the GNU SASL library.
	  *
	  * @param ctx SASL context
	  * @param name mechanism name
	  * @return a new mechanism object
	  * @throw exceptions::no_such_mechanism if no mechanism is
	  * registered for the specified name
	  */
	shared_ptr <SASLMechanism> create(const shared_ptr <SASLContext>& ctx, const string& name);

	/** Return a list of supported mechanisms. This includes mechanisms
	  * registered using registerMechanism(), the ones built into VMime
	  * and the ones provided by the GNU SASL library.
	  *
	  * @return list of supported mechanisms
	  */
	const std::vector <string> getSupportedMechanisms() const;

	/** Test whether an authentication mechanism is supported.
	  *
	  * @param name mechanism name
	  * @return true if the specified mechanism is supported,
	  * false otherwise
	  */
	bool isMechanismSupported(const string& name) const;

	/** Test whether an authentication mechanism is built into VMime,
	  * ie. implemented by VMime itself and available without having
	  * to register it.
	  *
	  * @param name mechanism name
	  * @return true if the specified mechanism is built-in,
	  * or false otherwise
	  */
	bool isBuiltinMechanism(const string& name) const;
};


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_SECURITY_SASL_SASLMECHANISMFACTORY_HPP_INCLUDED
