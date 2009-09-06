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

#include "vmime/security/digest/messageDigest.hpp"

#include <sstream>


namespace vmime {
namespace security {
namespace digest {


const string messageDigest::getHexDigest() const
{
	const byte_t* hash = getDigest();
	const int len = getDigestLength();

	static const unsigned char hex[] = "0123456789abcdef";

	std::ostringstream oss;
	oss.imbue(std::locale::classic());

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

