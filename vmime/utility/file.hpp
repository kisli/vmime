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

#ifndef VMIME_UTILITY_FILE_HPP_INCLUDED
#define VMIME_UTILITY_FILE_HPP_INCLUDED


#include "vmime/config.hpp"

#include "vmime/utility/path.hpp"
#include "vmime/utility/stream.hpp"


#if VMIME_HAVE_FILESYSTEM_FEATURES


namespace vmime {
namespace utility {


class file;


/** File list iterator (see file::getFiles).
  */

class fileIterator : public object
{
public:

	virtual ~fileIterator() { }

	/** Check whether the cursor has reach the end of the list.
	  *
	  * @return true if you can call nextElement(), or false
	  * if no more file is available
	  */
	virtual bool hasMoreElements() const = 0;

	/** Return the next file in the list.
	  *
	  * @return next file or NULL
	  */
	virtual ref <file> nextElement() = 0;
};


/** Write to a file.
  */

class fileWriter : public object
{
public:

	virtual ~fileWriter() { }

	virtual ref <utility::outputStream> getOutputStream() = 0;
};


/** Read from a file.
  */

class fileReader : public object
{
public:

	virtual ~fileReader() { }

	virtual ref <utility::inputStream> getInputStream() = 0;
};


/** Abstract representation of a file or directory.
  */

class file : public object
{
public:

	typedef utility::path path;
	typedef unsigned long length_type;


	virtual ~file() { }


	/** Create the file pointed by this file object.
	  *
	  * @throw exceptions::filesystem_exception if an error occurs
	  */
	virtual void createFile() = 0;

	/** Create the directory pointed by this file object.
	  *
	  * @param createAll if set to true, recursively create all
	  * parent directories if they do not exist
	  * @throw exceptions::filesystem_exception if an error occurs
	  */
	virtual void createDirectory(const bool createAll = false) = 0;

	/** Test whether this is a file.
	  *
	  * @return true if this is a file, false otherwise
	  */
	virtual bool isFile() const = 0;

	/** Test whether this is a directory.
	  *
	  * @return true if this is a directory, false otherwise
	  */
	virtual bool isDirectory() const = 0;

	/** Test whether this file is readible.
	  *
	  * @return true if we can read this file, false otherwise
	  */
	virtual bool canRead() const = 0;

	/** Test whether this file is writeable.
	  *
	  * @return true if we can write to this file, false otherwise
	  */
	virtual bool canWrite() const = 0;

	/** Return the length of this file.
	  *
	  * @return file size (in bytes)
	  */
	virtual length_type getLength() = 0;

	/** Return the full path of this file/directory.
	  *
	  * @return full path of the file
	  */
	virtual const path& getFullPath() const = 0;

	/** Test whether this file/directory exists.
	  *
	  * @return true if the file exists, false otherwise
	  */
	virtual bool exists() const = 0;

	/** Return the parent directory of this file/directory.
	  *
	  * @return parent directory (or NULL if root)
	  */
	virtual ref <file> getParent() const = 0;

	/** Rename the file/directory.
	  *
	  * @param newName full path of the new file
	  * @throw exceptions::filesystem_exception if an error occurs
	  */
	virtual void rename(const path& newName) = 0;

	/** Deletes this file/directory.
	  * If this is a directory, it must be empty.
	  *
	  * @throw exceptions::filesystem_exception if an error occurs
	  */
	virtual void remove() = 0;

	/** Return an object capable of writing to this file.
	  *
	  * @return file writer object
	  */
	virtual ref <fileWriter> getFileWriter() = 0;

	/** Return an object capable of reading from this file.
	  *
	  * @return file reader object
	  */
	virtual ref <fileReader> getFileReader() = 0;

	/** Enumerate files contained in this directory.
	  *
	  * @return file iterator to enumerate files
	  * @throw exceptions::not_a_directory if this is not a directory,
	  * exceptions::filesystem_exception if another error occurs
	  */
	virtual ref <fileIterator> getFiles() const = 0;

protected:

	file() { }

private:

	file(const file&) : object() { }
};


/** Constructs 'file' objects.
  */

class fileSystemFactory : public object
{
public:

	virtual ~fileSystemFactory() { }

	/** Create a new file object from the specified path.
	  *
	  * @param path full path (absolute) of the file
	  * @return new file object for the path
	  */
	virtual ref <file> create(const file::path& path) const = 0;

	/** Parse a path contained in a string.
	  *
	  * @param str string containing a path in a system-dependent representation
	  * @return path object (abstract representation)
	  */
	virtual const file::path stringToPath(const string& str) const = 0;

	/** Return the system-dependent string representation for the specified path.
	  *
	  * @param path abstract representation of the path
	  * @return string representation of the path
	  */
	virtual const string pathToString(const file::path& path) const = 0;

	/** Test whether the specified path component is syntactically
	  * valid (ie: does not contain any 'special' character).
	  *
	  * @param comp path component to test
	  * @return true if the component is valid, false otherwise
	  */
	virtual bool isValidPathComponent(const file::path::component& comp) const = 0;

	/** Test whether the specified path is syntactically valid
	  * (ie: components do not contain any 'special' character).
	  *
	  * @param path path to test
	  * @return true if the path is valid, false otherwise
	  */
	virtual bool isValidPath(const file::path& path) const = 0;
};


} // utility
} // vmime


#endif // VMIME_HAVE_FILESYSTEM_FEATURES


#endif // VMIME_UTILITY_FILE_HPP_INCLUDED
