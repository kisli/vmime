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

#include "vmime/utility/parserInputStreamAdapter.hpp"


namespace vmime {
namespace utility {


parserInputStreamAdapter::parserInputStreamAdapter(const shared_ptr <seekableInputStream>& stream)
	: m_stream(stream) {

}


bool parserInputStreamAdapter::eof() const {

	return m_stream->eof();
}


void parserInputStreamAdapter::reset() {

	m_stream->reset();
}


size_t parserInputStreamAdapter::read(byte_t* const data, const size_t count) {

	return m_stream->read(data, count);
}


shared_ptr <seekableInputStream> parserInputStreamAdapter::getUnderlyingStream() {

	return m_stream;
}


const string parserInputStreamAdapter::extract(const size_t begin, const size_t end) const {

	const size_t initialPos = m_stream->getPosition();

	try {

		// Read directly into the string: going through a byte_t buffer costs
		// a second copy, which is done byte by byte (types differ)
		string str(end - begin, '\0');

		m_stream->seek(begin);

		const size_t readBytes = m_stream->read(reinterpret_cast <byte_t*>(&str[0]), str.length());
		str.resize(readBytes);

		m_stream->seek(initialPos);

		return str;

	} catch (...) {

		m_stream->seek(initialPos);
		throw;
	}
}


size_t parserInputStreamAdapter::findNext(
	const string& token,
	const size_t startPosition,
	const size_t endPosition
) {

	static const size_t BUFFER_SIZE = 4096;

	// Token must not be longer than BUFFER_SIZE/2
	if (token.empty() || token.length() > BUFFER_SIZE / 2 || startPosition >= endPosition) {
		return npos;
	}

	const size_t initialPos = getPosition();

	seek(startPosition);

	try {

		byte_t findBuffer[BUFFER_SIZE];
		size_t findBufferLen = 0;

		// Position of the first byte of the buffer in the stream
		size_t findBufferOffset = startPosition;

		// Number of bytes which can still be read before reaching 'endPosition'
		size_t remaining = endPosition - startPosition;

		// Start with small reads, and double their size after each one: the
		// cost of a search is then proportional to the distance to the token,
		// which matters when the caller rejects a match and searches again
		size_t readSize = 64;

		while (true) {

			const size_t bytesRead = read(
				findBuffer + findBufferLen,
				std::min(std::min(readSize, BUFFER_SIZE - findBufferLen), remaining)
			);

			findBufferLen += bytesRead;
			remaining -= bytesRead;

			// Find token
			if (findBufferLen >= token.length()) {

				const byte_t* const last = findBuffer + findBufferLen - token.length();

				for (const byte_t* begin = findBuffer ; begin <= last ; ++begin) {

					begin = static_cast <const byte_t*>(memchr(begin, token[0], last - begin + 1));

					if (!begin) {
						break;
					}

					if (memcmp(begin + 1, token.data() + 1, token.length() - 1) == 0) {

						seek(initialPos);

						return findBufferOffset + (begin - findBuffer);
					}
				}
			}

			// End of stream, or 'endPosition' reached
			if (bytesRead == 0 || remaining == 0) {
				break;
			}

			// Keep the bytes which have not been tested yet as the start of
			// the token: they may be the beginning of an occurrence
			const size_t keep = std::min(findBufferLen, token.length() - 1);

			memmove(findBuffer, findBuffer + findBufferLen - keep, keep);

			findBufferOffset += findBufferLen - keep;
			findBufferLen = keep;

			readSize = std::min(readSize * 2, BUFFER_SIZE);
		}

		seek(initialPos);

	} catch (...) {

		seek(initialPos);
		throw;
	}

	return npos;
}


} // utility
} // vmime
