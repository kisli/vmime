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

#include "vmime/platforms/windows/windowsHandler.hpp"

#include <time.h>
#include <locale.h>
#include <process.h>
#include <mlang.h>

namespace vmime {
namespace platforms {
namespace windows {


windowsHandler::windowsHandler()
{
#if VMIME_HAVE_MESSAGING_FEATURES
	m_socketFactory = new windowsSocketFactory();
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
	m_fileSysFactory = new windowsFileSystemFactory();
#endif
}


windowsHandler::~windowsHandler()
{
#if VMIME_HAVE_MESSAGING_FEATURES
	delete (m_socketFactory);
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
	delete (m_fileSysFactory);
#endif
}


const unsigned int windowsHandler::getUnixTime() const
{
	return (unsigned int)::time(NULL);
}


const vmime::datetime windowsHandler::getCurrentLocalTime() const
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
	const int diff = (const int)(::mktime(&local) - ::mktime(&gmt));

	// Return the date
	return vmime::datetime(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
					local.tm_hour, local.tm_min, local.tm_sec, diff / 60);  // minutes needed
}


const vmime::charset windowsHandler::getLocaleCharset() const
{
	char szCharset[256];
	
	CoInitialize(NULL);
	{
		IMultiLanguage* pMultiLanguage;
		CoCreateInstance(
			CLSID_CMultiLanguage,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_IMultiLanguage,
			(void**)&pMultiLanguage);

		UINT codePage = GetACP();
		MIMECPINFO cpInfo;
		pMultiLanguage->GetCodePageInfo(codePage, &cpInfo);

		int nLengthW = lstrlenW(cpInfo.wszBodyCharset) + 1;
		
		WideCharToMultiByte(codePage, 0, cpInfo.wszBodyCharset, nLengthW, szCharset, sizeof(szCharset), NULL, NULL );
		
		pMultiLanguage->Release();

	}  
	CoUninitialize();

	return vmime::charset(szCharset);
}


const vmime::string windowsHandler::getHostName() const
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


const unsigned int windowsHandler::getProcessId() const
{
	return (::getpid());
}


#if VMIME_HAVE_MESSAGING_FEATURES

vmime::messaging::socketFactory* windowsHandler::getSocketFactory
	(const vmime::string& /* name */) const
{
	return (m_socketFactory);
}


vmime::messaging::timeoutHandlerFactory* windowsHandler::getTimeoutHandlerFactory
	(const vmime::string& /* name */) const
{
	// Not used by default
	return (NULL);
}

#endif


#if VMIME_HAVE_FILESYSTEM_FEATURES

vmime::utility::fileSystemFactory* windowsHandler::getFileSystemFactory() const
{
	return (m_fileSysFactory);
}

#endif


void windowsHandler::wait() const
{
	::Sleep(1000);
}


} // posix
} // platforms
} // vmime
