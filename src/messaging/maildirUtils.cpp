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

#include "maildirUtils.hpp"
#include "maildirStore.hpp"


namespace vmime {
namespace messaging {


const vmime::word maildirUtils::TMP_DIR("tmp", vmime::charset(vmime::charsets::US_ASCII));  // ensure reliable delivery (not to be listed)
const vmime::word maildirUtils::CUR_DIR("cur", vmime::charset(vmime::charsets::US_ASCII));  // no longer new messages
const vmime::word maildirUtils::NEW_DIR("new", vmime::charset(vmime::charsets::US_ASCII));  // unread messages


const utility::file::path maildirUtils::getFolderFSPath
	(maildirStore* store, const utility::path& folderPath, const FolderFSPathMode mode)
{
	// Root path
	utility::file::path path(store->getFileSystemPath());

	const int count = (mode == FOLDER_PATH_CONTAINER
		? folderPath.getSize() : folderPath.getSize() - 1);

	// Parent folders
	for (int i = 0 ; i < count ; ++i)
	{
		utility::file::path::component comp(folderPath[i]);

		// TODO: may not work with all encodings...
		comp.setBuffer("." + comp.getBuffer() + ".directory");

		path /= comp;
	}

	// Last component
	if (folderPath.getSize() != 0 &&
	    mode != FOLDER_PATH_CONTAINER)
	{
		path /= folderPath.getLastComponent();

		switch (mode)
		{
		case FOLDER_PATH_ROOT: break; // Nothing to do
		case FOLDER_PATH_NEW: path /= NEW_DIR; break;
		case FOLDER_PATH_CUR: path /= CUR_DIR; break;
		case FOLDER_PATH_TMP: path /= TMP_DIR; break;
		case FOLDER_PATH_CONTAINER: break; // Can't happen...
		}
	}

	return (path);
}


const bool maildirUtils::isSubfolderDirectory(const utility::file& file)
{
	// A directory which name does not start with '.'
	// is listed as a sub-folder...
	if (file.isDirectory() &&
	    file.fullPath().getLastComponent().getBuffer().length() >= 1 &&
	    file.fullPath().getLastComponent().getBuffer()[0] != '.')
	{
		return (true);
	}

	return (false);
}


/*
const int maildirUtils::extractFlags(const utility::file::path::component& comp)
{
	string::size_type sep = comp.buffer().rfind(':');
	if (sep == string::npos) return (0);

	const string flagsString(comp.buffer().begin() + sep + 1, comp.buffer().end());
	const string::size_type count = flagsString.length();

	int flags = 0;

	for (string::size_type i = 0 ; i < count ; ++i)
	{
		switch (flagsString[i])
		{
		case 'S': case 's': flags |= message::FLAG_SEEN; break;
		case 'R': case 'r': flags |= message::FLAG_REPLIED; break;
		}
	}

	return (flags);
}


const utility::file::component maildirUtils::changeFlags
	(const utility::file::component& comp, const int flags)
{
}
*/


} // messaging
} // vmime
