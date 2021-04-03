//
// VMime library (http://www.vmime.org)
// Copyright (C) 2020 Jan Osusky <jan@osusky.name>
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


#include "vmime/net/smtp/DSNAttributes.hpp"


namespace vmime {
namespace net {
namespace smtp {


DSNAttributes::DSNAttributes(const string& dsnNotify, const string& dsnRet, const string& dsnEnvelopId)
	: m_notifications(dsnNotify), m_returnFormat(dsnRet), m_envelopId(dsnEnvelopId) {

}


string DSNAttributes::getNotificationConditions() const {

	return m_notifications;
}


string DSNAttributes::getReturnFormat() const {

	return m_returnFormat;
}


string DSNAttributes::getEnvelopId() const {

	return m_envelopId;
}


bool DSNAttributes::isEmpty() const {

	return m_notifications.empty() && m_returnFormat.empty() && m_envelopId.empty();
}


} // smtp
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
