//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2008 Vincent Richard <vincent@vincent-richard.net>
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_PLATFORMS_POSIX_HANDLER_HPP_INCLUDED
#define VMIME_PLATFORMS_POSIX_HANDLER_HPP_INCLUDED


#include "vmime/config.hpp"
#include "vmime/platform.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES
	#include "vmime/platforms/posix/posixSocket.hpp"
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
     #include "vmime/platforms/posix/posixFile.hpp"
     #include "vmime/platforms/posix/posixChildProcess.hpp"
#endif


namespace vmime {
namespace platforms {
namespace posix {


class posixHandler : public vmime::platform::handler
{
public:

	posixHandler();
	~posixHandler();

	const unsigned int getUnixTime() const;

	const vmime::datetime getCurrentLocalTime() const;

	const vmime::charset getLocaleCharset() const;

	const vmime::string getHostName() const;

	const unsigned int getProcessId() const;

#if VMIME_HAVE_MESSAGING_FEATURES
	ref <vmime::net::socketFactory> getSocketFactory() const;
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	vmime::utility::fileSystemFactory* getFileSystemFactory() const;

	vmime::utility::childProcessFactory* getChildProcessFactory() const;
#endif

	void wait() const;

private:

#if VMIME_HAVE_MESSAGING_FEATURES
	ref <posixSocketFactory> m_socketFactory;
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	posixFileSystemFactory* m_fileSysFactory;
	posixChildProcessFactory* m_childProcFactory;
#endif
};


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORMS_POSIX_HANDLER_HPP_INCLUDED
