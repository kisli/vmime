//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_MESSAGING_STORE_HPP_INCLUDED
#define VMIME_MESSAGING_STORE_HPP_INCLUDED


#include "service.hpp"
#include "folder.hpp"


namespace vmime {
namespace messaging {


/** A store service.
  * Encapsulate protocols that provide access to user's mail drop.
  */

class store : public service
{
protected:

	store(class session& sess, const serviceInfos& infos, class authenticator* auth)
		: service(sess, infos, auth) { }

public:

	/** Return the default folder. This is protocol dependant
	  * and usually is the INBOX folder.
	  *
	  * @return default folder
	  */
	virtual folder* getDefaultFolder() = 0;

	/** Return the root folder. This is protocol dependant
	  * and usually is the user's mail drop root folder
	  *
	  * @return root folder
	  */
	virtual folder* getRootFolder() = 0;

	/** Return the folder specified by the path.
	  *
	  * @param path absolute folder path
	  * @return folder at the specified path
	  */
	virtual folder* getFolder(const folder::path& path) = 0;


	const Type type() const { return (TYPE_STORE); }
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_STORE_HPP_INCLUDED
