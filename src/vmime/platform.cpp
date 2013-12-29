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

#include "vmime/platform.hpp"
#include "vmime/config.hpp"

#include "vmime/platforms/posix/posixHandler.hpp"
#include "vmime/platforms/windows/windowsHandler.hpp"


namespace vmime
{


shared_ptr <platform::handler> platform::sm_handler;


platform::handler::~handler()
{
}


// static
shared_ptr <platform::handler> platform::getDefaultHandler()
{

#if VMIME_PLATFORM_IS_WINDOWS
	return make_shared <platforms::windows::windowsHandler>();
#elif VMIME_PLATFORM_IS_POSIX
	return make_shared <platforms::posix::posixHandler>();
#else
	return null;
#endif

}


// static
shared_ptr <platform::handler> platform::getHandler()
{
	// If a custom platform handler is installed, return it
	if (sm_handler)
		return sm_handler;

	// Else, use the default handler for this platform
	shared_ptr <handler> defaultHandler = getDefaultHandler();

	if (defaultHandler)
	{
		sm_handler = defaultHandler;
		return sm_handler;
	}

	// Oops... no platform handler!
	throw exceptions::no_platform_handler();
}


} // vmime
