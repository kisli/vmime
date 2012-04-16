//
// VMime library (http://vmime.sourceforge.net)
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

#ifndef VMIME_PLATFORMS_WINDOWS_FILE_HPP_INCLUDED
#define VMIME_PLATFORMS_WINDOWS_FILE_HPP_INCLUDED


#include "vmime/utility/file.hpp"
#include "vmime/utility/seekableInputStream.hpp"

#include <windows.h>

#if VMIME_HAVE_FILESYSTEM_FEATURES


namespace vmime {
namespace platforms {
namespace windows {


class windowsFileSystemFactory : public vmime::utility::fileSystemFactory
{
public:

	ref <vmime::utility::file> create(const vmime::utility::file::path& path) const;

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

	ref <file> getParent() const;

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


class windowsFileIterator : public vmime::utility::fileIterator
{
public:

	windowsFileIterator(const vmime::utility::file::path& path, const vmime::string& nativePath);
	~windowsFileIterator();

	bool hasMoreElements() const;
	vmime::ref <vmime::utility::file> nextElement();

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

	ref <vmime::utility::inputStream> getInputStream();

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
	size_type read(value_type* const data, const size_type count);
	size_type skip(const size_type count);
	size_type getPosition() const;
	void seek(const size_type pos);

private:

	const vmime::utility::file::path m_path;
	HANDLE m_hFile;
};


class windowsFileWriter : public vmime::utility::fileWriter
{
public:

	windowsFileWriter(const vmime::utility::file::path& path, const vmime::string& nativePath);

public:

	ref <vmime::utility::outputStream> getOutputStream();

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

	void write(const value_type* const data, const size_type count);
	void flush();

private:

	const vmime::utility::file::path m_path;
	HANDLE m_hFile;
};


} // windows
} // platforms
} // vmime


#endif // VMIME_HAVE_FILESYSTEM_FEATURES

#endif // VMIME_PLATFORMS_WINDOWS_FILE_HPP_INCLUDED
