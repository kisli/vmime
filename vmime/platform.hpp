//
// VMime library (http://www.vmime.org)
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

#ifndef VMIME_PLATFORM_HPP_INCLUDED
#define VMIME_PLATFORM_HPP_INCLUDED


#include "vmime/config.hpp"
#include "vmime/dateTime.hpp"
#include "vmime/exception.hpp"
#include "vmime/charset.hpp"

#if VMIME_HAVE_MESSAGING_FEATURES
	#include "vmime/net/socket.hpp"
	#include "vmime/net/timeoutHandler.hpp"
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
	#include "vmime/utility/file.hpp"
	#include "vmime/utility/childProcess.hpp"
#endif


namespace vmime
{


/** Allow setting or getting the current platform handler.
  */

class platform
{
public:

	/** Handles all platform-dependent operations. It offers an interface to
	  * access platform-dependent objects: sockets, date/time, file system, etc.
	  */

	class handler : public object
	{
	public:

		virtual ~handler();

		/** Return the current UNIX time (Epoch time): the number of
		  * seconds elapsed since Jan, 1st 1970 00:00.
		  *
		  * @return UNIX Epoch time
		  */
		virtual unsigned int getUnixTime() const = 0;

		/** Return the current date and time, in the local time zone.
		  *
		  * @return current date and time
		  */
		virtual const datetime getCurrentLocalTime() const = 0;

		/** Return the host name of the system.
		  * Used when generating message ids.
		  *
		  * @return host name
		  */
		virtual const string getHostName() const = 0;

		/** Return the current process identifier.
		  * Used when generating random strings (part boundaries or message ids).
		  *
		  * @return current process id
		  */
		virtual unsigned int getProcessId() const = 0;

		/** Return the charset used on the system.
		  *
		  * @return locale charset
		  */
		virtual const charset getLocaleCharset() const = 0;

		/** This function is called when VMime library is waiting for
		  * something (for example, it is called when there is no data
		  * available in a socket). On POSIX-compliant systems, a
		  * simple call to sched_yield() should suffice.
		  */
		virtual void wait() const = 0;

#if VMIME_HAVE_MESSAGING_FEATURES
		/** Return a pointer to the default socket factory for
		  * this platform.
		  *
		  * @return socket factory
		  */
		virtual ref <net::socketFactory> getSocketFactory() = 0;
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
		/** Return a pointer to a factory that creates file-system objects.
		  *
		  * @return file-system factory
		  */
		virtual ref <utility::fileSystemFactory> getFileSystemFactory() = 0;

		/** Return a pointer to a factory that creates child process objects,
		  * which are used to spawn processes (run executable files).
		  *
		  * @return child process factory
		  */
		virtual ref <utility::childProcessFactory> getChildProcessFactory() = 0;
#endif

	};


	template <class TYPE>
	static void setHandler()
	{
		sm_handler = vmime::create <TYPE>();
	}

	static ref <handler> getHandler()
	{
		if (!sm_handler)
			throw exceptions::no_platform_handler();

		return (sm_handler);
	}

private:

	static ref <handler> sm_handler;
};


/** Compatibility with older versions of VMime (before 0.8.1). */
typedef platform platformDependant;


} // vmime


#endif // VMIME_PLATFORM_HPP_INCLUDED

