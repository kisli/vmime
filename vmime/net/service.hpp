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

#ifndef VMIME_NET_SERVICE_HPP_INCLUDED
#define VMIME_NET_SERVICE_HPP_INCLUDED


#include "vmime/config.hpp"
#include "vmime/types.hpp"

#include "vmime/net/session.hpp"

#include "vmime/net/serviceInfos.hpp"
#include "vmime/net/connectionInfos.hpp"

#include "vmime/net/socket.hpp"
#include "vmime/net/timeoutHandler.hpp"

#if VMIME_HAVE_TLS_SUPPORT
	#include "vmime/security/cert/certificateVerifier.hpp"
#endif // VMIME_HAVE_TLS_SUPPORT

#include "vmime/utility/progressListener.hpp"


namespace vmime {
namespace net {


/** Base class for messaging services.
  */

class service : public object
{
protected:

	service(ref <session> sess, const serviceInfos& infos, ref <security::authenticator> auth);

public:

	virtual ~service();

	/** Possible service types. */
	enum Type
	{
		TYPE_STORE = 0,    /**< The service is a message store. */
		TYPE_TRANSPORT     /**< The service sends messages. */
	};

	/** Return the type of service.
	  *
	  * @return type of service
	  */
	virtual Type getType() const = 0;

	/** Return the protocol name of this service.
	  *
	  * @return protocol name
	  */
	virtual const string getProtocolName() const = 0;

	/** Return the session object associated with this service instance.
	  *
	  * @return session object
	  */
	ref <const session> getSession() const;

	/** Return the session object associated with this service instance.
	  *
	  * @return session object
	  */
	ref <session> getSession();

	/** Return information about this service.
	  *
	  * @return information about the service
	  */
	virtual const serviceInfos& getInfos() const = 0;

	/** Connect to service.
	  */
	virtual void connect() = 0;

	/** Disconnect from service.
	  */
	virtual void disconnect() = 0;

	/** Test whether this service is connected.
	  *
	  * @return true if the service is connected, false otherwise
	  */
	virtual bool isConnected() const = 0;

	/** Do nothing but ensure the server do not disconnect (for
	  * example, this can reset the auto-logout timer on the
	  * server, if one exists).
	  */
	virtual void noop() = 0;

	/** Return the authenticator object used with this service instance.
	  *
	  * @return authenticator object
	  */
	ref <const security::authenticator> getAuthenticator() const;

	/** Return the authenticator object used with this service instance.
	  *
	  * @return authenticator object
	  */
	ref <security::authenticator> getAuthenticator();

	/** Set the authenticator object used with this service instance.
	  *
	  * @param auth authenticator object
	  */
	void setAuthenticator(ref <security::authenticator> auth);

#if VMIME_HAVE_TLS_SUPPORT

	/** Set the object responsible for verifying certificates when
	  * using secured connections (TLS/SSL).
	  */
	void setCertificateVerifier(ref <security::cert::certificateVerifier> cv);

	/** Get the object responsible for verifying certificates when
	  * using secured connections (TLS/SSL).
	  */
	ref <security::cert::certificateVerifier> getCertificateVerifier();

#endif // VMIME_HAVE_TLS_SUPPORT

	/** Set the factory used to create socket objects for this
	  * service.
	  *
	  * @param sf socket factory
	  */
	void setSocketFactory(ref <socketFactory> sf);

	/** Return the factory used to create socket objects for this
	  * service.
	  *
	  * @return socket factory
	  */
	ref <socketFactory> getSocketFactory();

	/** Set the factory used to create timeoutHandler objects for
	  * this service. By default, no timeout handler is used. Not all
	  * services support timeout handling.
	  *
	  * @param thf timeoutHandler factory
	  */
	void setTimeoutHandlerFactory(ref <timeoutHandlerFactory> thf);

	/** Return the factory used to create timeoutHandler objects for
	  * this service.
	  *
	  * @return timeoutHandler factory
	  */
	ref <timeoutHandlerFactory> getTimeoutHandlerFactory();

	/** Set a property for this service (service prefix is added automatically).
	  *
	  * WARNING: this sets the property on the session object, so all service
	  * instances created with the session object will inherit the property.
	  *
	  * @param name property name
	  * @param value property value
	  */
	template <typename TYPE>
	void setProperty(const string& name, const TYPE& value)
	{
		m_session->getProperties()[getInfos().getPropertyPrefix() + name] = value;
	}

	/** Check whether the connection is secured.
	  *
	  * @return true if the connection is secured, false otherwise
	  */
	virtual bool isSecuredConnection() const = 0;

	/** Get information about the connection.
	  *
	  * @return information about the connection
	  */
	virtual ref <connectionInfos> getConnectionInfos() const = 0;

private:

	ref <session> m_session;
	ref <security::authenticator> m_auth;

#if VMIME_HAVE_TLS_SUPPORT
	ref <security::cert::certificateVerifier> m_certVerifier;
#endif // VMIME_HAVE_TLS_SUPPORT

	ref <socketFactory> m_socketFactory;

	ref <timeoutHandlerFactory> m_toHandlerFactory;
};


} // net
} // vmime


#endif // VMIME_NET_SERVICE_HPP_INCLUDED
