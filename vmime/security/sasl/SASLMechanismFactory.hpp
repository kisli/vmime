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

#ifndef VMIME_SECURITY_SASL_SASLMECHANISMFACTORY_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLMECHANISMFACTORY_HPP_INCLUDED


#include "vmime/types.hpp"
#include "vmime/base.hpp"

#include "vmime/security/sasl/SASLMechanism.hpp"

#include <map>


namespace vmime {
namespace security {
namespace sasl {


class SASLContext;


/** Constructs SASL mechanism objects.
  */
class SASLMechanismFactory : public object
{
private:

	SASLMechanismFactory();
	~SASLMechanismFactory();


	class registeredMechanism : public object
	{
	public:

		virtual ref <SASLMechanism> create
			(ref <SASLContext> ctx, const string& name) = 0;
	};

	template <typename T>
	class registeredMechanismImpl : public registeredMechanism
	{
	public:

		ref <SASLMechanism> create(ref <SASLContext> ctx, const string& name)
		{
			return vmime::create <T>(ctx, name);
		}
	};

	typedef std::map <string, ref <registeredMechanism> > MapType;
	MapType m_mechs;

public:

	static SASLMechanismFactory* getInstance();

	/** Register a mechanism into this factory, so that subsequent
	  * calls to create return a valid object for this mechanism.
	  *
	  * @param name mechanism name
	  */
	template <typename MECH_CLASS>
	void registerMechanism(const string& name)
	{
		m_mechs.insert(MapType::value_type(name,
			vmime::create <registeredMechanismImpl <MECH_CLASS> >()));
	}

	/** Create a mechanism object given its name.
	  *
	  * @param ctx SASL context
	  * @param name mechanism name
	  * @return a new mechanism object
	  * @throw exceptions::no_such_mechanism if no mechanism is
	  * registered for the specified name
	  */
	ref <SASLMechanism> create(ref <SASLContext> ctx, const string& name);

	/** Return a list of supported mechanisms. This includes mechanisms
	  * registered using registerMechanism() as well as the ones that
	  * are built-in.
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

private:

#ifdef GSASL_VERSION
	Gsasl* m_gsaslContext;
#else
	void* m_gsaslContext;
#endif // GSASL_VERSION

};


} // sasl
} // security
} // vmime


#endif // VMIME_SECURITY_SASL_SASLMECHANISMFACTORY_HPP_INCLUDED

