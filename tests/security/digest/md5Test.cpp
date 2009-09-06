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


#define VMIME_TEST_SUITE         md5Test
#define VMIME_TEST_SUITE_MODULE  "Security/Digest"


#define INIT_DIGEST(var, algo) \
	vmime::ref <vmime::security::digest::messageDigest> var = \
		vmime::security::digest::messageDigestFactory::getInstance()->create(algo)



VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testRFC1321_1)
		VMIME_TEST(testRFC1321_2)
		VMIME_TEST(testRFC1321_3)
		VMIME_TEST(testRFC1321_4)
		VMIME_TEST(testRFC1321_5)
		VMIME_TEST(testRFC1321_6)
		VMIME_TEST(testRFC1321_7)
		VMIME_TEST(testUpdate1)
		VMIME_TEST(testUpdate2)
		VMIME_TEST(testUpdate3)
		VMIME_TEST(testUpdate4)
		VMIME_TEST(testUpdate5)
		VMIME_TEST(testUpdate6)
		VMIME_TEST(testUpdate7)
	VMIME_TEST_LIST_END


	// Test suites from RFC #1321

	void testRFC1321_1()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("");
		algo->finalize();

		VASSERT_EQ("*", "d41d8cd98f00b204e9800998ecf8427e", algo->getHexDigest());
	}

	void testRFC1321_2()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("a");
		algo->finalize();

		VASSERT_EQ("*", "0cc175b9c0f1b6a831c399e269772661", algo->getHexDigest());
	}

	void testRFC1321_3()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("abc");
		algo->finalize();

		VASSERT_EQ("*", "900150983cd24fb0d6963f7d28e17f72", algo->getHexDigest());
	}

	void testRFC1321_4()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("message digest");
		algo->finalize();

		VASSERT_EQ("*", "f96b697d7cb7938d525a2f31aaf161d0", algo->getHexDigest());
	}

	void testRFC1321_5()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("abcdefghijklmnopqrstuvwxyz");
		algo->finalize();

		VASSERT_EQ("*", "c3fcd3d76192e4007dfb496cca67e13b", algo->getHexDigest());
	}

	void testRFC1321_6()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
		algo->finalize();

		VASSERT_EQ("*", "d174ab98d277d9f5a5611c2c9f419d9f", algo->getHexDigest());
	}

	void testRFC1321_7()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("12345678901234567890123456789012345678901234567890123456789012345678901234567890");
		algo->finalize();

		VASSERT_EQ("*", "57edf4a22be3c955ac49da2e2107b67a", algo->getHexDigest());
	}

	void testReset()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("foo");
		algo->update("bar");
		algo->finalize();

		algo->reset();
		algo->finalize();

		VASSERT_EQ("*", "d41d8cd98f00b204e9800998ecf8427e", algo->getHexDigest());  // empty string
	}

	void testUpdate1()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("");
		algo->finalize();

		VASSERT_EQ("*", "d41d8cd98f00b204e9800998ecf8427e", algo->getHexDigest());
	}

	void testUpdate2()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("a");
		algo->update("");
		algo->finalize();

		VASSERT_EQ("2", "0cc175b9c0f1b6a831c399e269772661", algo->getHexDigest());
	}

	void testUpdate3()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("ab");
		algo->update("c");
		algo->finalize();

		VASSERT_EQ("3", "900150983cd24fb0d6963f7d28e17f72", algo->getHexDigest());
	}

	void testUpdate4()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("");
		algo->update("message");
		algo->update(" ");
		algo->update("digest");
		algo->finalize();

		VASSERT_EQ("4", "f96b697d7cb7938d525a2f31aaf161d0", algo->getHexDigest());
	}

	void testUpdate5()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("abcd");
		algo->update("");
		algo->update("efghijklmnop");
		algo->update("qrstuvwx");
		algo->update("yz");
		algo->finalize();

		VASSERT_EQ("5", "c3fcd3d76192e4007dfb496cca67e13b", algo->getHexDigest());
	}

	void testUpdate6()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012");
		algo->update("345");
		algo->update("6");
		algo->update("7");
		algo->update("89");
		algo->finalize();

		VASSERT_EQ("6", "d174ab98d277d9f5a5611c2c9f419d9f", algo->getHexDigest());
	}

	void testUpdate7()
	{
		INIT_DIGEST(algo, "md5");

		algo->update("12345678901234567890123456789");
		algo->update("01234567890123456789012345678901");
		algo->update("234567890123456789");
		algo->update("");
		algo->update("0");
		algo->finalize();

		VASSERT_EQ("7", "57edf4a22be3c955ac49da2e2107b67a", algo->getHexDigest());
	}

VMIME_TEST_SUITE_END

