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


#if VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_FILESYSTEM_FEATURES


#include "vmime/platforms/posix/posixFile.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>

#include <stdio.h>
#include <string.h>

#include "vmime/exception.hpp"


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
	{
		if (::closedir(m_dir) == -1)
			posixFileSystemFactory::reportError(m_path, errno);
	}
}


bool posixFileIterator::hasMoreElements() const
{
	return (m_dirEntry != NULL);
}


shared_ptr <vmime::utility::file> posixFileIterator::nextElement()
{
	shared_ptr <posixFile> file = make_shared <posixFile>
		(m_path / vmime::utility::file::path::component(m_dirEntry->d_name));

	getNextElement();

	return (file);
}


void posixFileIterator::getNextElement()
{
	errno = 0;

	while ((m_dirEntry = ::readdir(m_dir)) != NULL)
	{
		const char* name = m_dirEntry->d_name;
		const size_t len = ::strlen(name);

		if (!(len == 1 && name[0] == '.') &&
		    !(len == 2 && name[0] == '.' && name[1] == '.'))
		{
			break;
		}
	}

	if (errno)
		posixFileSystemFactory::reportError(m_path, errno);
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


void posixFileWriterOutputStream::writeImpl
	(const byte_t* const data, const size_t count)
{
	const byte_t* array = data;
	size_t size = count;

	while (1)
	{
		ssize_t ret = ::write(m_fd, array, size);

		if (ret == -1)
		{
			if (errno == EINTR)
				continue;

			posixFileSystemFactory::reportError(m_path, errno);
			break;
		}
		else if (size_t(ret) < size)
		{
			array += ret;
			size -= ret;
		}

		break;
	}
}


void posixFileWriterOutputStream::flush()
{
	if (::fsync(m_fd) == -1)
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
	if (::close(m_fd) == -1)
		posixFileSystemFactory::reportError(m_path, errno);
}


bool posixFileReaderInputStream::eof() const
{
	return (m_eof);
}


void posixFileReaderInputStream::reset()
{
	if (::lseek(m_fd, 0, SEEK_SET) == off_t(-1))
		posixFileSystemFactory::reportError(m_path, errno);

	m_eof = false;
}


size_t posixFileReaderInputStream::read
	(byte_t* const data, const size_t count)
{
	ssize_t c = 0;

	if ((c = ::read(m_fd, data, count)) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	if (c == 0 && count != 0)
		m_eof = true;

	return static_cast <size_t>(c);
}


size_t posixFileReaderInputStream::skip(const size_t count)
{
	const off_t curPos = ::lseek(m_fd, 0, SEEK_CUR);

	if (curPos == off_t(-1))
		posixFileSystemFactory::reportError(m_path, errno);

	const off_t newPos = ::lseek(m_fd, count, SEEK_CUR);

	if (newPos == off_t(-1))
		posixFileSystemFactory::reportError(m_path, errno);

	return static_cast <size_t>(newPos - curPos);
}


size_t posixFileReaderInputStream::getPosition() const
{
	const off_t curPos = ::lseek(m_fd, 0, SEEK_CUR);

	if (curPos == off_t(-1))
		posixFileSystemFactory::reportError(m_path, errno);

	return static_cast <size_t>(curPos);
}


void posixFileReaderInputStream::seek(const size_t pos)
{
	const off_t newPos = ::lseek(m_fd, pos, SEEK_SET);

	if (newPos == off_t(-1))
		posixFileSystemFactory::reportError(m_path, errno);
}



//
// posixFileWriter
//

posixFileWriter::posixFileWriter(const vmime::utility::file::path& path, const vmime::string& nativePath)
	: m_path(path), m_nativePath(nativePath)
{
}


shared_ptr <vmime::utility::outputStream> posixFileWriter::getOutputStream()
{
	int fd = 0;

	if ((fd = ::open(m_nativePath.c_str(), O_WRONLY, 0660)) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	return make_shared <posixFileWriterOutputStream>(m_path, fd);
}



//
// posixFileReader
//

posixFileReader::posixFileReader(const vmime::utility::file::path& path, const vmime::string& nativePath)
	: m_path(path), m_nativePath(nativePath)
{
}


shared_ptr <vmime::utility::inputStream> posixFileReader::getInputStream()
{
	int fd = 0;

	if ((fd = ::open(m_nativePath.c_str(), O_RDONLY, 0640)) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	return make_shared <posixFileReaderInputStream>(m_path, fd);
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

	if (::fsync(fd) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	if (::close(fd) == -1)
		posixFileSystemFactory::reportError(m_path, errno);
}


void posixFile::createDirectory(const bool createAll)
{
	createDirectoryImpl(m_path, m_path, createAll);
}


bool posixFile::isFile() const
{
	struct stat buf;

	if (::stat(m_nativePath.c_str(), &buf) == -1)
	{
		if (errno == ENOENT)
			return false;

		posixFileSystemFactory::reportError(m_path, errno);
		return false;
	}

	return S_ISREG(buf.st_mode);
}


bool posixFile::isDirectory() const
{
	struct stat buf;

	if (::stat(m_nativePath.c_str(), &buf) == -1)
	{
		if (errno == ENOENT)
			return false;

		posixFileSystemFactory::reportError(m_path, errno);
		return false;
	}

	return S_ISDIR(buf.st_mode);
}


bool posixFile::canRead() const
{
	struct stat buf;

	if (::stat(m_nativePath.c_str(), &buf) == -1)
	{
		if (errno == ENOENT)
			return false;

		posixFileSystemFactory::reportError(m_path, errno);
		return false;
	}

	return S_ISREG(buf.st_mode) &&
		::access(m_nativePath.c_str(), R_OK | F_OK) == 0;
}


bool posixFile::canWrite() const
{
	struct stat buf;

	if (::stat(m_nativePath.c_str(), &buf) == -1)
	{
		if (errno == ENOENT)
			return false;

		posixFileSystemFactory::reportError(m_path, errno);
		return false;
	}

	return S_ISREG(buf.st_mode) &&
		::access(m_nativePath.c_str(), W_OK | F_OK) == 0;
}


posixFile::length_type posixFile::getLength()
{
	struct stat buf;

	if (::stat(m_nativePath.c_str(), &buf) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	return static_cast <length_type>(buf.st_size);
}


const posixFile::path& posixFile::getFullPath() const
{
	return (m_path);
}


bool posixFile::exists() const
{
	struct stat buf;
	return (::stat(m_nativePath.c_str(), &buf) == 0);
}


shared_ptr <vmime::utility::file> posixFile::getParent() const
{
	if (m_path.isEmpty())
		return null;
	else
		return make_shared <posixFile>(m_path.getParent());
}


void posixFile::rename(const path& newName)
{
	const vmime::string newNativePath = posixFileSystemFactory::pathToStringImpl(newName);

	posixFile dest(newName);

	if (isDirectory())
		dest.createDirectory();
	else
		dest.createFile();

	if (::rename(m_nativePath.c_str(), newNativePath.c_str()) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	m_path = newName;
	m_nativePath = newNativePath;
}


void posixFile::remove()
{
	struct stat buf;

	if (::stat(m_nativePath.c_str(), &buf) == -1)
		posixFileSystemFactory::reportError(m_path, errno);

	if (S_ISDIR(buf.st_mode))
	{
		if (::rmdir(m_nativePath.c_str()) == -1)
			posixFileSystemFactory::reportError(m_path, errno);
	}
	else if (S_ISREG(buf.st_mode))
	{
		if (::unlink(m_nativePath.c_str()) == -1)
			posixFileSystemFactory::reportError(m_path, errno);
	}
}


shared_ptr <vmime::utility::fileWriter> posixFile::getFileWriter()
{
	return make_shared <posixFileWriter>(m_path, m_nativePath);
}


shared_ptr <vmime::utility::fileReader> posixFile::getFileReader()
{
	return make_shared <posixFileReader>(m_path, m_nativePath);
}


shared_ptr <vmime::utility::fileIterator> posixFile::getFiles() const
{
	if (!isDirectory())
		throw vmime::exceptions::not_a_directory(m_path);

	return make_shared <posixFileIterator>(m_path, m_nativePath);
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

	if (::mkdir(nativePath.c_str(), 0750) == -1)
		posixFileSystemFactory::reportError(fullPath, errno);
}



//
// posixFileSystemFactory
//

shared_ptr <vmime::utility::file> posixFileSystemFactory::create(const vmime::utility::file::path& path) const
{
	return make_shared <posixFile>(path);
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
	vmime::size_t offset = 0;
	vmime::size_t prev = 0;

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


bool posixFileSystemFactory::isValidPathComponent(const vmime::utility::file::path::component& comp) const
{
	return (comp.getBuffer().find_first_of("/*") == vmime::string::npos);
}


bool posixFileSystemFactory::isValidPath(const vmime::utility::file::path& path) const
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
#ifndef AIX
	case ENOTEMPTY: desc = "ENOTEMPTY: directory is not empty."; break;
#endif

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


#endif // VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_FILESYSTEM_FEATURES
