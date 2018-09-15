//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002 Vincent Richard <vincent@vmime.org>
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

#include "vmime/utility/outputStreamAdapter.hpp"


VMIME_TEST_SUITE_BEGIN(stringContentHandlerTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testIsEmpty)
		VMIME_TEST(testGetLength)
		VMIME_TEST(testIsEncoded)
		VMIME_TEST(testGetLength_Encoded)
		VMIME_TEST(testExtract)
		VMIME_TEST(testExtract_Encoded)
		VMIME_TEST(testExtractRaw_Encoded)
		VMIME_TEST(testGenerate)
		VMIME_TEST(testGenerate_Encoded)
	VMIME_TEST_LIST_END


	void testIsEmpty() {

		vmime::stringContentHandler cth;

		VASSERT_TRUE("empty", cth.isEmpty());
	}

	void testGetLength() {

		vmime::stringContentHandler cth("Test Data");

		VASSERT_FALSE("empty", cth.isEmpty());
		VASSERT_EQ("length", 9, cth.getLength());
	}

	void testIsEncoded() {

		vmime::stringContentHandler cth("Test Data");

		VASSERT_FALSE("encoded", cth.isEncoded());
		VASSERT_EQ("encoding", vmime::contentHandler::NO_ENCODING, cth.getEncoding());


		vmime::stringContentHandler cth2("Zm9vEjRWYmFy=", vmime::encoding("base64"));

		VASSERT_TRUE("encoded", cth2.isEncoded());
		VASSERT_EQ("encoding", "base64", cth2.getEncoding().generate());
	}

	void testGetLength_Encoded() {

		vmime::stringContentHandler cth(
			"foo=12=34=56bar",
			vmime::encoding("quoted-printable")
		);

		// Reported length should be the length of encoded data
		VASSERT_EQ("length", 15, cth.getLength());
	}

	void testExtract() {

		vmime::stringContentHandler cth("Test Data");

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.extract(osa);

		VASSERT_EQ("extract", "Test Data", oss.str());
	}

	void testExtract_Encoded() {

		vmime::stringContentHandler cth(
			"QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ejAxMjM0NTY3ODk=",
			vmime::encoding("base64")
		);

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.extract(osa);

		// Data should be decoded from B64
		VASSERT_EQ(
			"extract",
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
			oss.str()
		);
	}

	void testExtractRaw_Encoded() {

		vmime::stringContentHandler cth(
			"QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ejAxMjM0NTY3ODk=",
			vmime::encoding("base64")
		);

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.extractRaw(osa);

		// Data should not be decoded
		VASSERT_EQ(
			"extractRaw",
			"QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ejAxMjM0NTY3ODk=",
			oss.str()
		);
	}

	void testGenerate() {

		vmime::stringContentHandler cth("foo\x12\x34\x56 bar");

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.generate(osa, vmime::encoding("base64"));

		// Data should be encoded to B64
		VASSERT_EQ("generate", "Zm9vEjRWIGJhcg==", oss.str());
	}

	void testGenerate_Encoded() {

		vmime::stringContentHandler cth(
			"foo=12=34=56bar",
			vmime::encoding("quoted-printable")
		);

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.generate(osa, vmime::encoding("base64"));

		// Data should be reencoded from QP to B64
		VASSERT_EQ("generate", "Zm9vEjRWYmFy", oss.str());
	}

VMIME_TEST_SUITE_END
