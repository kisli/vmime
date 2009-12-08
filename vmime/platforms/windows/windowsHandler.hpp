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

#ifndef VMIME_PLATFORMS_WINDOWS_HANDLER_HPP_INCLUDED
#define VMIME_PLATFORMS_WINDOWS_HANDLER_HPP_INCLUDED


#include "vmime/config.hpp"
#include "vmime/platform.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES
	#include "vmime/platforms/windows/windowsSocket.hpp"
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	 #include "vmime/platforms/windows/windowsFile.hpp"
#endif


namespace vmime {
namespace platforms {
namespace windows {


class windowsHandler : public vmime::platform::handler
{
public:

	windowsHandler();
	~windowsHandler();

	unsigned int getUnixTime() const;

	const vmime::datetime getCurrentLocalTime() const;

	const vmime::charset getLocaleCharset() const;

	const vmime::string getHostName() const;

	unsigned int getProcessId() const;

#if VMIME_HAVE_MESSAGING_FEATURES
	ref <vmime::net::socketFactory> getSocketFactory();
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	ref <vmime::utility::fileSystemFactory> getFileSystemFactory();

	ref <vmime::utility::childProcessFactory> getChildProcessFactory();
#endif

	void wait() const;

private:

#if VMIME_HAVE_MESSAGING_FEATURES
	ref <windowsSocketFactory> m_socketFactory;
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	ref <windowsFileSystemFactory> m_fileSysFactory;
#endif
};


} // windows
} // platforms
} // vmime


#endif // VMIME_PLATFORMS_WINDOWS_HANDLER_HPP_INCLUDED
