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

#include "vmime/utility/inputStreamStringAdapter.hpp"
#include "vmime/utility/seekableInputStreamRegionAdapter.hpp"
#include "vmime/utility/stringUtils.hpp"


using namespace vmime::utility;


VMIME_TEST_SUITE_BEGIN(seekableInputStreamRegionAdapterTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testInitialPosition)
		VMIME_TEST(testSeekAndGetPosition)
		VMIME_TEST(testRead)
		VMIME_TEST(testSkip)
		VMIME_TEST(testReset)
		VMIME_TEST(testOwnPosition)
	VMIME_TEST_LIST_END


	vmime::shared_ptr <seekableInputStreamRegionAdapter> createStream
		(vmime::shared_ptr <seekableInputStream>* underlyingStream = NULL)
	{
		vmime::string buffer("THIS IS A TEST BUFFER");

		vmime::shared_ptr <seekableInputStream> strStream =
			vmime::make_shared <inputStreamStringAdapter>(buffer);

		vmime::shared_ptr <seekableInputStreamRegionAdapter> rgnStream =
			vmime::make_shared <seekableInputStreamRegionAdapter>(strStream, 10, 11);

		if (underlyingStream)
			*underlyingStream = strStream;

		return rgnStream;
	}

	void testInitialPosition()
	{
		vmime::shared_ptr <seekableInputStreamRegionAdapter> stream = createStream();

		VASSERT_EQ("Pos", 0, stream->getPosition());
		VASSERT_FALSE("EOF", stream->eof());
	}

	void testSeekAndGetPosition()
	{
		vmime::shared_ptr <seekableInputStreamRegionAdapter> stream = createStream();

		stream->seek(5);

		VASSERT_EQ("Pos 1", 5, stream->getPosition());
		VASSERT_FALSE("EOF 1", stream->eof());

		stream->seek(20);

		VASSERT_EQ("Pos 2", 11, stream->getPosition());
		VASSERT_TRUE("EOF 2", stream->eof());
	}

	void testRead()
	{
		vmime::shared_ptr <seekableInputStreamRegionAdapter> stream = createStream();

		stream->seek(5);

		vmime::byte_t buffer[100];
		std::fill(vmime::begin(buffer), vmime::end(buffer), 0);
		vmime::size_t read = stream->read(buffer, 6);

		VASSERT_EQ("Pos", 11, stream->getPosition());
		VASSERT_EQ("Read", 6, read);
		VASSERT_TRUE("EOF", stream->eof());
		VASSERT_EQ("Buffer", "BUFFER",
			vmime::utility::stringUtils::makeStringFromBytes(buffer, 6));
	}

	void testSkip()
	{
		vmime::shared_ptr <seekableInputStreamRegionAdapter> stream = createStream();

		stream->skip(5);

		VASSERT_EQ("Pos 1", 5, stream->getPosition());
		VASSERT_FALSE("EOF 1", stream->eof());

		vmime::byte_t buffer[100];
		std::fill(vmime::begin(buffer), vmime::end(buffer), 0);
		vmime::size_t read = stream->read(buffer, 3);

		VASSERT_EQ("Pos 2", 8, stream->getPosition());
		VASSERT_EQ("Read", 3, read);
		VASSERT_FALSE("EOF 2", stream->eof());
		VASSERT_EQ("Buffer", "BUF",
			vmime::utility::stringUtils::makeStringFromBytes(buffer, 3));

		stream->skip(50);

		VASSERT_EQ("Pos 3", 11, stream->getPosition());
		VASSERT_TRUE("EOF 3", stream->eof());
	}

	void testReset()
	{
		vmime::shared_ptr <seekableInputStreamRegionAdapter> stream = createStream();

		stream->skip(100);
		stream->reset();

		VASSERT_EQ("Pos", 0, stream->getPosition());
		VASSERT_FALSE("EOF", stream->eof());
	}

	void testOwnPosition()
	{
		// seekableInputStreamRegionAdapter should keep track of its own position
		// in the underlying stream, and not be affected by possible seek/read
		// operations on it...
		vmime::shared_ptr <seekableInputStream> ustream;
		vmime::shared_ptr <seekableInputStreamRegionAdapter> stream = createStream(&ustream);

		stream->seek(5);

		vmime::byte_t buffer1[100];
		std::fill(vmime::begin(buffer1), vmime::end(buffer1), 0);
		vmime::size_t read = ustream->read(buffer1, 7);

		vmime::byte_t buffer2[100];
		std::fill(vmime::begin(buffer2), vmime::end(buffer2), 0);
		vmime::size_t read2 = stream->read(buffer2, 6);

		VASSERT_EQ("Buffer 1", "THIS IS",
			vmime::utility::stringUtils::makeStringFromBytes(buffer1, 7));
		VASSERT_EQ("Buffer 2", "BUFFER",
			vmime::utility::stringUtils::makeStringFromBytes(buffer2, 6));

		// ...but the underlying stream position is affected by read operations
		// from the region adapter (FIXME?)
		VASSERT_EQ("Pos", 21, ustream->getPosition());
	}

VMIME_TEST_SUITE_END
