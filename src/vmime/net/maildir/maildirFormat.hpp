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

#ifndef VMIME_NET_MAILDIR_FORMAT_MAILDIRFORMAT_HPP_INCLUDED
#define VMIME_NET_MAILDIR_FORMAT_MAILDIRFORMAT_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/net/folder.hpp"

#include "vmime/utility/file.hpp"
#include "vmime/utility/path.hpp"


namespace vmime {
namespace net {
namespace maildir {


class maildirStore;


/** Interface for an object capable of reading a specific Maildir format. */

class VMIME_EXPORT maildirFormat : public object
{
public:

	class context : public object
	{
	public:

		context(shared_ptr <maildirStore> store);

		shared_ptr <maildirStore> getStore() const;

	private:

		weak_ptr <maildirStore> m_store;
	};


	/** Physical directory types. */
	enum DirectoryType
	{
		ROOT_DIRECTORY,       /**< Root directory. */
		NEW_DIRECTORY,        /**< Directory containing unread messages. */
		CUR_DIRECTORY,        /**< Directory containing messages that have been seen. */
		TMP_DIRECTORY,        /**< Temporary directory used for reliable delivery. */
		CONTAINER_DIRECTORY   /**< Container for subfolders. */
	};

	/** Return the name of this Maildir format.
	  *
	  * @return format name
	  */
	virtual const string getName() const = 0;

	/** Create the specified folder.
	  *
	  * @param path virtual path of the folder
	  * @throw exceptions::filesystem_exception, invalid_folder_name
	  */
	virtual void createFolder(const folder::path& path) = 0;

	/** Destroy the specified folder.
	  *
	  * @param path virtual path of the folder
	  * @throw exceptions::filesystem_exception
	  */
	virtual void destroyFolder(const folder::path& path) = 0;

	/** Rename the specified folder.
	  *
	  * @param oldPath old virtual path of the folder
	  * @param newPath new virtual path of the folder
	  * @throw exceptions::filesystem_exception
	  */
	virtual void renameFolder(const folder::path& oldPath, const folder::path& newPath) = 0;

	/** Test whether the specified folder exists.
	  *
	  * @param path virtual path of the folder
	  * @return true if the folder exists, false otherwise
	  */
	virtual bool folderExists(const folder::path& path) const = 0;

	/** Test whether the specified folder has subfolders.
	  *
	  * @param path virtual path of the folder
	  * @return true if the folder has at least one subfolder,
	  * false otherwise
	  */
	virtual bool folderHasSubfolders(const folder::path& path) const = 0;

	/** Returns the directory which represents the specified
	  * folder on the file system.
	  *
	  * @param path virtual path of the folder
	  * @param type type of directory to return
	  * @return corresponding directory on the file system
	  */
	virtual const utility::file::path folderPathToFileSystemPath
		(const folder::path& path, const DirectoryType type) const = 0;

	/** List subfolders in the specified folder.
	  *
	  * @param root root folder in which to start the search
	  * @param recursive if set to true, all the descendant are
	  * returned; if set to false, only direct children are returned.
	  * @return list of subfolders
	  */
	virtual const std::vector <folder::path> listFolders
		(const folder::path& root, const bool recursive) const = 0;


	/** Try to detect the format of the specified Maildir store.
	  * If the format cannot be detected, a compatible implementation
	  * will be returned.
	  *
	  * @param store of which to detect format
	  * @return a Maildir format implementation for the specified store
	  */
	static shared_ptr <maildirFormat> detect(shared_ptr <maildirStore> store);

protected:

	static const utility::file::path::component TMP_DIR;  /**< Ensure reliable delivery (not to be listed). */
	static const utility::file::path::component CUR_DIR;  /**< No longer new messages. */
	static const utility::file::path::component NEW_DIR;  /**< Unread messages. */


	maildirFormat(shared_ptr <context> ctx);


	/** Returns the current context.
	  *
	  * @return current context
	  */
	shared_ptr <context> getContext();

	/** Returns the current context (const version).
	  *
	  * @return current context
	  */
	shared_ptr <const context> getContext() const;

	/** Quick checks whether this implementation can read the Maildir
	  * format in the specified directory.
	  *
	  * @return true if the implementation supports the specified
	  * Maildir, or false otherwise
	  */
	virtual bool supports() const = 0;

private:

	shared_ptr <context> m_context;
};


} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

#endif // VMIME_NET_MAILDIR_FORMAT_MAILDIRFORMAT_HPP_INCLUDED

