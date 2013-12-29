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

#ifndef VMIME_NET_SMTP_SMTPSERVICEINFOS_HPP_INCLUDED
#define VMIME_NET_SMTP_SMTPSERVICEINFOS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP


#include "vmime/net/serviceInfos.hpp"


namespace vmime {
namespace net {
namespace smtp {


/** Information about SMTP service.
  */

class VMIME_EXPORT SMTPServiceInfos : public serviceInfos
{
public:

	SMTPServiceInfos(const bool smtps);

	struct props
	{
		// SMTP-specific options
		serviceInfos::property PROPERTY_OPTIONS_NEEDAUTH;
#if VMIME_HAVE_SASL_SUPPORT
		serviceInfos::property PROPERTY_OPTIONS_SASL;
		serviceInfos::property PROPERTY_OPTIONS_SASL_FALLBACK;
#endif // VMIME_HAVE_SASL_SUPPORT

		serviceInfos::property PROPERTY_OPTIONS_PIPELINING;
		serviceInfos::property PROPERTY_OPTIONS_CHUNKING;

		// Common properties
		serviceInfos::property PROPERTY_AUTH_USERNAME;
		serviceInfos::property PROPERTY_AUTH_PASSWORD;

#if VMIME_HAVE_TLS_SUPPORT
		serviceInfos::property PROPERTY_CONNECTION_TLS;
		serviceInfos::property PROPERTY_CONNECTION_TLS_REQUIRED;
#endif // VMIME_HAVE_TLS_SUPPORT

		serviceInfos::property PROPERTY_SERVER_ADDRESS;
		serviceInfos::property PROPERTY_SERVER_PORT;
	};

	const props& getProperties() const;

	const string getPropertyPrefix() const;
	const std::vector <serviceInfos::property> getAvailableProperties() const;

private:

	const bool m_smtps;
};


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

#endif // VMIME_NET_SMTP_SMTPSERVICEINFOS_HPP_INCLUDED

