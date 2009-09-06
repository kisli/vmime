//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2009 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/net/maildir/maildirServiceInfos.hpp"


namespace vmime {
namespace net {
namespace maildir {


maildirServiceInfos::maildirServiceInfos()
{
}


const string maildirServiceInfos::getPropertyPrefix() const
{
	return "store.maildir.";
}


const maildirServiceInfos::props& maildirServiceInfos::getProperties() const
{
	static props maildirProps =
	{
		property(serviceInfos::property::SERVER_ROOTPATH, serviceInfos::property::FLAG_REQUIRED)
	};

	return maildirProps;
}


const std::vector <serviceInfos::property> maildirServiceInfos::getAvailableProperties() const
{
	std::vector <property> list;
	const props& p = getProperties();

	list.push_back(p.PROPERTY_SERVER_ROOTPATH);

	return list;
}


} // maildir
} // net
} // vmime

