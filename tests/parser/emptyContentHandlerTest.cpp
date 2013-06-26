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

#include "vmime/utility/outputStreamAdapter.hpp"


VMIME_TEST_SUITE_BEGIN(emptyContentHandlerTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testIsEmpty)
		VMIME_TEST(testGetLength)
		VMIME_TEST(testIsEncoded)
		VMIME_TEST(testExtract)
		VMIME_TEST(testExtractRaw)
		VMIME_TEST(testGenerate)
	VMIME_TEST_LIST_END


	void testIsEmpty()
	{
		vmime::emptyContentHandler cth;

		VASSERT_TRUE("empty", cth.isEmpty());
	}

	void testGetLength()
	{
		vmime::emptyContentHandler cth;

		VASSERT_EQ("length", 0, cth.getLength());
	}

	void testIsEncoded()
	{
		vmime::emptyContentHandler cth;

		VASSERT_FALSE("encoded", cth.isEncoded());
		VASSERT_EQ("encoding", vmime::contentHandler::NO_ENCODING, cth.getEncoding());
	}

	void testExtract()
	{
		vmime::emptyContentHandler cth;

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.extract(osa);

		VASSERT_EQ("extract", "", oss.str());
	}

	void testExtractRaw()
	{
		vmime::emptyContentHandler cth;

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.extractRaw(osa);

		VASSERT_EQ("extractRaw", "", oss.str());
	}

	void testGenerate()
	{
		vmime::emptyContentHandler cth;

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.generate(osa, vmime::encoding("base64"));

		VASSERT_EQ("generate", "", oss.str());
	}

VMIME_TEST_SUITE_END
