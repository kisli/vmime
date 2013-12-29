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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/pop3/POP3ServiceInfos.hpp"


namespace vmime {
namespace net {
namespace pop3 {


POP3ServiceInfos::POP3ServiceInfos(const bool pop3s)
	: m_pop3s(pop3s)
{
}


const string POP3ServiceInfos::getPropertyPrefix() const
{
	if (m_pop3s)
		return "store.pop3s.";
	else
		return "store.pop3.";
}


const POP3ServiceInfos::props& POP3ServiceInfos::getProperties() const
{
	static props pop3Props =
	{
		// POP3-specific options
		property("options.apop", serviceInfos::property::TYPE_BOOLEAN, "true"),
		property("options.apop.fallback", serviceInfos::property::TYPE_BOOLEAN, "true"),
#if VMIME_HAVE_SASL_SUPPORT
		property("options.sasl", serviceInfos::property::TYPE_BOOLEAN, "true"),
		property("options.sasl.fallback", serviceInfos::property::TYPE_BOOLEAN, "true"),
#endif // VMIME_HAVE_SASL_SUPPORT

		// Common properties
		property(serviceInfos::property::AUTH_USERNAME, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::AUTH_PASSWORD, serviceInfos::property::FLAG_REQUIRED),

#if VMIME_HAVE_TLS_SUPPORT
		property(serviceInfos::property::CONNECTION_TLS),
		property(serviceInfos::property::CONNECTION_TLS_REQUIRED),
#endif // VMIME_HAVE_TLS_SUPPORT

		property(serviceInfos::property::SERVER_ADDRESS, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::SERVER_PORT, "110"),
	};

	static props pop3sProps =
	{
		// POP3-specific options
		property("options.apop", serviceInfos::property::TYPE_BOOLEAN, "true"),
		property("options.apop.fallback", serviceInfos::property::TYPE_BOOLEAN, "true"),
#if VMIME_HAVE_SASL_SUPPORT
		property("options.sasl", serviceInfos::property::TYPE_BOOLEAN, "true"),
		property("options.sasl.fallback", serviceInfos::property::TYPE_BOOLEAN, "true"),
#endif // VMIME_HAVE_SASL_SUPPORT

		// Common properties
		property(serviceInfos::property::AUTH_USERNAME, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::AUTH_PASSWORD, serviceInfos::property::FLAG_REQUIRED),

#if VMIME_HAVE_TLS_SUPPORT
		property(serviceInfos::property::CONNECTION_TLS),
		property(serviceInfos::property::CONNECTION_TLS_REQUIRED),
#endif // VMIME_HAVE_TLS_SUPPORT

		property(serviceInfos::property::SERVER_ADDRESS, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::SERVER_PORT, "995"),
	};

	return m_pop3s ? pop3sProps : pop3Props;
}


const std::vector <serviceInfos::property> POP3ServiceInfos::getAvailableProperties() const
{
	std::vector <property> list;
	const props& p = getProperties();

	// POP3-specific options
	list.push_back(p.PROPERTY_OPTIONS_APOP);
	list.push_back(p.PROPERTY_OPTIONS_APOP_FALLBACK);
#if VMIME_HAVE_SASL_SUPPORT
	list.push_back(p.PROPERTY_OPTIONS_SASL);
	list.push_back(p.PROPERTY_OPTIONS_SASL_FALLBACK);
#endif // VMIME_HAVE_SASL_SUPPORT

	// Common properties
	list.push_back(p.PROPERTY_AUTH_USERNAME);
	list.push_back(p.PROPERTY_AUTH_PASSWORD);

#if VMIME_HAVE_TLS_SUPPORT
	if (!m_pop3s)
	{
		list.push_back(p.PROPERTY_CONNECTION_TLS);
		list.push_back(p.PROPERTY_CONNECTION_TLS_REQUIRED);
	}
#endif // VMIME_HAVE_TLS_SUPPORT

	list.push_back(p.PROPERTY_SERVER_ADDRESS);
	list.push_back(p.PROPERTY_SERVER_PORT);

	return list;
}


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

