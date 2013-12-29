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


#if VMIME_PLATFORM_IS_WINDOWS


#include "vmime/platforms/windows/windowsCriticalSection.hpp"


namespace vmime {
namespace platforms {
namespace windows {


windowsCriticalSection::windowsCriticalSection()
{
	InitializeCriticalSectionAndSpinCount(&m_cs, 0x400);
}


windowsCriticalSection::~windowsCriticalSection()
{
	DeleteCriticalSection(&m_cs);
}


void windowsCriticalSection::lock()
{
	EnterCriticalSection(&m_cs);
}


void windowsCriticalSection::unlock()
{
	LeaveCriticalSection(&m_cs);
}


} // windows
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS
