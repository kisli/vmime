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


VMIME_TEST_SUITE_BEGIN(fileContentHandlerTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testIsEmpty)
		VMIME_TEST(testGetLength)
		VMIME_TEST(testIsEncoded)
		VMIME_TEST(testExtract)
		VMIME_TEST(testExtractRaw)
		VMIME_TEST(testGenerate)
	VMIME_TEST_LIST_END


	vmime::shared_ptr <vmime::utility::file> testFile;
	vmime::string testDataEncoded, testDataDecoded;


	void setUp()
	{
		testDataDecoded = "ABCDEFGHIJKLMNOPQRSTUVWXYZ \x12\x34\x56\x78\x90 abcdefghijklmnopqrstuvwxyz0123456789";
		testDataEncoded = "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVogEjRWeJAgYWJjZGVmZ2hpamtsbW5vcHFyc3R1dnd4eXowMTIzNDU2Nzg5";

		std::ostringstream testFilePath;
		testFilePath << "/tmp/vmime_test_" << (rand() % 999999999);

		vmime::shared_ptr <vmime::utility::fileSystemFactory> fsf =
			vmime::platform::getHandler()->getFileSystemFactory();

		testFile = fsf->create(fsf->stringToPath(testFilePath.str()));
		testFile->createFile();
		testFile->getFileWriter()->getOutputStream()->write(testDataEncoded.data(), testDataEncoded.length());
	}

	void tearDown()
	{
		testFile->remove();
		testFile = vmime::null;
	}


	void testIsEmpty()
	{
		vmime::fileContentHandler cth;

		VASSERT_TRUE("empty", cth.isEmpty());
	}

	void testGetLength()
	{
		vmime::fileContentHandler cth(testFile);

		VASSERT_FALSE("empty", cth.isEmpty());
		VASSERT_EQ("length", testDataEncoded.length(), cth.getLength());
	}

	void testIsEncoded()
	{
		vmime::fileContentHandler cth(testFile, vmime::encoding("base64"));

		VASSERT_TRUE("encoded", cth.isEncoded());
		VASSERT_EQ("encoding", "base64", cth.getEncoding().generate());
	}

	void testExtract()
	{
		vmime::fileContentHandler cth(testFile, vmime::encoding("base64"));

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.extract(osa);

		// Data should be decoded from B64
		VASSERT_EQ("extract", testDataDecoded, oss.str());
	}

	void testExtractRaw()
	{
		vmime::fileContentHandler cth(testFile, vmime::encoding("base64"));

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.extractRaw(osa);

		// Data should not be decoded
		VASSERT_EQ("extractRaw", testDataEncoded, oss.str());
	}

	void testGenerate()
	{
		vmime::fileContentHandler cth(testFile, vmime::encoding("base64"));

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter osa(oss);

		cth.generate(osa, vmime::encoding("quoted-printable"));

		// Data should be reencoded from B64 to QP
		VASSERT_EQ("generate",
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ =124Vx=90 abcdefghijklmnopqrstuvwxyz0123456789", oss.str());
	}

VMIME_TEST_SUITE_END
