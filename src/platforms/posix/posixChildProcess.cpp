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

#include "vmime/platforms/posix/posixChildProcess.hpp"
#include "vmime/platforms/posix/posixFile.hpp"

#include "vmime/exception.hpp"

#include "vmime/utility/smartPtr.hpp"

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>


namespace vmime {
namespace platforms {
namespace posix {


// posixChildProcessFactory

ref <utility::childProcess> posixChildProcessFactory::create(const utility::file::path& path) const
{
	return vmime::create <posixChildProcess>(path);
}



#ifndef VMIME_BUILDING_DOC


// getPosixSignalMessage
// Returns the name of a POSIX signal.

static const string getPosixSignalMessage(const int num)
{
	switch (num)
	{
	case SIGHUP:  return "SIGHUP";
	case SIGINT:  return "SIGINT";
	case SIGQUIT: return "SIGQUIT";
	case SIGILL:  return "SIGILL";
	case SIGABRT: return "SIGABRT";
	case SIGFPE:  return "SIGFPE";
	case SIGKILL: return "SIGKILL";
	case SIGSEGV: return "SIGSEGV";
	case SIGPIPE: return "SIGPIPE";
	case SIGALRM: return "SIGALRM";
	case SIGTERM: return "SIGTERM";
	case SIGUSR1: return "SIGUSR1";
	case SIGUSR2: return "SIGUSR2";
	case SIGCHLD: return "SIGCHLD";
	case SIGCONT: return "SIGCONT";
	case SIGSTOP: return "SIGSTOP";
	case SIGTSTP: return "SIGTSTP";
	case SIGTTIN: return "SIGTTIN";
	case SIGTTOU: return "SIGTTOU";
	}

	return "(unknown)";
}


// getPosixErrorMessage
// Returns a message corresponding to an error code.

static const string getPosixErrorMessage(const int num)
{
#ifdef strerror_r
	char res[256];
	res[0] = '\0';

	strerror_r(num, res, sizeof(res));

	return string(res);
#else
	return string(strerror(num));
#endif
}


// Output stream adapter for POSIX pipe

class outputStreamPosixPipeAdapter : public utility::outputStream
{
public:

	outputStreamPosixPipeAdapter(const int desc)
		: m_desc(desc)
	{
	}

	void write(const value_type* const data, const size_type count)
	{
		if (::write(m_desc, data, count) == -1)
		{
			const string errorMsg = getPosixErrorMessage(errno);
			throw exceptions::system_error(errorMsg);
		}
	}

	void flush()
	{
		::fsync(m_desc);
	}

private:

	const int m_desc;
};


// Input stream adapter for POSIX pipe

class inputStreamPosixPipeAdapter : public utility::inputStream
{
public:

	inputStreamPosixPipeAdapter(const int desc)
		: m_desc(desc)
	{
	}

	bool eof() const
	{
		return (m_eof);
	}

	void reset()
	{
		// Do nothing: unsupported
	}

	size_type skip(const size_type count)
	{
		// TODO: not tested
		value_type buffer[4096];

		int bytesSkipped = 0;
		int bytesRead = 0;

		while ((bytesRead = ::read(m_desc, buffer,
			std::min(sizeof(buffer), count - bytesSkipped))) != 0)
		{
			if (bytesRead == -1)
			{
				const string errorMsg = getPosixErrorMessage(errno);
				throw exceptions::system_error(errorMsg);
			}

			bytesSkipped += bytesRead;
		}

		return static_cast <size_type>(bytesSkipped);
	}

	size_type read(value_type* const data, const size_type count)
	{
		int bytesRead = 0;

		if ((bytesRead = ::read(m_desc, data, count)) == -1)
		{
			const string errorMsg = getPosixErrorMessage(errno);
			throw exceptions::system_error(errorMsg);
		}

		m_eof = (bytesRead == 0);

		return static_cast <size_type>(bytesRead);
	}

private:

	const int m_desc;

	bool m_eof;
};


#endif // VMIME_BUILDING_DOC



// posixChildProcess

posixChildProcess::posixChildProcess(const utility::file::path& path)
	: m_processPath(path), m_started(false),
	  m_stdIn(NULL), m_stdOut(NULL), m_pid(0), m_argArray(NULL)
{
	m_pipe[0] = 0;
	m_pipe[1] = 0;

	sigemptyset(&m_oldProcMask);
}


posixChildProcess::~posixChildProcess()
{
	if (m_started)
		sigprocmask(SIG_SETMASK, &m_oldProcMask, NULL);

	if (m_pipe[0] != 0)
		close(m_pipe[0]);

	if (m_pipe[1] != 0)
		close(m_pipe[1]);

	delete [] (m_argArray);
}


// The following code is highly inspired and adapted from the 'sendmail'
// provider module in Evolution data server code.
//
// Original authors: Dan Winship <danw@ximian.com>
// Copyright 2000 Ximian, Inc. (www.ximian.com)

void posixChildProcess::start(const std::vector <string> args, const int flags)
{
	if (m_started)
		return;

	// Construct C-style argument array
	const char** argv = new const char*[args.size() + 2];

	m_argVector = args;  // for c_str() pointer to remain valid
	m_argArray = argv;   // to free later

	argv[0] = m_processPath.getLastComponent().getBuffer().c_str();
	argv[args.size() + 1] = NULL;

	for (unsigned int i = 0 ; i < m_argVector.size() ; ++i)
		argv[i + 1] = m_argVector[i].c_str();

	// Create a pipe to communicate with the child process
	int fd[2];

	if (pipe(fd) == -1)
	{
		throw exceptions::system_error(getPosixErrorMessage(errno));
	}

	m_pipe[0] = fd[0];
	m_pipe[1] = fd[1];

	// Block SIGCHLD so the calling application doesn't notice
	// process exiting before we do
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &m_oldProcMask);

	// Spawn process
	const pid_t pid = fork();

	if (pid == -1)  // error
	{
		const string errorMsg = getPosixErrorMessage(errno);

		sigprocmask(SIG_SETMASK, &m_oldProcMask, NULL);

		close(fd[0]);
		close(fd[1]);

		throw exceptions::system_error(errorMsg);
	}
	else if (pid == 0)  // child process
	{
		if (flags & FLAG_REDIRECT_STDIN)
			dup2(fd[0], STDIN_FILENO);
		else
			close(fd[0]);

		if (flags & FLAG_REDIRECT_STDOUT)
			dup2(fd[1], STDOUT_FILENO);
		else
			close(fd[1]);

		posixFileSystemFactory* pfsf = new posixFileSystemFactory();

		const string path = pfsf->pathToString(m_processPath);

		delete (pfsf);

		execv(path.c_str(), const_cast <char**>(argv));
		_exit(255);
	}

	if (flags & FLAG_REDIRECT_STDIN)
	{
		m_stdIn = vmime::create <outputStreamPosixPipeAdapter>(m_pipe[1]);
	}
	else
	{
		close(m_pipe[1]);
		m_pipe[1] = 0;
	}

	if (flags & FLAG_REDIRECT_STDOUT)
	{
		m_stdOut = vmime::create <inputStreamPosixPipeAdapter>(m_pipe[0]);
	}
	else
	{
		close(m_pipe[0]);
		m_pipe[0] = 0;
	}

	m_pid = pid;
	m_started = true;
}


ref <utility::outputStream> posixChildProcess::getStdIn()
{
	return (m_stdIn);
}


ref <utility::inputStream> posixChildProcess::getStdOut()
{
	return (m_stdOut);
}


void posixChildProcess::waitForFinish()
{
	// Close stdin pipe
	if (m_pipe[1] != 0)
	{
		close(m_pipe[1]);
		m_pipe[1] = 0;
	}

	int wstat;

	while (waitpid(m_pid, &wstat, 0) == -1 && errno == EINTR)
		;

	if (!WIFEXITED(wstat))
	{
		throw exceptions::system_error("Process exited with signal "
			+ getPosixSignalMessage(WTERMSIG(wstat)));
	}
	else if (WEXITSTATUS(wstat) != 0)
	{
		if (WEXITSTATUS(wstat) == 255)
		{
			vmime::utility::auto_ptr <posixFileSystemFactory> pfsf
				= new posixFileSystemFactory();

			throw exceptions::system_error("Could not execute '"
				+ pfsf->pathToString(m_processPath) + "'");
		}
		else
		{
			std::ostringstream oss;
			oss.imbue(std::locale::classic());

			oss << "Process exited with status " << WEXITSTATUS(wstat);

			throw exceptions::system_error(oss.str());
		}
	}
}


} // posix
} // platforms
} // vmime

