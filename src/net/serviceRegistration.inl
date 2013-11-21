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

#include "vmime/net/serviceFactory.hpp"


#ifndef VMIME_BUILDING_DOC


namespace vmime {
namespace net {


template <class S>
class registeredServiceImpl : public serviceFactory::registeredService
{
public:

	registeredServiceImpl(const string& name, const int type)
		: m_type(type), m_name(name), m_servInfos(S::getInfosInstance())
	{
	}

	shared_ptr <service> create
		(shared_ptr <session> sess,
		 shared_ptr <security::authenticator> auth) const
	{
		return make_shared <S>(sess, auth);
	}

	const serviceInfos& getInfos() const
	{
		return (m_servInfos);
	}

	const string& getName() const
	{
		return (m_name);
	}

	int getType() const
	{
		return (m_type);
	}

private:

	const int m_type;
	const string m_name;
	const serviceInfos& m_servInfos;
};


// Basic service registerer
template <class S>
class serviceRegisterer
{
public:

	serviceRegisterer(const string& protocol, const service::Type type)
	{
		serviceFactory::getInstance()->registerService
			(make_shared <registeredServiceImpl <S> >(protocol, type));
	}
};


} // net
} // vmime


#define REGISTER_SERVICE(p_class, p_name, p_type) \
	vmime::net::serviceRegisterer <vmime::net::p_class> \
		p_name(#p_name, vmime::net::service::p_type)


#endif // VMIME_BUILDING_DOC

