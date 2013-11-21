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

#ifndef VMIME_NET_MAILDIR_FORMAT_COURIERMAILDIRFORMAT_HPP_INCLUDED
#define VMIME_NET_MAILDIR_FORMAT_COURIERMAILDIRFORMAT_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/net/maildir/maildirFormat.hpp"


namespace vmime {
namespace net {
namespace maildir {
namespace format {


/** Reads Courier/QMail Maildir format.
  */

class VMIME_EXPORT courierMaildirFormat : public maildirFormat
{
public:

	courierMaildirFormat(shared_ptr <context> ctx);


	/* Folder types:
	 *
	 * - ROOT_DIRECTORY:       ~/Mail/.MyFolder
	 * - NEW_DIRECTORY:        ~/Mail/.MyFolder/new
	 * - CUR_DIRECTORY:        ~/Mail/.MyFolder/cur
	 * - TMP_DIRECTORY:        ~/Mail/.MyFolder/tmp
	 * - CONTAINER_DIRECTORY:  not used
	 */

	const string getName() const;

	void createFolder(const folder::path& path);
	void destroyFolder(const folder::path& path);
	void renameFolder(const folder::path& oldPath, const folder::path& newPath);

	bool folderExists(const folder::path& path) const;
	bool folderHasSubfolders(const folder::path& path) const;

	const utility::file::path folderPathToFileSystemPath
		(const folder::path& path, const DirectoryType type) const;

	const std::vector <folder::path> listFolders
		(const folder::path& root, const bool recursive) const;

protected:

	bool supports() const;


	static const string toModifiedUTF7(const folder::path::component& text);
	static const folder::path::component fromModifiedUTF7(const string& text);

	void renameFolderImpl(const folder::path& oldPath, const folder::path& newPath);

	/** Test whether the specified file system directory corresponds to
	  * a maildir subfolder. The name of the directory should start
	  * with a '.' to be listed as a subfolder.
	  *
	  * @param file reference to a file system directory
	  * @return true if the specified directory is a maildir subfolder,
	  * false otherwise
	  */
	static bool isSubfolderDirectory(const utility::file& file);

	/** List directories corresponding to folders which are (direct or
	  * indirect) children of specified folder.
	  *
	  * @param root root folder
	  * @param dirs list in which found directories will be added
	  * @param onlyTestForExistence if true, the function returns as soon
	  * as the first directory is found
	  * @return true if at least one directory has been found,
	  * false otherwise
	  */
	bool listDirectories(const folder::path& root,
		std::vector <string>& dirs, const bool onlyTestForExistence) const;
};


} // format
} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

#endif // VMIME_NET_MAILDIR_FORMAT_COURIERMAILDIRFORMAT_HPP_INCLUDED

