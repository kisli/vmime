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

#include "vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR


#include "vmime/net/maildir/format/courierMaildirFormat.hpp"

#include "vmime/net/maildir/maildirStore.hpp"
#include "vmime/net/maildir/maildirUtils.hpp"

#include "vmime/platform.hpp"


namespace vmime {
namespace net {
namespace maildir {
namespace format {


courierMaildirFormat::courierMaildirFormat(shared_ptr <context> ctx)
	: maildirFormat(ctx)
{
}


const string courierMaildirFormat::getName() const
{
	return "courier";
}


void courierMaildirFormat::createFolder(const folder::path& path)
{
	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	if (!fsf->isValidPath(folderPathToFileSystemPath(path, ROOT_DIRECTORY)))
		throw exceptions::invalid_folder_name();

	shared_ptr <utility::file> rootDir = fsf->create
		(folderPathToFileSystemPath(path, ROOT_DIRECTORY));

	shared_ptr <utility::file> newDir = fsf->create
		(folderPathToFileSystemPath(path, NEW_DIRECTORY));
	shared_ptr <utility::file> tmpDir = fsf->create
		(folderPathToFileSystemPath(path, TMP_DIRECTORY));
	shared_ptr <utility::file> curDir = fsf->create
		(folderPathToFileSystemPath(path, CUR_DIRECTORY));

	rootDir->createDirectory(true);

	newDir->createDirectory(false);
	tmpDir->createDirectory(false);
	curDir->createDirectory(false);

	shared_ptr <utility::file> maildirFile = fsf->create
		(folderPathToFileSystemPath(path, ROOT_DIRECTORY)
		 	/ utility::file::path::component("maildirfolder"));

	maildirFile->createFile();
}


void courierMaildirFormat::destroyFolder(const folder::path& path)
{
	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	// Recursively delete directories of subfolders
	const std::vector <folder::path> folders = listFolders(path, true);

	for (std::vector <folder::path>::size_type i = 0, n = folders.size() ; i < n ; ++i)
	{
		maildirUtils::recursiveFSDelete(fsf->create
			(folderPathToFileSystemPath(folders[i], ROOT_DIRECTORY)));
	}

	// Recursively delete the directory of this folder
	maildirUtils::recursiveFSDelete(fsf->create
		(folderPathToFileSystemPath(path, ROOT_DIRECTORY)));
}


void courierMaildirFormat::renameFolder
	(const folder::path& oldPath, const folder::path& newPath)
{
	const std::vector <folder::path> folders = listFolders(oldPath, true);

	for (std::vector <folder::path>::size_type i = 0, n = folders.size() ; i < n ; ++i)
	{
		const folder::path folderOldPath = folders[i];

		folder::path folderNewPath = folderOldPath;
		folderNewPath.renameParent(oldPath, newPath);

		renameFolderImpl(folderOldPath, folderNewPath);
	}

	renameFolderImpl(oldPath, newPath);
}


void courierMaildirFormat::renameFolderImpl
	(const folder::path& oldPath, const folder::path& newPath)
{
	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	const utility::file::path oldFSPath =
		folderPathToFileSystemPath(oldPath, ROOT_DIRECTORY);

	const utility::file::path newFSPath =
		folderPathToFileSystemPath(newPath, ROOT_DIRECTORY);

	shared_ptr <utility::file> rootDir = fsf->create(oldFSPath);
	rootDir->rename(newFSPath);
}


bool courierMaildirFormat::folderExists(const folder::path& path) const
{
	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	shared_ptr <utility::file> rootDir = fsf->create
		(folderPathToFileSystemPath(path, ROOT_DIRECTORY));

	shared_ptr <utility::file> newDir = fsf->create
		(folderPathToFileSystemPath(path, NEW_DIRECTORY));
	shared_ptr <utility::file> tmpDir = fsf->create
		(folderPathToFileSystemPath(path, TMP_DIRECTORY));
	shared_ptr <utility::file> curDir = fsf->create
		(folderPathToFileSystemPath(path, CUR_DIRECTORY));

	shared_ptr <utility::file> maildirFile = fsf->create
		(folderPathToFileSystemPath(path, ROOT_DIRECTORY)
		 	/ utility::file::path::component("maildirfolder"));

	bool exists = rootDir->exists() && rootDir->isDirectory() &&
	       newDir->exists() && newDir->isDirectory() &&
	       tmpDir->exists() && tmpDir->isDirectory() &&
	       curDir->exists() && curDir->isDirectory();

	// If this is not the root folder, then a file named "maildirfolder"
	// must also be present in the directory
	if (!path.isRoot())
		exists = exists && maildirFile->exists() && maildirFile->isFile();

	return exists;
}


bool courierMaildirFormat::folderHasSubfolders(const folder::path& path) const
{
	std::vector <string> dirs;
	return listDirectories(path, dirs, true);
}


const utility::file::path courierMaildirFormat::folderPathToFileSystemPath
	(const folder::path& path, const DirectoryType type) const
{
	// Virtual folder "/MyFolder/SubFolder" corresponds to physical
	// directory "[store root]/.MyFolder.SubFolder"
	utility::file::path fsPath = getContext()->getStore()->getFileSystemPath();

	if (!path.isRoot())
	{
		string folderComp;

		for (size_t i = 0, n = path.getSize() ; i < n ; ++i)
			folderComp += "." + toModifiedUTF7(path[i]);

		fsPath /= utility::file::path::component(folderComp);
	}

	// Last component
	switch (type)
	{
	case ROOT_DIRECTORY:

		// Nothing to add
		break;

	case NEW_DIRECTORY:

		fsPath /= NEW_DIR;
		break;

	case CUR_DIRECTORY:

		fsPath /= CUR_DIR;
		break;

	case TMP_DIRECTORY:

		fsPath /= TMP_DIR;
		break;

	case CONTAINER_DIRECTORY:

		// Not used
		break;
	}

	return fsPath;
}


const std::vector <folder::path> courierMaildirFormat::listFolders
	(const folder::path& root, const bool recursive) const
{
	// First, list directories
	std::vector <string> dirs;
	listDirectories(root, dirs, false);

	// Then, map directories to folders
	std::vector <folder::path> folders;

	for (std::vector <string>::size_type i = 0, n = dirs.size() ; i < n ; ++i)
	{
		const string dir = dirs[i].substr(1) + ".";
		folder::path path;

		for (size_t pos = dir.find("."), prev = 0 ;
		     pos != string::npos ; prev = pos + 1, pos = dir.find(".", pos + 1))
		{
			const string comp = dir.substr(prev, pos - prev);
			path /= fromModifiedUTF7(comp);
		}

		if (recursive || path.getSize() == root.getSize() + 1)
			folders.push_back(path);
	}

	return folders;
}


bool courierMaildirFormat::listDirectories(const folder::path& root,
	std::vector <string>& dirs, const bool onlyTestForExistence) const
{
	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	shared_ptr <utility::file> rootDir = fsf->create
		(getContext()->getStore()->getFileSystemPath());

	if (rootDir->exists())
	{
		// To speed up things, and if we are not searching in root folder,
		// search for directories with a common prefix
		string base;

		if (!root.isRoot())
		{
			for (size_t i = 0, n = root.getSize() ; i < n ; ++i)
				base += "." + toModifiedUTF7(root[i]);
		}

		// Enumerate directories
		shared_ptr <utility::fileIterator> it = rootDir->getFiles();

		while (it->hasMoreElements())
		{
			shared_ptr <utility::file> file = it->nextElement();

			if (isSubfolderDirectory(*file))
			{
				const string dir = file->getFullPath().getLastComponent().getBuffer();

				if (base.empty() || (dir.length() > base.length() && dir.substr(0, base.length()) == base))
				{
					dirs.push_back(dir);

					if (onlyTestForExistence)
						return true;
				}
			}
		}
	}
	else
	{
		// No sub-folder
	}

	std::sort(dirs.begin(), dirs.end());

	return !dirs.empty();
}


// static
bool courierMaildirFormat::isSubfolderDirectory(const utility::file& file)
{
	// A directory which names starts with '.' may be a subfolder
	if (file.isDirectory() &&
	    file.getFullPath().getLastComponent().getBuffer().length() >= 1 &&
	    file.getFullPath().getLastComponent().getBuffer()[0] == '.')
	{
		return true;
	}

	return false;
}


// static
const string courierMaildirFormat::toModifiedUTF7(const folder::path::component& text)
{
	// From http://www.courier-mta.org/?maildir.html:
	//
	// Folder names can contain any Unicode character, except for control
	// characters. US-ASCII characters, U+0x0020 - U+0x007F, except for the
	// period, forward-slash, and ampersand characters (U+0x002E, U+0x002F,
	// and U+0x0026) represent themselves. The ampersand is represented by
	// the two character sequence "&-". The period, forward slash, and non
	// US-ASCII Unicode characters are represented using the UTF-7 character
	// set, and encoded with a modified form of base64-encoding.
	//
	// The "&" character starts the modified base64-encoded sequence; the
	// sequence is terminated by the "-" character. The sequence of 16-bit
	// Unicode characters is written in big-endian order, and encoded using
	// the base64-encoding method described in section 5.2 of RFC 1521, with
	// the following modifications:
	//
	// * The "=" padding character is omitted. When decoding, an incomplete
	//   16-bit character is discarded.
	//
	// * The comma character, "," is used in place of the "/" character in
	//   the base64 alphabet.
	//
	// For example, the word "Resume" with both "e"s being the e-acute
	// character, U+0x00e9, is encoded as "R&AOk-sum&AOk-" (so a folder of
	// that name would be a maildir subdirectory called ".R&AOk-sum&AOk-").
	//

	// Transcode path component to UTF-7 charset.
	// WARNING: This may throw "exceptions::charset_conv_error"
	const string cvt = text.getConvertedText(charset(charsets::UTF_7));

	// Transcode to modified UTF-7 (RFC-2060).
	string out;
	out.reserve((cvt.length() * 3) / 2);

	bool inB64sequence = false;

	for (string::const_iterator it = cvt.begin() ; it != cvt.end() ; ++it)
	{
		const unsigned char c = *it;

		switch (c)
		{
		// Beginning of Base64 sequence: replace '+' with '&'
		case '+':
		{
			if (!inB64sequence)
			{
				inB64sequence = true;
				out += '&';
			}
			else
			{
				out += '+';
			}

			break;
		}
		// End of Base64 sequence
		case '-':
		{
			inB64sequence = false;
			out += '-';
			break;
		}
		// ',' is used instead of '/' in modified Base64,
		// and simply UTF7-encoded out of a Base64 sequence
		case '/':
		{
			if (inB64sequence)
				out += ',';
			else
				out += "&Lw-";

			break;
		}
		// Encode period (should not happen in a Base64 sequence)
		case '.':
		{
			out += "&Lg-";
			break;
		}
		// '&' (0x26) is represented by the two-octet sequence "&-"
		case '&':
		{
			if (!inB64sequence)
				out += "&-";
			else
				out += '&';

			break;
		}
		default:
		{
			out += c;
			break;
		}

		}
	}

	return out;
}


// static
const folder::path::component courierMaildirFormat::fromModifiedUTF7(const string& text)
{
	// Transcode from modified UTF-7
	string out;
	out.reserve(text.length());

	bool inB64sequence = false;
	unsigned char prev = 0;

	for (string::const_iterator it = text.begin() ; it != text.end() ; ++it)
	{
		const unsigned char c = *it;

		switch (c)
		{
		// Start of Base64 sequence
		case '&':
		{
			if (!inB64sequence)
			{
				inB64sequence = true;
				out += '+';
			}
			else
			{
				out += '&';
			}

			break;
		}
		// End of Base64 sequence (or "&-" --> "&")
		case '-':
		{
			if (inB64sequence && prev == '&')
				out += '&';
			else
				out += '-';

			inB64sequence = false;
			break;
		}
		// ',' is used instead of '/' in modified Base64
		case ',':
		{
			out += (inB64sequence ? '/' : ',');
			break;
		}
		default:
		{
			out += c;
			break;
		}

		}

		prev = c;
	}

	// Store it as UTF-8 by default
	string cvt;
	charset::convert(out, cvt,
		charset(charsets::UTF_7), charset(charsets::UTF_8));

	return (folder::path::component(cvt, charset(charsets::UTF_8)));
}


bool courierMaildirFormat::supports() const
{
	shared_ptr <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	shared_ptr <utility::file> rootDir = fsf->create
		(getContext()->getStore()->getFileSystemPath());

	if (rootDir->exists())
	{
		// Try to find a file named "maildirfolder", which indicates
		// the Maildir is in Courier format
		shared_ptr <utility::fileIterator> it = rootDir->getFiles();

		while (it->hasMoreElements())
		{
			shared_ptr <utility::file> file = it->nextElement();

			if (isSubfolderDirectory(*file))
			{
				shared_ptr <utility::file> folderFile = fsf->create
					(file->getFullPath() / utility::file::path::component("maildirfolder"));

				if (folderFile->exists() && folderFile->isFile())
					return true;
			}
		}
	}

	return false;
}


} // format
} // maildir
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_MAILDIR

