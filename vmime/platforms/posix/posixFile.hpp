//
// VMime library (http://vmime.sourceforge.net)
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

#ifndef VMIME_PLATFORMS_POSIX_FILE_HPP_INCLUDED
#define VMIME_PLATFORMS_POSIX_FILE_HPP_INCLUDED


#include "vmime/utility/file.hpp"


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

private:

	const vmime::utility::file::path m_path;
	const int m_fd;
};



class posixFileReaderInputStream : public vmime::utility::inputStream
{
public:

	posixFileReaderInputStream(const vmime::utility::file::path& path, const int fd);
	~posixFileReaderInputStream();

	const bool eof() const;

	void reset();

	const size_type read(value_type* const data, const size_type count);

	const size_type skip(const size_type count);

private:

	const vmime::utility::file::path m_path;
	const int m_fd;

	bool m_eof;
};



class posixFileWriter : public vmime::utility::fileWriter
{
public:

	posixFileWriter(const vmime::utility::file::path& path, const vmime::string& nativePath);

	vmime::utility::outputStream* getOutputStream();

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};



class posixFileReader : public vmime::utility::fileReader
{
public:

	posixFileReader(const vmime::utility::file::path& path, const vmime::string& nativePath);

	vmime::utility::inputStream* getInputStream();

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};



class posixFileIterator : public vmime::utility::fileIterator
{
public:

	posixFileIterator(const vmime::utility::file::path& path, const vmime::string& nativePath);
	~posixFileIterator();

	const bool hasMoreElements() const;
	vmime::utility::file* nextElement();

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

	const bool isFile() const;
	const bool isDirectory() const;

	const bool canRead() const;
	const bool canWrite() const;

	const length_type getLength();

	const path& getFullPath() const;

	const bool exists() const;

	const vmime::utility::file* getParent() const;

	void rename(const path& newName);

	void remove();

	vmime::utility::fileWriter* getFileWriter();
	vmime::utility::fileReader* getFileReader();

	vmime::utility::fileIterator* getFiles() const;

private:

	static void createDirectoryImpl(const vmime::utility::file::path& fullPath, const vmime::utility::file::path& path, const bool recursive = false);

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};



class posixFileSystemFactory : public vmime::utility::fileSystemFactory
{
public:

	vmime::utility::file* create(const vmime::utility::file::path& path) const;

	const vmime::utility::file::path stringToPath(const vmime::string& str) const;
	const vmime::string pathToString(const vmime::utility::file::path& path) const;

	static const vmime::utility::file::path stringToPathImpl(const vmime::string& str);
	static const vmime::string pathToStringImpl(const vmime::utility::file::path& path);

	const bool isValidPathComponent(const vmime::utility::file::path::component& comp) const;
	const bool isValidPath(const vmime::utility::file::path& path) const;

	static void reportError(const vmime::utility::path& path, const int err);
};


} // posix
} // platforms
} // vmime


#endif // VMIME_HAVE_FILESYSTEM_FEATURES

#endif // VMIME_PLATFORMS_POSIX_FILE_HPP_INCLUDED
