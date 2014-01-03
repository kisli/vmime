//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2014 Vincent Richard <vincent@vmime.org>
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


#include "vmime/net/folderAttributes.hpp"

#include <algorithm>


namespace vmime {
namespace net {


folderAttributes::folderAttributes()
	: m_type(TYPE_CONTAINS_FOLDERS | TYPE_CONTAINS_MESSAGES),
	  m_flags(0),
	  m_specialUse(SPECIALUSE_NONE)
{
}


folderAttributes::folderAttributes(const folderAttributes& attribs)
	: m_type(attribs.m_type),
	  m_flags(attribs.m_flags),
	  m_userFlags(attribs.m_userFlags),
	  m_specialUse(attribs.m_specialUse)
{
}


int folderAttributes::getType() const
{
	return m_type;
}


void folderAttributes::setType(const int type)
{
	m_type = type;
}


int folderAttributes::getSpecialUse() const
{
	return m_specialUse;
}


void folderAttributes::setSpecialUse(const int use)
{
	m_specialUse = use;
}


int folderAttributes::getFlags() const
{
	return m_flags;
}


void folderAttributes::setFlags(const int flags)
{
	m_flags = flags;
}


bool folderAttributes::hasFlag(const int flag)
{
	return (m_flags & flag) != 0;
}


const std::vector <string> folderAttributes::getUserFlags() const
{
	return m_userFlags;
}


void folderAttributes::setUserFlags(const std::vector <string>& flags)
{
	m_userFlags = flags;
}


bool folderAttributes::hasUserFlag(const string& flag)
{
	return std::find(m_userFlags.begin(), m_userFlags.end(), flag) != m_userFlags.end();
}


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

