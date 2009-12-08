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

#include "tests/testUtils.hpp"

#include "vmime/platform.hpp"

#include "vmime/net/maildir/maildirStore.hpp"
#include "vmime/net/maildir/maildirFormat.hpp"


#define VMIME_TEST_SUITE         maildirStoreTest
#define VMIME_TEST_SUITE_MODULE  "Net/Maildir"


// Shortcuts and helpers
typedef vmime::utility::file::path fspath;
typedef vmime::utility::file::path::component fspathc;

typedef vmime::net::folder::path fpath;
typedef vmime::net::folder::path::component fpathc;


const fpath operator/(const fpath& path, const std::string& c)
{
	return path / fpathc(c);
}


/** Test messages */
static const vmime::string TEST_MESSAGE_1 =
	"From: <test@vmime.org>\r\n"
	"Subject: VMime Test\r\n"
	"Date: Thu, 01 Mar 2007 09:49:35 +0100\r\n"
	"\r\n"
	"Hello, world!";


/** Maildir trees used in tests.
  * Structure:
  *
  *  .
  *  |-- Folder
  *  |   `-- SubFolder
  *  |       |-- SubSubFolder1
  *  |       `-- SubSubFolder2
  *  `-- Folder2
  *
  */

// KMail format
static const vmime::string TEST_MAILDIR_KMAIL[] =  // directories to create
{
	"/Folder",
	"/Folder/new",
	"/Folder/tmp",
	"/Folder/cur",
	"/.Folder.directory",
	"/.Folder.directory/SubFolder",
	"/.Folder.directory/SubFolder/new",
	"/.Folder.directory/SubFolder/tmp",
	"/.Folder.directory/SubFolder/cur",
	"/.Folder.directory/.SubFolder.directory",
	"/.Folder.directory/.SubFolder.directory/SubSubFolder1",
	"/.Folder.directory/.SubFolder.directory/SubSubFolder1/new",
	"/.Folder.directory/.SubFolder.directory/SubSubFolder1/tmp",
	"/.Folder.directory/.SubFolder.directory/SubSubFolder1/cur",
	"/.Folder.directory/.SubFolder.directory/SubSubFolder2",
	"/.Folder.directory/.SubFolder.directory/SubSubFolder2/new",
	"/.Folder.directory/.SubFolder.directory/SubSubFolder2/tmp",
	"/.Folder.directory/.SubFolder.directory/SubSubFolder2/cur",
	"/Folder2",
	"/Folder2/new",
	"/Folder2/tmp",
	"/Folder2/cur",
	"*"  // end
};

static const vmime::string TEST_MAILDIRFILES_KMAIL[] =  // files to create and their contents
{
	"/.Folder.directory/.SubFolder.directory/SubSubFolder2/cur/1043236113.351.EmqD:S", TEST_MESSAGE_1,
	"*"  // end
};

// Courier format
static const vmime::string TEST_MAILDIR_COURIER[] =  // directories to create
{
	"/.Folder",
	"/.Folder/new",
	"/.Folder/tmp",
	"/.Folder/cur",
	"/.Folder.SubFolder",
	"/.Folder.SubFolder",
	"/.Folder.SubFolder/new",
	"/.Folder.SubFolder/tmp",
	"/.Folder.SubFolder/cur",
	"/.Folder.SubFolder.SubSubFolder1",
	"/.Folder.SubFolder.SubSubFolder1/new",
	"/.Folder.SubFolder.SubSubFolder1/tmp",
	"/.Folder.SubFolder.SubSubFolder1/cur",
	"/.Folder.SubFolder.SubSubFolder2",
	"/.Folder.SubFolder.SubSubFolder2/new",
	"/.Folder.SubFolder.SubSubFolder2/tmp",
	"/.Folder.SubFolder.SubSubFolder2/cur",
	"/.Folder2",
	"/.Folder2/new",
	"/.Folder2/tmp",
	"/.Folder2/cur",
	"*"  // end
};

static const vmime::string TEST_MAILDIRFILES_COURIER[] =  // files to create and their contents
{
	"/.Folder/maildirfolder", "",
	"/.Folder.SubFolder/maildirfolder", "",
	"/.Folder.SubFolder.SubSubFolder1/maildirfolder", "",
	"/.Folder.SubFolder.SubSubFolder2/maildirfolder", "",
	"/.Folder.SubFolder.SubSubFolder2/cur/1043236113.351.EmqD:S", TEST_MESSAGE_1,
	"/.Folder2/maildirfolder", "",
	"*"  // end
};



VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testDetectFormat_KMail)
		VMIME_TEST(testDetectFormat_Courier)

		VMIME_TEST(testListRootFolders_KMail)
		VMIME_TEST(testListAllFolders_KMail)

		VMIME_TEST(testListRootFolders_Courier)
		VMIME_TEST(testListAllFolders_Courier)

		VMIME_TEST(testListMessages_KMail)
		VMIME_TEST(testListMessages_Courier)

		VMIME_TEST(testRenameFolder_KMail)
		VMIME_TEST(testRenameFolder_Courier)

		VMIME_TEST(testDestroyFolder_KMail)
		VMIME_TEST(testDestroyFolder_Courier)

		VMIME_TEST(testFolderExists_KMail)
		VMIME_TEST(testFolderExists_Courier)

		VMIME_TEST(testCreateFolder_KMail)
		VMIME_TEST(testCreateFolder_Courier)
	VMIME_TEST_LIST_END


public:

	maildirStoreTest()
	{
		// Temporary directory
		m_tempPath = fspath() / fspathc("tmp")   // Use /tmp
			/ fspathc("vmime" + vmime::utility::stringUtils::toString(std::time(NULL))
				+ vmime::utility::stringUtils::toString(std::rand()));
	}

	void tearDown()
	{
		// In case of an uncaught exception
		destroyMaildir();
	}

	void testDetectFormat_KMail()
	{
		createMaildir(TEST_MAILDIR_KMAIL, TEST_MAILDIRFILES_KMAIL);

		vmime::ref <vmime::net::maildir::maildirStore> store =
			vmime::dynamicCast <vmime::net::maildir::maildirStore>(createAndConnectStore());

		VASSERT_EQ("*", "kmail", store->getFormat()->getName());

		destroyMaildir();
	}

	void testDetectFormat_Courier()
	{
		createMaildir(TEST_MAILDIR_COURIER, TEST_MAILDIRFILES_COURIER);

		vmime::ref <vmime::net::maildir::maildirStore> store =
			vmime::dynamicCast <vmime::net::maildir::maildirStore>(createAndConnectStore());

		VASSERT_EQ("*", "courier", store->getFormat()->getName());

		destroyMaildir();
	}


	void testListRootFolders_KMail()
	{
		testListRootFoldersImpl(TEST_MAILDIR_KMAIL, TEST_MAILDIRFILES_KMAIL);
	}

	void testListRootFolders_Courier()
	{
		testListRootFoldersImpl(TEST_MAILDIR_COURIER, TEST_MAILDIRFILES_COURIER);
	}

	void testListRootFoldersImpl(const vmime::string* const dirs, const vmime::string* const files)
	{
		createMaildir(dirs, files);

		// Connect to store
		vmime::ref <vmime::net::store> store = createAndConnectStore();
		vmime::ref <vmime::net::folder> rootFolder = store->getRootFolder();

		// Get root folders, not recursive
		const std::vector <vmime::ref <vmime::net::folder> >
			rootFolders = rootFolder->getFolders(false);

		VASSERT_EQ("1", 2, rootFolders.size());
		VASSERT("2", findFolder(rootFolders, fpath() / "Folder") != NULL);
		VASSERT("3", findFolder(rootFolders, fpath() / "Folder2") != NULL);

		destroyMaildir();
	}


	void testListAllFolders_KMail()
	{
		testListAllFoldersImpl(TEST_MAILDIR_KMAIL, TEST_MAILDIRFILES_KMAIL);
	}

	void testListAllFolders_Courier()
	{
		testListAllFoldersImpl(TEST_MAILDIR_COURIER, TEST_MAILDIRFILES_COURIER);
	}

	void testListAllFoldersImpl(const vmime::string* const dirs, const vmime::string* const files)
	{
		createMaildir(dirs, files);

		// Connect to store
		vmime::ref <vmime::net::store> store = createAndConnectStore();
		vmime::ref <vmime::net::folder> rootFolder = store->getRootFolder();

		// Get all folders, recursive
		const std::vector <vmime::ref <vmime::net::folder> >
			allFolders = rootFolder->getFolders(true);

		VASSERT_EQ("1", 5, allFolders.size());
		VASSERT("2", findFolder(allFolders, fpath() / "Folder") != NULL);
		VASSERT("3", findFolder(allFolders, fpath() / "Folder" / "SubFolder") != NULL);
		VASSERT("4", findFolder(allFolders, fpath() / "Folder" / "SubFolder" / "SubSubFolder1") != NULL);
		VASSERT("5", findFolder(allFolders, fpath() / "Folder" / "SubFolder" / "SubSubFolder2") != NULL);
		VASSERT("6", findFolder(allFolders, fpath() / "Folder2") != NULL);

		destroyMaildir();
	}


	void testListMessages_KMail()
	{
		testListMessagesImpl(TEST_MAILDIR_KMAIL, TEST_MAILDIRFILES_KMAIL);
	}

	void testListMessages_Courier()
	{
		testListMessagesImpl(TEST_MAILDIR_COURIER, TEST_MAILDIRFILES_COURIER);
	}

	void testListMessagesImpl(const vmime::string* const dirs, const vmime::string* const files)
	{
		createMaildir(dirs, files);

		vmime::ref <vmime::net::store> store = createAndConnectStore();
		vmime::ref <vmime::net::folder> rootFolder = store->getRootFolder();

		vmime::ref <vmime::net::folder> folder = store->getFolder
			(fpath() / "Folder" / "SubFolder" / "SubSubFolder2");

		int count, unseen;
		folder->status(count, unseen);

		VASSERT_EQ("Message count", 1, count);

		folder->open(vmime::net::folder::MODE_READ_ONLY);

		vmime::ref <vmime::net::message> msg = folder->getMessage(1);

		folder->fetchMessage(msg, vmime::net::folder::FETCH_SIZE);

		VASSERT_EQ("Message size", TEST_MESSAGE_1.length(), msg->getSize());

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter os(oss);
		msg->extract(os);

		VASSERT_EQ("Message contents", TEST_MESSAGE_1, oss.str());

		folder->close(false);

		destroyMaildir();
	}


	void testRenameFolder_KMail()
	{
		try
		{
			testRenameFolderImpl(TEST_MAILDIR_KMAIL, TEST_MAILDIRFILES_KMAIL);
		}
		catch (vmime::exception& e)
		{
			std::cerr << e;
			throw e;
		}
	}

	void testRenameFolder_Courier()
	{
		try
		{
			testRenameFolderImpl(TEST_MAILDIR_COURIER, TEST_MAILDIRFILES_COURIER);
		}
		catch (vmime::exception& e)
		{
			std::cerr << e;
			throw e;
		}
	}

	void testRenameFolderImpl(const vmime::string* const dirs, const vmime::string* const files)
	{
		createMaildir(dirs, files);

		vmime::ref <vmime::net::store> store = createAndConnectStore();
		vmime::ref <vmime::net::folder> rootFolder = store->getRootFolder();

		// Rename "Folder/SubFolder" to "Folder/foo"
		vmime::ref <vmime::net::folder> folder = store->getFolder
			(fpath() / "Folder" / "SubFolder");

		folder->rename(fpath() / "Folder" / "foo");

		// Ensure folder and its subfolders have been renamed
		const std::vector <vmime::ref <vmime::net::folder> >
			allFolders = rootFolder->getFolders(true);

		VASSERT_EQ("1", 5, allFolders.size());
		VASSERT("2", findFolder(allFolders, fpath() / "Folder") != NULL);
		VASSERT("3", findFolder(allFolders, fpath() / "Folder" / "SubFolder") == NULL);
		VASSERT("4", findFolder(allFolders, fpath() / "Folder" / "SubFolder" / "SubSubFolder1") == NULL);
		VASSERT("5", findFolder(allFolders, fpath() / "Folder" / "SubFolder" / "SubSubFolder2") == NULL);
		VASSERT("6", findFolder(allFolders, fpath() / "Folder2") != NULL);
		VASSERT("7", findFolder(allFolders, fpath() / "Folder" / "foo") != NULL);
		VASSERT("8", findFolder(allFolders, fpath() / "Folder" / "foo" / "SubSubFolder1") != NULL);
		VASSERT("9", findFolder(allFolders, fpath() / "Folder" / "foo" / "SubSubFolder2") != NULL);

		destroyMaildir();
	}


	void testDestroyFolder_KMail()
	{
		testDestroyFolderImpl(TEST_MAILDIR_KMAIL, TEST_MAILDIRFILES_KMAIL);
	}

	void testDestroyFolder_Courier()
	{
		testDestroyFolderImpl(TEST_MAILDIR_COURIER, TEST_MAILDIRFILES_COURIER);
	}

	void testDestroyFolderImpl(const vmime::string* const dirs, const vmime::string* const files)
	{
		createMaildir(dirs, files);

		vmime::ref <vmime::net::store> store = createAndConnectStore();
		vmime::ref <vmime::net::folder> rootFolder = store->getRootFolder();

		// Destroy "Folder/SubFolder" (total: 3 folders)
		vmime::ref <vmime::net::folder> folder = store->getFolder
			(fpath() / "Folder" / "SubFolder");

		folder->destroy();

		// Ensure folder and its subfolders have been deleted and other folders still exist
		const std::vector <vmime::ref <vmime::net::folder> >
			allFolders = rootFolder->getFolders(true);

		VASSERT_EQ("1", 2, allFolders.size());
		VASSERT("2", findFolder(allFolders, fpath() / "Folder") != NULL);
		VASSERT("3", findFolder(allFolders, fpath() / "Folder" / "SubFolder") == NULL);
		VASSERT("4", findFolder(allFolders, fpath() / "Folder" / "SubFolder" / "SubSubFolder1") == NULL);
		VASSERT("5", findFolder(allFolders, fpath() / "Folder" / "SubFolder" / "SubSubFolder2") == NULL);
		VASSERT("6", findFolder(allFolders, fpath() / "Folder2") != NULL);

		destroyMaildir();
	}


	void testFolderExists_KMail()
	{
		testFolderExistsImpl(TEST_MAILDIR_KMAIL, TEST_MAILDIRFILES_KMAIL);
	}

	void testFolderExists_Courier()
	{
		testFolderExistsImpl(TEST_MAILDIR_COURIER, TEST_MAILDIRFILES_COURIER);
	}

	void testFolderExistsImpl(const vmime::string* const dirs, const vmime::string* const files)
	{
		createMaildir(dirs, files);

		vmime::ref <vmime::net::store> store = createAndConnectStore();
		vmime::ref <vmime::net::folder> rootFolder = store->getRootFolder();

		VASSERT("1",  store->getFolder(fpath() / "Folder" / "SubFolder")->exists());
		VASSERT("2", !store->getFolder(fpath() / "Folder" / "SubSubFolder1")->exists());
		VASSERT("3",  store->getFolder(fpath() / "Folder2")->exists());
		VASSERT("4",  store->getFolder(fpath() / "Folder" / "SubFolder" / "SubSubFolder2")->exists());

		destroyMaildir();
	}


	void testCreateFolder_KMail()
	{
		testCreateFolderImpl(TEST_MAILDIR_KMAIL, TEST_MAILDIRFILES_KMAIL);
	}

	void testCreateFolder_Courier()
	{
		testCreateFolderImpl(TEST_MAILDIR_COURIER, TEST_MAILDIRFILES_COURIER);
	}

	void testCreateFolderImpl(const vmime::string* const dirs, const vmime::string* const files)
	{
		createMaildir(dirs, files);

		vmime::ref <vmime::net::store> store = createAndConnectStore();
		vmime::ref <vmime::net::folder> rootFolder = store->getRootFolder();

		VASSERT("Before", !store->getFolder(fpath() / "Folder" / "NewFolder")->exists());

		VASSERT_NO_THROW("Creation", store->getFolder(fpath() / "Folder" / "NewFolder")->
			create(vmime::net::folder::TYPE_CONTAINS_MESSAGES));

		VASSERT("After", store->getFolder(fpath() / "Folder" / "NewFolder")->exists());

		destroyMaildir();
	}

private:

	vmime::utility::file::path m_tempPath;


	vmime::ref <vmime::net::store> createAndConnectStore()
	{
		vmime::ref <vmime::net::session> session =
			vmime::create <vmime::net::session>();

		vmime::ref <vmime::net::store> store =
			session->getStore(getStoreURL());

		store->connect();

		return store;
	}

	const vmime::ref <vmime::net::folder> findFolder
		(const std::vector <vmime::ref <vmime::net::folder> >& folders,
		 const vmime::net::folder::path& path)
	{
		for (unsigned int i = 0, n = folders.size() ; i < n ; ++i)
		{
			if (folders[i]->getFullPath() == path)
				return folders[i];
		}

		return NULL;
	}

	const vmime::utility::url getStoreURL()
	{
		vmime::ref <vmime::utility::fileSystemFactory> fsf =
			vmime::platform::getHandler()->getFileSystemFactory();

		vmime::utility::url url(std::string("maildir://localhost")
			+ fsf->pathToString(m_tempPath));

		return url;
	}

	void createMaildir(const vmime::string* const dirs, const vmime::string* const files)
	{
		vmime::ref <vmime::utility::fileSystemFactory> fsf =
			vmime::platform::getHandler()->getFileSystemFactory();

		vmime::ref <vmime::utility::file> rootDir = fsf->create(m_tempPath);
		rootDir->createDirectory(false);

		for (vmime::string const* dir = dirs ; *dir != "*" ; ++dir)
		{
			vmime::ref <vmime::utility::file> fdir = fsf->create(m_tempPath / fsf->stringToPath(*dir));
			fdir->createDirectory(false);
		}

		for (vmime::string const* file = files ; *file != "*" ; file += 2)
		{
			const vmime::string& contents = *(file + 1);

			vmime::ref <vmime::utility::file> ffile = fsf->create(m_tempPath / fsf->stringToPath(*file));
			ffile->createFile();

			vmime::ref <vmime::utility::fileWriter> fileWriter = ffile->getFileWriter();
			vmime::ref <vmime::utility::outputStream> os = fileWriter->getOutputStream();

			os->write(contents.data(), contents.length());
			os->flush();

			fileWriter = NULL;
		}

	}

	void destroyMaildir()
	{
		vmime::ref <vmime::utility::fileSystemFactory> fsf =
			vmime::platform::getHandler()->getFileSystemFactory();

		recursiveDelete(fsf->create(m_tempPath));
	}

	void recursiveDelete(vmime::ref <vmime::utility::file> dir)
	{
		if (!dir->exists() || !dir->isDirectory())
			return;

		vmime::ref <vmime::utility::fileIterator> files = dir->getFiles();

		// First, delete files and subdirectories in this directory
		while (files->hasMoreElements())
		{
			vmime::ref <vmime::utility::file> file = files->nextElement();

			if (file->isDirectory())
			{
				recursiveDelete(file);
			}
			else
			{
				try
				{
					file->remove();
				}
				catch (vmime::exceptions::filesystem_exception&)
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
		catch (vmime::exceptions::filesystem_exception&)
		{
			// Ignore
		}
	}

VMIME_TEST_SUITE_END

