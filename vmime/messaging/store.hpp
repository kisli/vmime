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

#ifndef VMIME_MESSAGING_STORE_HPP_INCLUDED
#define VMIME_MESSAGING_STORE_HPP_INCLUDED


#include "vmime/messaging/service.hpp"
#include "vmime/messaging/folder.hpp"


namespace vmime {
namespace messaging {


/** A store service.
  * Encapsulate protocols that provide access to user's mail drop.
  */

class store : public service
{
protected:

	store(session* sess, const serviceInfos& infos, authenticator* auth)
		: service(sess, infos, auth) { }

public:

	/** Return the default folder. This is protocol dependant
	  * and usually is the INBOX folder.
	  *
	  * @return default folder
	  */
	virtual folder* getDefaultFolder() = 0;

	/** Return the root folder. This is protocol dependant
	  * and usually is the user's mail drop root folder.
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

	/** Test whether the specified folder name is a syntactically
	  * a valid name.
	  *
	  * @return true if the specified folder name is valid, false otherwise
	  */
	virtual const bool isValidFolderName(const folder::path::component& name) const = 0;

	/** Store capabilities. */
	enum Capabilities
	{
		CAPABILITY_CREATE_FOLDER = (1 << 0),    /**< Can create folders. */
		CAPABILITY_RENAME_FOLDER = (1 << 1),    /**< Can rename folders. */
		CAPABILITY_ADD_MESSAGE = (1 << 2),      /**< Can append message to folders. */
		CAPABILITY_COPY_MESSAGE = (1 << 3),     /**< Can copy messages from a folder to another one. */
		CAPABILITY_DELETE_MESSAGE = (1 << 4),   /**< Can delete messages. */
		CAPABILITY_PARTIAL_FETCH = (1 << 5),    /**< Is partial fetch supported? */
		CAPABILITY_MESSAGE_FLAGS = (1 << 6),    /**< Can set flags on messages. */
		CAPABILITY_EXTRACT_PART = (1 << 7)      /**< Can extract a specific part of the message. */
	};

	/** Return the features supported by this service. This is
	  * a combination of store::CAPABILITY_xxx flags.
	  *
	  * @return features supported by this service
	  */
	virtual const int getCapabilities() const = 0;


	const Type getType() const { return (TYPE_STORE); }
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_STORE_HPP_INCLUDED
