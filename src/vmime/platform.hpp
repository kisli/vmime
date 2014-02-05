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

#include "vmime/utility/sync/criticalSection.hpp"


namespace vmime
{


/** Allow setting or getting the current platform handler.
  */

class VMIME_EXPORT platform
{
public:

	/** Takes care of all platform-dependent operations. It offers an interface to
	  * access platform-dependent objects: sockets, date/time, file system, etc.
	  */

	class VMIME_EXPORT handler : public object
	{
	public:

		virtual ~handler();

		/** Return the current UNIX time (Epoch time): the number of
		  * seconds elapsed since Jan, 1st 1970 00:00.
		  *
		  * @return UNIX Epoch time
		  */
		virtual unsigned long getUnixTime() const = 0;

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

		/** Return an unique identifier for the current thread.
		  * Used for multi-threading synchronization.
		  *
		  * @return current thread id
		  */
		virtual unsigned int getThreadId() const = 0;

		/** Return the charset used on the system.
		  *
		  * @return local charset
		  */
		virtual const charset getLocalCharset() const = 0;

#if VMIME_HAVE_MESSAGING_FEATURES
		/** Return a pointer to the default socket factory for
		  * this platform.
		  *
		  * @return socket factory
		  */
		virtual shared_ptr <net::socketFactory> getSocketFactory() = 0;
#endif

#if VMIME_HAVE_FILESYSTEM_FEATURES
		/** Return a pointer to a factory that creates file-system objects.
		  *
		  * @return file-system factory
		  */
		virtual shared_ptr <utility::fileSystemFactory> getFileSystemFactory() = 0;

		/** Return a pointer to a factory that creates child process objects,
		  * which are used to spawn processes (run executable files).
		  *
		  * @return child process factory
		  */
		virtual shared_ptr <utility::childProcessFactory> getChildProcessFactory() = 0;
#endif

		/** Fills a buffer with cryptographically random bytes.
		  *
		  * @param buffer buffer to fill in with random bytes
		  * @param count number of random bytes to write in buffer
		  */
		virtual void generateRandomBytes(unsigned char* buffer, const unsigned int count) = 0;

		/** Creates and initializes a critical section.
		  */
		virtual shared_ptr <utility::sync::criticalSection> createCriticalSection() = 0;
	};


	template <class TYPE>
	static void setHandler()
	{
		sm_handler = vmime::make_shared <TYPE>();
	}

	static shared_ptr <handler> getDefaultHandler();
	static shared_ptr <handler> getHandler();

private:

	static shared_ptr <handler> sm_handler;
};


} // vmime


#endif // VMIME_PLATFORM_HPP_INCLUDED

