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

#ifndef VMIME_NET_FOLDERATTRIBUTES_HPP_INCLUDED
#define VMIME_NET_FOLDERATTRIBUTES_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES


#include <vector>

#include "vmime/types.hpp"


namespace vmime {
namespace net {


/** Holds a set of attributes for a folder.
  */
class VMIME_EXPORT folderAttributes : public object
{
public:

	/** Folder types.
	  */
	enum Types
	{
		TYPE_CONTAINS_FOLDERS  = (1 << 0),   /**< Folder can contain folders. */
		TYPE_CONTAINS_MESSAGES = (1 << 1)    /**< Folder can contain messages. */
	};

	/** Folder flags.
	  */
	enum Flags
	{
		FLAG_HAS_CHILDREN = (1 << 0),        /**< Folder contains subfolders. */
		FLAG_NO_OPEN  = (1 << 1)             /**< Folder cannot be open. */
	};

	/** Folder special uses.
	  * Not all protocols support this. At the current time, only IMAP supports this,
	  * if the server has the SPECIAL-USE capability.
	  */
	enum SpecialUses
	{
		SPECIALUSE_NONE,         /**< User folder, no special use (or unknown). */
		SPECIALUSE_ALL,          /**< Virtual folder containing all messages. */
		SPECIALUSE_ARCHIVE,      /**< Folder is used to archives messages (server-dependent). */
		SPECIALUSE_DRAFTS,       /**< Folder is used to hold draft messages - typically, messages
		                              that are being composed but have not yet been sent. */
		SPECIALUSE_FLAGGED,      /**< Virtual folder containing all messages which are marked
		                              in some way as "important" or "flagged". */
		SPECIALUSE_JUNK,         /**< Folder is used to hold junk mail. */
		SPECIALUSE_SENT,         /**< Folder is is used to hold copies of messages that have
		                              been sent. */
		SPECIALUSE_TRASH,        /**< Folder is used to hold messages that have been deleted or
		                              marked for deletion (may be a virtual folder). */
		SPECIALUSE_IMPORTANT     /**< Folder contains messages that are likely important to the
		                              user. */
	};


	/** Construct a new folderAttributes object with the default set of attributes.
	  */
	folderAttributes();

	/** Construct a new folderAttributes object by copying an existing object.
	  *
	  * @param attribs object to copy
	  */
	folderAttributes(const folderAttributes& attribs);

	/** Return the type of the folder.
	  *
	  * @return combination of one ore more folder types (see folderAttributes::Types enum)
	  */
	int getType() const;

	/** Set the type of the folder.
	  *
	  * @param type combination of one ore more folder types (see folderAttributes::Types enum)
	  */
	void setType(const int type);

	/** Return the special use of the folder.
	  * Not all protocols support this. At the current time, only IMAP supports this,
	  * if the server has the SPECIAL-USE capability.
	  *
	  * @return a value which indicates a special use (see folderAttributes::SpecialUses enum)
	  */
	int getSpecialUse() const;

	/** Set the special use of the folder.
	  * Not all protocols support this. At the current time, only IMAP supports this,
	  * if the server has the SPECIAL-USE capability.
	  *
	  * @param use a value which indicates a special use (see folderAttributes::SpecialUses enum)
	  */
	void setSpecialUse(const int use);

	/** Return the standard (non-user) flags of the folder.
	  *
	  * @return combination of one ore more folder flags (see folderAttributes::Flags enum)
	  */
	int getFlags() const;

	/** Set the standard (non-user) flags of the folder.
	  *
	  * @param type combination of one ore more folder flags (see folderAttributes::Flags enum)
	  */
	void setFlags(const int flags);

	/** Return whether the specified folder flag(s) is/are set.
	  *
	  * @param flag combination of one ore more folder flags (see folderAttributes::Flags enum)
	  * @return true if the specified flags are all set, or false otherwise
	  */ 
	bool hasFlag(const int flag);

	/** Set the user-defined flags of the folder.
	  *
	  * @return a list of user-defined flags
	  */
	const std::vector <string> getUserFlags() const;

	/** Set the user-defined flags of the folder.
	  *
	  * @param flags a list of user-defined flags
	  */
	void setUserFlags(const std::vector <string>& flags);

	/** Return whether the specified user-defined flag is set.
	  *
	  * @return true if the specified flag is set, or false otherwise
	  */
	bool hasUserFlag(const string& flag);

private:

	int m_type;
	int m_flags;
	std::vector <string> m_userFlags;
	int m_specialUse;
};


} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES


#endif // VMIME_NET_FOLDERATTRIBUTES_HPP_INCLUDED
