//
// VMime library (http://www.vmime.org)
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
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#ifndef VMIME_PLATFORMDEPENDANT_HPP_INCLUDED
#define VMIME_PLATFORMDEPENDANT_HPP_INCLUDED


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

class platformDependant
{
public:

	/** Handles all platform-dependant operations. It offers an interface to
	  * access platform-dependant objects: sockets, date/time, file system, etc.
	  */

	class handler
	{
	public:

		virtual ~handler();

		/** Return the current UNIX time (Epoch time): the number of
		  * seconds elapsed since Jan, 1st 1970 00:00.
		  *
		  * @return UNIX Epoch time
		  */
		virtual const unsigned int getUnixTime() const = 0;

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
		virtual const unsigned int getProcessId() const = 0;

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
		/** Return a pointer to a socket factory for the specified socket
		  * type name (this is user-defined, and used for example when you
		  * want to set up a SSL connection to a server).
		  * The returned object will not be deleted by VMime, so it can be
		  * a pointer to a static object.
		  *
		  * @param name socket type name (user-dependant): this is usually
		  * the value of the property "server.socket-factory" set in the
		  * session object
		  * @return socket factory
		  */
		virtual net::socketFactory* getSocketFactory(const string& name = "default") const = 0;

		/** Return a pointer to a timeout-handler factory for the specified name.
		  * The returned object will not be deleted by VMime, so it can be a
		  * pointer to a static object.
		  *
		  * This is used when you want to handle a timeout-mechanism when
		  * connecting to messaging servers (please read the documentation to
		  * learn how to use it). If you are not using time-out handlers, you
		  * can safely return NULL here.
		  *
		  * @param name time-out type name
		  * @return time-out factory
		  */
		virtual net::timeoutHandlerFactory* getTimeoutHandlerFactory(const string& name = "default") const = 0;
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
		/** Return a pointer to a factory that creates file-system objects.
		  *
		  * @return file-system factory
		  */
		virtual utility::fileSystemFactory* getFileSystemFactory() const = 0;

		/** Return a pointer to a factory that creates child process objects,
		  * which are used to spawn processes (run executable files).
		  *
		  * @return child process factory
		  */
		virtual utility::childProcessFactory* getChildProcessFactory() const = 0;
#endif

	};


	template <class TYPE>
	static void setHandler()
	{
		delete (sm_handler);
		sm_handler = new TYPE;
	}

	static const handler* getHandler()
	{
		if (!sm_handler)
			throw exceptions::no_platform_dependant_handler();

		return (sm_handler);
	}

private:

	static handler* sm_handler;
};


} // vmime


#endif // VMIME_PLATFORMDEPENDANT_HPP_INCLUDED
