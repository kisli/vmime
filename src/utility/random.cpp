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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include "vmime/utility/random.hpp"
#include "vmime/platformDependant.hpp"

#include <ctime>


namespace vmime {
namespace utility {


unsigned int random::m_next(static_cast<unsigned int>(::std::time(NULL)));


const unsigned int random::getNext()
{
	// Park and Miller's minimal standard generator:
	// xn+1 = (a * xn + b) mod c
	// xn+1 = (16807 * xn) mod (2^31 - 1)
	static const unsigned long a = 16807;
	static const unsigned long c = (1 << ((sizeof(int) << 3) - 1));

	m_next = static_cast<unsigned int>((a * m_next) % c);
	return (m_next);
}


const unsigned int random::getTime()
{
	return (platformDependant::getHandler()->getUnixTime());
}


const unsigned int random::getProcess()
{
	return (platformDependant::getHandler()->getProcessId());
}


const string random::getString(const int length, const string& randomChars)
{
	string res;
	res.resize(length);

	const unsigned int x = randomChars.length();
	int c = 0;

	while (c < length)
	{
		for (unsigned int n = random::getNext() ; n != 0 ; n /= x)
		{
			res[c++] = randomChars[n % x];
		}
	}

	return (res);
}


} // utility
} // vmime
