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

#include "vmime/utility/filteredStream.hpp"
#include "vmime/utility/stringUtils.hpp"


VMIME_TEST_SUITE_BEGIN(filteredStreamTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testDotFilteredInputStream)
		VMIME_TEST(testDotFilteredOutputStream)
		VMIME_TEST(testCRLFToLFFilteredOutputStream)
		VMIME_TEST(testStopSequenceFilteredInputStream1)
		VMIME_TEST(testStopSequenceFilteredInputStreamN_2)
		VMIME_TEST(testStopSequenceFilteredInputStreamN_3)
		VMIME_TEST(testLFToCRLFFilteredOutputStream_Global)
		VMIME_TEST(testLFToCRLFFilteredOutputStream_Edge)
	VMIME_TEST_LIST_END


	class chunkInputStream : public vmime::utility::inputStream
	{
	private:

		std::vector <std::string> m_chunks;
		size_t m_index;

	public:

		chunkInputStream() : m_index(0) { }

		void addChunk(const std::string& chunk) { m_chunks.push_back(chunk); }

		bool eof() const { return (m_index >= m_chunks.size()); }
		void reset() { m_index = 0; }

		vmime::size_t read(vmime::byte_t* const data, const vmime::size_t /* count */)
		{
			if (eof())
				return 0;

			const std::string chunk = m_chunks[m_index];

			// Warning: 'count' should be larger than chunk length.
			// This is OK for our tests.
			std::copy(chunk.begin(), chunk.end(), data);

			++m_index;

			return chunk.length();
		}

		vmime::size_t skip(const vmime::size_t /* count */)
		{
			// Not supported
			return 0;
		}
	};


	const std::string readWhole(vmime::utility::inputStream& is)
	{
		vmime::byte_t buffer[256];
		std::string whole;

		while (!is.eof())
		{
			const vmime::size_t read = is.read(buffer, sizeof(buffer));

			whole += vmime::utility::stringUtils::makeStringFromBytes(buffer, read);
		}

		return (whole);
	}


	// dotFilteredInputStream

	void testDotFilteredInputStreamHelper
		(const std::string& number, const std::string& expected,
		 const std::string& c1, const std::string& c2 = "",
		 const std::string& c3 = "", const std::string& c4 = "")
	{
		chunkInputStream cis;
		cis.addChunk(c1);
		if (!c2.empty()) cis.addChunk(c2);
		if (!c3.empty()) cis.addChunk(c3);
		if (!c4.empty()) cis.addChunk(c4);

		vmime::utility::dotFilteredInputStream is(cis);

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter os(oss);

		vmime::utility::bufferedStreamCopy(is, os);

		VASSERT_EQ(number, expected, oss.str());
	}

	void testDotFilteredInputStream()
	{
		testDotFilteredInputStreamHelper("1", "foo\n.bar", "foo\n..bar");
		testDotFilteredInputStreamHelper("2", "foo\n.bar", "foo\n", "..bar");
		testDotFilteredInputStreamHelper("3", "foo\n.bar", "foo\n.", ".bar");
		testDotFilteredInputStreamHelper("4", "foo\n.bar", "foo\n..", "bar");
		testDotFilteredInputStreamHelper("5", "foo\n.bar", "foo\n", ".", ".bar");
		testDotFilteredInputStreamHelper("6", "foo\n.bar", "foo\n", ".", ".", "bar");
	}

	// dotFilteredOutputStream
	// CRLFToLFFilteredOutputStream

	template <typename FILTER>
	void testFilteredOutputStreamHelper
		(const std::string& number, const std::string& expected,
		 const std::string& c1, const std::string& c2 = "",
		 const std::string& c3 = "", const std::string& c4 = "")
	{
		std::ostringstream oss;
		vmime::utility::outputStreamAdapter os(oss);

		FILTER fos(os);

		fos.write(c1.data(), c1.length());
		if (!c2.empty()) fos.write(c2.data(), c2.length());
		if (!c3.empty()) fos.write(c3.data(), c3.length());
		if (!c4.empty()) fos.write(c4.data(), c4.length());

		VASSERT_EQ(number, expected, oss.str());
	}

	void testDotFilteredOutputStream()
	{
		typedef vmime::utility::dotFilteredOutputStream FILTER;

		testFilteredOutputStreamHelper<FILTER>("1", "foo\n..bar", "foo\n.bar");
		testFilteredOutputStreamHelper<FILTER>("2", "foo\n..bar", "foo\n", ".bar");
		testFilteredOutputStreamHelper<FILTER>("3", "foo\n..bar", "foo", "\n.bar");
		testFilteredOutputStreamHelper<FILTER>("4", "foo\n..bar", "foo", "\n", ".bar");
		testFilteredOutputStreamHelper<FILTER>("5", "foo\n..bar", "foo", "\n", ".", "bar");

		testFilteredOutputStreamHelper<FILTER>("6", "..\nfoobar", ".\nfoobar");
		testFilteredOutputStreamHelper<FILTER>("7", "..\r\nfoobar", ".\r\nfoobar");
		testFilteredOutputStreamHelper<FILTER>("8", "..\r\nfoobar", ".\r", "\nfoobar");
		testFilteredOutputStreamHelper<FILTER>("9", ".foobar", ".foobar");
		testFilteredOutputStreamHelper<FILTER>("10", ".foobar", ".", "foobar");
	}

	void testCRLFToLFFilteredOutputStream()
	{
		typedef vmime::utility::CRLFToLFFilteredOutputStream FILTER;

		testFilteredOutputStreamHelper<FILTER>("1", "foo\nbar", "foo\r\nbar");
		testFilteredOutputStreamHelper<FILTER>("2", "foo\nbar", "foo\r\n", "bar");
		testFilteredOutputStreamHelper<FILTER>("3", "foo\nbar", "foo\r", "\nbar");
		testFilteredOutputStreamHelper<FILTER>("4", "foo\nbar", "foo", "\r\nbar");
		testFilteredOutputStreamHelper<FILTER>("5", "foo\nbar", "foo", "\r", "\nbar");
		testFilteredOutputStreamHelper<FILTER>("6", "foo\nbar", "foo", "\r", "\n", "bar");
		testFilteredOutputStreamHelper<FILTER>("7", "foo\nba\nr", "foo\r", "\nba\r\nr");
	}

	// stopSequenceFilteredInputStream

	template <int N>
	void testStopSequenceFISHelper
		(const std::string& number, const std::string& sequence,
		 const std::string& expected, const std::string& c1,
		 const std::string& c2 = "", const std::string& c3 = "",
		 const std::string& c4 = "", const std::string& c5 = "")
	{
		chunkInputStream cis;
		cis.addChunk(c1);
		if (!c2.empty()) cis.addChunk(c2);
		if (!c3.empty()) cis.addChunk(c3);
		if (!c4.empty()) cis.addChunk(c4);
		if (!c5.empty()) cis.addChunk(c5);

		vmime::utility::stopSequenceFilteredInputStream <N> is(cis, sequence.data());

		VASSERT_EQ(number, expected, readWhole(is));
	}

	void testStopSequenceFilteredInputStream1()
	{
		testStopSequenceFISHelper <1>("1", "x", "foo", "fooxbar");
		testStopSequenceFISHelper <1>("2", "x", "foo", "foox", "bar");
		testStopSequenceFISHelper <1>("3", "x", "foo", "foo", "x", "bar");
		testStopSequenceFISHelper <1>("4", "x", "foo", "fo", "o", "x", "bar");
		testStopSequenceFISHelper <1>("5", "x", "foo", "fo", "o", "x", "b", "ar");

		testStopSequenceFISHelper <1>("6", "x", "foobar", "fo", "o", "b", "ar");
		testStopSequenceFISHelper <1>("7", "x", "foobar", "foo", "bar");
		testStopSequenceFISHelper <1>("8", "x", "foobar", "foo", "b", "ar");

		testStopSequenceFISHelper <1>("9", "x", "foobar", "foobar");
		testStopSequenceFISHelper <1>("10", "x", "foobar", "foobarx");

		testStopSequenceFISHelper <1>("11", "x", "", "");
		testStopSequenceFISHelper <1>("12", "x", "", "x");
		testStopSequenceFISHelper <1>("13", "x", "", "", "x");
	}

	void testStopSequenceFilteredInputStreamN_2()
	{
		testStopSequenceFISHelper <2>("1", "xy", "foo", "fooxybar");
		testStopSequenceFISHelper <2>("2", "xy", "foo", "foox", "ybar");
		testStopSequenceFISHelper <2>("3", "xy", "foo", "foox", "y", "bar");
		testStopSequenceFISHelper <2>("4", "xy", "foo", "foo", "x", "ybar");
		testStopSequenceFISHelper <2>("5", "xy", "foo", "foo", "xy", "bar");
		testStopSequenceFISHelper <2>("6", "xy", "foo", "foo", "x", "y", "bar");

		testStopSequenceFISHelper <2>("7", "xy", "fooxbar", "foox", "bar");
		testStopSequenceFISHelper <2>("8", "xy", "fooxbar", "foo", "xbar");
		testStopSequenceFISHelper <2>("9", "xy", "fooxbar", "foo", "x", "bar");
		testStopSequenceFISHelper <2>("10", "xy", "foobarx", "foo", "barx");

		testStopSequenceFISHelper <2>("11", "xy", "foobar", "foobarxy");
		testStopSequenceFISHelper <2>("12", "xy", "foobar", "foo", "barxy");
		testStopSequenceFISHelper <2>("13", "xy", "foobar", "foo", "bar", "xy");

		testStopSequenceFISHelper <2>("14", "xy", "", "");
		testStopSequenceFISHelper <2>("15", "xy", "x", "x");
		testStopSequenceFISHelper <2>("16", "xy", "", "xy");
		testStopSequenceFISHelper <2>("17", "xy", "", "x", "y");
	}

	void testStopSequenceFilteredInputStreamN_3()
	{
		testStopSequenceFISHelper <3>("1", "xyz", "foo", "fooxyzbar");
		testStopSequenceFISHelper <3>("2", "xyz", "foo", "foox", "yzbar");
		testStopSequenceFISHelper <3>("3", "xyz", "foo", "foox", "y", "zbar");
		testStopSequenceFISHelper <3>("4", "xyz", "foo", "foox", "yz", "bar");
		testStopSequenceFISHelper <3>("5", "xyz", "foo", "foo", "xyz", "bar");
		testStopSequenceFISHelper <3>("6", "xyz", "foo", "foo", "xy", "zbar");
		testStopSequenceFISHelper <3>("7", "xyz", "foo", "foo", "x", "y", "zbar");
		testStopSequenceFISHelper <3>("8", "xyz", "foo", "foo", "x", "y", "z", "bar");
		testStopSequenceFISHelper <3>("9", "xyz", "foo", "fooxy", "z", "bar");

		testStopSequenceFISHelper <3>("10", "xyz", "fooxybar", "foox", "y", "bar");
		testStopSequenceFISHelper <3>("11", "xyz", "fooxybar", "fooxy", "bar");
		testStopSequenceFISHelper <3>("12", "xyz", "fooxybar", "fo", "ox", "y", "bar");
		testStopSequenceFISHelper <3>("13", "xyz", "fooxybar", "fo", "o", "x", "y", "bar");
		testStopSequenceFISHelper <3>("14", "xyz", "fooxybar", "foo", "x", "ybar");
		testStopSequenceFISHelper <3>("15", "xyz", "fooxybar", "foo", "xybar");

		testStopSequenceFISHelper <3>("16", "xyz", "xfoxoxybxar", "xfoxo", "xybxar");
		testStopSequenceFISHelper <3>("17", "xyz", "xfoxoxybxarx", "xfoxo", "xybxarx");
		testStopSequenceFISHelper <3>("18", "xyz", "xfoxoxybxarxy", "xfoxo", "xybxarxy");

		testStopSequenceFISHelper <3>("19", "xyz", "", "");
		testStopSequenceFISHelper <3>("20", "xyz", "x", "x");
		testStopSequenceFISHelper <3>("21", "xyz", "xy", "xy");
		testStopSequenceFISHelper <3>("22", "xyz", "", "xyz");
		testStopSequenceFISHelper <3>("23", "xyz", "", "x", "yz");
		testStopSequenceFISHelper <3>("24", "xyz", "", "x", "y", "z");
	}


	// LFToCRLFFilteredOutputStream

	void testLFToCRLFFilteredOutputStream_Global()
	{
		typedef vmime::utility::LFToCRLFFilteredOutputStream FILTER;

		testFilteredOutputStreamHelper<FILTER>("1",  "ABC\r\nDEF",       "ABC\nDEF");
		testFilteredOutputStreamHelper<FILTER>("2",  "ABC\r\nDEF",       "ABC\rDEF");
		testFilteredOutputStreamHelper<FILTER>("3",  "\r\n\r\nAB\r\n\r\nA\r\nB\r\n", "\n\nAB\n\nA\nB\n");
		testFilteredOutputStreamHelper<FILTER>("4",  "ABCDE\r\nF",       "ABCDE\nF");
		testFilteredOutputStreamHelper<FILTER>("5",  "ABCDE\r\nF",       "ABCDE\r\nF");
		testFilteredOutputStreamHelper<FILTER>("6",  "\r\n\r\n\r\n",     "\n\n\n");
		testFilteredOutputStreamHelper<FILTER>("7",  "\r\n\r\n\r\n",     "\r\r\n\n");
		testFilteredOutputStreamHelper<FILTER>("8",  "\r\n\r\n\r\n\r\n", "\r\r\r\r");
		testFilteredOutputStreamHelper<FILTER>("9",  "\r\n\r\n\r\n\r\n", "\n\n\n\n");
		testFilteredOutputStreamHelper<FILTER>("10", "\r\n\r\n\r\n",     "\r\n\n\n");
		testFilteredOutputStreamHelper<FILTER>("11", "\r\n\r\n\r\n\r\n", "\n\n\n\r\n");
	}

	void testLFToCRLFFilteredOutputStream_Edge()
	{
		typedef vmime::utility::LFToCRLFFilteredOutputStream FILTER;

		testFilteredOutputStreamHelper<FILTER>("1",  "\r\n\r\n",         "\r", "\r");
		testFilteredOutputStreamHelper<FILTER>("2",  "\r\n\r\n",         "\r", "\n\r");
		testFilteredOutputStreamHelper<FILTER>("3",  "ABC\r\n\r\n",      "ABC\r", "\n\r");
		testFilteredOutputStreamHelper<FILTER>("4",  "ABC\r\n\r\n\r\n",  "ABC\r", "\n\r", "\n\n");
		testFilteredOutputStreamHelper<FILTER>("5",  "\r\n\r\n",         "\n", "\n");
		testFilteredOutputStreamHelper<FILTER>("6",  "\r\n\r\n",         "\r\n\r\n");
		testFilteredOutputStreamHelper<FILTER>("7",  "\r\n\r\n",         "\r\n\r", "\n");

		testFilteredOutputStreamHelper<FILTER>("8",  "A\r\nB\r\nC\r\nD",         "A\rB", "\nC\r\nD");
		testFilteredOutputStreamHelper<FILTER>("9",  "\r\nA\r\nB\r\nC\r\nD",     "\rA\r", "B\nC\r\nD");
		testFilteredOutputStreamHelper<FILTER>("10", "\r\nA\r\nB\r\nC\r\nD",     "\nA\r", "B\nC\r\nD");
		testFilteredOutputStreamHelper<FILTER>("11", "\r\nA\r\nB\r\nC\r\nD\r\n", "\nA\rB", "\nC\r\nD\r");
	}

VMIME_TEST_SUITE_END

