//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_MESSAGING_SESSION_HPP_INCLUDED
#define VMIME_MESSAGING_SESSION_HPP_INCLUDED


#include "vmime/messaging/authenticator.hpp"
#include "vmime/messaging/progressionListener.hpp"
#include "vmime/messaging/url.hpp"

#include "vmime/propertySet.hpp"


namespace vmime {
namespace messaging {


class store;
class transport;


/** An object that contains all the information needed
  * for connection to a service.
  */

class session
{
public:

	session();
	session(const propertySet& props);

	virtual ~session();

	/** Return a transport service instance for the protocol specified
	  * in the session properties.
	  *
	  * The property "transport.protocol" specify the protocol to use.
	  *
	  * @param auth authenticator object to use for the new transport service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new transport service
	  */
	transport* getTransport(authenticator* auth = NULL);

	/** Return a transport service instance for the specified protocol.
	  *
	  * @param protocol transport protocol to use (eg. "smtp")
	  * @param auth authenticator object to use for the new transport service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new transport service
	  */
	transport* getTransport(const string& protocol, authenticator* auth = NULL);

	/** Return a transport service instance for the specified URL.
	  *
	  * @param url full URL with at least the protocol to use (eg: "smtp://myserver.com/")
	  * @param auth authenticator object to use for the new transport service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new transport service
	  */
	transport* getTransport(const messaging::url& url, authenticator* auth = NULL);

	/** Return a transport service instance for the protocol specified
	  * in the session properties.
	  *
	  * The property "store.protocol" specify the protocol to use.
	  *
	  * @param auth authenticator object to use for the new store service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new store service
	  */
	store* getStore(authenticator* auth = NULL);

	/** Return a store service instance for the specified protocol.
	  *
	  * @param protocol store protocol to use (eg. "imap")
	  * @param auth authenticator object to use for the new store service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new store service
	  */
	store* getStore(const string& protocol, authenticator* auth = NULL);

	/** Return a store service instance for the specified URL.
	  *
	  * @param url full URL with at least the protocol to use (eg: "imap://username:password@myserver.com/")
	  * @param auth authenticator object to use for the new store service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new store service
	  */
	store* getStore(const messaging::url& url, authenticator* auth = NULL);

	/** Properties for the session and for the services.
	  */
	const propertySet& getProperties() const;

	/** Properties for the session and for the services.
	  */
	propertySet& getProperties();

private:

	propertySet m_props;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_SESSION_HPP_INCLUDED
