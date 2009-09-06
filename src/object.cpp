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

#include "vmime/types.hpp"
#include "vmime/object.hpp"


#ifndef VMIME_BUILDING_DOC


namespace vmime
{


object::object()
	: m_refMgr(utility::refManager::create(this))
{
}


object::object(const object&)
	: m_refMgr(utility::refManager::create(this))
{
}


object& object::operator=(const object&)
{
	// Do _NOT_ copy 'm_refMgr'
	return *this;
}


object::~object()
{
	delete m_refMgr;
	m_refMgr = 0;
}


ref <object> object::thisRef()
{
	m_refMgr->addStrong();
	return ref <object>::fromPtr(this);
}


ref <const object> object::thisRef() const
{
	m_refMgr->addStrong();
	return ref <const object>::fromPtr(this);
}


weak_ref <object> object::thisWeakRef()
{
	return weak_ref <object>(thisRef());
}


weak_ref <const object> object::thisWeakRef() const
{
	return weak_ref <const object>(thisRef());
}


void object::setRefManager(utility::refManager* mgr)
{
	m_refMgr = mgr;
}


utility::refManager* object::getRefManager() const
{
	return m_refMgr;
}


} // vmime


#endif // VMIME_BUILDING_DOC

