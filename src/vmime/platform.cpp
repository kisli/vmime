//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#include <mutex>


namespace vmime {


shared_ptr <platform::handler> platform::sm_handler;


// Protects 'sm_handler'. Created on first use, as getHandler() may be
// called during static initialization (eg. by OpenSSLInitializer).
static std::mutex& getHandlerMutex() {

	static std::mutex mutex;
	return mutex;
}


platform::handler::~handler() {

}


// static
shared_ptr <platform::handler> platform::getDefaultHandler() {

#if VMIME_PLATFORM_IS_WINDOWS
	return make_shared <platforms::windows::windowsHandler>();
#elif VMIME_PLATFORM_IS_POSIX
	return make_shared <platforms::posix::posixHandler>();
#else
	return null;
#endif

}


// static
void platform::installHandler(shared_ptr <handler> h) {

	std::lock_guard <std::mutex> lock(getHandlerMutex());

	// The previous handler ends up in 'h', so if it is not used anymore,
	// it is destroyed after the mutex has been released
	sm_handler.swap(h);
}


// static
shared_ptr <platform::handler> platform::getHandler() {

	std::lock_guard <std::mutex> lock(getHandlerMutex());

	// If no handler is installed yet, use the default handler for this platform
	if (!sm_handler) {
		sm_handler = getDefaultHandler();
	}

	if (sm_handler) {
		return sm_handler;
	}

	// Oops... no platform handler!
	throw exceptions::no_platform_handler();
}


} // vmime
