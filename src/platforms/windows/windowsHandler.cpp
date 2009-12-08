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

#include "vmime/platforms/windows/windowsHandler.hpp"
#include "vmime/config.hpp"

#include <time.h>
#include <locale.h>
#include <process.h>
#include <windows.h>  // for winnls.h
#include <winsock2.h> // for WSAStartup()

#ifdef VMIME_HAVE_MLANG_H
#   include <mlang.h>
#endif


namespace vmime {
namespace platforms {
namespace windows {


windowsHandler::windowsHandler()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);

#if VMIME_HAVE_MESSAGING_FEATURES
	m_socketFactory = vmime::create <windowsSocketFactory>();
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
	m_fileSysFactory = vmime::create <windowsFileSystemFactory>();
#endif
}


windowsHandler::~windowsHandler()
{
	WSACleanup();
}


unsigned int windowsHandler::getUnixTime() const
{
	return static_cast <unsigned int>(::time(NULL));
}


const vmime::datetime windowsHandler::getCurrentLocalTime() const
{
	const time_t t(::time(NULL));

	// Get the local time
#if defined(_REENTRANT) && defined(localtime_r)
	tm local;
	::localtime_r(&t, &local);
#else
	tm local = *::localtime(&t);  // WARNING: this is not thread-safe!
#endif

	// Get the UTC time
#if defined(_REENTRANT) && defined(gmtime_r)
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
#ifdef VMIME_HAVE_MLANG_H
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
#else // VMIME_HAVE_MLANG_H
	vmime::string ch = vmime::charsets::ISO8859_1; // default

	switch (GetACP())
	{
	case 437: ch = vmime::charsets::CP_437; break;
	case 737: ch = vmime::charsets::CP_737; break;
	case 775: ch = vmime::charsets::CP_775; break;
	case 850: ch = vmime::charsets::CP_850; break;
	case 852: ch = vmime::charsets::CP_852; break;
	case 853: ch = vmime::charsets::CP_853; break;
	case 855: ch = vmime::charsets::CP_855; break;
	case 857: ch = vmime::charsets::CP_857; break;
	case 858: ch = vmime::charsets::CP_858; break;
	case 860: ch = vmime::charsets::CP_860; break;
	case 861: ch = vmime::charsets::CP_861; break;
	case 862: ch = vmime::charsets::CP_862; break;
	case 863: ch = vmime::charsets::CP_863; break;
	case 864: ch = vmime::charsets::CP_864; break;
	case 865: ch = vmime::charsets::CP_865; break;
	case 866: ch = vmime::charsets::CP_866; break;
	case 869: ch = vmime::charsets::CP_869; break;
	case 874: ch = vmime::charsets::CP_874; break;

	case 1125: ch = vmime::charsets::CP_1125; break;
	case 1250: ch = vmime::charsets::CP_1250; break;
	case 1251: ch = vmime::charsets::CP_1251; break;
	case 1252: ch = vmime::charsets::CP_1252; break;
	case 1253: ch = vmime::charsets::CP_1253; break;
	case 1254: ch = vmime::charsets::CP_1254; break;
	case 1255: ch = vmime::charsets::CP_1255; break;
	case 1256: ch = vmime::charsets::CP_1256; break;
	case 1257: ch = vmime::charsets::CP_1257; break;

	case 28591: ch = vmime::charsets::ISO8859_1; break;
	case 28592: ch = vmime::charsets::ISO8859_2; break;
	case 28593: ch = vmime::charsets::ISO8859_3; break;
	case 28594: ch = vmime::charsets::ISO8859_4; break;
	case 28595: ch = vmime::charsets::ISO8859_5; break;
	case 28596: ch = vmime::charsets::ISO8859_6; break;
	case 28597: ch = vmime::charsets::ISO8859_7; break;
	case 28598: ch = vmime::charsets::ISO8859_8; break;
	case 28599: ch = vmime::charsets::ISO8859_9; break;
	case 28605: ch = vmime::charsets::ISO8859_15; break;

	case 65000: ch = vmime::charsets::UTF_7; break;
	case 65001: ch = vmime::charsets::UTF_8; break;
	}

	return (vmime::charset(ch));
#endif
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


unsigned int windowsHandler::getProcessId() const
{
	return (static_cast <unsigned int>(::GetCurrentProcessId()));
}


#if VMIME_HAVE_MESSAGING_FEATURES

ref <vmime::net::socketFactory> windowsHandler::getSocketFactory()
{
	return m_socketFactory;
}

#endif


#if VMIME_HAVE_FILESYSTEM_FEATURES

ref <vmime::utility::fileSystemFactory> windowsHandler::getFileSystemFactory()
{
	return m_fileSysFactory;
}


ref <vmime::utility::childProcessFactory> windowsHandler::getChildProcessFactory()
{
	// TODO: Not implemented
	return (NULL);
}

#endif


void windowsHandler::wait() const
{
	::Sleep(1000);
}


} // posix
} // platforms
} // vmime
