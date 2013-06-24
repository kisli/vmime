//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
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

#include "encoderTestUtils.hpp"


VMIME_TEST_SUITE_BEGIN(b64EncoderTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testBase64)
	VMIME_TEST_LIST_END


	void testBase64()
	{
		static const vmime::string testSuites[] =
		{
			// Test 1
			"",

			"",

			// Test 2
			"A",

			"QQ==",

			// Test 3
			"AB",

			"QUI=",

			// Test 4
			"ABC",

			"QUJD",

			// Test 5
			"foo",

			"Zm9v",

			// Test 6
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",

			"QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ejAx"
			"MjM0NTY3ODk=",

			// Test 7
			vmime::string(
			"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
			"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
			"\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f"
			"\x30\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x3e\x3f"
			"\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x4e\x4f"
			"\x50\x51\x52\x53\x54\x55\x56\x57\x58\x59\x5a\x5b\x5c\x5d\x5e\x5f"
			"\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f"
			"\x70\x71\x72\x73\x74\x75\x76\x77\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f"
			"\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f"
			"\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f"
			"\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf"
			"\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf"
			"\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf"
			"\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf"
			"\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef"
			"\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff",
			256),

			"AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1"
			"Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWpr"
			"bG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6Ch"
			"oqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX"
			"2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w=="
		};


		for (unsigned int i = 0 ; i < sizeof(testSuites) / sizeof(testSuites[0]) / 2 ; ++i)
		{
			const vmime::string decoded = testSuites[i * 2];
			const vmime::string encoded = testSuites[i * 2 + 1];

			std::ostringstream oss;
			oss << "[Base64] Test " << (i + 1) << ": ";

			// Encoding
			VASSERT_EQ(oss.str() + "encoding", encoded, encode("base64", decoded));

			// Decoding
			VASSERT_EQ(oss.str() + "decoding", decoded, decode("base64", encoded));

			// Multiple and successive encoding/decoding
			VASSERT_EQ(oss.str() + "multiple1", decoded,
				decode("base64",
					encode("base64", decoded)));

			VASSERT_EQ(oss.str() + "multiple2", decoded,
				decode("base64",
					decode("base64",
						encode("base64",
							encode("base64", decoded)))));

			VASSERT_EQ(oss.str() + "multiple3", decoded,
				decode("base64",
					decode("base64",
						decode("base64",
							encode("base64",
								encode("base64",
									encode("base64", decoded)))))));

			VASSERT_EQ(oss.str() + "multiple4", decoded,
				decode("base64",
					decode("base64",
						decode("base64",
							decode("base64",
								encode("base64",
									encode("base64",
										encode("base64",
											encode("base64", decoded)))))))));

			VASSERT(oss.str() + "encoded size",
				getEncoder("base64")->getEncodedSize(decoded.length())
				>= encode("base64", decoded).length());

			VASSERT(oss.str() + "decoded size",
				getEncoder("base64")->getDecodedSize(encoded.length())
				>= decode("base64", encoded).length());
		}
	}

VMIME_TEST_SUITE_END

