//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2006 Vincent Richard <vincent@vincent-richard.net>
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

protected:

	object();
	object(const object&);

	virtual ~object();

#ifndef VMIME_BUILDING_DOC

	/** Add a strong reference to this object. A strong
	  * reference ensure the object remains alive.
	  */
	void addStrong() const;

	/** Add a weak reference to this object. A weak
	  * reference helps to resolve circular references.
	  */
	void addWeak(utility::weak_ref_base* w) const;

	/** Release a strong reference to this object.
	  *
	  * @return true if the object is not referenced anymore.
	  */
	void releaseStrong() const;

	/** Release a weak reference to this object.
	  *
	  * @return true if the object is not referenced anymore.
	  */
	void releaseWeak(utility::weak_ref_base* w) const;

	/** Return a reference to this object.
	  * \warning NEVER CALL THIS FROM A CONSTRUCTOR!
	  *
	  * @return reference to self
	  */
	ref <object> thisRef();

	/** Return a reference to this object (const version).
	  * \warning NEVER CALL THIS FROM A CONSTRUCTOR!
	  *
	  * @return reference to self
	  */
	ref <const object> thisRef() const;

	/** Return a weak reference to this object.
	  * \warning NEVER CALL THIS FROM A CONSTRUCTOR!
	  *
	  * @return weak reference to self
	  */
	weak_ref <object> thisWeakRef();

	/** Return a weak reference to this object (const version).
	  * \warning NEVER CALL THIS FROM A CONSTRUCTOR!
	  *
	  * @return weak reference to self
	  */
	weak_ref <const object> thisWeakRef() const;

	/** Return the number of strong refs to this object.
	  * For debugging purposes only.
	  *
	  * @return strong reference count
	  */
	const int getStrongRefCount() const;

	/** Return the number of weak refs to this object.
	  * For debugging purposes only.
	  *
	  * @return weak reference count
	  */
	const int getWeakRefCount() const;
#endif // VMIME_BUILDING_DOC

private:

	mutable int m_strongCount;
	mutable std::vector <utility::weak_ref_base*> m_weakRefs;
};


} // vmime


#endif // VMIME_OBJECT_HPP_INCLUDED

