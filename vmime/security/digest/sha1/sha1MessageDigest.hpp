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

#ifndef VMIME_SECURITY_DIGEST_SHA1_SHA1MESSAGEDIGEST_HPP_INCLUDED
#define VMIME_SECURITY_DIGEST_SHA1_SHA1MESSAGEDIGEST_HPP_INCLUDED


#include "vmime/security/digest/messageDigest.hpp"


namespace vmime {
namespace security {
namespace digest {
namespace sha1 {


class sha1MessageDigest : public messageDigest
{
public:

	sha1MessageDigest();

	void update(const byte b);
	void update(const string& s);
	void update(const byte* buffer, const unsigned long len);
	void update(const byte* buffer, const unsigned long offset, const unsigned long len);

	void finalize();
	void finalize(const string& s);
	void finalize(const byte* buffer, const unsigned long len);
	void finalize(const byte* buffer, const unsigned long offset, const unsigned long len);

	const int getDigestLength() const;
	const byte* getDigest() const;

	void reset();

protected:

	void init();

	static void transform(unsigned long state[5], const byte buffer[64]);

	unsigned long m_state[5];
	unsigned long m_count[2];
	byte m_buffer[64];

	byte m_digest[20];
};


} // sha1
} // digest
} // security
} // vmime


#endif // VMIME_SECURITY_DIGEST_SHA1_SHA1MESSAGEDIGEST_HPP_INCLUDED

