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

#ifndef VMIME_SECURITY_AUTHENTICATOR_HPP_INCLUDED
#define VMIME_SECURITY_AUTHENTICATOR_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/types.hpp"


// Forward declarations
namespace vmime {
namespace net {

class service;

} // net
} // vmime


namespace vmime {
namespace security {


/** Provides required information for user authentication. The same
  * information can be requested multiple time (eg. in IMAP, there is a
  * new connection started each time a folder is open), so the object is
  * responsible for caching the information to avoid useless interactions
  * with the user.
  *
  * Usually, you should not inherit from this class, but instead from the
  * more convenient defaultAuthenticator class.
  *
  * WARNING: an authenticator should be used with one and ONLY ONE messaging
  * service at a time.
  */
class VMIME_EXPORT authenticator : public object
{
public:

	/** Return the authentication identity (usually, this
	  * is the username).
	  *
	  * @return username
	  * @throw exceptions::no_auth_information if the information
	  * could not be provided
	  */
	virtual const string getUsername() const = 0;

	/** Return the password of the authentication identity.
	  *
	  * @return password
	  * @throw exceptions::no_auth_information if the information
	  * could not be provided
	  */
	virtual const string getPassword() const = 0;

	/** Return the local host name of the machine.
	  *
	  * @return hostname
	  * @throw exceptions::no_auth_information if the information
	  * could not be provided
	  */
	virtual const string getHostname() const = 0;

	/** Return the anonymous token (usually, this is the user's
	  * email address).
	  *
	  * @return anonymous token
	  * @throw exceptions::no_auth_information if the information
	  * could not be provided
	  */
	virtual const string getAnonymousToken() const = 0;

	/** Return the registered service name of the application
	  * service (eg: "imap"). This can be used by GSSAPI or DIGEST-MD5
	  * mechanisms with SASL.
	  *
	  * @return service name
	  * @throw exceptions::no_auth_information if the information
	  * could not be provided
	  */
	virtual const string getServiceName() const = 0;

	/** Called by the messaging service to allow this authenticator to
	  * know which service is currently using it. This is called just
	  * before the service starts the authentication process.
	  *
	  * @param serv messaging service instance
	  */
	virtual void setService(shared_ptr <net::service> serv) = 0;
};


} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_SECURITY_AUTHENTICATOR_HPP_INCLUDED

