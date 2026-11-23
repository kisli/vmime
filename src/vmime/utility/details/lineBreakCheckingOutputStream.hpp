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

#ifndef VMIME_UTILITY_DETAILS_LINEBREAKCHECKINGOUTPUTSTREAM_HPP_INCLUDED
#define VMIME_UTILITY_DETAILS_LINEBREAKCHECKINGOUTPUTSTREAM_HPP_INCLUDED


#include "vmime/exception.hpp"
#include "vmime/utility/filteredStream.hpp"


namespace vmime {
namespace utility {
namespace details {


/** A filtered output stream which only lets CR and LF through in folds,
  * that is CRLF followed by white-space (RFC 5322, section 2.2.3), as in
  * a header field, which they would end elsewhere. A CRLF is only written
  * once the white-space after it is.
  *
  * This is an internal class: it is not part of the public API.
  */
class lineBreakCheckingOutputStream : public filteredOutputStream {

public:

	/** Construct a new filter for the specified output stream.
	  *
	  * @param os stream into which write filtered data
	  */
	lineBreakCheckingOutputStream(outputStream& os)
		: m_stream(os),
		  m_pendingLineBreakLength(0) {

	}

	outputStream& getNextOutputStream() override {

		return m_stream;
	}

	void flush() override {

		m_stream.flush();
	}

	size_t getBlockSize() override {

		return m_stream.getBlockSize();
	}

	/** Check that the data written does not end with CR or CRLF, which
	  * cannot be a fold then.
	  *
	  * @throw exceptions::invalid_argument if it does
	  */
	void checkEnd() const {

		if (m_pendingLineBreakLength != 0) {
			throw exceptions::invalid_argument();
		}
	}

protected:

	/** @throw exceptions::invalid_argument if CR or LF is not in a fold
	  */
	void writeImpl(const byte_t* const data, const size_t count) override {

		const byte_t* start = data;  // data not written yet
		const byte_t* const end = data + count;

		for (const byte_t* pos = data ; pos < end ; ++pos) {

			if (m_pendingLineBreakLength == 2) {

				// A fold: CRLF followed by white-space
				if (*pos != ' ' && *pos != '\t') {
					throw exceptions::invalid_argument();
				}

				m_stream.write("\r\n", 2);
				m_pendingLineBreakLength = 0;

			} else if (m_pendingLineBreakLength == 1) {

				if (*pos != '\n') {
					throw exceptions::invalid_argument();
				}

				m_pendingLineBreakLength = 2;
				start = pos + 1;

			} else if (*pos == '\r') {

				m_stream.write(start, pos - start);
				m_pendingLineBreakLength = 1;
				start = pos + 1;

			} else if (*pos == '\n') {

				m_stream.write(start, pos - start);
				throw exceptions::invalid_argument();
			}
		}

		m_stream.write(start, end - start);
	}

private:

	outputStream& m_stream;
	size_t m_pendingLineBreakLength;  // CR or CRLF not written yet
};


} // details
} // utility
} // vmime


#endif // VMIME_UTILITY_DETAILS_LINEBREAKCHECKINGOUTPUTSTREAM_HPP_INCLUDED
