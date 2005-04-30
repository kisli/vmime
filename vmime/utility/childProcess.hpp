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

#ifndef VMIME_UTILITY_CHILDPROCESS_HPP_INCLUDED
#define VMIME_UTILITY_CHILDPROCESS_HPP_INCLUDED


#include "vmime/utility/stream.hpp"
#include "vmime/utility/file.hpp"

#include <vector>


namespace vmime {
namespace utility {


/** Spawn a process and redirect its standard input
  * and/or standard output.
  */

class childProcess
{
public:

	virtual ~childProcess() { }

	/** Flags used with start(). */
	enum Flags
	{
		FLAG_REDIRECT_STDIN = (1 << 0),
		FLAG_REDIRECT_STDOUT = (1 << 1)
	};

	/** Start the child process.
	  *
	  * @param args list of arguments
	  * @param flags one or more childProcess::Flags
	  * @throws exceptions::system_error if the an error occurs
	  * before the process can be started
	  */
	virtual void start(const std::vector <string> args, const int flags = 0) = 0;

	/** Return a wrapper to the child process standard input.
	  *
	  * @return output stream wrapper for child's stdin
	  */
	virtual utility::outputStream* getStdIn() = 0;

	/** Return a wrapper to the child process standard output.
	  *
	  * @return input stream wrapper for child's stdout
	  */
	virtual utility::inputStream* getStdOut() = 0;

	/** Wait for the process to finish.
	  *
	  * @throws exceptions::system_error if the process does
	  * not exit normally
	  */
	virtual void waitForFinish() = 0;
};


/** Create 'childProcess' objects.
  */

class childProcessFactory
{
public:

	virtual ~childProcessFactory() { }

	/** Create a new child process.
	  *
	  * @param path full path of the process executable file
	  */
	virtual childProcess* create(const utility::file::path& path) const = 0;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_CHILDPROCESS_HPP_INCLUDED

