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

#include "vmime/platforms/posix/posixHandler.hpp"

#include <time.h>

#include <unistd.h>
#include <locale.h>
#include <langinfo.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <netdb.h>

#include <string.h>


namespace vmime {
namespace platforms {
namespace posix {


posixHandler::posixHandler()
{
#if VMIME_HAVE_MESSAGING_FEATURES
	m_socketFactory = new posixSocketFactory();
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
	m_fileSysFactory = new posixFileSystemFactory();
#endif
}


posixHandler::~posixHandler()
{
#if VMIME_HAVE_MESSAGING_FEATURES
	delete (m_socketFactory);
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
	delete (m_fileSysFactory);
#endif
}


const unsigned int posixHandler::getUnixTime() const
{
	return ::time(NULL);
}


const vmime::datetime posixHandler::getCurrentLocalTime() const
{
	const time_t t(::time(NULL));

	// Get the local time
#ifdef _REENTRANT
	tm local;
	::localtime_r(&t, &local);
#else
	tm local = *::localtime(&t);  // WARNING: this is not thread-safe!
#endif

	// Get the UTC time
#ifdef _REENTRANT
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
	const int diff = ::mktime(&local) - ::mktime(&gmt);

	// Return the date
	return vmime::datetime(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
					local.tm_hour, local.tm_min, local.tm_sec, diff / 60);  // minutes needed
}


const vmime::charset posixHandler::getLocaleCharset() const
{
	vmime::string prevLocale(::setlocale(LC_ALL, ""));
	vmime::charset ch(::nl_langinfo(CODESET));
	::setlocale(LC_ALL, prevLocale.c_str());

	return (ch);
}


const vmime::string posixHandler::getHostName() const
{
	std::vector <vmime::string> hostnames;
	char buffer[256];

	// Try with 'gethostname'
	::gethostname(buffer, sizeof(buffer));
	buffer[sizeof(buffer) - 1] = '\0';

	if (::strlen(buffer) == 0)
		::strcpy(buffer, "localhost");

	hostnames.push_back(buffer);

	// And with 'gethostbyname'
	struct hostent* he = ::gethostbyname(buffer);

	if (he != NULL)
	{
		if (::strlen(he->h_name) != 0)
			hostnames.push_back(he->h_name);

		char** alias = he->h_aliases;

		while (alias && *alias)
		{
			if (::strlen(*alias) != 0)
				hostnames.push_back(*alias);

			++alias;
		}
	}

	// Find a Fully-Qualified Domain Name (FQDN)
	for (unsigned int i = 0 ; i < hostnames.size() ; ++i)
	{
		if (hostnames[i].find_first_of(".") != vmime::string::npos)
			return (hostnames[i]);
	}

	return (hostnames[0]);
}


const unsigned int posixHandler::getProcessId() const
{
	return (::getpid());
}


#if VMIME_HAVE_MESSAGING_FEATURES

vmime::messaging::socketFactory* posixHandler::getSocketFactory
	(const vmime::string& /* name */) const
{
	return (m_socketFactory);
}


vmime::messaging::timeoutHandlerFactory* posixHandler::getTimeoutHandlerFactory
	(const vmime::string& /* name */) const
{
	// Not used by default
	return (NULL);
}

#endif


#if VMIME_HAVE_FILESYSTEM_FEATURES

vmime::utility::fileSystemFactory* posixHandler::getFileSystemFactory() const
{
	return (m_fileSysFactory);
}

#endif


void posixHandler::wait() const
{
	::sleep(1);
}


} // posix
} // platforms
} // vmime
