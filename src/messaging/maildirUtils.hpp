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

#ifndef VMIME_MESSAGING_MAILDIRUTILS_HPP_INCLUDED
#define VMIME_MESSAGING_MAILDIRUTILS_HPP_INCLUDED


#include "../utility/file.hpp"
#include "../utility/path.hpp"


namespace vmime {
namespace messaging {


class maildirStore;


class maildirUtils
{
public:

	/** Mode for return value of getFolderFSPath(). */
	enum FolderFSPathMode
	{
		FOLDER_PATH_ROOT,       /**< Root folder (eg. ~/Mail/MyFolder) */
		FOLDER_PATH_NEW,        /**< Folder containing unread messages (eg. ~/Mail/MyFolder/new) */
		FOLDER_PATH_CUR,        /**< Folder containing messages that have been seen (eg. ~/Mail/MyFolder/cur) */
		FOLDER_PATH_TMP,        /**< Temporary folder used for reliable delivery (eg. ~/Mail/MyFolder/tmp) */
		FOLDER_PATH_CONTAINER   /**< Container for sub-folders (eg. ~/Mail/.MyFolder.directory) */
	};

	/** Return the path on the filesystem for the folder in specified store.
	  *
	  * @param store parent store
	  * @param folderPath path of the folder
	  * @param mode type of path to return (see FolderFSPathMode)
	  * @return filesystem path for the specified folder
	  */
	static const utility::file::path getFolderFSPath(maildirStore* store, const utility::path& folderPath, const FolderFSPathMode mode);

	static const bool isSubfolderDirectory(const utility::file& file);

private:

	static const vmime::word TMP_DIR;
	static const vmime::word CUR_DIR;
	static const vmime::word NEW_DIR;
};


} // messaging
} // vmime


#endif // VMIME_MESSAGING_MAILDIRUTILS_HPP_INCLUDED
