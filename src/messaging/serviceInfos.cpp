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

#include "vmime/messaging/serviceInfos.hpp"


namespace vmime {
namespace messaging {


// Common properties
const serviceInfos::property serviceInfos::property::SERVER_ADDRESS
	("server.address", serviceInfos::property::TYPE_STRING);

const serviceInfos::property serviceInfos::property::SERVER_PORT
	("server.port", serviceInfos::property::TYPE_INTEGER);

const serviceInfos::property serviceInfos::property::SERVER_ROOTPATH
	("server.rootpath", serviceInfos::property::TYPE_STRING);

const serviceInfos::property serviceInfos::property::SERVER_SOCKETFACTORY
	("server.socket-factory", serviceInfos::property::TYPE_STRING, "default");

const serviceInfos::property serviceInfos::property::AUTH_USERNAME
	("auth.username", serviceInfos::property::TYPE_STRING);

const serviceInfos::property serviceInfos::property::AUTH_PASSWORD
	("auth.password", serviceInfos::property::TYPE_STRING);

const serviceInfos::property serviceInfos::property::TIMEOUT_FACTORY
	("timeout.factory", serviceInfos::property::TYPE_STRING);



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


const bool serviceInfos::hasProperty(ref <session> s, const property& p) const
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


const serviceInfos::property::Types serviceInfos::property::getType() const
{
	return (m_type);
}


const int serviceInfos::property::getFlags() const
{
	return (m_flags);
}


} // messaging
} // vmime

