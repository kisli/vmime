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

#include <ctime>

#include <unistd.h>
#include <locale.h>
#include <langinfo.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include <pthread.h>

#include <sched.h>

#include "../src/vmime"


class my_socket : public vmime::messaging::socket
{
private:

	char m_buffer[65536];
	int m_desc;

public:

	my_socket()
		: m_desc(0)
	{
		if ((m_desc = ::socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			// Error
		}

		//::fcntl(m_desc, F_SETFL, ::fcntl(m_desc, F_GETFL, 0) | O_NONBLOCK);
	}

	~my_socket()
	{
		::close(m_desc);
	}

	void connect(const vmime::string& address, const vmime::port_t port)
	{
		::sockaddr_in addr;

		memset(&addr, 0, sizeof(addr));

		addr.sin_family = AF_INET;
		addr.sin_port = ::htons((unsigned short) port);
		addr.sin_addr.s_addr = ::inet_addr(address.c_str());

		std::cout << "Connecting to '" << address << "' port " << port << "..." << std::endl;

		if (addr.sin_addr.s_addr == (::in_addr_t) -1)
		{
			::hostent* hostInfo = (hostent*) ::gethostbyname(address.c_str());

			if (hostInfo == NULL)
			{
				// Error: cannot resolve address
				throw vmime::exceptions::connection_error();
			}

			bcopy(hostInfo->h_addr, (char*) &addr.sin_addr, hostInfo->h_length);

			std::cout << "Address resolved to " << inet_ntoa(addr.sin_addr) << std::endl;
		}
		else
		{
			// Error: cannot resolve address
			throw vmime::exceptions::connection_error();
		}

		std::cout << std::endl;

		if (::connect(m_desc, (sockaddr*) &addr, sizeof(addr)) == -1)
		{
			// Error
			throw vmime::exceptions::connection_error();
		}
	}

	const bool isConnected() const
	{
		return (m_desc != 0);
	}

	void disconnect()
	{
		::shutdown(m_desc, SHUT_RDWR);
	}

	void receive(vmime::string& buffer)
	{
		::ssize_t ret = ::recv(m_desc, m_buffer, sizeof(m_buffer), 0);

		if (ret == -1)
		{
			// Error or no data
			return;
		}
		else if (ret > 0)
		{
			buffer = vmime::string(m_buffer, ret);

#if DEBUG_SOCKET_IO
			std::cout << "S: " << buffer;
			std::cout.flush();
#endif
		}
	}

	const int receiveRaw(char* buffer, const int count)
	{
		::ssize_t ret = ::recv(m_desc, buffer, count, 0);

		if (ret == -1)
		{
			// Error or no data
			return (0);
		}
		else
		{
#if DEBUG_SOCKET_IO
			std::cout << "S: " << vmime::string(buffer, count);
			std::cout.flush();
#endif

			return (ret);
		}
	}

	void send(const vmime::string& buffer)
	{
#if DEBUG_SOCKET_IO
		std::cout << "C: " << buffer;
		std::cout.flush();
#endif

		::send(m_desc, buffer.data(), buffer.length(), 0);
	}

	void sendRaw(const char* buffer, const int count)
	{
#if DEBUG_SOCKET_IO
		std::cout << "C: " << vmime::string(buffer, count);
		std::cout.flush();
#endif

		::send(m_desc, buffer, count, 0);
	}
};


class my_socket_factory : public vmime::messaging::socketFactory
{
public:

	vmime::messaging::socket* create()
	{
		return (new my_socket);
	}
};



/*

TODO: file, fileSystemFactory



#include <errno.h>



Erreurs pour fileIterator [ opendir() ] :
===> switch + exception appropriée (exceptions::fileIteratorException ?)

* EACESS Permission denied.

* EMFILE Too many file descriptors in use by process.

* ENFILE Too many files are currently open in the system.

* ENOENT Directory  does  not  exist,  or  name  is an empty string.

* ENOMEM Insufficient memory to complete the operation.

* ENOTDIR Name is not a directory.

*/



class my_handler : public vmime::platformDependant::handler
{
private:

	my_socket_factory* m_sf;

public:

	my_handler()
		: m_sf(new my_socket_factory)
	{
	}

	~my_handler()
	{
		delete (m_sf);
	}

	const unsigned int getUnixTime() const
	{
		return ::std::time(NULL);
	}

	// WARNING: this is not thread-safe!
	const vmime::datetime getCurrentLocalTime() const
	{
		const ::std::time_t t(::std::time(NULL));

		// Get the local time
		::std::tm local = *::std::localtime(&t);

		// Get the UTC time
		::std::tm gmt = *::std::gmtime(&t);

		// "A negative value for tm_isdst causes mktime() to attempt
		//  to determine whether Daylight Saving Time is in effect
		//  for the specified time."
		local.tm_isdst = -1;
		gmt.tm_isdst = -1;

		// Calculate the difference (in seconds)
		const int diff = ::std::mktime(&local) - ::std::mktime(&gmt);

		// Return the date
		return vmime::datetime(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
		                       local.tm_hour, local.tm_min, local.tm_sec, diff / 60);  // minutes needed
	}

	const vmime::charset getLocaleCharset() const
	{
		vmime::string prevLocale(::setlocale(::LC_ALL, ""));
		vmime::charset ch(::nl_langinfo(::CODESET));
		::setlocale(::LC_ALL, prevLocale.c_str());

		return (ch);
	}

	const vmime::string getHostName() const
	{
		// TODO: you should return the real (unique) host name
		return "localhost";
	}

	const unsigned int getProcessId() const
	{
		return getpid();
	}

	vmime::messaging::socketFactory* getSocketFactory(const vmime::string& name) const
	{
		return m_sf;
	}

	vmime::messaging::timeoutHandlerFactory* getTimeoutHandlerFactory(const vmime::string& name) const
	{
		// Not used for now
		return NULL;
	}

	vmime::utility::fileSystemFactory* getFileSystemFactory() const
	{
		// TODO
		return NULL;
	}

	void wait() const
	{
		//::sched_yield();
		std::cout << "WAIT" << std::endl;
		sleep(1);
	}
};


