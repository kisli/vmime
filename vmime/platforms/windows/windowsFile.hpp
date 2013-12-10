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

#ifndef VMIME_PLATFORMS_WINDOWS_FILE_HPP_INCLUDED
#define VMIME_PLATFORMS_WINDOWS_FILE_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_FILESYSTEM_FEATURES


#include "vmime/utility/file.hpp"
#include "vmime/utility/seekableInputStream.hpp"

#include <windows.h>


namespace vmime {
namespace platforms {
namespace windows {


class windowsFileSystemFactory : public vmime::utility::fileSystemFactory
{
public:

	shared_ptr <vmime::utility::file> create(const vmime::utility::file::path& path) const;

	const vmime::utility::file::path stringToPath(const vmime::string& str) const;
	const vmime::string pathToString(const vmime::utility::file::path& path) const;

	static const vmime::utility::file::path stringToPathImpl(const vmime::string& str);
	static const vmime::string pathToStringImpl(const vmime::utility::file::path& path);

	bool isValidPathComponent(const vmime::utility::file::path::component& comp) const;
	bool isValidPathComponent(const vmime::utility::file::path::component& comp,
		                              bool firstComponent) const;
	bool isValidPath(const vmime::utility::file::path& path) const;

	static void reportError(const vmime::utility::path& path, const int err);
};


class windowsFile : public vmime::utility::file
{
public:

	windowsFile(const vmime::utility::file::path& path);

	void createFile();
	void createDirectory(const bool createAll = false);

	bool isFile() const;
	bool isDirectory() const;

	bool canRead() const;
	bool canWrite() const;

	length_type getLength();

	const path& getFullPath() const;

	bool exists() const;

	shared_ptr <file> getParent() const;

	void rename(const path& newName);
	void remove();

	shared_ptr <vmime::utility::fileWriter> getFileWriter();

	shared_ptr <vmime::utility::fileReader> getFileReader();

	shared_ptr <vmime::utility::fileIterator> getFiles() const;

private:

	static void createDirectoryImpl(const vmime::utility::file::path& fullPath, const vmime::utility::file::path& path, const bool recursive = false);

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};


class windowsFileIterator : public vmime::utility::fileIterator
{
public:

	windowsFileIterator(const vmime::utility::file::path& path, const vmime::string& nativePath);
	~windowsFileIterator();

	bool hasMoreElements() const;
	shared_ptr <vmime::utility::file> nextElement();

private:

	void findFirst();
	void findNext();
	bool isCurrentOrParentDir() const;

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
	WIN32_FIND_DATA m_findData;
	bool m_moreElements;
	HANDLE m_hFind;
};


class windowsFileReader : public vmime::utility::fileReader
{
public:

	windowsFileReader(const vmime::utility::file::path& path, const vmime::string& nativePath);

public:

	shared_ptr <vmime::utility::inputStream> getInputStream();

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};


class windowsFileReaderInputStream : public vmime::utility::inputStream
{
public:

	windowsFileReaderInputStream(const vmime::utility::file::path& path, HANDLE hFile);
	~windowsFileReaderInputStream();

public:

	bool eof() const;
	void reset();
	size_t read(byte_t* const data, const size_t count);
	size_t skip(const size_t count);
	size_t getPosition() const;
	void seek(const size_t pos);

private:

	const vmime::utility::file::path m_path;
	HANDLE m_hFile;
};


class windowsFileWriter : public vmime::utility::fileWriter
{
public:

	windowsFileWriter(const vmime::utility::file::path& path, const vmime::string& nativePath);

public:

	shared_ptr <vmime::utility::outputStream> getOutputStream();

private:

	vmime::utility::file::path m_path;
	vmime::string m_nativePath;
};


class windowsFileWriterOutputStream : public vmime::utility::outputStream
{
public:

	windowsFileWriterOutputStream(const vmime::utility::file::path& path, HANDLE hFile);
	~windowsFileWriterOutputStream();

public:

	void flush();

protected:

	void writeImpl(const byte_t* const data, const size_t count);

private:

	const vmime::utility::file::path m_path;
	HANDLE m_hFile;
};


} // windows
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_FILESYSTEM_FEATURES

#endif // VMIME_PLATFORMS_WINDOWS_FILE_HPP_INCLUDED
