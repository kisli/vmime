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

#ifndef VMIME_NET_TLS_OPENSSL_OPENSSLINITIALIZER_HPP_INCLUDED
#define VMIME_NET_TLS_OPENSSL_OPENSSLINITIALIZER_HPP_INCLUDED


#ifndef VMIME_BUILDING_DOC


#include "vmime/config.hpp"

#include <vector>


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL


#include "vmime/utility/sync/criticalSection.hpp"


namespace vmime {
namespace net {
namespace tls {


/** Class responsible for setting up OpenSSL
  */
class OpenSSLInitializer
{
public:

	/** Automatically initialize OpenSSL
	  */
	class autoInitializer
	{
	public:

		autoInitializer();
		~autoInitializer();
	};

protected:

	class oneTimeInitializer
	{
	public:

		oneTimeInitializer();
		~oneTimeInitializer();
	};


	/** Initializes the OpenSSL lib
	  */
	static void initialize();

	/** Shutdown the OpenSSL lib
	  */
	static void uninitialize();


	static shared_ptr <vmime::utility::sync::criticalSection> getMutex();

	enum
	{
		SEEDSIZE = 256
	};


	// OpenSSL multithreading support
	static void lock(int mode, int n, const char* file, int line);
	static unsigned long id();

private:

	static shared_ptr <vmime::utility::sync::criticalSection >* sm_mutexes;
};


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

#endif // VMIME_BUILDING_DOC

#endif // VMIME_NET_TLS_OPENSSL_OPENSSLINITIALIZER_HPP_INCLUDED

