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

#include "tests/testUtils.hpp"

#include "vmime/security/digest/messageDigestFactory.hpp"


#define VMIME_TEST_SUITE         sha1Test
#define VMIME_TEST_SUITE_MODULE  "Security/Digest"


#define INIT_DIGEST(var, algo) \
	vmime::ref <vmime::security::digest::messageDigest> var = \
		vmime::security::digest::messageDigestFactory::getInstance()->create(algo)



VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testFIPS180_1)
		VMIME_TEST(testFIPS180_2)
		VMIME_TEST(testFIPS180_3)
		VMIME_TEST(testReset)
		VMIME_TEST(testUpdate)
	VMIME_TEST_LIST_END


	// Test suites from FIPS PUB 180-1
	// http://www.itl.nist.gov/fipspubs/fip180-1.htm

	void testFIPS180_1()
	{
		INIT_DIGEST(algo, "sha1");

		algo->update("abc");
		algo->finalize();

		VASSERT_EQ("*", "a9993e364706816aba3e25717850c26c9cd0d89d", algo->getHexDigest());
	}

	void testFIPS180_2()
	{
		INIT_DIGEST(algo, "sha1");

		algo->update("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq");
		algo->finalize();

		VASSERT_EQ("*", "84983e441c3bd26ebaae4aa1f95129e5e54670f1", algo->getHexDigest());
	}

	void testFIPS180_3()
	{
		INIT_DIGEST(algo, "sha1");

		vmime::byte_t* buffer = new vmime::byte_t[1000000];

		for (int i = 0 ; i < 1000000 ; ++i)
			buffer[i] = 'a';

		algo->update(buffer, 1000000);
		algo->finalize();

		delete [] buffer;

		VASSERT_EQ("*", "34aa973cd4c4daa4f61eeb2bdbad27316534016f", algo->getHexDigest());
	}

	void testReset()
	{
		INIT_DIGEST(algo, "sha1");

		algo->update("ab");
		algo->update("c");
		algo->finalize();

		algo->reset();
		algo->finalize();

		VASSERT_EQ("*", "da39a3ee5e6b4b0d3255bfef95601890afd80709", algo->getHexDigest());  // empty string
	}

	void testUpdate()
	{
		INIT_DIGEST(algo, "sha1");

		algo->update("a");
		algo->update("");
		algo->update("bcdbcdecdefd");
		algo->update("efgef");
		algo->update("ghfghighijhijkijkljklmklmnlmnomnopnop");
		algo->update("");
		algo->update("q");
		algo->update("");
		algo->update("");
		algo->finalize();

		VASSERT_EQ("*", "84983e441c3bd26ebaae4aa1f95129e5e54670f1", algo->getHexDigest());
	}

VMIME_TEST_SUITE_END

