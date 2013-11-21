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

#ifndef VMIME_PLATFORMS_POSIX_POSIXCHILDPROCESS_HPP_INCLUDED
#define VMIME_PLATFORMS_POSIX_POSIXCHILDPROCESS_HPP_INCLUDED


#include "vmime/config.hpp"


#if VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_FILESYSTEM_FEATURES


#include "vmime/utility/childProcess.hpp"

#include <sys/types.h>
#include <signal.h>


namespace vmime {
namespace platforms {
namespace posix {


class posixChildProcess : public utility::childProcess
{
public:

	posixChildProcess(const utility::file::path& path);
	~posixChildProcess();

	void start(const std::vector <string> args, const int flags = 0);

	shared_ptr <utility::outputStream> getStdIn();
	shared_ptr <utility::inputStream> getStdOut();

	void waitForFinish();

private:

	utility::file::path m_processPath;
	bool m_started;

	shared_ptr <utility::outputStream> m_stdIn;
	shared_ptr <utility::inputStream> m_stdOut;

	sigset_t m_oldProcMask;
	pid_t m_pid;
	int m_pipe[2];

	std::vector <string> m_argVector;
	const char** m_argArray;
};


class posixChildProcessFactory : public utility::childProcessFactory
{
public:

	shared_ptr <utility::childProcess> create(const utility::file::path& path) const;
};


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_POSIX && VMIME_HAVE_FILESYSTEM_FEATURES

#endif // VMIME_PLATFORMS_POSIX_POSIXCHILDPROCESS_HPP_INCLUDED

