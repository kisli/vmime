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

#ifndef VMIME_PLATFORMS_POSIX_FILE_HPP_INCLUDED
#define VMIME_PLATFORMS_POSIX_FILE_HPP_INCLUDED


#include "vmime/utility/file.hpp"
#include "vmime/utility/seekableInputStream.hpp"


#if VMIME_HAVE_FILESYSTEM_FEATURES


#include <dirent.h>


namespace vmime {
namespace platforms {
namespace posix {


class posixFileWriterOutputStream : public vmime::utility::outputStream
{
public:

	posixFileWriterOutputStream(const vmime::utility::file::path& path, const int fd);
	~posixFileWriterOutputStream();

	void write(const value_type* const data, const size_type count);
	void flush();

private:

	const vmime::utility::file::path m_path;
	const int m_fd;
};



class posixFileReaderInputStream : public vmime::utility::seekableInputStream
{
public:

	posixFileReaderInputStream(const vmime::utility::file::path& path, const int fd);
	~posixFileReaderInputStream();

	bool eof() const;

	void reset();

	size_type read(value_type* const data, const size_type count);

	size_type skip(const size_type count);

	size_type getPosition() const;
	void seek(const size_type pos);

private:

	const vmime::utility::file::path m_path;
	const int m_fd;

	bool m_eof;
};



class posixFileWriter : public vmime::utility::fileWriter
{
public:

	posixFileWriter(const vmime::utility::file::path& path, const vmime::string& nativePath);

	ref <vmime::utility::outputStream> getOutputStream();

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};



class posixFileReader : public vmime::utility::fileReader
{
public:

	posixFileReader(const vmime::utility::file::path& path, const vmime::string& nativePath);

	ref <vmime::utility::inputStream> getInputStream();

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};



class posixFileIterator : public vmime::utility::fileIterator
{
public:

	posixFileIterator(const vmime::utility::file::path& path, const vmime::string& nativePath);
	~posixFileIterator();

	bool hasMoreElements() const;
	ref <vmime::utility::file> nextElement();

private:

	void getNextElement();

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;

	DIR* m_dir;
	struct dirent* m_dirEntry;
};



class posixFile : public vmime::utility::file
{
public:

	posixFile(const vmime::utility::file::path& path);

	void createFile();
	void createDirectory(const bool createAll = false);

	bool isFile() const;
	bool isDirectory() const;

	bool canRead() const;
	bool canWrite() const;

	length_type getLength();

	const path& getFullPath() const;

	bool exists() const;

	ref <vmime::utility::file> getParent() const;

	void rename(const path& newName);

	void remove();

	ref <vmime::utility::fileWriter> getFileWriter();
	ref <vmime::utility::fileReader> getFileReader();

	ref <vmime::utility::fileIterator> getFiles() const;

private:

	static void createDirectoryImpl(const vmime::utility::file::path& fullPath, const vmime::utility::file::path& path, const bool recursive = false);

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};



class posixFileSystemFactory : public vmime::utility::fileSystemFactory
{
public:

	ref <vmime::utility::file> create(const vmime::utility::file::path& path) const;

	const vmime::utility::file::path stringToPath(const vmime::string& str) const;
	const vmime::string pathToString(const vmime::utility::file::path& path) const;

	static const vmime::utility::file::path stringToPathImpl(const vmime::string& str);
	static const vmime::string pathToStringImpl(const vmime::utility::file::path& path);

	bool isValidPathComponent(const vmime::utility::file::path::component& comp) const;
	bool isValidPath(const vmime::utility::file::path& path) const;

	static void reportError(const vmime::utility::path& path, const int err);
};


} // posix
} // platforms
} // vmime


#endif // VMIME_HAVE_FILESYSTEM_FEATURES

#endif // VMIME_PLATFORMS_POSIX_FILE_HPP_INCLUDED
