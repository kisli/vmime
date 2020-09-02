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


#include "vmime/net/dsnAttributes.hpp"


namespace vmime {
namespace net {


dsnAttributes::dsnAttributes(const string& dsnNotify, const string& dsnRet, const string& dsnEnvelopId)
	: m_notifications(dsnNotify), m_returnFormat(dsnRet), m_envelopId(dsnEnvelopId) {

}


string dsnAttributes::getNotificationConditions() const {

	return m_notifications;
}


string dsnAttributes::getReturnFormat() const {

	return m_returnFormat;
}


string dsnAttributes::getEnvelopId() const {

	return m_envelopId;
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES
