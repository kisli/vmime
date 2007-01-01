//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2007 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/net/imap/IMAPSStore.hpp"


namespace vmime {
namespace net {
namespace imap {


IMAPSStore::IMAPSStore(ref <session> sess, ref <security::authenticator> auth)
	: IMAPStore(sess, auth, true)
{
}


IMAPSStore::~IMAPSStore()
{
}


const string IMAPSStore::getProtocolName() const
{
	return "imaps";
}



// Service infos

IMAPServiceInfos IMAPSStore::sm_infos(true);


const serviceInfos& IMAPSStore::getInfosInstance()
{
	return sm_infos;
}


const serviceInfos& IMAPSStore::getInfos() const
{
	return sm_infos;
}


} // imap
} // net
} // vmime
