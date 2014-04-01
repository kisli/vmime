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

#ifndef VMIME_NET_SESSION_HPP_INCLUDED
#define VMIME_NET_SESSION_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/security/authenticator.hpp"

#if VMIME_HAVE_TLS_SUPPORT
#	include "vmime/net/tls/TLSProperties.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT

#include "vmime/utility/url.hpp"

#include "vmime/propertySet.hpp"


namespace vmime {
namespace net {


class store;
class transport;


/** An object that contains all the information needed
  * for connection to a service.
  */

class VMIME_EXPORT session : public object
{
public:

	session();
	session(const session& sess);
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
	  * @return a new transport service, or NULL if no service is registered for this
	  * protocol or is not a transport protocol
	  */
	shared_ptr <transport> getTransport
		(shared_ptr <security::authenticator> auth = null);

	/** Return a transport service instance for the specified protocol.
	  *
	  * @param protocol transport protocol to use (eg. "smtp")
	  * @param auth authenticator object to use for the new transport service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new transport service, or NULL if no service is registered for this
	  * protocol or is not a transport protocol
	  */
	shared_ptr <transport> getTransport
		(const string& protocol,
		 shared_ptr <security::authenticator> auth = null);

	/** Return a transport service instance for the specified URL.
	  *
	  * @param url full URL with at least the protocol to use (eg: "smtp://myserver.com/")
	  * @param auth authenticator object to use for the new transport service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new transport service, or NULL if no service is registered for this
	  * protocol or is not a transport protocol
	  */
	shared_ptr <transport> getTransport
		(const utility::url& url,
		 shared_ptr <security::authenticator> auth = null);

	/** Return a transport service instance for the protocol specified
	  * in the session properties.
	  *
	  * The property "store.protocol" specify the protocol to use.
	  *
	  * @param auth authenticator object to use for the new store service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new store service, or NULL if no service is registered for this
	  * protocol or is not a store protocol
	  */
	shared_ptr <store> getStore(shared_ptr <security::authenticator> auth = null);

	/** Return a store service instance for the specified protocol.
	  *
	  * @param protocol store protocol to use (eg. "imap")
	  * @param auth authenticator object to use for the new store service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new store service, or NULL if no service is registered for this
	  * protocol or is not a store protocol
	  */
	shared_ptr <store> getStore
		(const string& protocol,
		 shared_ptr <security::authenticator> auth = null);

	/** Return a store service instance for the specified URL.
	  *
	  * @param url full URL with at least the protocol to use (eg: "imap://username:password@myserver.com/")
	  * @param auth authenticator object to use for the new store service. If
	  * NULL, a default one is used. The default authenticator simply return user
	  * credentials by reading the session properties "auth.username" and "auth.password".
	  * @return a new store service, or NULL if no service is registered for this
	  * protocol or is not a store protocol
	  */
	shared_ptr <store> getStore
		(const utility::url& url,
		 shared_ptr <security::authenticator> auth = null);

	/** Properties for the session and for the services.
	  */
	const propertySet& getProperties() const;

	/** Properties for the session and for the services.
	  */
	propertySet& getProperties();

#if VMIME_HAVE_TLS_SUPPORT

	/** Set properties for SSL/TLS secured connections in this session.
	  *
	  * @param tlsProps SSL/TLS properties
	  */
	void setTLSProperties(shared_ptr <tls::TLSProperties> tlsProps);

	/** Get properties for SSL/TLS secured connections in this session.
	  *
	  * @return SSL/TLS properties
	  */
	shared_ptr <tls::TLSProperties> getTLSProperties() const;

#endif // VMIME_HAVE_TLS_SUPPORT

private:

	propertySet m_props;

#if VMIME_HAVE_TLS_SUPPORT
	shared_ptr <tls::TLSProperties> m_tlsProps;
#endif // VMIME_HAVE_TLS_SUPPORT

};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_NET_SESSION_HPP_INCLUDED
