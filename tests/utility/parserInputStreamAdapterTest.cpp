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

#include "vmime/utility/parserInputStreamAdapter.hpp"


// Seekable stream which returns at most a few bytes on each read
class shortReadInputStream : public vmime::utility::seekableInputStream {

public:

	shortReadInputStream(const vmime::string& str, const size_t maxRead)
		: m_stream(str), m_maxRead(maxRead) {

	}

	bool eof() const { return m_stream.eof(); }
	void reset() { m_stream.reset(); }
	size_t skip(const size_t count) { return m_stream.skip(count); }
	size_t getPosition() const { return m_stream.getPosition(); }
	void seek(const size_t pos) { m_stream.seek(pos); }

	size_t read(vmime::byte_t* const data, const size_t count) {

		return m_stream.read(data, std::min(count, m_maxRead));
	}

private:

	vmime::utility::inputStreamStringAdapter m_stream;
	const size_t m_maxRead;
};


VMIME_TEST_SUITE_BEGIN(parserInputStreamAdapterTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testEndlessLoopBufferSize)
		VMIME_TEST(testFindNext)
		VMIME_TEST(testFindNextEndPosition)
		VMIME_TEST(testFindNextAllPositions)
		VMIME_TEST(testFindNextLongToken)
		VMIME_TEST(testFindNextShortReads)
	VMIME_TEST_LIST_END


	static vmime::shared_ptr <vmime::utility::parserInputStreamAdapter> createParser(
		const vmime::string& str
	) {

		return vmime::make_shared <vmime::utility::parserInputStreamAdapter>(
			vmime::make_shared <vmime::utility::inputStreamStringAdapter>(str)
		);
	}

	void testFindNext() {

		vmime::shared_ptr <vmime::utility::parserInputStreamAdapter> parser =
			createParser("token-before-token-after-token");

		parser->seek(7);

		VASSERT_EQ("1", 0, parser->findNext("token"));
		VASSERT_EQ("2", 13, parser->findNext("token", 1));
		VASSERT_EQ("3", 13, parser->findNext("token", 13));
		VASSERT_EQ("4", 25, parser->findNext("token", 14));
		VASSERT_EQ("5", vmime::string::npos, parser->findNext("token", 26));
		VASSERT_EQ("6", vmime::string::npos, parser->findNext("not-found"));
		VASSERT_EQ("7", vmime::string::npos, parser->findNext(""));

		// Current position is not updated
		VASSERT_EQ("8", 7, parser->getPosition());
	}

	void testFindNextEndPosition() {

		vmime::shared_ptr <vmime::utility::parserInputStreamAdapter> parser =
			createParser("0123token9token");

		// Token must end at or before end position
		VASSERT_EQ("1", 4, parser->findNext("token", 0, 9));
		VASSERT_EQ("2", vmime::string::npos, parser->findNext("token", 0, 8));
		VASSERT_EQ("3", vmime::string::npos, parser->findNext("token", 5, 14));
		VASSERT_EQ("4", 10, parser->findNext("token", 5, 15));
		VASSERT_EQ("5", 10, parser->findNext("token", 5, 1000));

		VASSERT_EQ("6", vmime::string::npos, parser->findNext("token", 4, 4));
		VASSERT_EQ("7", vmime::string::npos, parser->findNext("token", 5, 4));
	}

	void testFindNextAllPositions() {

		// Larger than the sum of the first internal reads
		static const size_t LENGTH = 4300;

		// Token at each position, including across internal reads,
		// and with partial matches everywhere before it
		vmime::string str(LENGTH, 'a');

		for (size_t pos = 0 ; pos + 3 <= LENGTH ; ++pos) {

			str.replace(pos, 3, "aab");

			vmime::shared_ptr <vmime::utility::parserInputStreamAdapter> parser = createParser(str);

			VASSERT_EQ("found", pos, parser->findNext("aab"));
			VASSERT_EQ("end", pos, parser->findNext("aab", 0, pos + 3));
			VASSERT_EQ("before-end", vmime::string::npos, parser->findNext("aab", 0, pos + 2));

			str.replace(pos, 3, "aaa");
		}
	}

	void testFindNextLongToken() {

		static const size_t BUFFER_SIZE = 4096;  // same as in parserInputStreamAdapter::findNext()

		const vmime::string token = vmime::string(BUFFER_SIZE / 2 - 1, 'a') + "b";

		for (size_t pos = 0 ; pos < 3 * BUFFER_SIZE ; pos += 997) {

			vmime::shared_ptr <vmime::utility::parserInputStreamAdapter> parser =
				createParser(vmime::string(pos, 'a') + token + vmime::string(100, 'a'));

			VASSERT_EQ("found", pos, parser->findNext(token));
		}

		// Token longer than BUFFER_SIZE/2 is not supported
		VASSERT_EQ("too-long", vmime::string::npos,
			createParser(vmime::string(BUFFER_SIZE, 'a'))->findNext(vmime::string(BUFFER_SIZE / 2 + 1, 'a')));
	}

	void testFindNextShortReads() {

		const vmime::string str = vmime::string(10000, 'X') + "token" + vmime::string(100, 'X');

		vmime::shared_ptr <vmime::utility::parserInputStreamAdapter> parser =
			vmime::make_shared <vmime::utility::parserInputStreamAdapter>(
				vmime::make_shared <shortReadInputStream>(str, 7)
			);

		VASSERT_EQ("found", 10000, parser->findNext("token"));
		VASSERT_EQ("not-found", vmime::string::npos, parser->findNext("token", 10001));
	}

	void testEndlessLoopBufferSize() {

		static const unsigned int BUFFER_SIZE = 4096;  // same as in parserInputStreamAdapter::findNext()

		vmime::string str(BUFFER_SIZE, 'X');

		vmime::shared_ptr <vmime::utility::inputStreamStringAdapter> iss =
			vmime::make_shared <vmime::utility::inputStreamStringAdapter>(str);

		vmime::shared_ptr <vmime::utility::parserInputStreamAdapter> parser =
			vmime::make_shared <vmime::utility::parserInputStreamAdapter>(iss);

		VASSERT_EQ("Not found", vmime::string::npos, parser->findNext("token"));
	}

VMIME_TEST_SUITE_END
