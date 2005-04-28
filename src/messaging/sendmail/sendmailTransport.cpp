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

#include "vmime/messaging/sendmail/sendmailTransport.hpp"

#include "vmime/exception.hpp"
#include "vmime/platformDependant.hpp"
#include "vmime/message.hpp"
#include "vmime/mailboxList.hpp"

#include "vmime/messaging/authHelper.hpp"

#include "vmime/utility/filteredStream.hpp"


#if VMIME_BUILTIN_PLATFORM_POSIX


#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>


namespace vmime {
namespace messaging {
namespace sendmail {


sendmailTransport::sendmailTransport(session* sess, authenticator* auth)
	: transport(sess, getInfosInstance(), auth), m_connected(false)
{
}


sendmailTransport::~sendmailTransport()
{
	if (isConnected())
		disconnect();
}


const string sendmailTransport::getProtocolName() const
{
	return "sendmail";
}


void sendmailTransport::connect()
{
	if (isConnected())
		throw exceptions::already_connected();

	// Use the specified path for 'sendmail' or a default one if no path is specified
	m_sendmailPath = getSession()->getProperties().getProperty
		(sm_infos.getPropertyPrefix() + "binpath", string(VMIME_SENDMAIL_PATH));

	m_connected = true;
}


const bool sendmailTransport::isConnected() const
{
	return (m_connected);
}


void sendmailTransport::disconnect()
{
	if (!isConnected())
		throw exceptions::not_connected();

	internalDisconnect();
}


void sendmailTransport::internalDisconnect()
{
	m_connected = false;
}


void sendmailTransport::noop()
{
	// Do nothing
}


void sendmailTransport::send
	(const mailbox& expeditor, const mailboxList& recipients,
	 utility::inputStream& is, const utility::stream::size_type size,
         utility::progressionListener* progress)
{
	// If no recipient/expeditor was found, throw an exception
	if (recipients.isEmpty())
		throw exceptions::no_recipient();
	else if (expeditor.isEmpty())
		throw exceptions::no_expeditor();

	// Construct the argument list
	std::vector <string> args;

	args.push_back("-i");
	args.push_back("-f");
	args.push_back(expeditor.getEmail());
	args.push_back("--");

	for (int i = 0 ; i < recipients.getMailboxCount() ; ++i)
		args.push_back(recipients.getMailboxAt(i)->getEmail());

	// Call sendmail
	try
	{
		internalSend(args, is, size, progress);
	}
	catch (vmime::exception& e)
	{
		throw exceptions::command_error("SEND", "", "sendmail failed", e);
	}
}


static const string getSignalMessage(const int num)
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


static const string getErrorMessage(const int num)
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



#ifndef VMIME_BUILDING_DOC

// Output stream adapter for UNIX pipe

class outputStreamPipeAdapter : public utility::outputStream
{
public:

	outputStreamPipeAdapter(const int desc)
		: m_desc(desc)
	{
	}

	void write(const value_type* const data, const size_type count)
	{
		if (::write(m_desc, data, count) == -1)
		{
			const string errorMsg = getErrorMessage(errno);
			throw exceptions::system_error(errorMsg);
		}
	}

private:

	int m_desc;
};

#endif // VMIME_BUILDING_DOC



// The following code is highly inspired and adapted from the 'sendmail'
// provider module in Evolution data server code.
//
// Original authors: Dan Winship <danw@ximian.com>
// Copyright 2000 Ximian, Inc. (www.ximian.com)

void sendmailTransport::internalSend
	(const std::vector <string> args, utility::inputStream& is,
	 const utility::stream::size_type size, utility::progressionListener* progress)
{
	// Construct C-style argument array
	const char** argv = new const char*[args.size() + 2];

	argv[0] = "sendmail";
	argv[args.size()] = NULL;

	for (unsigned int i = 0 ; i < args.size() ; ++i)
		argv[i + 1] = args[i].c_str();

	// Create a pipe to communicate with sendmail
	int fd[2];

	if (pipe(fd) == -1)
	{
		throw exceptions::system_error(getErrorMessage(errno));
	}

	// Block SIGCHLD so the calling application doesn't notice
	// sendmail exiting before we do
	sigset_t mask, oldMask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask, &oldMask);

	// Spawn 'sendmail' process
	pid_t pid = fork();

	if (pid == -1)  // error
	{
		const string errorMsg = getErrorMessage(errno);

		sigprocmask(SIG_SETMASK, &oldMask, NULL);

		close(fd[0]);
		close(fd[1]);

		throw exceptions::system_error(errorMsg);
	}
	else if (pid == 0)  // child process
	{
		dup2(fd[0], STDIN_FILENO);
		close(fd[1]);

		execv(m_sendmailPath.c_str(), const_cast <char**>(argv));
		_exit(255);
	}

	close(fd[0]);

	// Copy message data from input stream to output pipe
	try
	{
		outputStreamPipeAdapter pos(fd[1]);

		// Workaround for lame sendmail implementations that
		// can't handle CRLF eoln sequences: we transform CRLF
		// sequences into LF characters.
		utility::CRLFToLFFilteredOutputStream fos(pos);

		// TODO: remove 'Bcc:' field from message header

		utility::bufferedStreamCopy(is, fos, size, progress);
	}
	catch (exception& e)
	{
		close(fd[1]);

		int wstat;

		while (waitpid(pid, &wstat, 0) == -1 && errno == EINTR)
			;

		sigprocmask(SIG_SETMASK, &oldMask, NULL);

		throw;
	}

	close(fd[1]);

	// Wait for sendmail to exit
	int wstat;

	while (waitpid(pid, &wstat, 0) == -1 && errno == EINTR)
		;

	sigprocmask(SIG_SETMASK, &oldMask, NULL);

	if (!WIFEXITED(wstat))
	{
		throw exceptions::system_error("sendmail exited with signal "
			+ getSignalMessage(WTERMSIG(wstat)) + ", mail not sent");
	}
	else if (WEXITSTATUS(wstat) != 0)
	{
		if (WEXITSTATUS(wstat) == 255)
		{
			std::ostringstream oss;
			oss << "Could not execute '" << m_sendmailPath;
			oss << "', mail not sent";

			throw exceptions::system_error(oss.str());
		}
		else
		{
			std::ostringstream oss;
			oss << "sendmail exited with status " << WEXITSTATUS(wstat);
			oss << ", mail not sent";

			throw exceptions::system_error(oss.str());
		}
	}
}


// Service infos

sendmailTransport::_infos sendmailTransport::sm_infos;


const serviceInfos& sendmailTransport::getInfosInstance()
{
	return (sm_infos);
}


const serviceInfos& sendmailTransport::getInfos() const
{
	return (sm_infos);
}


const port_t sendmailTransport::_infos::getDefaultPort() const
{
	return (0);
}


const string sendmailTransport::_infos::getPropertyPrefix() const
{
	return "transport.sendmail.";
}


const std::vector <string> sendmailTransport::_infos::getAvailableProperties() const
{
	std::vector <string> list;

	// Path to sendmail (override default)
	list.push_back("binpath");

	return (list);
}


} // sendmail
} // messaging
} // vmime


#endif // VMIME_BUILTIN_PLATFORM_POSIX
