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

#include "../lib/unit++/unit++.h"

#include <iostream>
#include <ostream>
#include <algorithm>

#include "vmime/vmime.hpp"
#include "vmime/platforms/posix/posixHandler.hpp"

#include "vmime/utility/filteredStream.hpp"

using namespace unitpp;


namespace
{
	class filteredStreamTest : public suite
	{
		class chunkInputStream : public vmime::utility::inputStream
		{
		private:

			std::vector <std::string> m_chunks;
			std::vector <std::string>::size_type m_index;

		public:

			chunkInputStream() : m_index(0) { }

			void addChunk(const std::string& chunk) { m_chunks.push_back(chunk); }

			const bool eof() const { return (m_index >= m_chunks.size()); }
			void reset() { m_index = 0; }

			const size_type read(value_type* const data, const size_type /* count */)
			{
				const std::string chunk = m_chunks[m_index];

				// Warning: 'count' should be larger than chunk length.
				// This is OK for our tests.
				std::copy(chunk.begin(), chunk.end(), data);

				++m_index;

				return chunk.length();
			}

			const size_type skip(const size_type /* count */)
			{
				// Not supported
				return 0;
			}
		};


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

			assert_eq(number, expected, oss.str());
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

			assert_eq(number, expected, oss.str());
		}

		void testDotFilteredOutputStream()
		{
			typedef vmime::utility::dotFilteredOutputStream FILTER;

			testFilteredOutputStreamHelper<FILTER>("1", "foo\n..bar", "foo\n.bar");
			testFilteredOutputStreamHelper<FILTER>("2", "foo\n..bar", "foo\n", ".bar");
			testFilteredOutputStreamHelper<FILTER>("3", "foo\n..bar", "foo", "\n.bar");
			testFilteredOutputStreamHelper<FILTER>("4", "foo\n..bar", "foo", "\n", ".bar");
			testFilteredOutputStreamHelper<FILTER>("5", "foo\n..bar", "foo", "\n", ".", "bar");
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
		}

	public:

		filteredStreamTest() : suite("vmime::utility::filteredStream")
		{
			// VMime initialization
			vmime::platformDependant::setHandler<vmime::platforms::posix::posixHandler>();

			add("dotFilteredInputStream", testcase(this, "dotFilteredInputStream", &filteredStreamTest::testDotFilteredInputStream));
			add("dotFilteredOutputStream", testcase(this, "dotFilteredOutputStream", &filteredStreamTest::testDotFilteredOutputStream));
			add("CRLFToLFFilteredOutputStream", testcase(this, "CRLFToLFFilteredOutputStream", &filteredStreamTest::testCRLFToLFFilteredOutputStream));

			suite::main().add("vmime::utility::filteredStream", this);
		}

	};

	filteredStreamTest* theTest = new filteredStreamTest();
}

