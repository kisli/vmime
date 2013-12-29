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

#ifndef VMIME_UTILITY_PARSERINPUTSTREAMADAPTER_HPP_INCLUDED
#define VMIME_UTILITY_PARSERINPUTSTREAMADAPTER_HPP_INCLUDED


#include "vmime/utility/seekableInputStream.hpp"

#include <cstring>


namespace vmime {
namespace utility {


/** An adapter class used for parsing from an input stream.
  */

class VMIME_EXPORT parserInputStreamAdapter : public seekableInputStream
{
public:

	/** @param stream input stream to wrap
	  */
	parserInputStreamAdapter(shared_ptr <seekableInputStream> stream);

	shared_ptr <seekableInputStream> getUnderlyingStream();

	bool eof() const;
	void reset();
	size_t read(byte_t* const data, const size_t count);

	void seek(const size_t pos)
	{
		m_stream->seek(pos);
	}

	size_t skip(const size_t count)
	{
		return m_stream->skip(count);
	}

	size_t getPosition() const
	{
		return m_stream->getPosition();
	}

	/** Get the byte at the current position without updating the
	  * current position.
	  *
	  * @return byte at the current position
	  */
	byte_t peekByte() const
	{
		const size_t initialPos = m_stream->getPosition();

		try
		{
			byte_t buffer[1];
			const size_t readBytes = m_stream->read(buffer, 1);

			m_stream->seek(initialPos);

			return (readBytes == 1 ? buffer[0] : static_cast <byte_t>(0));
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
	byte_t getByte()
	{
		byte_t buffer[1];
		const size_t readBytes = m_stream->read(buffer, 1);

		return (readBytes == 1 ? buffer[0] : static_cast <byte_t>(0));
	}

	/** Check whether the bytes following the current position match
	  * the specified bytes. Position is not updated.
	  *
	  * @param bytes bytes to compare
	  * @param length number of bytes
	  * @return true if the next bytes match the pattern, false otherwise
	  */
	template <typename T>
	bool matchBytes(const T* bytes, const size_t length) const
	{
		const size_t initialPos = m_stream->getPosition();

		try
		{
			byte_t buffer[32];
			const size_t readBytes = m_stream->read(buffer, length);

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

	const string extract(const size_t begin, const size_t end) const;

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
	size_t skipIf(PREDICATE pred, const size_t endPosition)
	{
		const size_t initialPos = getPosition();
		size_t pos = initialPos;

		while (!m_stream->eof() && pos < endPosition && pred(getByte()))
			++pos;

		m_stream->seek(pos);

		return pos - initialPos;
	}

	size_t findNext(const string& token, const size_t startPosition = 0);

private:

	mutable shared_ptr <seekableInputStream> m_stream;
};


} // utility
} // vmime


#endif // VMIME_UTILITY_PARSERINPUTSTREAMADAPTER_HPP_INCLUDED

