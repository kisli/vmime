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

#include "vmime/net/session.hpp"
#include "vmime/net/serviceFactory.hpp"

#include "vmime/net/store.hpp"
#include "vmime/net/transport.hpp"


namespace vmime {
namespace net {


session::session()
{
}


session::session(const session& sess)
	: object(), m_props(sess.m_props)
{
}


session::session(const propertySet& props)
	: m_props(props)
{
}


session::~session()
{
}


ref <transport> session::getTransport(ref <authenticator> auth)
{
	return (getTransport(m_props["transport.protocol"], auth));
}


ref <transport> session::getTransport(const string& protocol, ref <authenticator> auth)
{
	ref <session> sess = thisRef().dynamicCast <session>();
	ref <service> sv = serviceFactory::getInstance()->create(sess, protocol, auth);

	if (sv->getType() != service::TYPE_TRANSPORT)
		throw exceptions::no_service_available();

	return sv.staticCast <transport>();
}


ref <transport> session::getTransport(const utility::url& url, ref <authenticator> auth)
{
	ref <session> sess = thisRef().dynamicCast <session>();
	ref <service> sv = serviceFactory::getInstance()->create(sess, url, auth);

	if (sv->getType() != service::TYPE_TRANSPORT)
		throw exceptions::no_service_available();

	return sv.staticCast <transport>();
}


ref <store> session::getStore(ref <authenticator> auth)
{
	return (getStore(m_props["store.protocol"], auth));
}


ref <store> session::getStore(const string& protocol, ref <authenticator> auth)
{
	ref <session> sess = thisRef().dynamicCast <session>();
	ref <service> sv = serviceFactory::getInstance()->create(sess, protocol, auth);

	if (sv->getType() != service::TYPE_STORE)
		throw exceptions::no_service_available();

	return sv.staticCast <store>();
}


ref <store> session::getStore(const utility::url& url, ref <authenticator> auth)
{
	ref <session> sess = thisRef().dynamicCast <session>();
	ref <service> sv = serviceFactory::getInstance()->create(sess, url, auth);

	if (sv->getType() != service::TYPE_STORE)
		throw exceptions::no_service_available();

	return sv.staticCast <store>();
}


const propertySet& session::getProperties() const
{
	return (m_props);
}


propertySet& session::getProperties()
{
	return (m_props);
}


} // net
} // vmime
