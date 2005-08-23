//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_NET_SERVICEFACTORY_HPP_INCLUDED
#define VMIME_NET_SERVICEFACTORY_HPP_INCLUDED


#include <map>

#include "vmime/types.hpp"
#include "vmime/base.hpp"

#include "vmime/utility/stringUtils.hpp"
#include "vmime/utility/url.hpp"

#include "vmime/net/serviceInfos.hpp"
#include "vmime/net/authenticator.hpp"
#include "vmime/net/timeoutHandler.hpp"

#include "vmime/utility/progressionListener.hpp"


namespace vmime {
namespace net {


class service;
class session;


/** A factory to create 'service' objects for a specified protocol.
  */

class serviceFactory
{
private:

	serviceFactory();
	~serviceFactory();

public:

	static serviceFactory* getInstance();

	/** Information about a registered service. */
	class registeredService : public object
	{
		friend class serviceFactory;

	protected:

		virtual ~registeredService() { }

	public:

		virtual ref <service> create(ref <session> sess, ref <authenticator> auth) const = 0;

		virtual const string& getName() const = 0;
		virtual const serviceInfos& getInfos() const = 0;
	};

private:

	template <class S>
	class registeredServiceImpl : public registeredService
	{
		friend class serviceFactory;
		friend class vmime::creator;

	protected:

		registeredServiceImpl(const string& name)
			: m_name(name), m_servInfos(S::getInfosInstance())
		{
		}

	public:

		ref <service> create(ref <session> sess, ref <authenticator> auth) const
		{
			return vmime::create <S>(sess, auth);
		}

		const serviceInfos& getInfos() const
		{
			return (m_servInfos);
		}

		const string& getName() const
		{
			return (m_name);
		}

	private:

		const string m_name;
		const serviceInfos& m_servInfos;
	};

	std::vector <ref <registeredService> > m_services;

public:

	/** Register a new service by its protocol name.
	  *
	  * @param protocol protocol name
	  */
	template <class S>
	void registerServiceByProtocol(const string& protocol)
	{
		const string name = utility::stringUtils::toLower(protocol);
		m_services.push_back(vmime::create <registeredServiceImpl <S> >(name));
	}

	/** Create a new service instance from a protocol name.
	  *
	  * @param sess session
	  * @param protocol protocol name (eg. "pop3")
	  * @param auth authenticator used to provide credentials (can be NULL if not used)
	  * @return a new service instance for the specified protocol
	  * @throw exceptions::no_service_available if no service is registered
	  * for this protocol
	  */
	ref <service> create(ref <session> sess, const string& protocol, ref <authenticator> auth = NULL);

	/** Create a new service instance from a URL.
	  *
	  * @param sess session
	  * @param u full URL with at least protocol and server (you can also specify
	  * port, username and password)
	  * @param auth authenticator used to provide credentials (can be NULL if not used)
	  * @return a new service instance for the specified protocol
	  * @throw exceptions::no_service_available if no service is registered
	  * for this protocol
	  */
	ref <service> create(ref <session> sess, const utility::url& u, ref <authenticator> auth = NULL);

	/** Return information about a registered protocol.
	  *
	  * @param protocol protocol name
	  * @return information about this protocol
	  * @throw exceptions::no_service_available if no service is registered
	  * for this protocol
	  */
	ref <const registeredService> getServiceByProtocol(const string& protocol) const;

	/** Return the number of registered services.
	  *
	  * @return number of registered services
	  */
	const int getServiceCount() const;

	/** Return the registered service at the specified position.
	  *
	  * @param pos position of the registered service to return
	  * @return registered service at the specified position
	  */
	ref <const registeredService> getServiceAt(const int pos) const;

	/** Return a list of all registered services.
	  *
	  * @return list of registered services
	  */
	const std::vector <ref <const registeredService> > getServiceList() const;
};


} // net
} // vmime


#endif // VMIME_NET_SERVICEFACTORY_HPP_INCLUDED
