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

#ifndef VMIME_UTILITY_FILTEREDSTREAM_HPP_INCLUDED
#define VMIME_UTILITY_FILTEREDSTREAM_HPP_INCLUDED


#include <algorithm>

#include "vmime/utility/inputStream.hpp"
#include "vmime/utility/outputStream.hpp"


namespace vmime {
namespace utility {


/** A stream whose input is filtered.
  */

class VMIME_EXPORT filteredInputStream : public inputStream
{
public:

	virtual size_t getBlockSize();

	/** Return a reference to the stream being filtered.
	  *
	  * @return stream being filtered
	  */
	virtual inputStream& getPreviousInputStream() = 0;
};


/** A stream whose output is filtered.
  */

class VMIME_EXPORT filteredOutputStream : public outputStream
{
public:

	virtual size_t getBlockSize();

	/** Return a reference to the stream being filtered.
	  *
	  * @return destination stream for filtered data
	  */
	virtual outputStream& getNextOutputStream() = 0;
};


/** A filtered input stream which replaces "\n.."
  * sequences with "\n." sequences.
  */

class VMIME_EXPORT dotFilteredInputStream : public filteredInputStream
{
public:

	/** Construct a new filter for the specified input stream.
	  *
	  * @param is stream from which to read data to be filtered
	  */
	dotFilteredInputStream(inputStream& is);

	inputStream& getPreviousInputStream();

	bool eof() const;

	void reset();

	size_t read(byte_t* const data, const size_t count);

	size_t skip(const size_t count);

private:

	inputStream& m_stream;

	byte_t m_previousChar2; // (N - 1)th character of previous buffer
	byte_t m_previousChar1; // (N)th (last) character of previous buffer
};


/** A filtered output stream which replaces "\n."
  * sequences with "\n.." sequences.
  */

class VMIME_EXPORT dotFilteredOutputStream : public filteredOutputStream
{
public:

	/** Construct a new filter for the specified output stream.
	  *
	  * @param os stream into which write filtered data
	  */
	dotFilteredOutputStream(outputStream& os);

	outputStream& getNextOutputStream();

	void flush();

	size_t getBlockSize();

protected:

	void writeImpl(const byte_t* const data, const size_t count);

private:

	outputStream& m_stream;
	byte_t m_previousChar;
	bool m_start;
};


/** A filtered output stream which replaces CRLF sequences
  * with single LF characters.
  */

class VMIME_EXPORT CRLFToLFFilteredOutputStream : public filteredOutputStream
{
public:

	/** Construct a new filter for the specified output stream.
	  *
	  * @param os stream into which write filtered data
	  */
	CRLFToLFFilteredOutputStream(outputStream& os);

	outputStream& getNextOutputStream();

	void flush();

	size_t getBlockSize();

protected:

	void writeImpl(const byte_t* const data, const size_t count);

private:

        outputStream& m_stream;
	byte_t m_previousChar;
};


/** A filtered output stream which replaces CR or LF characters
  * with CRLF sequences.
  */

class VMIME_EXPORT LFToCRLFFilteredOutputStream : public filteredOutputStream
{
public:

	/** Construct a new filter for the specified output stream.
	  *
	  * @param os stream into which write filtered data
	  */
	LFToCRLFFilteredOutputStream(outputStream& os);

	outputStream& getNextOutputStream();

	void flush();

	size_t getBlockSize();

protected:

	void writeImpl(const byte_t* const data, const size_t count);

private:

	outputStream& m_stream;
	byte_t m_previousChar;
};


/** A filtered input stream which stops when a specified sequence
  * is found (eof() method will return 'true').
  */

template <int COUNT>
class VMIME_EXPORT stopSequenceFilteredInputStream : public filteredInputStream
{
public:

	/** Construct a new filter for the specified input stream.
	  *
	  * @param is stream from which to read data to be filtered
	  * @param sequence sequence on which to stop
	  */
	stopSequenceFilteredInputStream(inputStream& is, const byte_t* sequence)
		: m_stream(is), m_sequence(sequence), m_found(0), m_eof(false)
	{
	}

	/** Construct a new filter for the specified input stream.
	  *
	  * @param is stream from which to read data to be filtered
	  * @param sequence sequence on which to stop
	  */
	stopSequenceFilteredInputStream(inputStream& is, const char* sequence)
		: m_stream(is), m_sequence(reinterpret_cast <const byte_t*>(sequence)),
		  m_found(0), m_eof(false)
	{
	}

	inputStream& getPreviousInputStream()
	{
		return (m_stream);
	}

	bool eof() const
	{
		return (m_found == COUNT || m_eof);
	}

	void reset()
	{
		m_found = 0;
		m_stream.reset();
	}

	size_t read(byte_t* const data, const size_t count);

	size_t skip(const size_t /* count */)
	{
		// Not supported
		return 0;
	}

private:

	inputStream& m_stream;

	const byte_t* m_sequence;
	size_t m_found;

	bool m_eof;
};


template <>
size_t stopSequenceFilteredInputStream <1>::read
	(byte_t* const data, const size_t count);


template <int COUNT>
size_t stopSequenceFilteredInputStream <COUNT>::read
	(byte_t* const data, const size_t count)
{
	// Read buffer must be at least 'COUNT' size + 1 byte
	if (eof() || count <= COUNT)
		return 0;

	if (m_stream.eof())
	{
		if (m_found != 0)
		{
			const size_t found = m_found;

			for (size_t f = 0 ; f < found ; ++f)
				data[f] = m_sequence[f];

			m_found = 0;
			m_eof = true;

			return (found);
		}
		else
		{
			m_eof = true;
			return 0;
		}
	}

	size_t read = m_stream.read(data, count - COUNT);

	byte_t* end = data + read;
	byte_t* pos = data;

	while (pos < end)
	{
		// Very simple case, search for the whole sequence
		if (m_found == 0)
		{
			while (pos < end)
			{
				pos = std::find(pos, end, m_sequence[0]);

				if (pos == end)
					return (read);

				m_found = 1;
				++pos;

				while (pos < end && m_found < COUNT && m_sequence[m_found] == *pos)
				{
					++m_found;
					++pos;
				}

				// Didn't found whole sequence
				if (m_found != COUNT)
				{
					// We reached the end of the buffer
					if (pos == end)
					{
						return (read - m_found);
					}
					// Common prefix but not whole sequence
					else
					{
						m_found = 0;
					}
				}
				// Whole sequence found
				else
				{
					// End of stream
					return (pos - data - m_found);
				}
			}
		}
		// More complex case: search for a sequence which has begun
		// in a previous buffer
		else
		{
			// Search for the end of the previously started sequence
			while (pos < end && m_found < COUNT && m_sequence[m_found] == *pos)
			{
				++m_found;
				++pos;
			}

			if (m_found != COUNT)
			{
				// End of buffer
				if (pos == end)
				{
					// No data: this buffer is a sub-sequence of the
					// searched sequence
					return 0;
				}
				// Common prefix
				else
				{
					// We have to reinject the incomplete sequence into
					// the stream data

					// -- shift right data
					const size_t n = pos - data;

					byte_t* newEnd = data + read + m_found - n;
					byte_t* oldEnd = data + read;

					for (size_t i = 0 ; i < read - n ; ++i)
					{
						--newEnd;
						--oldEnd;

						*newEnd = *oldEnd;
					}

					// -- copy the prefix just before data
					for (size_t f = 0 ; f < m_found ; ++f)
						data[f] = m_sequence[f];

					read += m_found - n;
					end += m_found - n;

					m_found = 0;
				}
			}
			else
			{
				return 0;  // no more data
			}
		}
	}

	return read;
}


} // utility
} // vmime


#endif // VMIME_UTILITY_FILTEREDSTREAM_HPP_INCLUDED

