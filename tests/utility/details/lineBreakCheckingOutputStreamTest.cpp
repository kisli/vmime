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

#include "vmime/utility/details/lineBreakCheckingOutputStream.hpp"


VMIME_TEST_SUITE_BEGIN(lineBreakCheckingOutputStreamTest)

	VMIME_TEST_LIST_BEGIN
		VMIME_TEST(testFolds)
		VMIME_TEST(testLineBreaks)
	VMIME_TEST_LIST_END


	typedef vmime::utility::details::lineBreakCheckingOutputStream FILTER;


	// Write each string to the filter, and return what it wrote
	template <typename... Args>
	static std::string write(Args &&...args) {

		std::ostringstream oss;
		vmime::utility::outputStreamAdapter os(oss);

		FILTER fos(os);

		for (auto &&s : std::initializer_list <std::string>{ std::forward <Args>(args)... }) {
			fos.write(s.data(), s.length());
		}

		fos.checkEnd();

		return oss.str();
	}

	// Folds (CRLF followed by white-space) are let through
	void testFolds() {

		VASSERT_EQ("1", "ABC", write("ABC"));
		VASSERT_EQ("2", "ABC\r\n DEF", write("ABC\r\n DEF"));
		VASSERT_EQ("3", "ABC\r\n\tDEF", write("ABC\r\n\tDEF"));
		VASSERT_EQ("4", "A\r\n B\r\n  C", write("A\r\n B\r\n  C"));
		VASSERT_EQ("5", "ABC\r\n DEF", write("ABC\r", "\n", " DEF"));
		VASSERT_EQ("6", "ABC\r\n DEF", write("ABC\r\n", " DEF"));
		VASSERT_EQ("7", "ABC\r\n DEF", write("ABC", "\r\n DEF"));

		// CRLF waits for the white-space after it
		std::ostringstream oss;
		vmime::utility::outputStreamAdapter os(oss);
		FILTER fos(os);

		fos.write("ABC\r\n", 5);
		VASSERT_EQ("8", "ABC", oss.str());

		fos.write(" ", 1);
		VASSERT_EQ("9", "ABC\r\n ", oss.str());

		VASSERT_NO_THROW("10", fos.checkEnd());
	}

	// Other line breaks are refused, and not written
	void testLineBreaks() {

		const char* const cases[] = { "ABC\r\nDEF", "ABC\nDEF", "ABC\rDEF", "ABC\r\r\n DEF", "ABC\r\n\r\n DEF" };

		for (const char* const data : cases) {

			std::ostringstream oss;
			vmime::utility::outputStreamAdapter os(oss);
			FILTER fos(os);

			VASSERT_THROW(data, fos.write(data, strlen(data)), vmime::exceptions::invalid_argument);
			VASSERT_EQ(data, "ABC", oss.str());
		}

		// At the end of the data, CR or CRLF cannot be a fold
		const char* const endCases[] = { "ABC\r\n", "ABC\r" };

		for (const char* const data : endCases) {

			std::ostringstream oss;
			vmime::utility::outputStreamAdapter os(oss);
			FILTER fos(os);

			fos.write(data, strlen(data));

			VASSERT_THROW(data, fos.checkEnd(), vmime::exceptions::invalid_argument);
			VASSERT_EQ(data, "ABC", oss.str());
		}
	}

VMIME_TEST_SUITE_END
