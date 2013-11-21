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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/net/serviceInfos.hpp"


namespace vmime {
namespace net {


// Common properties
const serviceInfos::property serviceInfos::property::SERVER_ADDRESS
	("server.address", serviceInfos::property::TYPE_STRING);

const serviceInfos::property serviceInfos::property::SERVER_PORT
	("server.port", serviceInfos::property::TYPE_INTEGER);

const serviceInfos::property serviceInfos::property::SERVER_ROOTPATH
	("server.rootpath", serviceInfos::property::TYPE_STRING);

const serviceInfos::property serviceInfos::property::AUTH_USERNAME
	("auth.username", serviceInfos::property::TYPE_STRING);

const serviceInfos::property serviceInfos::property::AUTH_PASSWORD
	("auth.password", serviceInfos::property::TYPE_STRING);

#if VMIME_HAVE_TLS_SUPPORT

const serviceInfos::property serviceInfos::property::CONNECTION_TLS
	("connection.tls", serviceInfos::property::TYPE_BOOLEAN, "false");

const serviceInfos::property serviceInfos::property::CONNECTION_TLS_REQUIRED
	("connection.tls.required", serviceInfos::property::TYPE_BOOLEAN, "false");

#endif // VMIME_HAVE_TLS_SUPPORT



// serviceInfos

serviceInfos::serviceInfos()
{
}


serviceInfos::serviceInfos(const serviceInfos&)
{
}


serviceInfos& serviceInfos::operator=(const serviceInfos&)
{
	return (*this);
}


serviceInfos::~serviceInfos()
{
}


bool serviceInfos::hasProperty(shared_ptr <session> s, const property& p) const
{
	return s->getProperties().hasProperty(getPropertyPrefix() + p.getName());
}



// serviceInfos::property

serviceInfos::property::property
	(const string& name, const Types type,
	 const string& defaultValue, const int flags)
	: m_name(name), m_defaultValue(defaultValue),
	  m_type(type), m_flags(flags)
{
}


serviceInfos::property::property
	(const property& p, const int addFlags, const int removeFlags)
{
	m_name = p.m_name;
	m_type = p.m_type;
	m_defaultValue = p.m_defaultValue;
	m_flags = (p.m_flags | addFlags) & ~removeFlags;
}


serviceInfos::property::property
	(const property& p, const string& newDefaultValue,
	 const int addFlags, const int removeFlags)
{
	m_name = p.m_name;
	m_type = p.m_type;
	m_defaultValue = newDefaultValue;
	m_flags = (p.m_flags | addFlags) & ~removeFlags;
}


serviceInfos::property& serviceInfos::property::operator=(const property& p)
{
	m_name = p.m_name;
	m_type = p.m_type;
	m_defaultValue = p.m_defaultValue;
	m_flags = p.m_flags;

	return (*this);
}


const string& serviceInfos::property::getName() const
{
	return (m_name);
}


const string& serviceInfos::property::getDefaultValue() const
{
	return (m_defaultValue);
}


serviceInfos::property::Types serviceInfos::property::getType() const
{
	return (m_type);
}


int serviceInfos::property::getFlags() const
{
	return (m_flags);
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

