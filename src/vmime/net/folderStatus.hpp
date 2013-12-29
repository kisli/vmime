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

#ifndef VMIME_NET_FOLDERSTATUS_HPP_INCLUDED
#define VMIME_NET_FOLDERSTATUS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include "vmime/base.hpp"


namespace vmime {
namespace net {


/** Holds the status of a mail store folder.
  */

class VMIME_EXPORT folderStatus : public object
{
public:

	/** Returns the total number of messages in the folder.
	  *
	  * @return number of messages
	  */
	virtual unsigned int getMessageCount() const = 0;

	/** Returns the number of unseen messages in the folder.
	  *
	  * @return number of unseen messages
	  */
	virtual unsigned int getUnseenCount() const = 0;

	/** Clones this object.
	  *
	  * @return a copy of this object
	  */
	virtual shared_ptr <folderStatus> clone() const = 0;
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES

#endif // VMIME_NET_FOLDERSTATUS_HPP_INCLUDED
