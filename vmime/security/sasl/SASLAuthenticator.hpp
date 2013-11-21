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

#ifndef VMIME_SECURITY_SASL_SASLAUTHENTICATOR_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLAUTHENTICATOR_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "vmime/types.hpp"

#include "vmime/security/authenticator.hpp"


namespace vmime {
namespace security {
namespace sasl {


class SASLMechanism;
class SASLSession;


/** SASL-aware authenticator.
  *
  * Usually, you should not inherit from this class, but instead from the
  * more convenient defaultSASLAuthenticator class.
  */
class VMIME_EXPORT SASLAuthenticator : public authenticator
{
public:

	/** This method is called to allow the client to choose the
	  * authentication mechanisms that will be used. By default,
	  * the most secure mechanisms are chosen.
	  *
	  * @param available available mechanisms
	  * @param suggested suggested mechanism (or NULL if the system
	  * could not suggest a mechanism)
	  * @return ordered list of mechanism to use among the available
	  * mechanisms (from the first to try to the last)
	  */
	virtual const std::vector <shared_ptr <SASLMechanism> > getAcceptableMechanisms
		(const std::vector <shared_ptr <SASLMechanism> >& available,
	         shared_ptr <SASLMechanism> suggested) const = 0;

	/** Set the SASL session which is using this authenticator.
	  *
	  * @param sess SASL session
	  */
	virtual void setSASLSession(shared_ptr <SASLSession> sess) = 0;

	/** Set the SASL mechanism which has been selected for the
	  * SASL authentication process. This may be called several times
	  * if the multiple mechanisms are tried by the service which
	  * use this authentication.
	  *
	  * @param mech SASL mechanism
	  */
	virtual void setSASLMechanism(shared_ptr <SASLMechanism> mech) = 0;
};


} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

#endif // VMIME_SECURITY_SASL_SASLAUTHENTICATOR_HPP_INCLUDED

