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

#include "vmime/wordEncoder.hpp"


VMIME_TEST_SUITE_BEGIN(wordEncoderTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testGetNextChunk)
		VMIME_TEST(testGetNextChunk_integral)
		VMIME_TEST(testIsEncodingNeeded_ascii)
		VMIME_TEST(testIsEncodingNeeded_withLanguage)
		VMIME_TEST(testIsEncodingNeeded_specialChars)
		VMIME_TEST(testGuessBestEncoding_QP)
		VMIME_TEST(testGuessBestEncoding_B64)
		VMIME_TEST(testEncodeQP_RFC2047)
	VMIME_TEST_LIST_END


	void testGetNextChunk()
	{
		// An integral number of characters should be encoded
		vmime::wordEncoder we(
			"bufferfoobarbaz",
			vmime::charset("utf-8"),
			vmime::wordEncoder::ENCODING_AUTO);

		VASSERT_EQ("1", "buffer", we.getNextChunk(6));
		VASSERT_EQ("2", "foo", we.getNextChunk(3));
		VASSERT_EQ("3", "barbaz", we.getNextChunk(10));
	}

	void testGetNextChunk_integral()
	{
		// An integral number of characters should be encoded
		vmime::wordEncoder we(
			"buffer\xc3\xa0plop",
			vmime::charset("utf-8"),
			vmime::wordEncoder::ENCODING_AUTO);

		VASSERT_EQ("1", "buffer=C3=A0", we.getNextChunk(7));
		VASSERT_EQ("2", "plop", we.getNextChunk(10));
	}

	void testIsEncodingNeeded_ascii()
	{
		vmime::generationContext ctx(vmime::generationContext::getDefaultContext());
		ctx.setInternationalizedEmailSupport(false);

		VASSERT_FALSE("ascii", vmime::wordEncoder::isEncodingNeeded
			(ctx, "ASCII-only buffer", vmime::charset("utf-8"), ""));

		VASSERT_TRUE("non-ascii", vmime::wordEncoder::isEncodingNeeded
			(ctx, "Buffer with some UTF-8 '\xc3\xa0'", vmime::charset("utf-8"), ""));
	}

	void testIsEncodingNeeded_withLanguage()
	{
		VASSERT_TRUE("ascii", vmime::wordEncoder::isEncodingNeeded
			(vmime::generationContext::getDefaultContext(), "ASCII-only buffer", vmime::charset("utf-8"), "en"));
	}

	void testIsEncodingNeeded_specialChars()
	{
		VASSERT_TRUE("rfc2047", vmime::wordEncoder::isEncodingNeeded
			(vmime::generationContext::getDefaultContext(),
			 "foo bar =? foo bar", vmime::charset("us-ascii"), ""));

		VASSERT_TRUE("new line 1", vmime::wordEncoder::isEncodingNeeded
			(vmime::generationContext::getDefaultContext(),
			 "foo bar \n foo bar", vmime::charset("us-ascii"), ""));

		VASSERT_TRUE("new line 2", vmime::wordEncoder::isEncodingNeeded
			(vmime::generationContext::getDefaultContext(),
			 "foo bar \r foo bar", vmime::charset("us-ascii"), ""));
	}

	void testGuessBestEncoding_QP()
	{
		VASSERT_EQ("1", vmime::wordEncoder::ENCODING_QP,
			vmime::wordEncoder::guessBestEncoding("ASCII only buffer", vmime::charset("us-ascii")));
	}

	void testGuessBestEncoding_B64()
	{
		// >= 40% non-ASCII => Base64...
		VASSERT_EQ("1", vmime::wordEncoder::ENCODING_B64,
			vmime::wordEncoder::guessBestEncoding("xxxxx\xc3\xa0\xc3\xa0", vmime::charset("utf-8")));

		// ...else Quoted-Printable
		VASSERT_EQ("2", vmime::wordEncoder::ENCODING_QP,
			vmime::wordEncoder::guessBestEncoding("xxxxxx\xc3\xa0\xc3\xa0", vmime::charset("utf-8")));
	}

	void testEncodeQP_RFC2047()
	{
		// When Quoted-Printable is used, it should be RFC-2047 QP encoding
		vmime::wordEncoder we(
			"buffer\xc3\xa0 foo_bar",
			vmime::charset("utf-8"),
			vmime::wordEncoder::ENCODING_AUTO);

		VASSERT_EQ("1", "buffer=C3=A0_foo=5Fbar", we.getNextChunk(100));
	}

VMIME_TEST_SUITE_END
