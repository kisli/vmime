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

#ifndef VMIME_NET_POP3_POP3SERVICEINFOS_HPP_INCLUDED
#define VMIME_NET_POP3_POP3SERVICEINFOS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3


#include "vmime/net/serviceInfos.hpp"


namespace vmime {
namespace net {
namespace pop3 {


/** Information about POP3 service.
  */

class VMIME_EXPORT POP3ServiceInfos : public serviceInfos
{
public:

	POP3ServiceInfos(const bool pop3s);

	struct props
	{
		// POP3-specific options
		serviceInfos::property PROPERTY_OPTIONS_APOP;
		serviceInfos::property PROPERTY_OPTIONS_APOP_FALLBACK;
#if VMIME_HAVE_SASL_SUPPORT
		serviceInfos::property PROPERTY_OPTIONS_SASL;
		serviceInfos::property PROPERTY_OPTIONS_SASL_FALLBACK;
#endif // VMIME_HAVE_SASL_SUPPORT

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

	const bool m_pop3s;
};


} // pop3
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

#endif // VMIME_NET_POP3_POP3SERVICEINFOS_HPP_INCLUDED

