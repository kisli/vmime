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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "vmime/types.hpp"
#include "vmime/object.hpp"

#include <algorithm>  // std::find
#include <sstream>    // std::ostringstream
#include <stdexcept>  // std::runtime_error


namespace vmime
{


object::object()
	: m_strongCount(0)
{
}


object::object(const object&)
	: m_strongCount(0)
{
	// Not used
}


object::~object()
{
	for (std::vector <utility::weak_ref_base*>::iterator
	     it = m_weakRefs.begin() ; it != m_weakRefs.end() ; ++it)
	{
		(*it)->notifyObjectDestroyed();
	}

#if VMIME_DEBUG
	if (m_strongCount != 0)
	{
		std::ostringstream oss;
		oss << "ERROR: Deleting object and strong count != 0."
		    << " (" << __FILE__ << ", line " << __LINE__ << ")" << std::endl;

		throw std::runtime_error(oss.str());
	}
#endif // VMIME_DEBUG
}


void object::addStrong() const
{
	++m_strongCount;
}


void object::addWeak(utility::weak_ref_base* w) const
{
	m_weakRefs.push_back(w);
}


void object::releaseStrong() const
{
	if (--m_strongCount == 0)
		delete this;
}


void object::releaseWeak(utility::weak_ref_base* w) const
{
	std::vector <utility::weak_ref_base*>::iterator
		it = std::find(m_weakRefs.begin(), m_weakRefs.end(), w);

	if (it != m_weakRefs.end())
		m_weakRefs.erase(it);
#if VMIME_DEBUG
	else
	{
		std::ostringstream oss;
		oss << "ERROR: weak ref does not exist anymore!"
		    << " (" << __FILE__ << ", line " << __LINE__ << ")" << std::endl;

		throw std::runtime_error(oss.str());
	}
#endif // VMIME_DEBUG
}


ref <object> object::thisRef()
{
	return ref <object>::fromPtr(this);
}


ref <const object> object::thisRef() const
{
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


} // vmime

