//
// VMime library (http://vmime.sourceforge.net)
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

#include "vmime/messaging/authHelper.hpp"

#include "vmime/config.hpp"
#include "vmime/utility/md5.hpp"


namespace vmime {
namespace messaging {


//
// This code is based on the Sample Code published in the Appendix of
// the RFC-2104: "HMAC: Keyed-Hashing for Message Authentication".
//

void hmac_md5(const string& text, const string& key, string& hexDigest)
{
	vmime_uint8 digest[16];

	unsigned char ipad[65];    // inner padding - key XORd with ipad
	unsigned char opad[65];    // outer padding - key XORd with opad

	unsigned char tkey[16];
	int tkeyLen;

	// If key is longer than 64 bytes reset it to key = MD5(key)
	if (key.length() > 64)
	{
		utility::md5 keyMD5;
		keyMD5.update(reinterpret_cast <const vmime_uint8*>(key.data()), key.length());

		std::copy(keyMD5.hash(), keyMD5.hash() + 16, tkey);
		tkeyLen = 16;
	}
	else
	{
		std::copy(key.begin(), key.end(), tkey);
		tkeyLen = key.length();
	}

	//
	// the HMAC_MD5 transform looks like:
	//
	// MD5(K XOR opad, MD5(K XOR ipad, text))
	//
	// where K is an n byte key
	// ipad is the byte 0x36 repeated 64 times
	//
	// opad is the byte 0x5c repeated 64 times
	// and text is the data being protected
	//

	// Start out by storing key in pads
	std::fill(ipad, ipad + sizeof(ipad), 0);
	std::fill(opad, opad + sizeof(opad), 0);

	std::copy(tkey, tkey + tkeyLen, ipad);
	std::copy(tkey, tkey + tkeyLen, opad);

	// XOR key with ipad and opad values
	for (int i = 0 ; i < 64 ; ++i)
	{
		ipad[i] ^= 0x36;
		opad[i] ^= 0x5c;
	}

	// Perform inner MD5
	utility::md5 innerMD5;
	innerMD5.update(ipad, 64);
	innerMD5.update(text);

	std::copy(innerMD5.hash(), innerMD5.hash() + 16, digest);

	// Perform outer MD5
	utility::md5 outerMD5;
	outerMD5.update(opad, 64);
	outerMD5.update(digest, 16);

	//std::copy(outerMD5.hash(), outerMD5.hash() + 16, digest);

	hexDigest = outerMD5.hex();
}


} // messaging
} // vmime
