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
//
// This is an implementation by Steve Reid <steve@edmweb.com>
// 100% public domain.

#include "vmime/security/digest/sha1/sha1MessageDigest.hpp"

#include <cstring>
#include <cassert>


namespace vmime {
namespace security {
namespace digest {
namespace sha1 {


#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

// blk0() and blk() perform the initial expand.
// I got the idea of expanding during the round function from SSLeay
#if VMIME_BYTE_ORDER_LITTLE_ENDIAN
	#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) \
	                             | (rol(block->l[i],  8) & 0x00FF00FF))
#else
	#define blk0(i) block->l[i]
#endif

#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] \
                ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

// (R0+R1), R2, R3, R4 are the different operations used in SHA1
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


sha1MessageDigest::sha1MessageDigest()
{
	init();
}


void sha1MessageDigest::reset()
{
	init();
}


void sha1MessageDigest::init()
{
	m_state[0] = 0x67452301;
	m_state[1] = 0xefcdab89;
	m_state[2] = 0x98badcfe;
	m_state[3] = 0x10325476;
	m_state[4] = 0xc3d2e1f0;

	m_count[0] = 0;
	m_count[1] = 0;
}


void sha1MessageDigest::update(const byte_t b)
{
	update(&b, 1);
}


void sha1MessageDigest::update(const string& s)
{
	update(reinterpret_cast <const byte_t*>(s.data()), s.length());
}


void sha1MessageDigest::update(const byte_t* buffer, const unsigned long offset,
	const unsigned long len)
{
	update(buffer + offset, len);
}


void sha1MessageDigest::update(const byte_t* buffer, const unsigned long len)
{
	unsigned int i, j;

	j = (m_count[0] >> 3) & 63;

	if ((m_count[0] += len << 3) < (len << 3))
		m_count[1]++;

	m_count[1] += (len >> 29);

	if ((j + len) > 63)
	{
		memcpy(&m_buffer[j], buffer, (i = 64 - j));

		transform(m_state, m_buffer);

		for ( ; i + 63 < len ; i += 64)
			transform(m_state, &buffer[i]);

		j = 0;
	}
	else
	{
		i = 0;
	}

	std::memcpy(&m_buffer[j], &buffer[i], len - i);
}


void sha1MessageDigest::finalize()
{
	unsigned int i, j;
	unsigned char finalcount[8];

	for (i = 0 ; i < 8 ; i++)
	{
		finalcount[i] = static_cast <unsigned char>
			((m_count[(i >= 4 ? 0 : 1)]
			 >> ((3-(i & 3)) * 8) ) & 255);  // Endian independent
	}

	update(reinterpret_cast <const byte_t*>("\200"), 1);

	while ((m_count[0] & 504) != 448)
		update(reinterpret_cast <const byte_t*>("\0"), 1);

	update(finalcount, 8);  // Should cause a transform()

	for (i = 0 ; i < 20 ; i++)
	{
		m_digest[i] = static_cast <unsigned char>
			((m_state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
	}

	// Wipe variables
	i = j = 0;

	std::memset(m_buffer, 0, 64);
	std::memset(m_state, 0, 5 * sizeof(unsigned int));
	std::memset(m_count, 0, 2 * sizeof(unsigned int));
	std::memset(&finalcount, 0, 8);
}


void sha1MessageDigest::finalize(const string& s)
{
	finalize(reinterpret_cast <const byte_t*>(s.data()), s.length());
}


void sha1MessageDigest::finalize(const byte_t* buffer, const unsigned long len)
{
	update(buffer, len);
	finalize();
}


void sha1MessageDigest::finalize(const byte_t* buffer,
	const unsigned long offset, const unsigned long len)
{
	finalize(buffer + offset, len);
}


/** Hash a single 512-bit block.
  * This is the core of the algorithm.
  */
void sha1MessageDigest::transform
	(unsigned int state[5], const unsigned char buffer[64])
{
	unsigned int a, b, c, d, e;

	typedef union
	{
		unsigned char c[64];
		unsigned int l[16];
	} CHAR64LONG16;

	assert(sizeof(unsigned int) == 4);

	CHAR64LONG16* block;
	static unsigned char workspace[64];

	block = reinterpret_cast <CHAR64LONG16*>(workspace);
	memcpy(block, buffer, 64);

	// Copy context->state[] to working vars
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];

	// 4 rounds of 20 operations each. Loop unrolled.
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

	// Add the working vars back into context.state[]
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;

	// Wipe variables
	a = b = c = d = e = 0;
}


int sha1MessageDigest::getDigestLength() const
{
	return 20;
}


const byte_t* sha1MessageDigest::getDigest() const
{
	return m_digest;
}


} // sha1
} // digest
} // security
} // vmime


