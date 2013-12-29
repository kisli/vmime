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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/smtp/SMTPServiceInfos.hpp"


namespace vmime {
namespace net {
namespace smtp {


SMTPServiceInfos::SMTPServiceInfos(const bool smtps)
	: m_smtps(smtps)
{
}


const string SMTPServiceInfos::getPropertyPrefix() const
{
	if (m_smtps)
		return "transport.smtps.";
	else
		return "transport.smtp.";
}


const SMTPServiceInfos::props& SMTPServiceInfos::getProperties() const
{
	static props smtpProps =
	{
		// SMTP-specific options
		property("options.need-authentication", serviceInfos::property::TYPE_BOOLEAN, "false"),
#if VMIME_HAVE_SASL_SUPPORT
		property("options.sasl", serviceInfos::property::TYPE_BOOLEAN, "true"),
		property("options.sasl.fallback", serviceInfos::property::TYPE_BOOLEAN, "false"),
#endif // VMIME_HAVE_SASL_SUPPORT

		property("options.pipelining", serviceInfos::property::TYPE_BOOLEAN, "true"),
		property("options.chunking", serviceInfos::property::TYPE_BOOLEAN, "true"),

		// Common properties
		property(serviceInfos::property::AUTH_USERNAME, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::AUTH_PASSWORD, serviceInfos::property::FLAG_REQUIRED),

#if VMIME_HAVE_TLS_SUPPORT
		property(serviceInfos::property::CONNECTION_TLS),
		property(serviceInfos::property::CONNECTION_TLS_REQUIRED),
#endif // VMIME_HAVE_TLS_SUPPORT

		property(serviceInfos::property::SERVER_ADDRESS, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::SERVER_PORT, "25"),
	};

	static props smtpsProps =
	{
		// SMTP-specific options
		property("options.need-authentication", serviceInfos::property::TYPE_BOOLEAN, "false"),
#if VMIME_HAVE_SASL_SUPPORT
		property("options.sasl", serviceInfos::property::TYPE_BOOLEAN, "true"),
		property("options.sasl.fallback", serviceInfos::property::TYPE_BOOLEAN, "false"),
#endif // VMIME_HAVE_SASL_SUPPORT

		property("options.pipelining", serviceInfos::property::TYPE_BOOLEAN, "true"),
		property("options.chunking", serviceInfos::property::TYPE_BOOLEAN, "true"),

		// Common properties
		property(serviceInfos::property::AUTH_USERNAME, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::AUTH_PASSWORD, serviceInfos::property::FLAG_REQUIRED),

#if VMIME_HAVE_TLS_SUPPORT
		property(serviceInfos::property::CONNECTION_TLS),
		property(serviceInfos::property::CONNECTION_TLS_REQUIRED),
#endif // VMIME_HAVE_TLS_SUPPORT

		property(serviceInfos::property::SERVER_ADDRESS, serviceInfos::property::FLAG_REQUIRED),
		property(serviceInfos::property::SERVER_PORT, "465"),
	};

	return m_smtps ? smtpsProps : smtpProps;
}


const std::vector <serviceInfos::property> SMTPServiceInfos::getAvailableProperties() const
{
	std::vector <property> list;
	const props& p = getProperties();

	// SMTP-specific options
	list.push_back(p.PROPERTY_OPTIONS_NEEDAUTH);
#if VMIME_HAVE_SASL_SUPPORT
	list.push_back(p.PROPERTY_OPTIONS_SASL);
	list.push_back(p.PROPERTY_OPTIONS_SASL_FALLBACK);
#endif // VMIME_HAVE_SASL_SUPPORT

	// Common properties
	list.push_back(p.PROPERTY_AUTH_USERNAME);
	list.push_back(p.PROPERTY_AUTH_PASSWORD);

#if VMIME_HAVE_TLS_SUPPORT
	if (!m_smtps)
	{
		list.push_back(p.PROPERTY_CONNECTION_TLS);
		list.push_back(p.PROPERTY_CONNECTION_TLS_REQUIRED);
	}
#endif // VMIME_HAVE_TLS_SUPPORT

	list.push_back(p.PROPERTY_SERVER_ADDRESS);
	list.push_back(p.PROPERTY_SERVER_PORT);

	return list;
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

