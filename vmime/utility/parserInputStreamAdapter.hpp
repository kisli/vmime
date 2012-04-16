//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2012 Vincent Richard <vincent@vincent-richard.net>
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

#ifndef VMIME_UTILITY_PARSERINPUTSTREAMADAPTER_HPP_INCLUDED
#define VMIME_UTILITY_PARSERINPUTSTREAMADAPTER_HPP_INCLUDED


#include "vmime/utility/seekableInputStream.hpp"

#include <cstring>


namespace vmime {
namespace utility {


/** An adapter class used for parsing from an input stream.
  */

class parserInputStreamAdapter : public seekableInputStream
{
public:

	/** @param is input stream to wrap
	  */
	parserInputStreamAdapter(ref <seekableInputStream> inputStream);

	ref <seekableInputStream> getUnderlyingStream();

	bool eof() const;
	void reset();
	size_type read(value_type* const data, const size_type count);

	void seek(const size_type pos)
	{
		m_stream->seek(pos);
	}

	size_type skip(const size_type count)
	{
		return m_stream->skip(count);
	}

	size_type getPosition() const
	{
		return m_stream->getPosition();
	}

	/** Get the byte at the current position without updating the
	  * current position.
	  *
	  * @return byte at the current position
	  */
	value_type peekByte() const
	{
		const size_type initialPos = m_stream->getPosition();

		try
		{
			value_type buffer[1];
			const size_type readBytes = m_stream->read(buffer, 1);

			m_stream->seek(initialPos);

			return (readBytes == 1 ? buffer[0] : 0);
		}
		catch (...)
		{
			m_stream->seek(initialPos);
			throw;
		}
	}

	/** Get the byte at the current position and advance current
	  * position by one byte.
	  *
	  * @return byte at the current position
	  */
	value_type getByte()
	{
		value_type buffer[1];
		const size_type readBytes = m_stream->read(buffer, 1);

		return (readBytes == 1 ? buffer[0] : 0);
	}

	/** Check whether the bytes following the current position match
	  * the specified bytes. Position is not updated.
	  *
	  * @param bytes bytes to compare
	  * @param length number of bytes
	  * @return true if the next bytes match the pattern, false otherwise
	  */
	bool matchBytes(const value_type* bytes, const size_type length) const
	{
		const size_type initialPos = m_stream->getPosition();

		try
		{
			value_type buffer[32];
			const size_type readBytes = m_stream->read(buffer, length);

			m_stream->seek(initialPos);

			return readBytes == length &&
			       ::memcmp(bytes, buffer, length) == 0;
		}
		catch (...)
		{
			m_stream->seek(initialPos);
			throw;
		}
	}

	const string extract(const size_type begin, const size_type end) const;

	/** Skips bytes matching a predicate from the current position.
	  * The current position is updated to the next following byte
	  * which does not match the predicate.
	  *
	  * @param pred predicate
	  * @param endPosition stop at this position (or at end of the stream,
	  * whichever comes first)
	  * @return number of bytes skipped
	  */
	template <typename PREDICATE>
	size_type skipIf(PREDICATE pred, const size_type endPosition)
	{
		const size_type initialPos = getPosition();
		size_type pos = initialPos;

		while (!m_stream->eof() && pos < endPosition && pred(getByte()))
			++pos;

		m_stream->seek(pos);

		return pos - initialPos;
	}

	size_type findNext(const std::string& token, const size_type startPosition = 0);

private:

	mutable ref <seekableInputStream> m_stream;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_PARSERINPUTSTREAMADAPTER_HPP_INCLUDED

