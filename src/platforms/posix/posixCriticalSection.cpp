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

#include "vmime/config.hpp"


#if VMIME_PLATFORM_IS_POSIX


#include "vmime/platforms/posix/posixCriticalSection.hpp"


namespace vmime {
namespace platforms {
namespace posix {


posixCriticalSection::posixCriticalSection()
{
	pthread_mutex_init(&m_cs, NULL);
}


posixCriticalSection::~posixCriticalSection()
{
	pthread_mutex_destroy(&m_cs);
}


void posixCriticalSection::lock()
{
	pthread_mutex_lock(&m_cs);
}


void posixCriticalSection::unlock()
{
	pthread_mutex_unlock(&m_cs);
}


} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_POSIX
