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

	void update(const byte_t b);
	void update(const string& s);
	void update(const byte_t* buffer, const unsigned long len);
	void update(const byte_t* buffer, const unsigned long offset, const unsigned long len);

	void finalize();
	void finalize(const string& s);
	void finalize(const byte_t* buffer, const unsigned long len);
	void finalize(const byte_t* buffer, const unsigned long offset, const unsigned long len);

	int getDigestLength() const;
	const byte_t* getDigest() const;

	void reset();

protected:

	void init();

	static void transform(unsigned int state[5], const byte_t buffer[64]);

	unsigned int m_state[5];
	unsigned int m_count[2];
	byte_t m_buffer[64];

	byte_t m_digest[20];
};


} // sha1
} // digest
} // security
} // vmime


#endif // VMIME_SECURITY_DIGEST_SHA1_SHA1MESSAGEDIGEST_HPP_INCLUDED

