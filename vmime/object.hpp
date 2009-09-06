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

#ifndef VMIME_OBJECT_HPP_INCLUDED
#define VMIME_OBJECT_HPP_INCLUDED


#include "vmime/types.hpp"


#include <vector>


namespace vmime
{


/** Base object for all objects in the library. This implements
  * reference counting and auto-deletion.
  */

class object
{
	template <class T> friend class utility::ref;
	template <class T> friend class utility::weak_ref;

	friend class utility::refManager;

protected:

	object();
	object(const object&);

	object& operator=(const object&);

	virtual ~object();

#ifndef VMIME_BUILDING_DOC

	/** Return a reference to this object.
	  *
	  * @return reference to self
	  */
	ref <object> thisRef();

	/** Return a reference to this object (const version).
	  *
	  * @return reference to self
	  */
	ref <const object> thisRef() const;

	/** Return a weak reference to this object.
	  *
	  * @return weak reference to self
	  */
	weak_ref <object> thisWeakRef();

	/** Return a weak reference to this object (const version).
	  *
	  * @return weak reference to self
	  */
	weak_ref <const object> thisWeakRef() const;


	void setRefManager(utility::refManager* mgr);
	utility::refManager* getRefManager() const;

#endif // VMIME_BUILDING_DOC

private:

	mutable utility::refManager* m_refMgr;
};


} // vmime


#endif // VMIME_OBJECT_HPP_INCLUDED

