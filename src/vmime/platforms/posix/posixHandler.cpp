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


#if VMIME_PLATFORM_IS_POSIX


#include "vmime/platforms/posix/posixHandler.hpp"

#include "vmime/platforms/posix/posixCriticalSection.hpp"

#include "vmime/utility/stringUtils.hpp"

#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <locale.h>
#include <langinfo.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#if VMIME_HAVE_SYSCALL
#	include <sys/syscall.h>
#endif

#include <netdb.h>

#include <string.h>
#include <cassert>
#include <cstdlib>

#if VMIME_HAVE_PTHREAD
#	include <pthread.h>
#endif // VMIME_HAVE_PTHREAD

/*
#ifdef _POSIX_PRIORITY_SCHEDULING
	#include <sched.h>
#endif // _POSIX_PRIORITY_SCHEDULING
*/


#if VMIME_HAVE_PTHREAD

namespace
{
	// This construction ensures mutex will be initialized in compile-time
	// and will be available any time in the runtime.
	pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

	// Helper lock, to be exception safe all the time.
	class PLockHelper
	{
	public:

		PLockHelper()
		{
			if (pthread_mutex_lock(&g_mutex) != 0)
				assert(!"unable to lock mutex - thread safety's void");
		}

		~PLockHelper()
		{
			if (pthread_mutex_unlock(&g_mutex) != 0)
				assert(!"unable to unlock mutex - application's dead...");
		}

	private:

		// Object cannot be copied
		PLockHelper(const PLockHelper&);
		const PLockHelper& operator=(const PLockHelper&);
	};

} // unnamed namespace

#endif // VMIME_HAVE_PTHREAD


namespace vmime {
namespace platforms {
namespace posix {


posixHandler::posixHandler()
{
#if VMIME_HAVE_MESSAGING_FEATURES
	m_socketFactory = make_shared <posixSocketFactory>();
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
	m_fileSysFactory = make_shared <posixFileSystemFactory>();
	m_childProcFactory = make_shared <posixChildProcessFactory>();
#endif
}


posixHandler::~posixHandler()
{
}


unsigned long posixHandler::getUnixTime() const
{
	return static_cast <unsigned long>(::time(NULL));
}


const vmime::datetime posixHandler::getCurrentLocalTime() const
{
	const time_t t(::time(NULL));

	// Get the local time
#if VMIME_HAVE_LOCALTIME_R
	tm local;
	::localtime_r(&t, &local);
#else
	tm local = *::localtime(&t);  // WARNING: this is not thread-safe!
#endif

	// Get the UTC time
#if VMIME_HAVE_GMTIME_R
	tm gmt;
	::gmtime_r(&t, &gmt);
#else
	tm gmt = *::gmtime(&t);  // WARNING: this is not thread-safe!
#endif

	// "A negative value for tm_isdst causes mktime() to attempt
	//  to determine whether Daylight Saving Time is in effect
	//  for the specified time."
	local.tm_isdst = -1;
	gmt.tm_isdst = -1;

	// Calculate the difference (in seconds)
	const long diff = ::mktime(&local) - ::mktime(&gmt);

	// Return the date
	return vmime::datetime(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
					local.tm_hour, local.tm_min, local.tm_sec, diff / 60);  // minutes needed
}


const vmime::charset posixHandler::getLocalCharset() const
{
	const PLockHelper lock;

	return vmime::charset(::nl_langinfo(CODESET));
}


static inline bool isFQDN(const vmime::string& str)
{
	if (utility::stringUtils::isStringEqualNoCase(str, "localhost", 9))
		return false;

	const vmime::size_t p = str.find_first_of(".");
	return p != vmime::string::npos && p > 0 && p != str.length() - 1;
}


const vmime::string posixHandler::getHostName() const
{
	char hostname[256];

	// Try with 'gethostname'
	::gethostname(hostname, sizeof(hostname));
	hostname[sizeof(hostname) - 1] = '\0';

	// If this is a Fully-Qualified Domain Name (FQDN), return immediately
	if (isFQDN(hostname))
		return hostname;

	if (::strlen(hostname) == 0)
		::strcpy(hostname, "localhost");

	// Try to get canonical name for the hostname
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // either IPV4 or IPV6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	struct addrinfo* info;

	if (getaddrinfo(hostname, "http", &hints, &info) == 0)
	{
		for (struct addrinfo* p = info ; p != NULL ; p = p->ai_next)
		{
			if (p->ai_canonname && isFQDN(p->ai_canonname))
			{
				const string ret(p->ai_canonname);
				freeaddrinfo(info);
				return ret;
			}
		}

		freeaddrinfo(info);
	}

	return hostname;
}


unsigned int posixHandler::getProcessId() const
{
	return (::getpid());
}


unsigned int posixHandler::getThreadId() const
{
#if VMIME_HAVE_GETTID
	return static_cast <unsigned int>(::gettid());
#elif VMIME_HAVE_SYSCALL && VMIME_HAVE_SYSCALL_GETTID
	return static_cast <unsigned int>(::syscall(SYS_gettid));
#else
	#error We have no implementation of getThreadId() for this platform!
#endif
}


#if VMIME_HAVE_MESSAGING_FEATURES

shared_ptr <vmime::net::socketFactory> posixHandler::getSocketFactory()
{
	return m_socketFactory;
}

#endif


#if VMIME_HAVE_FILESYSTEM_FEATURES

shared_ptr <vmime::utility::fileSystemFactory> posixHandler::getFileSystemFactory()
{
	return m_fileSysFactory;
}


shared_ptr <vmime::utility::childProcessFactory> posixHandler::getChildProcessFactory()
{
	return m_childProcFactory;
}

#endif


void posixHandler::generateRandomBytes(unsigned char* buffer, const unsigned int count)
{
	int fd = open("/dev/urandom", O_RDONLY);

	if (fd != -1)
	{
		read(fd, buffer, count);
		close(fd);
	}
	else  // fallback
	{
		for (unsigned int i = 0 ; i < count ; ++i)
			buffer[i] = static_cast <unsigned char>(rand() % 255);
	}
}


shared_ptr <utility::sync::criticalSection> posixHandler::createCriticalSection()
{
	return make_shared <posixCriticalSection>();
}


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_POSIX
