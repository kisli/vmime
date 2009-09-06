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

#ifndef VMIME_NET_STORE_HPP_INCLUDED
#define VMIME_NET_STORE_HPP_INCLUDED


#include "vmime/net/service.hpp"
#include "vmime/net/folder.hpp"


namespace vmime {
namespace net {


/** A store service.
  * Encapsulate protocols that provide access to user's mail drop.
  */

class store : public service
{
protected:

	store(ref <session> sess, const serviceInfos& infos, ref <security::authenticator> auth)
		: service(sess, infos, auth) { }

public:

	/** Return the default folder. This is protocol dependent
	  * and usually is the INBOX folder.
	  *
	  * @return default folder
	  */
	virtual ref <folder> getDefaultFolder() = 0;

	/** Return the root folder. This is protocol dependent
	  * and usually is the user's mail drop root folder.
	  *
	  * @return root folder
	  */
	virtual ref <folder> getRootFolder() = 0;

	/** Return the folder specified by the path.
	  *
	  * @param path absolute folder path
	  * @return folder at the specified path
	  */
	virtual ref <folder> getFolder(const folder::path& path) = 0;

	/** Test whether the specified folder name is a syntactically
	  * a valid name.
	  *
	  * @return true if the specified folder name is valid, false otherwise
	  */
	virtual bool isValidFolderName(const folder::path::component& name) const = 0;

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
	virtual int getCapabilities() const = 0;


	Type getType() const { return (TYPE_STORE); }
};


} // net
} // vmime


#endif // VMIME_NET_STORE_HPP_INCLUDED
