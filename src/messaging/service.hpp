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

#ifndef VMIME_MESSAGING_SERVICE_HPP_INCLUDED
#define VMIME_MESSAGING_SERVICE_HPP_INCLUDED


#include "../types.hpp"
#include "session.hpp"

#include "authenticator.hpp"
#include "progressionListener.hpp"

#include "serviceFactory.hpp"
#include "serviceInfos.hpp"


namespace vmime {
namespace messaging {


class service
{
protected:

	service(class session& sess, const serviceInfos& infos, class authenticator* auth);

public:

	virtual ~service();

	// Possible service types
	enum Type
	{
		TYPE_STORE = 0,    /**< The service is a message store. */
		TYPE_TRANSPORT     /**< The service sends messages. */
	};

	/** Return the type of service.
	  *
	  * @return type of service
	  */
	virtual const Type type() const = 0;

	/** Return the protocol name of this service.
	  *
	  * @return protocol name
	  */
	virtual const string protocolName() const = 0;

	/** Return the session object associated with this service instance.
	  *
	  * @return session object
	  */
	const class session& session() const { return (m_session); }

	/** Return the session object associated with this service instance.
	  *
	  * @return session object
	  */
	class session& session() { return (m_session); }

	/** Return information about this service.
	  *
	  * @return information about the service
	  */
	virtual const serviceInfos& infos() const = 0;

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
	virtual const bool isConnected() const = 0;

	/** Do nothing but ensure the server do not disconnect (for
	  * example, this can reset the auto-logout timer on the
	  * server, if one exists).
	  */
	virtual void noop() = 0;

	/** Return the authenticator object used with this service instance.
	  *
	  * @return authenticator object
	  */
	const class authenticator& authenticator() const { return (*m_auth); }

	/** Return the authenticator object used with this service instance.
	  *
	  * @return authenticator object
	  */
	class authenticator& authenticator() { return (*m_auth); }

	// Basic service registerer
	template <class S>
	class initializer
	{
	public:

		initializer(const string& protocol)
		{
			serviceFactory::getInstance()->
				template registerName <S>(protocol);
		}
	};

private:

	bool m_deleteAuth;

	class session& m_session;
	class authenticator* m_auth;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_SERVICE_HPP_INCLUDED
