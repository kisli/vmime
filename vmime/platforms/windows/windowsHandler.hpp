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

#ifndef VMIME_PLATFORMS_WINDOWS_HANDLER_HPP_INCLUDED
#define VMIME_PLATFORMS_WINDOWS_HANDLER_HPP_INCLUDED


#include "vmime/config.hpp"
#include "vmime/platformDependant.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES
	#include "vmime/platforms/windows/windowsSocket.hpp"
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	 #include "vmime/platforms/windows/windowsFile.hpp"
#endif


namespace vmime {
namespace platforms {
namespace windows {


class windowsHandler : public vmime::platformDependant::handler
{
public:

	windowsHandler();
	~windowsHandler();

	const unsigned int getUnixTime() const;

	const vmime::datetime getCurrentLocalTime() const;

	const vmime::charset getLocaleCharset() const;

	const vmime::string getHostName() const;

	const unsigned int getProcessId() const;

#if VMIME_HAVE_MESSAGING_FEATURES
	vmime::messaging::socketFactory* getSocketFactory(const vmime::string& name) const;

	vmime::messaging::timeoutHandlerFactory* getTimeoutHandlerFactory(const vmime::string& name) const;
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	vmime::utility::fileSystemFactory* getFileSystemFactory() const;

	vmime::utility::childProcessFactory* getChildProcessFactory() const;
#endif

	void wait() const;

private:

#if VMIME_HAVE_MESSAGING_FEATURES
	windowsSocketFactory* m_socketFactory;
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	windowsFileSystemFactory* m_fileSysFactory;
#endif
};


} // windows
} // platforms
} // vmime


#endif // VMIME_PLATFORMS_WINDOWS_HANDLER_HPP_INCLUDED
