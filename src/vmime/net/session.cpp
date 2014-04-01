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


#include "vmime/net/session.hpp"
#include "vmime/net/serviceFactory.hpp"

#include "vmime/net/store.hpp"
#include "vmime/net/transport.hpp"


namespace vmime {
namespace net {


session::session()
{

#if VMIME_HAVE_TLS_SUPPORT
	m_tlsProps = make_shared <tls::TLSProperties>();
#endif // VMIME_HAVE_TLS_SUPPORT

}


session::session(const session& sess)
	: object(), m_props(sess.m_props)
{

#if VMIME_HAVE_TLS_SUPPORT
	m_tlsProps = make_shared <tls::TLSProperties>(*sess.m_tlsProps);
#endif // VMIME_HAVE_TLS_SUPPORT

}


session::session(const propertySet& props)
	: m_props(props)
{

#if VMIME_HAVE_TLS_SUPPORT
	m_tlsProps = make_shared <tls::TLSProperties>();
#endif // VMIME_HAVE_TLS_SUPPORT

}


session::~session()
{
}


shared_ptr <transport> session::getTransport(shared_ptr <security::authenticator> auth)
{
	return (getTransport(m_props["transport.protocol"], auth));
}


shared_ptr <transport> session::getTransport
	(const string& protocol, shared_ptr <security::authenticator> auth)
{
	shared_ptr <session> sess(dynamicCast <session>(shared_from_this()));
	shared_ptr <service> sv = serviceFactory::getInstance()->create(sess, protocol, auth);

	if (!sv || sv->getType() != service::TYPE_TRANSPORT)
		return null;

	return dynamicCast <transport>(sv);
}


shared_ptr <transport> session::getTransport
	(const utility::url& url, shared_ptr <security::authenticator> auth)
{
	shared_ptr <session> sess(dynamicCast <session>(shared_from_this()));
	shared_ptr <service> sv = serviceFactory::getInstance()->create(sess, url, auth);

	if (!sv || sv->getType() != service::TYPE_TRANSPORT)
		return null;

	return dynamicCast <transport>(sv);
}


shared_ptr <store> session::getStore(shared_ptr <security::authenticator> auth)
{
	return (getStore(m_props["store.protocol"], auth));
}


shared_ptr <store> session::getStore
	(const string& protocol, shared_ptr <security::authenticator> auth)
{
	shared_ptr <session> sess(dynamicCast <session>(shared_from_this()));
	shared_ptr <service> sv = serviceFactory::getInstance()->create(sess, protocol, auth);

	if (!sv || sv->getType() != service::TYPE_STORE)
		return null;

	return dynamicCast <store>(sv);
}


shared_ptr <store> session::getStore
	(const utility::url& url, shared_ptr <security::authenticator> auth)
{
	shared_ptr <session> sess(dynamicCast <session>(shared_from_this()));
	shared_ptr <service> sv = serviceFactory::getInstance()->create(sess, url, auth);

	if (!sv || sv->getType() != service::TYPE_STORE)
		return null;

	return dynamicCast <store>(sv);
}


const propertySet& session::getProperties() const
{
	return (m_props);
}


propertySet& session::getProperties()
{
	return (m_props);
}


#if VMIME_HAVE_TLS_SUPPORT

void session::setTLSProperties(shared_ptr <tls::TLSProperties> tlsProps)
{
	m_tlsProps = make_shared <tls::TLSProperties>(*tlsProps);
}


shared_ptr <tls::TLSProperties> session::getTLSProperties() const
{
	return m_tlsProps;
}

#endif // VMIME_HAVE_TLS_SUPPORT


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

