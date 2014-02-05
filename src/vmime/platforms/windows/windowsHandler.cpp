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


#if VMIME_PLATFORM_IS_WINDOWS


#include "vmime/platforms/windows/windowsHandler.hpp"

#include "vmime/platforms/windows/windowsCriticalSection.hpp"

#include "vmime/utility/stringUtils.hpp"

#include <time.h>
#include <locale.h>
#include <process.h>
#include <winsock2.h> // for WSAStartup()
#include <windows.h>  // for winnls.h
#include <ws2tcpip.h>
#include <wincrypt.h>

#if VMIME_HAVE_MLANG
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
	m_socketFactory = make_shared <windowsSocketFactory>();
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
	m_fileSysFactory = make_shared <windowsFileSystemFactory>();
#endif
}


windowsHandler::~windowsHandler()
{
	WSACleanup();
}


unsigned long windowsHandler::getUnixTime() const
{
	return static_cast <unsigned long>(::time(NULL));
}


const vmime::datetime windowsHandler::getCurrentLocalTime() const
{
	const time_t t(::time(NULL));

	// Get the local time
#if VMIME_HAVE_LOCALTIME_S
	tm local;
	::localtime_s(&local, &t);
#elif VMIME_HAVE_LOCALTIME_R
	tm local;
	::localtime_r(&t, &local);
#else
	tm local = *::localtime(&t);  // WARNING: this is not thread-safe!
#endif

	// Get the UTC time
#if VMIME_HAVE_GMTIME_S
	tm gmt;
	::gmtime_s(&gmt, &t);
#elif VMIME_HAVE_GMTIME_R
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


const vmime::charset windowsHandler::getLocalCharset() const
{
#if VMIME_HAVE_MLANG
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
#else // VMIME_HAVE_MLANG
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


static inline bool isFQDN(const vmime::string& str)
{
	if (utility::stringUtils::isStringEqualNoCase(str, "localhost", 9))
		return false;

	const vmime::size_t p = str.find_first_of(".");
	return p != vmime::string::npos && p > 0 && p != str.length() - 1;
}


const vmime::string windowsHandler::getHostName() const
{
	char hostname[256];

	// Try with 'gethostname'
	::gethostname(hostname, sizeof(hostname));
	hostname[sizeof(hostname) - 1] = '\0';

	// If this is a Fully-Qualified Domain Name (FQDN), return immediately
	if (isFQDN(hostname))
		return hostname;

	if (::strlen(hostname) == 0)
	{
#if VMIME_HAVE_STRCPY_S
		::strcpy_s(hostname, "localhost");
#else
		::strcpy(hostname, "localhost");
#endif // VMIME_HAVE_STRCPY_S
	}

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


unsigned int windowsHandler::getProcessId() const
{
	return (static_cast <unsigned int>(::GetCurrentProcessId()));
}


unsigned int windowsHandler::getThreadId() const
{
	return static_cast <unsigned int>(::GetCurrentThreadId());
}


#if VMIME_HAVE_MESSAGING_FEATURES

shared_ptr <vmime::net::socketFactory> windowsHandler::getSocketFactory()
{
	return m_socketFactory;
}

#endif


#if VMIME_HAVE_FILESYSTEM_FEATURES

shared_ptr <vmime::utility::fileSystemFactory> windowsHandler::getFileSystemFactory()
{
	return m_fileSysFactory;
}


shared_ptr <vmime::utility::childProcessFactory> windowsHandler::getChildProcessFactory()
{
	// TODO: Not implemented
	return null;
}

#endif


void windowsHandler::generateRandomBytes(unsigned char* buffer, const unsigned int count)
{
	HCRYPTPROV cryptProvider = 0;
	CryptAcquireContext(&cryptProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptGenRandom(cryptProvider, static_cast <unsigned long>(count), static_cast <unsigned char*>(buffer));
	CryptReleaseContext(cryptProvider, 0);
}


shared_ptr <utility::sync::criticalSection> windowsHandler::createCriticalSection()
{
	return make_shared <windowsCriticalSection>();
}


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS

