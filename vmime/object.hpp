//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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


namespace vmime
{


/** Base object for all objects in the library.
  */

class VMIME_EXPORT object : public enable_shared_from_this <object>
{
protected:

	object();
	object(const object&);

	object& operator=(const object&);

	virtual ~object();
};


} // vmime


#endif // VMIME_OBJECT_HPP_INCLUDED

