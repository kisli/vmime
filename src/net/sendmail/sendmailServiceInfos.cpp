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
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/net/sendmail/sendmailServiceInfos.hpp"


#if VMIME_BUILTIN_PLATFORM_POSIX


namespace vmime {
namespace net {
namespace sendmail {


sendmailServiceInfos::sendmailServiceInfos()
{
}


const string sendmailServiceInfos::getPropertyPrefix() const
{
	return "transport.sendmail.";
}


const sendmailServiceInfos::props& sendmailServiceInfos::getProperties() const
{
	static props sendmailProps =
	{
		// Path to sendmail (override default)
		property("binpath", serviceInfos::property::TYPE_STRING, string(VMIME_SENDMAIL_PATH))
	};

	return sendmailProps;
}


const std::vector <serviceInfos::property> sendmailServiceInfos::getAvailableProperties() const
{
	std::vector <property> list;
	const props& p = getProperties();

	list.push_back(p.PROPERTY_BINPATH);

	return list;
}


} // sendmail
} // net
} // vmime


#endif // VMIME_BUILTIN_PLATFORM_POSIX

