//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2005 Vincent Richard <vincent@vincent-richard.net>
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

#include "vmime/platforms/posix/posixFile.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>

#include <string.h>

#include "vmime/exception.hpp"


#if VMIME_HAVE_FILESYSTEM_FEATURES


namespace vmime {
namespace platforms {
namespace posix {


//
// posixFileIterator
//

posixFileIterator::posixFileIterator(const vmime::utility::file::path& path, const vmime::string& nativePath)
	: m_path(path), m_nativePath(nativePath), m_dir(NULL), m_dirEntry(NULL)
{
	if ((m_dir = ::opendir(m_nativePath.c_str())) == NULL)
		posixFileSystemFactory::reportError(path, errno);

	getNextElement();
}


posixFileIterator::~posixFileIterator()
{
	if (m_dir != NULL)
		::closedir(m_dir);
}


const bool posixFileIterator::hasMoreElements() const
{
	return (m_dirEntry != NULL);
}


ref <vmime::utility::file> posixFileIterator::nextElement()
{
	ref <posixFile> file = vmime::create <posixFile>
		(m_path / vmime::utility::file::path::component(m_dirEntry->d_name));

	getNextElement();

	return (file);
}


void posixFileIterator::getNextElement()
{
	while ((m_dirEntry = ::readdir(m_dir)) != NULL)
	{
		const char* name = m_dirEntry->d_name;
		const int len = ::strlen(name);

		if (!(len == 1 && name[0] == '.') &&
		    !(len == 2 && name[0] == '.' && name[1] == '.'))
		{
			break;
		}
	}
}



//
// posixFileWriterOutputStream
//

posixFileWriterOutputStream::posixFileWriterOutputStream(const vmime::utility::file::path& path, const int fd)
	: m_path(path), m_fd(fd)
{
}


posixFileWriterOutputStream::~posixFileWriterOutputStream()
{
	::close(m_fd);
}


void posixFileWriterOutputStream::write(const value_type* const data, const size_type count)
{
	if (::write(m_fd, data, count) == -1)
		posixFileSystemFactory::reportError(m_path, errno);
}



//
// posixFileReaderInputStream
//

posixFileReaderInputStream::posixFileReaderInputStream(const vmime::utility::file::path& path, const int fd)
	: m_path(path), m_fd(fd), m_eof(false)
{
}


posixFileReaderInputStream::~posixFileReaderInputStream()
{
	::close(m_fd);
}


const bool posixFileReaderInputStream::eof() const
{
	return (m_eof);
}


void posixFileReaderInputStream::reset()
{
	::lseek(m_fd, 0, SEEK_SET);
}


const vmime::utility::stream::size_type posixFileReaderInputStream::read
	(value_type* const data, const size_type count)
{
	ssize_t c = 0;

	if ((c = ::read(m_fd, data, count)) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	if (c == 0)
		m_eof = true;

	return static_cast <size_type>(c);
}


const vmime::utility::stream::size_type posixFileReaderInputStream::skip(const size_type count)
{
	const off_t curPos = ::lseek(m_fd, 0, SEEK_CUR);
	const off_t newPos = ::lseek(m_fd, count, SEEK_CUR);

	return static_cast <size_type>(newPos - curPos);
}



//
// posixFileWriter
//

posixFileWriter::posixFileWriter(const vmime::utility::file::path& path, const vmime::string& nativePath)
	: m_path(path), m_nativePath(nativePath)
{
}


ref <vmime::utility::outputStream> posixFileWriter::getOutputStream()
{
	int fd = 0;

	if ((fd = ::open(m_nativePath.c_str(), O_WRONLY, 0660)) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	return vmime::create <posixFileWriterOutputStream>(m_path, fd);
}



//
// posixFileReader
//

posixFileReader::posixFileReader(const vmime::utility::file::path& path, const vmime::string& nativePath)
	: m_path(path), m_nativePath(nativePath)
{
}


ref <vmime::utility::inputStream> posixFileReader::getInputStream()
{
	int fd = 0;

	if ((fd = ::open(m_nativePath.c_str(), O_RDONLY, 0640)) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	return vmime::create <posixFileReaderInputStream>(m_path, fd);
}



//
// posixFile
//

posixFile::posixFile(const vmime::utility::file::path& path)
	: m_path(path), m_nativePath(posixFileSystemFactory::pathToStringImpl(path))
{
}


void posixFile::createFile()
{
	int fd = 0;

	if ((fd = ::open(m_nativePath.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0660)) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	::close(fd);
}


void posixFile::createDirectory(const bool createAll)
{
	createDirectoryImpl(m_path, m_path, createAll);
}


const bool posixFile::isFile() const
{
	struct stat buf;
	return (::stat(m_nativePath.c_str(), &buf) == 0 && S_ISREG(buf.st_mode));
}


const bool posixFile::isDirectory() const
{
	struct stat buf;
	return (::stat(m_nativePath.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode));
}


const bool posixFile::canRead() const
{
	struct stat buf;
	return (::stat(m_nativePath.c_str(), &buf) == 0 &&
			S_ISREG(buf.st_mode) &&
			::access(m_nativePath.c_str(), R_OK | F_OK) == 0);
}


const bool posixFile::canWrite() const
{
	struct stat buf;
	return (::stat(m_nativePath.c_str(), &buf) == 0 &&
			S_ISREG(buf.st_mode) &&
			::access(m_nativePath.c_str(), W_OK | F_OK) == 0);
}


const posixFile::length_type posixFile::getLength()
{
	struct stat buf;

	if (::stat(m_nativePath.c_str(), &buf) != 0)
		posixFileSystemFactory::reportError(m_path, errno);

	return static_cast <length_type>(buf.st_size);
}


const posixFile::path& posixFile::getFullPath() const
{
	return (m_path);
}


const bool posixFile::exists() const
{
	struct stat buf;
	return (::stat(m_nativePath.c_str(), &buf) == 0);
}


ref <vmime::utility::file> posixFile::getParent() const
{
	if (m_path.isEmpty())
		return NULL;
	else
		return vmime::create <posixFile>(m_path.getParent());
}


void posixFile::rename(const path& newName)
{
	const vmime::string newNativePath = posixFileSystemFactory::pathToStringImpl(newName);

	if (::rename(m_nativePath.c_str(), newNativePath.c_str()) != 0)
		posixFileSystemFactory::reportError(m_path, errno);

	m_path = newName;
	m_nativePath = newNativePath;
}


void posixFile::remove()
{
	struct stat buf;

	if (::stat(m_nativePath.c_str(), &buf) != 0)
		posixFileSystemFactory::reportError(m_path, errno);

	if (S_ISDIR(buf.st_mode))
	{
		if (::rmdir(m_nativePath.c_str()) != 0)
			posixFileSystemFactory::reportError(m_path, errno);
	}
	else if (S_ISREG(buf.st_mode))
	{
		if (::unlink(m_nativePath.c_str()) != 0)
			posixFileSystemFactory::reportError(m_path, errno);
	}
}


ref <vmime::utility::fileWriter> posixFile::getFileWriter()
{
	return vmime::create <posixFileWriter>(m_path, m_nativePath);
}


ref <vmime::utility::fileReader> posixFile::getFileReader()
{
	return vmime::create <posixFileReader>(m_path, m_nativePath);
}


ref <vmime::utility::fileIterator> posixFile::getFiles() const
{
	if (!isDirectory())
		throw vmime::exceptions::not_a_directory(m_path);

	return vmime::create <posixFileIterator>(m_path, m_nativePath);
}


void posixFile::createDirectoryImpl(const vmime::utility::file::path& fullPath,
	const vmime::utility::file::path& path, const bool recursive)
{
	const vmime::string nativePath = posixFileSystemFactory::pathToStringImpl(path);
	struct stat buf;

	if (::stat(nativePath.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode))
		return;

	if (!path.isEmpty() && recursive)
		createDirectoryImpl(fullPath, path.getParent(), true);

	if (::mkdir(nativePath.c_str(), 0750) != 0)
		posixFileSystemFactory::reportError(fullPath, errno);
}



//
// posixFileSystemFactory
//

ref <vmime::utility::file> posixFileSystemFactory::create(const vmime::utility::file::path& path) const
{
	return vmime::create <posixFile>(path);
}


const vmime::utility::file::path posixFileSystemFactory::stringToPath(const vmime::string& str) const
{
	return (stringToPathImpl(str));
}


const vmime::string posixFileSystemFactory::pathToString(const vmime::utility::file::path& path) const
{
	return (pathToStringImpl(path));
}


const vmime::utility::file::path posixFileSystemFactory::stringToPathImpl(const vmime::string& str)
{
	vmime::string::size_type offset = 0;
	vmime::string::size_type prev = 0;

	vmime::utility::file::path path;

	while ((offset = str.find_first_of("/", offset)) != vmime::string::npos)
	{
		if (offset != prev)
			path.appendComponent(vmime::string(str.begin() + prev, str.begin() + offset));

		prev = offset + 1;
		offset++;
	}

	if (prev < str.length())
		path.appendComponent(vmime::string(str.begin() + prev, str.end()));

	return (path);
}


const vmime::string posixFileSystemFactory::pathToStringImpl(const vmime::utility::file::path& path)
{
	vmime::string native = "/";

	for (int i = 0 ; i < path.getSize() ; ++i)
	{
		if (i > 0)
			native += "/";

		native += path[i].getBuffer();
	}

	return (native);
}


const bool posixFileSystemFactory::isValidPathComponent(const vmime::utility::file::path::component& comp) const
{
	return (comp.getBuffer().find_first_of("/*") == vmime::string::npos);
}


const bool posixFileSystemFactory::isValidPath(const vmime::utility::file::path& path) const
{
	for (int i = 0 ; i < path.getSize() ; ++i)
	{
		if (!isValidPathComponent(path[i]))
			return false;
	}

	return true;
}


void posixFileSystemFactory::reportError(const vmime::utility::path& path, const int err)
{
	vmime::string desc;

	switch (err)
	{
	case EEXIST: desc = "EEXIST: file already exists."; break;
	case EISDIR: desc = "EISDIR: path refers to a directory."; break;
	case EACCES: desc = "EACCES: permission denied"; break;
	case ENAMETOOLONG: desc = "ENAMETOOLONG: path too long."; break;
	case ENOENT: desc = "ENOENT: a directory in the path does not exist."; break;
	case ENOTDIR: desc = "ENOTDIR: path is not a directory."; break;
	case EROFS: desc = "EROFS: read-only filesystem."; break;
	case ELOOP: desc = "ELOOP: too many symbolic links."; break;
	case ENOSPC: desc = "ENOSPC: no space left on device."; break;
	case ENOMEM: desc = "ENOMEM: insufficient kernel memory."; break;
	case EMFILE: desc = "ENFILE: limit on number of files open by the process has been reached."; break;
	case ENFILE: desc = "ENFILE: limit on number of files open on the system has been reached."; break;
	case ENOTEMPTY: desc = "ENOTEMPTY: directory is not empty."; break;

	default:

		std::ostringstream oss;
		oss << ::strerror(err) << ".";

		desc = oss.str();
		break;
	}

	throw vmime::exceptions::filesystem_exception(desc, path);
}



} // posix
} // platforms
} // vmime


#endif // VMIME_HAVE_FILESYSTEM_FEATURES
