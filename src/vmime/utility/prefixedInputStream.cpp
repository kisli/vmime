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

#include "vmime/utility/prefixedInputStream.hpp"

#include <algorithm>


namespace vmime {
namespace utility {


prefixedInputStream::prefixedInputStream(const string& prefix, inputStream& is)
	: m_prefix(prefix),
	  m_pos(0),
	  m_stream(is) {

}


bool prefixedInputStream::eof() const {

	return m_pos >= m_prefix.length() && m_stream.eof();
}


void prefixedInputStream::reset() {

	m_pos = 0;
	m_stream.reset();
}


size_t prefixedInputStream::read(byte_t* const data, const size_t count) {

	if (m_pos < m_prefix.length()) {

		const size_t n = std::min(count, m_prefix.length() - m_pos);

		std::copy(m_prefix.begin() + m_pos, m_prefix.begin() + m_pos + n, data);
		m_pos += n;

		return n;
	}

	return m_stream.read(data, count);
}


size_t prefixedInputStream::skip(const size_t count) {

	if (m_pos < m_prefix.length()) {

		const size_t n = std::min(count, m_prefix.length() - m_pos);
		m_pos += n;

		return n;
	}

	return m_stream.skip(count);
}


size_t prefixedInputStream::getBlockSize() {

	return std::min(inputStream::getBlockSize(), m_stream.getBlockSize());
}


} // utility
} // vmime
