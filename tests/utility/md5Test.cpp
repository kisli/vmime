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

#include "tests/testUtils.hpp"

#include "vmime/utility/md5.hpp"


#define VMIME_TEST_SUITE         md5Test
#define VMIME_TEST_SUITE_MODULE  "Utility"


VMIME_TEST_SUITE_BEGIN

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testString)
		VMIME_TEST(testUpdate)
	VMIME_TEST_LIST_END


	void testString()
	{
		// Test suites from RFC #1321
		VASSERT_EQ("1", "d41d8cd98f00b204e9800998ecf8427e", vmime::utility::md5("").hex());
		VASSERT_EQ("2", "0cc175b9c0f1b6a831c399e269772661", vmime::utility::md5("a").hex());
		VASSERT_EQ("3", "900150983cd24fb0d6963f7d28e17f72", vmime::utility::md5("abc").hex());
		VASSERT_EQ("4", "f96b697d7cb7938d525a2f31aaf161d0", vmime::utility::md5("message digest").hex());
		VASSERT_EQ("5", "c3fcd3d76192e4007dfb496cca67e13b", vmime::utility::md5("abcdefghijklmnopqrstuvwxyz").hex());
		VASSERT_EQ("6", "d174ab98d277d9f5a5611c2c9f419d9f", vmime::utility::md5("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789").hex());
		VASSERT_EQ("7", "57edf4a22be3c955ac49da2e2107b67a", vmime::utility::md5("12345678901234567890123456789012345678901234567890123456789012345678901234567890").hex());
	}

	void testUpdate()
	{
		vmime::utility::md5 m1;
		m1.update("");
		VASSERT_EQ("1", "d41d8cd98f00b204e9800998ecf8427e", m1.hex());

		vmime::utility::md5 m2;
		m2.update("a");
		m2.update("");
		VASSERT_EQ("2", "0cc175b9c0f1b6a831c399e269772661", m2.hex());

		vmime::utility::md5 m3;
		m3.update("ab");
		m3.update("c");
		VASSERT_EQ("3", "900150983cd24fb0d6963f7d28e17f72", m3.hex());

		vmime::utility::md5 m4;
		m4.update("");
		m4.update("message");
		m4.update(" ");
		m4.update("digest");
		VASSERT_EQ("4", "f96b697d7cb7938d525a2f31aaf161d0", m4.hex());

		vmime::utility::md5 m5;
		m5.update("abcd");
		m5.update("");
		m5.update("efghijklmnop");
		m5.update("qrstuvwx");
		m5.update("yz");
		VASSERT_EQ("5", "c3fcd3d76192e4007dfb496cca67e13b", m5.hex());

		vmime::utility::md5 m6;
		m6.update("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012");
		m6.update("345");
		m6.update("6");
		m6.update("7");
		m6.update("89");
		VASSERT_EQ("6", "d174ab98d277d9f5a5611c2c9f419d9f", m6.hex());

		vmime::utility::md5 m7;
		m7.update("12345678901234567890123456789");
		m7.update("01234567890123456789012345678901");
		m7.update("234567890123456789");
		m7.update("");
		m7.update("0");
		VASSERT_EQ("7", "57edf4a22be3c955ac49da2e2107b67a", m7.hex());
	}

VMIME_TEST_SUITE_END

