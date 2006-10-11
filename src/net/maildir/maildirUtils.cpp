//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2006 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along along
// with this program; if not, write to the Free Software Foundation, Inc., Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA..
//

#include "vmime/net/maildir/maildirUtils.hpp"
#include "vmime/net/maildir/maildirStore.hpp"

#include "vmime/utility/random.hpp"

#include "vmime/exception.hpp"


namespace vmime {
namespace net {
namespace maildir {


const vmime::word maildirUtils::TMP_DIR("tmp", vmime::charset(vmime::charsets::US_ASCII));  // ensure reliable delivery (not to be listed)
const vmime::word maildirUtils::CUR_DIR("cur", vmime::charset(vmime::charsets::US_ASCII));  // no longer new messages
const vmime::word maildirUtils::NEW_DIR("new", vmime::charset(vmime::charsets::US_ASCII));  // unread messages


const utility::file::path maildirUtils::getFolderFSPath
	(ref <const maildirStore> store, const utility::path& folderPath,
	 const FolderFSPathMode mode)
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
	    file.getFullPath().getLastComponent().getBuffer().length() >= 1 &&
	    file.getFullPath().getLastComponent().getBuffer()[0] != '.')
	{
		return (true);
	}

	return (false);
}


const bool maildirUtils::isMessageFile(const utility::file& file)
{
	// Ignore files which name begins with '.'
	if (file.isFile() &&
	    file.getFullPath().getLastComponent().getBuffer().length() >= 1 &&
	    file.getFullPath().getLastComponent().getBuffer()[0] != '.')
	{
		return (true);
	}

	return (false);
}


// NOTE ABOUT ID/FLAGS SEPARATOR
// -----------------------------
// In the maildir specification, the character ':' is used to separate
// the unique identifier and the message flags.
//
// On Windows (and particularly FAT file systems), ':' is not allowed
// in a filename, so we use a dash ('-') instead. This is the solution
// used by Mutt/Win32, so we also use it here.
//
// To be compatible between implementations, we check for both
// characters when reading file names.


const utility::file::path::component maildirUtils::extractId
	(const utility::file::path::component& filename)
{
	string::size_type sep = filename.getBuffer().rfind(':');  // try colon

	if (sep == string::npos)
	{
		sep = filename.getBuffer().rfind('-');  // try dash (Windows)
		if (sep == string::npos) return (filename);
	}

	return (utility::path::component
		(string(filename.getBuffer().begin(), filename.getBuffer().begin() + sep)));
}


const int maildirUtils::extractFlags(const utility::file::path::component& comp)
{
	string::size_type sep = comp.getBuffer().rfind(':');  // try colon

	if (sep == string::npos)
	{
		sep = comp.getBuffer().rfind('-');  // try dash (Windows)
		if (sep == string::npos) return 0;
	}

	const string flagsString(comp.getBuffer().begin() + sep + 1, comp.getBuffer().end());
	const string::size_type count = flagsString.length();

	int flags = 0;

	for (string::size_type i = 0 ; i < count ; ++i)
	{
		switch (flagsString[i])
		{
		case 'R': case 'r': flags |= message::FLAG_REPLIED; break;
		case 'S': case 's': flags |= message::FLAG_SEEN; break;
		case 'T': case 't': flags |= message::FLAG_DELETED; break;
		case 'F': case 'f': flags |= message::FLAG_MARKED; break;
		case 'P': case 'p': flags |= message::FLAG_PASSED; break;
		}
	}

	return (flags);
}


const utility::file::path::component maildirUtils::buildFlags(const int flags)
{
	string str;
	str.reserve(8);

	str += "2,";

	if (flags & message::FLAG_MARKED)  str += "F";
	if (flags & message::FLAG_PASSED)  str += "P";
	if (flags & message::FLAG_REPLIED) str += "R";
	if (flags & message::FLAG_SEEN)    str += "S";
	if (flags & message::FLAG_DELETED) str += "T";

	return (utility::file::path::component(str));
}


const utility::file::path::component maildirUtils::buildFilename
	(const utility::file::path::component& id, const int flags)
{
	return (buildFilename(id, buildFlags(flags)));
}


const utility::file::path::component maildirUtils::buildFilename
	(const utility::file::path::component& id, const utility::file::path::component& flags)
{
#if VMIME_BUILTIN_PLATFORM_WINDOWS
	return (utility::path::component(id.getBuffer() + "-" + flags.getBuffer()));  // use dash
#else
	return (utility::path::component(id.getBuffer() + ":" + flags.getBuffer()));  // use colon
#endif
}


const utility::file::path::component maildirUtils::generateId()
{
	std::ostringstream oss;

	oss << utility::random::getTime();
	oss << ".";
	oss << utility::random::getProcess();
	oss << ".";
	oss << utility::random::getString(6);

	return (utility::file::path::component(oss.str()));
}


void maildirUtils::recursiveFSDelete(ref <utility::file> dir)
{
	ref <utility::fileIterator> files = dir->getFiles();

	// First, delete files and subdirectories in this directory
	while (files->hasMoreElements())
	{
		ref <utility::file> file = files->nextElement();

		if (file->isDirectory())
		{
			maildirUtils::recursiveFSDelete(file);
		}
		else
		{
			try
			{
				file->remove();
			}
			catch (exceptions::filesystem_exception&)
			{
				// Ignore
			}
		}
	}

	// Then, delete this (empty) directory
	try
	{
		dir->remove();
	}
	catch (exceptions::filesystem_exception&)
	{
		// Ignore
	}
}



//
// messageIdComparator
//

maildirUtils::messageIdComparator::messageIdComparator
	(const utility::file::path::component& comp)
	: m_comp(maildirUtils::extractId(comp))
{
}


const bool maildirUtils::messageIdComparator::operator()
	(const utility::file::path::component& other) const
{
	return (m_comp == maildirUtils::extractId(other));
}


} // maildir
} // net
} // vmime
