//
// VMime library (http://vmime.sourceforge.net)
// Copyright (C) 2002-2004 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_PLATFORMS_POSIX_HANDLER_HPP_INCLUDED
#define VMIME_PLATFORMS_POSIX_HANDLER_HPP_INCLUDED


#include "../../config.hpp"
#include "../../platformDependant.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES
	#include "socket.hpp"
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
     #include "file.hpp"
#endif


namespace vmime {
namespace platforms {
namespace posix {


class posixHandler : public vmime::platformDependant::handler
{
public:

	posixHandler();
	~posixHandler();

	const unsigned int getUnixTime() const;

	const vmime::datetime getCurrentLocalTime() const;

	const vmime::charset getLocaleCharset() const;

	const vmime::string getHostName() const;

	const unsigned int getProcessId() const;

	vmime::messaging::socketFactory* getSocketFactory(const vmime::string& name) const;

	vmime::messaging::timeoutHandlerFactory* getTimeoutHandlerFactory(const vmime::string& name) const;

	vmime::utility::fileSystemFactory* getFileSystemFactory() const;

	void wait() const;

private:

	posixSocketFactory* m_socketFactory;
	posixFileSystemFactory* m_fileSysFactory;
};


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORMS_POSIX_HANDLER_HPP_INCLUDED
