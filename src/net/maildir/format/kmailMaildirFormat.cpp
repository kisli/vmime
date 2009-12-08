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

#include "vmime/net/maildir/format/kmailMaildirFormat.hpp"

#include "vmime/net/maildir/maildirStore.hpp"
#include "vmime/net/maildir/maildirUtils.hpp"

#include "vmime/platform.hpp"


namespace vmime {
namespace net {
namespace maildir {
namespace format {


kmailMaildirFormat::kmailMaildirFormat(ref <context> ctx)
	: maildirFormat(ctx)
{
}


const string kmailMaildirFormat::getName() const
{
	return "kmail";
}


void kmailMaildirFormat::createFolder(const folder::path& path)
{
	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	if (!fsf->isValidPath(folderPathToFileSystemPath(path, ROOT_DIRECTORY)))
		throw exceptions::invalid_folder_name();

	ref <utility::file> rootDir = fsf->create
		(folderPathToFileSystemPath(path, ROOT_DIRECTORY));

	ref <utility::file> newDir = fsf->create
		(folderPathToFileSystemPath(path, NEW_DIRECTORY));
	ref <utility::file> tmpDir = fsf->create
		(folderPathToFileSystemPath(path, TMP_DIRECTORY));
	ref <utility::file> curDir = fsf->create
		(folderPathToFileSystemPath(path, CUR_DIRECTORY));

	rootDir->createDirectory(true);

	newDir->createDirectory(false);
	tmpDir->createDirectory(false);
	curDir->createDirectory(false);
}


void kmailMaildirFormat::destroyFolder(const folder::path& path)
{
	// Delete 'folder' and '.folder.directory' directories
	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	maildirUtils::recursiveFSDelete(fsf->create
		(folderPathToFileSystemPath(path, ROOT_DIRECTORY)));  // root

	maildirUtils::recursiveFSDelete(fsf->create
		(folderPathToFileSystemPath(path, CONTAINER_DIRECTORY)));  // container
}


bool kmailMaildirFormat::folderExists(const folder::path& path) const
{
	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	ref <utility::file> rootDir = fsf->create
		(folderPathToFileSystemPath(path, ROOT_DIRECTORY));

	ref <utility::file> newDir = fsf->create
		(folderPathToFileSystemPath(path, NEW_DIRECTORY));
	ref <utility::file> tmpDir = fsf->create
		(folderPathToFileSystemPath(path, TMP_DIRECTORY));
	ref <utility::file> curDir = fsf->create
		(folderPathToFileSystemPath(path, CUR_DIRECTORY));

	return rootDir->exists() && rootDir->isDirectory() &&
	       newDir->exists() && newDir->isDirectory() &&
	       tmpDir->exists() && tmpDir->isDirectory() &&
	       curDir->exists() && curDir->isDirectory();
}


const utility::file::path kmailMaildirFormat::folderPathToFileSystemPath
	(const folder::path& path, const DirectoryType type) const
{
	// Root path
	utility::file::path fsPath = getContext()->getStore()->getFileSystemPath();

	const int count = (type == CONTAINER_DIRECTORY
		? path.getSize() : path.getSize() - 1);

	// Parent folders
	for (int i = 0 ; i < count ; ++i)
	{
		utility::file::path::component comp(path[i]);

		// TODO: may not work with all encodings...
		comp.setBuffer("." + comp.getBuffer() + ".directory");

		fsPath /= comp;
	}

	// Last component
	if (path.getSize() != 0 && type != CONTAINER_DIRECTORY)
	{
		fsPath /= path.getLastComponent();

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

			// Can't happen...
			break;
		}
	}

	return fsPath;
}


const std::vector <folder::path> kmailMaildirFormat::listFolders
	(const folder::path& root, const bool recursive) const
{
	std::vector <folder::path> list;
	listFoldersImpl(list, root, recursive);

	return list;
}


void kmailMaildirFormat::listFoldersImpl
	(std::vector <folder::path>& list, const folder::path& root, const bool recursive) const
{
	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	ref <utility::file> rootDir = fsf->create(folderPathToFileSystemPath(root,
		root.isEmpty() ? ROOT_DIRECTORY : CONTAINER_DIRECTORY));

	if (rootDir->exists())
	{
		ref <utility::fileIterator> it = rootDir->getFiles();

		while (it->hasMoreElements())
		{
			ref <utility::file> file = it->nextElement();

			if (isSubfolderDirectory(*file))
			{
				const utility::path subPath =
					root / file->getFullPath().getLastComponent();

				list.push_back(subPath);

				if (recursive)
					listFoldersImpl(list, subPath, true);
			}
		}
	}
	else
	{
		// No sub-folder
	}
}


// static
bool kmailMaildirFormat::isSubfolderDirectory(const utility::file& file)
{
	// A directory which name does not start with '.' is listed as a sub-folder
	if (file.isDirectory() &&
	    file.getFullPath().getLastComponent().getBuffer().length() >= 1 &&
	    file.getFullPath().getLastComponent().getBuffer()[0] != '.')
	{
		return true;
	}

	return false;
}


void kmailMaildirFormat::renameFolder(const folder::path& oldPath, const folder::path& newPath)
{
	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	ref <utility::file> rootDir = fsf->create
		(folderPathToFileSystemPath(oldPath, ROOT_DIRECTORY));
	ref <utility::file> contDir = fsf->create
		(folderPathToFileSystemPath(oldPath, CONTAINER_DIRECTORY));

	try
	{
		const utility::file::path newRootPath =
			folderPathToFileSystemPath(newPath, ROOT_DIRECTORY);
		const utility::file::path newContPath =
			folderPathToFileSystemPath(newPath, CONTAINER_DIRECTORY);

		rootDir->rename(newRootPath);

		// Container directory may not exist, so ignore error when trying to rename it
		try
		{
			contDir->rename(newContPath);
		}
		catch (exceptions::filesystem_exception& e)
		{
			// Ignore
		}
	}
	catch (exceptions::filesystem_exception& e)
	{
		// Revert to old location
		const utility::file::path rootPath =
			folderPathToFileSystemPath(oldPath, ROOT_DIRECTORY);
		const utility::file::path contPath =
			folderPathToFileSystemPath(oldPath, CONTAINER_DIRECTORY);

		try
		{
			rootDir->rename(rootPath);
			contDir->rename(contPath);
		}
		catch (exceptions::filesystem_exception& e)
		{
			// Ignore (not recoverable)
		}

		throw;
	}
}


bool kmailMaildirFormat::folderHasSubfolders(const folder::path& path) const
{
	ref <utility::fileSystemFactory> fsf = platform::getHandler()->getFileSystemFactory();

	ref <utility::file> rootDir = fsf->create
		(folderPathToFileSystemPath(path, CONTAINER_DIRECTORY));

	ref <utility::fileIterator> it = rootDir->getFiles();

	while (it->hasMoreElements())
	{
		ref <utility::file> file = it->nextElement();

		if (isSubfolderDirectory(*file))
			return true;
	}

	return false;
}


bool kmailMaildirFormat::supports() const
{
	// This is the default
	return true;
}


} // format
} // maildir
} // net
} // vmime

