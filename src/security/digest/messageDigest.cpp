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

#include "vmime/security/digest/messageDigest.hpp"

#include <sstream>


namespace vmime {
namespace security {
namespace digest {


const string messageDigest::getHexDigest() const
{
	const byte* hash = getDigest();
	const int len = getDigestLength();

	static const unsigned char hex[] = "0123456789abcdef";

	std::ostringstream oss;

	for (int i = 0 ; i < len ; ++i)
	{
		oss << hex[(hash[i] & 0xf0) >> 4];
		oss << hex[(hash[i] & 0x0f)];
	}

	return oss.str();

}


} // digest
} // security
} // vmime

