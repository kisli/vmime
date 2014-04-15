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


#if VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_FILESYSTEM_FEATURES


#include "vmime/platforms/windows/windowsFile.hpp"

#include <windows.h>
#include <string.h>

#include "vmime/exception.hpp"
#include "vmime/utility/stringUtils.hpp"


namespace vmime {
namespace platforms {
namespace windows {


shared_ptr <vmime::utility::file> windowsFileSystemFactory::create(const vmime::utility::file::path& path) const
{
	return make_shared <windowsFile>(path);
}


const vmime::utility::file::path windowsFileSystemFactory::stringToPath(const vmime::string& str) const
{
	return (stringToPathImpl(str));
}


const vmime::string windowsFileSystemFactory::pathToString(const vmime::utility::file::path& path) const
{
	return (pathToStringImpl(path));
}


const vmime::utility::file::path windowsFileSystemFactory::stringToPathImpl(const vmime::string& str)
{
	vmime::size_t offset = 0;
	vmime::size_t prev = 0;

	vmime::utility::file::path path;

	while ((offset = str.find_first_of("\\", offset)) != vmime::string::npos)
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


const vmime::string windowsFileSystemFactory::pathToStringImpl(const vmime::utility::file::path& path)
{
	vmime::string native = "";

	for (int i = 0 ; i < path.getSize() ; ++i)
	{
		if (i > 0)
			native += "\\";

		native += path[i].getBuffer();
	}

	return (native);
}

bool windowsFileSystemFactory::isValidPathComponent(const vmime::utility::file::path::component& comp) const
{
	return isValidPathComponent(comp, false);
}

bool windowsFileSystemFactory::isValidPathComponent(
	const vmime::utility::file::path::component& comp,
	bool firstComponent) const
{
	const string& buffer = comp.getBuffer();

	// If first component, check if component is a drive
	if (firstComponent && (buffer.length() == 2) && (buffer[1] == ':'))
	{
		char drive = tolower(buffer[0]);
		if ((drive >= 'a') && (drive <= 'z'))
			return true;
	}

	// Check for invalid characters
	for (size_t i = 0 ; i < buffer.length() ; ++i)
	{
		const unsigned char c = buffer[i];

		switch (c)
		{
		// Reserved characters
		case '<': case '>': case ':':
		case '"': case '/': case '\\':
		case '|': case '$': case '*':

			return false;

		default:

			if (c <= 31)
				return false;
		}
	}

	string upperBuffer = vmime::utility::stringUtils::toUpper(buffer);

	// Check for reserved names
	if (upperBuffer.length() == 3)
	{
		if (upperBuffer == "CON" || buffer == "PRN" || buffer == "AUX" || buffer == "NUL")
			return false;
	}
	else if (upperBuffer.length() == 4)
	{
		if ((upperBuffer.substr(0, 3) == "COM") && // COM0 to COM9
		    (upperBuffer[3] >= '0') && (upperBuffer[3] <= '9'))
		{
			return false;
		}
		else if ((upperBuffer.substr(0, 3) == "LPT") && // LPT0 to LPT9
		         (upperBuffer[3] >= '0') && (upperBuffer[3] <= '9'))
		{
			return false;
		}
	}
	return true;
}


bool windowsFileSystemFactory::isValidPath(const vmime::utility::file::path& path) const
{
	for (int i = 0 ; i < path.getSize() ; ++i)
	{
		if (!isValidPathComponent(path[i], (i==0)))
			return false;
	}

	return true;
}


void windowsFileSystemFactory::reportError(const vmime::utility::path& path, const int err)
{
	vmime::string desc;

	LPVOID lpMsgBuf;
	if (FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL ))
	{
		desc = (char*)lpMsgBuf;
		LocalFree( lpMsgBuf );
	}

	throw vmime::exceptions::filesystem_exception(desc, path);
}

windowsFile::windowsFile(const vmime::utility::file::path& path)
: m_path(path), m_nativePath(windowsFileSystemFactory::pathToStringImpl(path))
{
}

void windowsFile::createFile()
{
	HANDLE hFile = CreateFile(
		m_nativePath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		windowsFileSystemFactory::reportError(m_path, GetLastError());

	CloseHandle(hFile);
}

void windowsFile::createDirectory(const bool createAll)
{
	createDirectoryImpl(m_path, m_path, createAll);
}

bool windowsFile::isFile() const
{
	DWORD dwFileAttribute = GetFileAttributes(m_nativePath.c_str());
	if (dwFileAttribute == INVALID_FILE_ATTRIBUTES)
		return false;
	return (dwFileAttribute & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool windowsFile::isDirectory() const
{
	DWORD dwFileAttribute = GetFileAttributes(m_nativePath.c_str());
	if (dwFileAttribute == INVALID_FILE_ATTRIBUTES)
		return false;
	return (dwFileAttribute & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
}

bool windowsFile::canRead() const
{
	HANDLE hFile = CreateFile(
		m_nativePath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	CloseHandle(hFile);
	return true;
}

bool windowsFile::canWrite() const
{
	HANDLE hFile = CreateFile(
		m_nativePath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	CloseHandle(hFile);
	return true;
}

windowsFile::length_type windowsFile::getLength()
{
	HANDLE hFile = CreateFile(
		m_nativePath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		windowsFileSystemFactory::reportError(m_path, GetLastError());

	DWORD dwSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);

	return dwSize;
}

const vmime::utility::path& windowsFile::getFullPath() const
{
	return m_path;
}

bool windowsFile::exists() const
{
	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(m_nativePath.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return true;
	}
	return false;
}

shared_ptr <vmime::utility::file> windowsFile::getParent() const
{
	if (m_path.isEmpty())
		return null;
	else
		return make_shared <windowsFile>(m_path.getParent());
}

void windowsFile::rename(const path& newName)
{
	const vmime::string newNativeName = windowsFileSystemFactory::pathToStringImpl(newName);
	if (MoveFile(m_nativePath.c_str(), newNativeName.c_str()))
	{
		m_path = newName;
		m_nativePath = newNativeName;
	}
	else
		windowsFileSystemFactory::reportError(m_path, GetLastError());
}

void windowsFile::remove()
{
	if (!DeleteFile(m_nativePath.c_str()))
		windowsFileSystemFactory::reportError(m_path, GetLastError());
}

shared_ptr <vmime::utility::fileWriter> windowsFile::getFileWriter()
{
	return make_shared <windowsFileWriter>(m_path, m_nativePath);
}

shared_ptr <vmime::utility::fileReader> windowsFile::getFileReader()
{
	return make_shared <windowsFileReader>(m_path, m_nativePath);
}

shared_ptr <vmime::utility::fileIterator> windowsFile::getFiles() const
{
	return make_shared <windowsFileIterator>(m_path, m_nativePath);
}

void windowsFile::createDirectoryImpl(const vmime::utility::file::path& fullPath, const vmime::utility::file::path& path, const bool recursive)
{
	const vmime::string nativePath = windowsFileSystemFactory::pathToStringImpl(path);

	windowsFile tmp(path);
	if (tmp.isDirectory())
		return;

	if (!path.isEmpty() && recursive)
		createDirectoryImpl(fullPath, path.getParent(), true);

	if (!CreateDirectory(nativePath.c_str(), NULL))
		windowsFileSystemFactory::reportError(fullPath, GetLastError());
}

windowsFileIterator::windowsFileIterator(const vmime::utility::file::path& path, const vmime::string& nativePath)
: m_path(path), m_nativePath(nativePath), m_moreElements(false), m_hFind(INVALID_HANDLE_VALUE)
{
	findFirst();
}

windowsFileIterator::~windowsFileIterator()
{
	if (m_hFind != INVALID_HANDLE_VALUE)
		FindClose(m_hFind);
}

bool windowsFileIterator::hasMoreElements() const
{
	return m_moreElements;
}

shared_ptr <vmime::utility::file> windowsFileIterator::nextElement()
{
	shared_ptr <vmime::utility::file> pFile = make_shared <windowsFile>
		(m_path / vmime::utility::file::path::component(m_findData.cFileName));

	findNext();

	return pFile;
}

void windowsFileIterator::findFirst()
{
	m_hFind = FindFirstFile(m_nativePath.c_str(), &m_findData);
	if (m_hFind == INVALID_HANDLE_VALUE)
	{
		m_moreElements = false;
		return;
	}

	m_moreElements = true;
	if (isCurrentOrParentDir())
		findNext();
}

void windowsFileIterator::findNext()
{
	do
	{
		if (!FindNextFile(m_hFind, &m_findData))
		{
			m_moreElements = false;
			return;
		}
	}
	while (isCurrentOrParentDir());
}

bool windowsFileIterator::isCurrentOrParentDir() const
{
	vmime::string s(m_findData.cFileName);
	if ((s == ".") || (s == ".."))
		return true;
	return false;
}

windowsFileReader::windowsFileReader(const vmime::utility::file::path& path, const vmime::string& nativePath)
: m_path(path), m_nativePath(nativePath)
{
}

shared_ptr <vmime::utility::inputStream> windowsFileReader::getInputStream()
{
	HANDLE hFile = CreateFile(
		m_nativePath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		windowsFileSystemFactory::reportError(m_path, GetLastError());
	return make_shared <windowsFileReaderInputStream>(m_path, hFile);
}

windowsFileReaderInputStream::windowsFileReaderInputStream(const vmime::utility::file::path& path, HANDLE hFile)
: m_path(path), m_hFile(hFile)
{
}

windowsFileReaderInputStream::~windowsFileReaderInputStream()
{
	CloseHandle(m_hFile);
}

bool windowsFileReaderInputStream::eof() const
{
	DWORD dwSize = GetFileSize(m_hFile, NULL);
	DWORD dwPosition = SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
	return (dwSize == dwPosition);
}

void windowsFileReaderInputStream::reset()
{
	SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);
}

size_t windowsFileReaderInputStream::read(byte_t* const data, const size_t count)
{
	DWORD dwBytesRead;
	if (!ReadFile(m_hFile, (LPVOID)data, (DWORD)count, &dwBytesRead, NULL))
		windowsFileSystemFactory::reportError(m_path, GetLastError());
	return dwBytesRead;
}

size_t windowsFileReaderInputStream::skip(const size_t count)
{
	DWORD dwCurPos = SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
	DWORD dwNewPos = SetFilePointer(m_hFile, (LONG)count, NULL, FILE_CURRENT);
	return (dwNewPos - dwCurPos);
}

size_t windowsFileReaderInputStream::getPosition() const
{
	DWORD dwCurPos = SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);

	if (dwCurPos == INVALID_SET_FILE_POINTER)
		windowsFileSystemFactory::reportError(m_path, GetLastError());

	return static_cast <size_t>(dwCurPos);
}

void windowsFileReaderInputStream::seek(const size_t pos)
{
	DWORD dwNewPos = SetFilePointer(m_hFile, (LONG)pos, NULL, FILE_BEGIN);

	if (dwNewPos == INVALID_SET_FILE_POINTER)
		windowsFileSystemFactory::reportError(m_path, GetLastError());
}

windowsFileWriter::windowsFileWriter(const vmime::utility::file::path& path, const vmime::string& nativePath)
: m_path(path), m_nativePath(nativePath)
{
}

shared_ptr <vmime::utility::outputStream> windowsFileWriter::getOutputStream()
{
	HANDLE hFile = CreateFile(
		m_nativePath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		windowsFileSystemFactory::reportError(m_path, GetLastError());
	return make_shared <windowsFileWriterOutputStream>(m_path, hFile);
}

windowsFileWriterOutputStream::windowsFileWriterOutputStream(const vmime::utility::file::path& path, HANDLE hFile)
: m_path(path), m_hFile(hFile)
{
}

windowsFileWriterOutputStream::~windowsFileWriterOutputStream()
{
	CloseHandle(m_hFile);
}

void windowsFileWriterOutputStream::writeImpl(const byte_t* const data, const size_t count)
{
	DWORD dwBytesWritten;
	if (!WriteFile(m_hFile, data, (DWORD)count, &dwBytesWritten, NULL))
		windowsFileSystemFactory::reportError(m_path, GetLastError());
}


void windowsFileWriterOutputStream::flush()
{
	// TODO
}


} // windows
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_FILESYSTEM_FEATURES

