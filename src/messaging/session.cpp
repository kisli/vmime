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

#include "session.hpp"
#include "serviceFactory.hpp"

#include "store.hpp"
#include "transport.hpp"


namespace vmime {
namespace messaging {


session::session()
{
}


session::session(const propertySet& props)
	: m_props(props)
{
}


session::~session()
{
}


transport* session::getTransport(authenticator* auth)
{
	return (getTransport(m_props["transport.protocol"], auth));
}


transport* session::getTransport(const string& protocol, authenticator* auth)
{
	service* sv = serviceFactory::getInstance()->create(*this, protocol, auth);

	if (sv->type() != service::TYPE_TRANSPORT)
	{
		delete (sv);
		throw exceptions::no_service_available();
	}

	return static_cast<transport*>(sv);
}


store* session::getStore(authenticator* auth)
{
	return (getStore(m_props["store.protocol"], auth));
}


store* session::getStore(const string& protocol, authenticator* auth)
{
	service* sv = serviceFactory::getInstance()->create(*this, protocol, auth);

	if (sv->type() != service::TYPE_STORE)
	{
		delete (sv);
		throw exceptions::no_service_available();
	}

	return static_cast<store*>(sv);
}


} // messaging
} // vmime
