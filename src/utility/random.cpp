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

#include "vmime/utility/random.hpp"
#include "vmime/platform.hpp"

#include <ctime>


namespace vmime {
namespace utility {


unsigned int random::m_next(static_cast<unsigned int>(::std::time(NULL)));


unsigned int random::getNext()
{
	// Park and Miller's minimal standard generator:
	// xn+1 = (a * xn + b) mod c
	// xn+1 = (16807 * xn) mod (2^31 - 1)
	m_next = static_cast<unsigned int>((16807 * m_next) % 2147483647ul);
	return (m_next);
}


unsigned int random::getTime()
{
	return (platform::getHandler()->getUnixTime());
}


unsigned int random::getProcess()
{
	return (platform::getHandler()->getProcessId());
}


const string random::getString(const int length, const string& randomChars)
{
	string res;
	res.resize(length);

	const unsigned int x = randomChars.length();
	int c = 0;

	while (c < length)
	{
		for (unsigned int n = random::getNext() ; n != 0 && c < length ; n /= x)
		{
			res[c++] = randomChars[n % x];
		}
	}

	return (res);
}


} // utility
} // vmime
